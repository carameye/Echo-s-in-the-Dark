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
	const size_t UPDATE_FREEZE_DURATION = 2000;
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

	m_robot_ls_pos = { -1000.f, -1000.f };

	// Initialize the screen texture
	m_screen_tex.create_from_screen(m_window);

	poll_keys(window);

	return true;
}

void World::set_pl_functions(void (*l)(), void (*e)())
{
	m_load = l;
	m_exit = e;
}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	// free all sound resources
	stop_sounds();

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
	float follow_speed = 0.05f;
	vec2 follow_point = player_pos;
	// check whether still showing the player the path through the level
	bool done_panning_x = within_range(camera_pos.x, player_pos.x - w, player_pos.x + w);
	bool done_panning_y = within_range(camera_pos.y, player_pos.y - h, player_pos.y + h);
	is_level_load_pan = is_level_load_pan && (!done_panning_x || !done_panning_y);
	if (!is_level_load_pan) {
		follow_speed = 0.1f;
		follow_point = add(player_pos, { 0.f, camera_offset });
		std::string sound_effect = m_level.update(elapsed_ms);
		if (sound_effect.length() > 0) {
			if (sound_effect == "collision") {
				Mix_PlayChannel(-1, m_collision_effect, 0);
			} else {
				Mix_PlayChannel(-1, m_robot_hurt_effect, 0);
			}
		}
	} else if (on_load_delay > 0) {
		follow_speed = 0.f;
		on_load_delay -= elapsed_ms;
	}
	camera_pos = add(camera_pos, { follow_speed * (follow_point.x - camera_pos.x), follow_speed * (follow_point.y - camera_pos.y) });

	if (m_level.get_current_level() == "level_select")
	{
		m_robot_ls_pos = m_level.get_player_position();
	}
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
	const float clear_color[3] = { 19.f/255.f, 41.f/255.f, 60.f/255.f };
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
bool World::handle_key_press(GLFWwindow* window, int key, int action)
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
	std::string r = m_level.handle_key_press(key, action, key_input_states);
	if (r.length() > 0) {
		if (r == "flying") {
			int channel = Mix_PlayChannel(-1, m_rocket_effect, -1);
			Mix_GroupChannel(channel, 1);
		} else if (r == "falling") {
			int channel = Mix_FadeOutGroup(1, 1050);
		} else if (r == "door locked") {
			int channel = Mix_PlayChannel(-1, m_locked_door_effect, 0);
		} else {
			if (r == "quit" || r == "complete")
			{
				m_exit();
				return true;
			}
			else
			{
				if (find(m_unlocked.begin(), m_unlocked.end(), r) == m_unlocked.end())
				{
					m_unlocked.push_back(r);
				}
				Mix_PlayChannel(-1, m_open_door_effect, 0);
				load_level(r);
			}
		}
	}
	return true;
}

void World::poll_keys(GLFWwindow* window)
{
	for (auto& input_state : key_input_states) {
		int key = input_state.first;
		int old_key_state = input_state.second;
		int key_state = glfwGetKey(window, key);
		if (key_state != old_key_state) {
			handle_key_press(window, key, key_state);
		}
	}
}

void World::handle_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	if (is_level_load_pan) {
		return;
	}
	m_level.handle_mouse_move(xpos, ypos, camera_pos);
}

void World::handle_mouse_click(int button, int action)
{
    if (is_level_load_pan) {
        return;
    }
    m_level.handle_mouse_click(button, action);
}

void World::handle_mouse_scroll(double yoffset) {
    if (is_level_load_pan) {
        return;
    }
    m_level.handle_mouse_scroll(yoffset);
}

void World::start_sounds()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return;
	}

	const int numGhosts = m_level.get_num_ghosts();

	m_background_music =  Mix_LoadMUS(numGhosts > 0 ? audio_path("ghosts.wav") : audio_path("background.wav"));
	m_robot_hurt_effect = Mix_LoadWAV(audio_path("salmon_dead.wav"));
	m_open_door_effect = Mix_LoadWAV(audio_path("open_door.wav"));
	m_locked_door_effect = Mix_LoadWAV(audio_path("locked.wav"));
	m_rocket_effect = Mix_LoadWAV(audio_path("rocket.wav"));
	m_collision_effect = Mix_LoadWAV(audio_path("collision.wav"));

	// set the volume for the music and sound effects
	Mix_VolumeMusic((int)(MIX_MAX_VOLUME / 2.5));
	Mix_VolumeChunk(m_robot_hurt_effect, MIX_MAX_VOLUME/2);
	Mix_VolumeChunk(m_open_door_effect, MIX_MAX_VOLUME/4);
	Mix_VolumeChunk(m_locked_door_effect, MIX_MAX_VOLUME); // locked door effect kind of quiet, so make it louder
	Mix_VolumeChunk(m_rocket_effect, MIX_MAX_VOLUME/3);
	Mix_VolumeChunk(m_collision_effect, MIX_MAX_VOLUME/9);

	if (
		m_background_music == nullptr || m_open_door_effect == nullptr || m_robot_hurt_effect == nullptr ||
		m_locked_door_effect == nullptr || m_rocket_effect == nullptr
	)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("background.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("open_door.wav"),
			audio_path("locked.wav"),
			audio_path("rocket.wav"),
			audio_path("collision.wav"));
		return;
	}

	// Playing background music indefinitely
	Mix_FadeInMusic(m_background_music, -1, 1500);
}

void World::stop_sounds()
{
	// free sound effects
	if (m_robot_hurt_effect != nullptr) {
		Mix_FreeChunk(m_robot_hurt_effect);
		m_robot_hurt_effect = nullptr;
	}
	if (m_open_door_effect != nullptr) {
		Mix_FreeChunk(m_open_door_effect);
		m_open_door_effect = nullptr;
	}
	if (m_locked_door_effect != nullptr) {
		Mix_FreeChunk(m_locked_door_effect);
		m_locked_door_effect = nullptr;
	}
	// free background music
	stop_music();
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

bool World::start_maker_level()
{
	std::ifstream f(maker_file);
	if (f.good())
	{
		load_level("maker_level");
		return true;
	}

	return false;
}

void World::reset()
{
	m_level.reset_level();
}

void World::load_level(std::string level)
{
	stop_music();
	m_load();
	bool valid = m_level.parse_level(level, m_unlocked, m_robot_ls_pos);
	start_sounds();

	if (valid)
	{
		camera_pos = m_level.get_starting_camera_position();
		on_load_delay = UPDATE_FREEZE_DURATION;
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
