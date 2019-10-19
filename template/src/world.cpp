// Header
#include "world.hpp"

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
	const size_t MAX_TURTLES = 15;
	const size_t MAX_FISH = 5;
	const size_t TURTLE_DELAY_MS = 2000;
	const size_t FISH_DELAY_MS = 5000;

	namespace
	{
		void glfw_err_cb(int error, const char* desc)
		{
			fprintf(stderr, "%d: %s", error, desc);
		}
	}
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
bool World::init(vec2 screen)
{
	//-------------------------------------------------------------------------
	// GLFW / OGL Initialization
	// Core Opengl 3.
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);
	m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "ECHO's in the Dark", nullptr, nullptr);
	if (m_window == nullptr)
		return false;

	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	gl3w_init();

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(m_window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((World*)glfwGetWindowUserPointer(wnd))->on_key(wnd, _0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((World*)glfwGetWindowUserPointer(wnd))->on_mouse_move(wnd, _0, _1); };
	glfwSetKeyCallback(m_window, key_redirect);
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);

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

	//-------------------------------------------------------------------------
	// Loading music and sounds
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	m_background_music = Mix_LoadMUS(audio_path("goldleaf.wav"));

	if (m_background_music == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("goldleaf.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("salmon_eat.wav"));
		return false;
	}

	// Playing background music indefinitely
	Mix_PlayMusic(m_background_music, -1);

	fprintf(stderr, "Loaded music\n");

	// Setting window title
	std::stringstream title_ss;
	title_ss << "ECHO's in the Dark";
	glfwSetWindowTitle(m_window, title_ss.str().c_str());

	bool valid = parse_level_json("level_select") && m_light.init();

	if (valid)
		camera_pos = m_robot.get_position();
	else
		camera_pos = { 0.f, 0.f };

	camera_offset = 0.f;

	return valid;
}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	if (m_background_music != nullptr)
		Mix_FreeMusic(m_background_music);

	Mix_CloseAudio();

	for (auto& brick : m_bricks)
		brick.destroy();
	for (auto& door : m_doors)
		door.destroy();
	for (auto& ghost : m_ghosts)
		ghost.destroy();
	m_robot.destroy();
	m_light.destroy();
	m_bricks.clear();
	m_doors.clear();
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w / m_screen_scale, (float)h / m_screen_scale };

	//-------------------------------------------------------------------------

	vec2 robot_pos = m_robot.get_position();

    m_robot.update_velocity(elapsed_ms);

    vec2 new_robot_vel = m_robot.get_velocity();
    vec2 new_robot_pos = m_robot.get_next_position();

	float translation = new_robot_vel.x;
	for (auto& i_brick : m_bricks) 
	{
		const auto& robot_hitbox_x = m_robot.get_hitbox({ translation, 0.f });
		Brick brick = i_brick;
		if (brick.get_hitbox().collides_with(robot_hitbox_x)) {
			m_robot.set_velocity({ 0.f, m_robot.get_velocity().y });

			float circle_width = brick_size / 2.f;
			if (abs(m_robot.get_position().y - brick.get_position().y) > brick_size / 2.f)
			{
				float param = abs(m_robot.get_position().y - brick.get_position().y) - brick_size / 2.f;
				float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
				if (dist_no_sqrt >= 0.f)
					circle_width = sqrt(dist_no_sqrt);
			}

			new_robot_pos.x = get_closest_point(robot_pos.x, brick.get_position().x, circle_width, brick_size / 2.f);
			translation = new_robot_pos.x - robot_pos.x;
		}
	}

	m_robot.set_position({ new_robot_pos.x, robot_pos.y });

	translation = new_robot_vel.y;
	for (auto& i_brick : m_bricks)
	{
		const auto& robot_hitbox_y = m_robot.get_hitbox({ 0.f, translation });
		Brick brick = i_brick;
		if (brick.get_hitbox().collides_with(robot_hitbox_y)) {
			m_robot.set_velocity({ m_robot.get_velocity().x, 0.f });

			float circle_width = brick_size / 2.f;
			if (abs(m_robot.get_position().x - brick.get_position().x) > brick_size / 2.f)
			{
				float param = abs(m_robot.get_position().x - brick.get_position().x) - brick_size / 2.f;
				float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
				if (dist_no_sqrt >= 0.f)
					circle_width = sqrt(dist_no_sqrt);
			}

			new_robot_pos.y = get_closest_point(robot_pos.y, brick.get_position().y, circle_width, brick_size / 2.f);
			translation = new_robot_pos.y -robot_pos.y;

			if (brick.get_position().y > new_robot_pos.y)
				m_robot.set_grounded();
		}
	}

	m_robot.set_position(new_robot_pos);
	m_robot.update(elapsed_ms);
	m_light.set_position(new_robot_pos);

	for (auto& ghost : m_ghosts)
	{
		ghost.set_goal(m_robot.get_position());
		ghost.update(elapsed_ms);
	}

	const Hitbox robot_hitbox = m_robot.get_hitbox({0.f, 0.f});
	for (auto& door : m_doors)
	{
		if (door.get_hitbox().collides_with(robot_hitbox)) {
			fprintf(stderr, "Collided with door\n");
			m_interactable_door = &door;
		}
	}

	float follow_speed = 0.1f;
	vec2 follow_point = add(m_robot.get_position(), {0.f, camera_offset});
	camera_pos = add(camera_pos, { follow_speed * (follow_point.x - camera_pos.x), follow_speed * (follow_point.y - camera_pos.y) });
	return true;
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
	vec2 camera_shift = { right / 2 - camera_pos.x, bottom / 2 - camera_pos.y };

	// Drawing entities
	for (auto& brick : m_bricks)
		brick.draw(projection_2D, camera_shift);
	for(auto& door : m_doors)
		door.draw(projection_2D, camera_shift);
	m_robot.draw(projection_2D, camera_shift);
	for (auto& ghost : m_ghosts)
		ghost.draw(projection_2D, camera_shift);

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

	m_light.draw(projection_2D, camera_shift);

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
void World::on_key(GLFWwindow*, int key, int, int action, int mod)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
		m_robot.start_flying();
	}
	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)) {
        m_robot.set_is_accelerating_left(true);
	}
	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)) {
        m_robot.set_is_accelerating_right(true);
	}
	if (action == GLFW_PRESS && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		camera_offset -= 100;
	}
	if (action == GLFW_PRESS && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		camera_offset += 100;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE) {
		m_robot.stop_flying();
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)) {
	    m_robot.set_is_accelerating_left(false);
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)) {
	    m_robot.set_is_accelerating_right(false);
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		camera_offset += 100;
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		camera_offset -= 100;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_F && !!(m_interactable_door)) {
		bool perform_action = m_interactable_door->perform_action();
		if (perform_action)
			parse_level_json(m_interactable_door->get_destination());
	}
}

void World::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{

}

bool World::parse_level_json(std::string level)
{
	// Construct file name with path
	std::string filename = level_path;
	filename.append(level);
	filename.append(".json");

	// Open file
	std::ifstream file(filename);
	if (!file.is_open())
	{
		return false;
	}
	fprintf(stderr, "Opened level file\n");

	// clear all level-dependent resources
	for (auto& brick : m_bricks)
		brick.destroy();
	for (auto& door : m_doors)
		door.destroy();
	for (auto& ghost : m_ghosts)
		ghost.destroy();
	m_bricks.clear();
	m_ghosts.clear();
	m_doors.clear();

	// Parse the json
	json j = json::parse(file);

	int width = j["size"]["width"];
	int height = j["size"]["height"];

	// Get ambient light level
	m_light.set_ambient(j["ambient_light"]);

	// Get the doors
	fprintf(stderr, "	getting doors\n");
	for (json door : j["doors"])
	{
		vec2 pos = { door["pos"]["x"], door["pos"]["y"] };
		spawn_door(to_pixel_position(pos), door["next_level"]);
	}

	// Get the signs
	fprintf(stderr, "	getting signs\n");
	for (json sign : j["signs"])
	{
		vec2 pos = { sign["pos"]["x"], sign["pos"]["y"] };
		spawn_sign(to_pixel_position(pos), sign["text"]);
	}

	// Get the ghosts
	fprintf(stderr, "	getting ghosts\n");
	for (json ghost : j["ghosts"])
	{
		vec2 pos = { ghost["pos"]["x"], ghost["pos"]["y"] };
		spawn_ghost(to_pixel_position(pos));
	}

	// Get the bricks
	fprintf(stderr, "	getting bricks\n");
	std::vector<vec2> potential_cp;
	std::vector<vec2> diffs = { { -1.f, -1.f }, { 1.f, -1.f }, { -1.f, 1.f }, { 1.f, 1.f } };

	std::vector<bool> empty(width, false);
	std::vector<std::vector<bool>> bricks(height, empty);

	for (json brick : j["bricks"])
	{
		vec2 pos = { brick["pos"]["x"], brick["pos"]["y"] };
		vec3 colour = { brick["colour"]["r"], brick["colour"]["g"], brick["colour"]["b"] };

		// Set brick here
		bricks[pos.y][pos.x] = true;

		// Add brick to critical points if not already cancelled
		for (vec2 diff : diffs)
		{
			vec2 pot = add(pos, diff);
			if (pot.x >= 0.f && pot.x < width && pot.y >= 0.f && pot.y < height)
			{
				potential_cp.push_back(pot);
			}
		}

		spawn_brick(to_pixel_position(pos), colour);
	}

	fprintf(stderr, "	built world with %d doors, %d ghosts, and %d bricks\n",
		m_doors.size(), m_ghosts.size(), m_bricks.size());

	// Generate the graph
	m_graph.generate(potential_cp, bricks, width, height);

	// Spawn the robot
	vec2 pos = { j["spawn"]["pos"]["x"], j["spawn"]["pos"]["y"] };
	spawn_robot(to_pixel_position(pos));

	return true;
}

bool World::spawn_door(vec2 position, std::string next_level)
{
	Door door;
	if (door.init())
	{
		door.set_position(position);
		door.set_destination(next_level);
		m_doors.push_back(door);
		return true;
	}
	fprintf(stderr, "	door spawn at (%f, %f) failed\n", position.x, position.y);
	return false;
}

bool World::spawn_ghost(vec2 position)
{
	Ghost ghost;
	if (ghost.init())
	{
		ghost.set_position(position);
		ghost.set_level_graph(&m_graph);
		m_ghosts.push_back(ghost);
		return true;
	}
	return false;
}

bool World::spawn_robot(vec2 position)
{
	if (m_robot.init())
	{
		m_robot.set_position(position);
		m_robot.set_head_position(position);
        m_robot.set_shoulder_position(position);
        if (m_light.init())
            m_light.set_position(m_robot.get_position());

		return true;
		// TODO: init light when robot is spawned
	}
	fprintf(stderr, "	robot spawn failed\n");
	return false;
}

bool World::spawn_sign(vec2 position, std::string text)
{
	// TODO: add sign code
	return true;
}

bool World::spawn_brick(vec2 position, vec3 colour)
{
	// TODO: make bricks respond to different colours
	Brick brick;
	if (brick.init())
	{
		brick.set_position(position);
		m_bricks.push_back(brick);
		return true;
	}
	fprintf(stderr, "	brick spawn failed\n");
	return false;
}
