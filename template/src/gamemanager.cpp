#include "gamemanager.hpp"

#include <sstream>

namespace
{
	void glfw_err_cb(int error, const char* desc)
	{
		fprintf(stderr, "%d: %s", error, desc);
	}
}

bool GameManager::init(vec2 screen)
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
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((GameManager*)glfwGetWindowUserPointer(wnd))->on_key(wnd, _0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((GameManager*)glfwGetWindowUserPointer(wnd))->on_mouse_move(wnd, _0, _1); };
	glfwSetKeyCallback(m_window, key_redirect);
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);

	bool success = m_world.init(m_window, screen);

	// Setting window title
	std::stringstream title_ss;
	title_ss << "ECHO's in the Dark";
	glfwSetWindowTitle(m_window, title_ss.str().c_str());

	return success;
}

void GameManager::update(float elapsed_ms)
{
	m_world.update(elapsed_ms);
}

void GameManager::draw()
{
	m_world.draw();
}

bool GameManager::game_over()
{
	return m_world.is_over();
}

void GameManager::destroy()
{
	m_world.destroy();
}

void GameManager::on_key(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	m_world.handle_key_press(window, key, scancode, action, mod);
}

void GameManager::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	m_world.handle_mouse_move(window, xpos, ypos);
}
