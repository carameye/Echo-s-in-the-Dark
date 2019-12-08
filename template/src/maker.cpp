// Header
#include "maker.hpp"
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

Maker::Maker()
{
	
}

Maker::~Maker()
{

}

// Maker initialization
bool Maker::init(GLFWwindow* window, vec2 screen)
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

	camera_pos = { 0.f, 0.f };
	mouse_pos = { 0.f, 0.f };

	poll_keys(window);

	return true;
}

void Maker::set_load_trigger(void(*l)())
{
	m_load = l;
}

void Maker::generate_starter()
{
	m_load();
	camera_pos = m_maker_level.generate_starter();
}

// Releases all the associated resources
void Maker::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	m_maker_level.destroy();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void Maker::draw()
{
	// Clearing error buffer
	gl_flush_errors();

	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);

	/////////////////////////////////////
	// Truely render to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	const float clear_color[3] = { 19.f / 255.f, 41.f / 255.f, 60.f / 255.f };
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

	vec2 camera_shift = { right / 2.f - camera_pos.x, bottom / 2.f - camera_pos.y };

	m_maker_level.draw_entities(projection_2D, camera_shift);

	//////////////////
	// Presenting
	glfwSwapBuffers(m_window);
}

void Maker::update(float ms)
{
	float step_size;
	if (faster)
	{
		step_size = 100.f;
	}
	else
	{
		step_size = 30.f;
	}

	if (movement[0]) {
		camera_pos.y -= step_size * ms / 100.f;
	}
	if (movement[1]) {
		camera_pos.x -= step_size * ms / 100.f;
	}
	if (movement[2]) {
		camera_pos.y += step_size * ms / 100.f;
	}
	if (movement[3]) {
		camera_pos.x += step_size * ms / 100.f;
	}

	if (mouse_moved)
	{
		m_maker_level.handle_mouse_move(mouse_pos.x, mouse_pos.y, camera_pos);
		mouse_moved = false;
	}

	if (m_left_click_hold)
	{
		m_maker_level.handle_mouse_click(mouse_pos.x, mouse_pos.y, camera_pos, true);
	}
	if (m_right_click_hold)
	{
		m_maker_level.handle_mouse_click(mouse_pos.x, mouse_pos.y, camera_pos, false);
	}
}

// Should the game be over ?
bool Maker::is_over() const
{
	return glfwWindowShouldClose(m_window);
}

// On key callback
bool Maker::handle_key_press(GLFWwindow*, int key, int action)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		return false;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_W) {
		input_key_states[key] = action;
		movement[0] = true;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_A) {
		input_key_states[key] = action;
		movement[1] = true;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		input_key_states[key] = action;
		movement[2] = true;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_D) {
		input_key_states[key] = action;
		movement[3] = true;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_LEFT_SHIFT) {
		input_key_states[key] = action;
		faster = true;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_W) {
		input_key_states[key] = action;
		movement[0] = false;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_A) {
		input_key_states[key] = action;
		movement[1] = false;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_S) {
		input_key_states[key] = action;
		movement[2] = false;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_D) {
		input_key_states[key] = action;
		movement[3] = false;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT_SHIFT) {
		input_key_states[key] = action;
		faster = false;
	}

	m_maker_level.handle_key_press(key, action);
	mouse_moved = true;

	return true;
}

void Maker::poll_keys(GLFWwindow* window)
{
	for (auto& input_state : input_key_states) {
		int key = input_state.first;
		int old_key_state = input_state.second;
		int key_state = glfwGetKey(window, key);
		if (key_state != old_key_state) {
			handle_key_press(window, key, key_state);
		}
	}
}

void Maker::handle_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	mouse_pos = { (float)xpos, (float)ypos };
	mouse_moved = true;
}

void Maker::handle_mouse_click(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_RELEASE)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			m_left_click_hold = false;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			m_right_click_hold = false;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			m_left_click_hold = true;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			m_right_click_hold = true;
			break;
		default:
			break;
		}
	}
}

void Maker::load()
{
	m_load();
	camera_pos = m_maker_level.load_level();
}

void Maker::save()
{
	m_load();
	m_maker_level.process();
}
