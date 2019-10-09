// Header
#include "world.hpp"

// stlib
#include <string>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

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
	m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "Salmon Game Assignment", nullptr, nullptr);
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

	m_background_music = Mix_LoadMUS(audio_path("music.wav"));

	if (m_background_music == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("salmon_eat.wav"));
		return false;
	}

	// Playing background music indefinitely
	Mix_PlayMusic(m_background_music, -1);

	fprintf(stderr, "Loaded music\n");

	// Setting window title
	std::stringstream title_ss;
	title_ss << "ECHOs in the Dark";
	glfwSetWindowTitle(m_window, title_ss.str().c_str());

	bool valid = parse_level("test") && m_light.init();

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
	m_robot.destroy();
	m_light.destroy();
	m_bricks.clear();
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w / m_screen_scale, (float)h / m_screen_scale };

	//-------------------------------------------------------------------------
	// Handle robot physics update

	// Maximum velocity in absolute value
	vec2 max_vel = { 400.f, 600.f };

	float time_factor = elapsed_ms / 1000;
	vec2 robot_pos = m_robot.get_position();
	vec2 robot_vel = m_robot.get_velocity();
	vec2 robot_acc = m_robot.get_acceleration();
	robot_acc.y += 1000; // gravity

	// Update velocity
	vec2 new_robot_vel = { robot_vel.x + robot_acc.x * time_factor, robot_vel.y + robot_acc.y * time_factor};
	if (new_robot_vel.x > max_vel.x) {
		new_robot_vel.x = max_vel.x;
	} else if (new_robot_vel.x < max_vel.x * -1.f) {
		new_robot_vel.x = max_vel.x * -1.f;
	}
	if (new_robot_vel.y > max_vel.y) {
		new_robot_vel.y = max_vel.y;
	} else if (new_robot_vel.y < max_vel.y * -1.f) {
		new_robot_vel.y = max_vel.y * -1.f;
	}
	m_robot.set_velocity(new_robot_vel);

	// Update position
	vec2 new_robot_pos = { robot_pos.x + new_robot_vel.x * time_factor, robot_pos.y + new_robot_vel.y * time_factor};

	// Detect collision
	// If the player will collide with an object next tick with the new velocity,
	// it will set velocity and acceleration to 0 and not update the position
	float translation = new_robot_vel.x * time_factor;
	for (auto& i_brick : m_bricks) 
	{
		const auto& robot_hitbox_x = m_robot.get_hitbox({ translation, 0.f });
		Brick brick = i_brick;
		if (brick.get_hitbox().collides_with(robot_hitbox_x)) {
			m_robot.set_velocity({ 0.f, m_robot.get_velocity().y });

			float circle_width = brick_size.x / 2.f;
			if (abs(m_robot.get_position().y - brick.get_position().y) > brick_size.y / 2.f)
			{
				float param = abs(m_robot.get_position().y - brick.get_position().y) - brick_size.y / 2.f;
				float dist_no_sqrt = pow(brick_size.y / 2.f, 2.f) - pow(param, 2.f);
				if (dist_no_sqrt >= 0.f)
					circle_width = sqrt(dist_no_sqrt);
			}

			new_robot_pos.x = get_closest_point(robot_pos.x, brick.get_position().x, circle_width, brick_size.x / 2.f);
			translation = new_robot_pos.x - robot_pos.x;
		}
	}

	m_robot.set_position({ new_robot_pos.x, robot_pos.y });

	translation = new_robot_vel.y * time_factor;
	for (auto& i_brick : m_bricks)
	{
		const auto& robot_hitbox_y = m_robot.get_hitbox({ 0.f, translation });
		Brick brick = i_brick;
		if (brick.get_hitbox().collides_with(robot_hitbox_y)) {
			m_robot.set_velocity({ m_robot.get_velocity().x, 0.f });

			float circle_width = brick_size.y / 2.f;
			if (abs(m_robot.get_position().x - brick.get_position().x) > brick_size.x / 2.f)
			{
				float param = abs(m_robot.get_position().x - brick.get_position().x) - brick_size.x / 2.f;
				float dist_no_sqrt = pow(brick_size.x / 2.f, 2.f) - pow(param, 2.f);
				if (dist_no_sqrt >= 0.f)
					circle_width = sqrt(dist_no_sqrt);
			}

			new_robot_pos.y = get_closest_point(robot_pos.y, brick.get_position().y, circle_width, brick_size.y / 2.f);
			translation = new_robot_pos.y -robot_pos.y;

			if (brick.get_position().y > new_robot_pos.y)
				m_robot.set_grounded();
		}
	}

	m_robot.set_position(new_robot_pos);
	m_robot.update(time_factor);
	m_light.set_position(new_robot_pos);

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
	m_robot.draw(projection_2D, camera_shift);

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
	float acceleration = 1800.f;
	vec2 robot_vel = m_robot.get_velocity();
	vec2 robot_acc = m_robot.get_acceleration();

	if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
		m_robot.set_acceleration({ robot_acc.x, robot_acc.y + acceleration * -1.f });
	}
	if (action == GLFW_PRESS && (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)) {
		m_robot.set_acceleration({ robot_acc.x + acceleration * -1.f, robot_acc.y });
	}
	if (action == GLFW_PRESS && (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)) {
		m_robot.set_acceleration({ robot_acc.x + acceleration, robot_acc.y });
	}
	if (action == GLFW_PRESS && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		camera_offset -= 100;
	}
	if (action == GLFW_PRESS && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		camera_offset += 100;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE) {
		m_robot.set_acceleration({ robot_acc.x, robot_acc.y - acceleration * -1.f });
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)) {
		m_robot.set_acceleration({ robot_acc.x - acceleration * -1.f, robot_acc.y });
		m_robot.set_velocity({ 0.f, robot_vel.y });
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)) {
		m_robot.set_acceleration({ robot_acc.x - acceleration, robot_acc.y });
		m_robot.set_velocity({ 0.f, robot_vel.y });
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_UP || key == GLFW_KEY_W)) {
		camera_offset += 100;
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)) {
		camera_offset -= 100;
	}
}

void World::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{

}

bool World::parse_level(std::string level)
{
	static std::map<char, vec3> colours;
	colours['B'] = { 1.f, 1.f, 1.f };
	colours['C'] = { 1.f, 0.f, 0.f };
	colours['M'] = { 0.f, 1.f, 0.f };
	colours['N'] = { 0.f, 0.f, 1.f };
	colours['Y'] = { 1.f, 1.f, 0.f };
	colours['Z'] = { 1.f, 0.f, 1.f };
	colours['L'] = { 0.f, 1.f, 1.f };

	std::string filename = level_path;
	filename.append(level);
	filename.append(".txt");
	std::ifstream file;
	file.open(filename);
	if (file.is_open())
	{
		fprintf(stderr, "Opened level file\n");

		m_bricks.clear();

		float x = 0.f;
		float y = 0.f;
		std::string line;

		// Get ambient light level
		if (!getline(file, line))
			return false;
		m_light.set_ambient(std::stof(line));

		// Get file name for level through the door
		if (!getline(file, line))
			return false;
		std::string next_level = line;

		// Get the text from the signs in the order they will appear 
		// starting from top to bottom/left to right
		if (!getline(file, line))
			return false;

		std::vector<std::string> signs;
		int sign_i = 0;
		while (line.compare("endsigns") != 0)
		{
			signs.push_back(line);
			if (!getline(file, line))
				return false;
		}

		// Finally get the actual map data
		while (getline(file, line))
		{
			for (x = 0.f; x < line.length(); x++)
			{
				vec2 position;
				position.x = x * brick_size.x;
				position.y = y * brick_size.y;
				switch (line[x])
				{
				case 'D':
					if (!spawn_door(position, next_level))
						return false;
					break;
				case 'G':
					if (!spawn_ghost(position))
						return false;
					break;
				case 'R':
					if (!spawn_robot(position))
						return false;
					break;
				case 'S':
					if (!spawn_sign(position, signs[sign_i++]))
						return false;
					break;
				case 'T':
					m_light.add_torch(position);
					break;
				case ' ':
					break;
				default:
					if (colours.find(line[x]) == colours.end())
						return false;

					if (!spawn_brick(position, colours[line[x]]))
						return false;
				}
			}
			y++;
		}
	}
	else
		return false;

	return true;
}

bool World::spawn_door(vec2 position, std::string next_level)
{
	// TODO: add door code
	fprintf(stderr, "door at (%f, %f) goes to level \"%s\"\n", position.x, position.y, next_level.c_str()); // remove once real code is done
	return true;
}

bool World::spawn_ghost(vec2 position)
{
	// TODO: add ghost code
	fprintf(stderr, "ghost at (%f, %f)\n", position.x, position.y);
	return true;
}

bool World::spawn_robot(vec2 position)
{
	if (m_robot.init())
	{
		m_robot.set_position(position);
		m_robot.set_head_position(position);
        if (m_light.init())
            m_light.set_position(m_robot.get_position());

		return true;
		// TODO: init light when robot is spawned
	}
	fprintf(stderr, "Robot spawn failed\n");
	return false;
}

bool World::spawn_sign(vec2 position, std::string text)
{
	// TODO: add sign code
	fprintf(stderr, "sign at (%f, %f) has text \"%s\"\n", position.x, position.y, text.c_str()); // remove once real code is done
	return true;
}

bool World::spawn_brick(vec2 position, vec3 colour)
{
	// TODO: make bricks respond to different colours
	bool x = colour.x == 1.f;
	bool y = colour.y == 1.f;
	bool z = colour.z == 1.f;
	if (!x || !y || !z)
		fprintf(stderr, "brick at (%f, %f)is coloured\n", position.x, position.y); // remove once real code is done

	Brick brick;
	if (brick.init())
	{
		brick.set_position(position);
		m_bricks.push_back(brick);
		return true;
	}
	fprintf(stderr, "Brick spawn failed\n");
	return false;
}
