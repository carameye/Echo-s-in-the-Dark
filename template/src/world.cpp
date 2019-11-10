// Header
#include "world.hpp"
#include "level.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

using json = nlohmann::json;

// Same as static in c, local to compilation unit
namespace
{
	const size_t CAMERA_PAN_OFFSET = 200;
}

World::World()
{
	// Seeding rng with random device
	m_rng = std::default_random_engine(std::random_device()());
}

World::~World()
{

}

// World initialization
bool World::init(GLFWwindow* window, vec2 screen)
{
	m_window = window;
	m_screen = screen;

	// Create a frame buffer
	m_frame_buffer = 0;
	glGenFramebuffers(1, &m_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int fb_width, fb_height;
	glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
	m_screen_scale = static_cast<float>(fb_width) / screen.x;

	// Initialize the screen texture
	m_screen_tex.create_from_screen(m_window);

	return true;
}

void World::set_pl_functions(void (*l)())
{
	m_load = l;
}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	stop_music();

	m_level.destroy();
}

// Update our game world
void World::update(float elapsed_ms)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w / m_screen_scale, (float)h / m_screen_scale };

	//-------------------------------------------------------------------------
	vec2 player_pos = m_level.get_player_position();
	float follow_speed = 0.1f;
	vec2 follow_point = player_pos;
	// check whether still showing the player the path through the level
	bool done_panning_x = within_range(camera_pos.x, player_pos.x - w, player_pos.x + w);
	bool done_panning_y = within_range(camera_pos.y, player_pos.y - h, player_pos.y + h);
	is_level_load_pan = is_level_load_pan && (!done_panning_x || !done_panning_y);
	if (!is_level_load_pan) {
		follow_point = add(player_pos, { 0.f, camera_offset });
		m_level.update(elapsed_ms);
	} else {
		float multiplier = sq_len(sub(player_pos, m_level.get_starting_camera_position()))/100000000000;
		follow_speed_input++;
		follow_speed = exp(follow_speed_input*multiplier) - 1;
	}
	camera_pos = add(camera_pos, { follow_speed * (follow_point.x - camera_pos.x), follow_speed * (follow_point.y - camera_pos.y) });
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void World::draw()
{
	// Clearing error buffer
	gl_flush_errors();

	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);

	/////////////////////////////////////
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	const float clear_color[3] = { 1.f, 1.f, 0.8f };
	glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fake projection matrix, scales with respect to window coordinates
	// PS: 1.f / w in [1][1] is correct.. do you know why ? (:
	float left = 0.f;// *-0.5;
	float top = 0.f;// (float)h * -0.5;
	float right = (float)w / m_screen_scale;// *0.5;
	float bottom = (float)h / m_screen_scale;// *0.5;


	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };

	// TODO: to fix lulus screen
	vec2 camera_shift = { right / 2.f - camera_pos.x, bottom / 2.f - camera_pos.y };

	m_level.draw_entities(projection_2D, camera_shift);

	/////////////////////
	// Truely render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_screen_tex.id);

	m_level.draw_light(projection_2D, camera_shift);
	//////////////////
	// Presenting
	glfwSwapBuffers(m_window);
}

// Should the game be over ?
bool World::is_over() const
{
	return glfwWindowShouldClose(m_window);
}

// On key callback
bool World::handle_key_press(GLFWwindow*, int key, int, int action, int mod)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		return false;
	}

	if (action == GLFW_PRESS && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		camera_offset -= CAMERA_PAN_OFFSET;
	}
	if (action == GLFW_PRESS && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		camera_offset += CAMERA_PAN_OFFSET;
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		camera_offset += CAMERA_PAN_OFFSET;
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		camera_offset -= CAMERA_PAN_OFFSET;
	}
	std::string r = m_level.handle_key_press(key, action);
	if (r.length() > 0) {
		if (find(m_unlocked.begin(), m_unlocked.end(), r) == m_unlocked.end())
		{
			m_unlocked.push_back(r);
		}
		load_level(r);
	}
	return true;
}

void World::handle_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	if (is_level_load_pan) {
		return;
	}
	m_level.handle_mouse_move(xpos, ypos);
}

void World::start_music()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return;
	}

	m_background_music = Mix_LoadMUS(audio_path("goldleaf.wav"));

	if (m_background_music == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("goldleaf.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("salmon_eat.wav"));
		return;
	}

	// Playing background music indefinitely
	Mix_PlayMusic(m_background_music, -1);
}

void World::stop_music()
{
	if (m_background_music != nullptr) {
		Mix_FreeMusic(m_background_music);
		m_background_music = nullptr;
	}

	Mix_CloseAudio();
}

void World::start_level(bool new_game)
{
	if (new_game)
	{
		m_unlocked = { "level_select", "level_1" };
		load_level("level_select");
	}
	else
	{
		load();
		load_level("level_select");
	}
}

void World::reset()
{
	m_level.reset_level();
}

void World::load_level(std::string level)
{
	stop_music();
	m_load();
	bool valid = m_level.parse_level(level, m_unlocked);
	start_music();

	if (valid)
	{
		camera_pos = m_level.get_starting_camera_position();
	}
	else
	{
		camera_pos = { 0.f, 0.f };
	}

	is_level_load_pan = valid;
	camera_offset = 0.f;
}

void World::load()
{
	std::ifstream file(save_file);
	if (file.is_open()) {
		if (file.good()) {
			json j = json::parse(file);
			m_unlocked.clear();
			for (auto& i : j)
			{
				m_unlocked.push_back(i);
			}
		} else {
			m_unlocked = { "level_select", "level_1" };
		}
		file.close();
	}
}

void World::save()
{
	json j(m_unlocked);
	std::ofstream o(save_file);
	if (o.is_open())
	{
		o << j.dump() << std::endl;
		o.close();
	}
}
