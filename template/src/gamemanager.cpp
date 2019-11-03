#include "gamemanager.hpp"

#include <sstream>
#include <vector>
#include <utility>

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
	auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((GameManager*)glfwGetWindowUserPointer(wnd))->on_click(wnd, _0, _1, _2); };
	glfwSetKeyCallback(m_window, key_redirect);
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(m_window, mouse_button_redirect);

	// Initialize audio
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	bool success = m_world.init(m_window, screen);
	success &= m_menu.init(m_window, screen);
	m_in_menu = true;
	load_main_menu();

	// Setting window title
	std::stringstream title_ss;
	title_ss << "ECHO's in the Dark";
	glfwSetWindowTitle(m_window, title_ss.str().c_str());

	return success;
}

void GameManager::update(float elapsed_ms)
{
	if (!m_in_menu)
	{
		m_world.update(elapsed_ms);
	}
}

void GameManager::draw()
{
	if (m_in_menu)
	{
		m_menu.draw();
	}
	else
	{
		m_world.draw();
	}
}

bool GameManager::game_over()
{
	if (m_is_over)
	{
		return true;
	}

	if (m_in_menu)
	{
		return m_menu.is_over();
	}
	else 
	{
		return m_world.is_over();
	}
}

void GameManager::destroy()
{
	m_menu.destroy();
	m_world.destroy();

	glfwDestroyWindow(m_window);
}

void GameManager::on_key(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	if (m_in_menu)
	{
		if (!m_menu.handle_key_press(window, key, scancode, action, mod))
		{
			m_menu.stop_music();
			m_in_menu = false;
			m_world.start_music();
		}
	}
	else
	{
		if (!m_world.handle_key_press(window, key, scancode, action, mod))
		{
			m_world.stop_music();
			m_in_menu = true;
			m_menu.start_music();
		}
	}
}

void GameManager::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	if (m_in_menu)
	{
		m_menu.handle_mouse_move(window, xpos, ypos);
	}
	else 
	{
		m_world.handle_mouse_move(window, xpos, ypos);
	}
}

void GameManager::on_click(GLFWwindow* window, int button, int action, int mods)
{
	if (m_in_menu)
	{
		Status s = m_menu.handle_mouse_button(button, action);

		switch (s)
		{
		case Status::nothing:
			return;
			break;
		case Status::resume:
			m_menu.stop_music();
			m_in_menu = false;
			m_world.start_music();
			break;
		case Status::new_game:
			m_menu.stop_music(); 
			m_menu.destroy();
			clear_ui_components();
			load_pause_menu();
			m_in_menu = false;
			m_world.start_level("level_select");
			m_world.start_music();
			break;
		case Status::load_game:
			m_menu.stop_music();
			m_menu.destroy();
			clear_ui_components();
			load_pause_menu();
			m_in_menu = false;
			m_world.start_level("level_select");
			m_world.start_music();
			break;
		case Status::main_menu:
			m_menu.destroy();
			clear_ui_components();
			load_main_menu();
			break;
		case Status::reset:
			m_menu.stop_music();
			m_in_menu = false;
			m_world.reset();
			m_world.start_music();
			break;
		case Status::exit:
			destroy();
			m_is_over = true;
			break;
		default:
			break;
		}
	}
}

void GameManager::load_main_menu()
{
	std::vector<std::pair<std::string, Status>> buttons;
	buttons.push_back(std::make_pair("new_game.png", Status::new_game));
	buttons.push_back(std::make_pair("load_game.png", Status::load_game));
	buttons.push_back(std::make_pair("exit.png", Status::exit));
	m_menu.setup(buttons);
}

void GameManager::load_pause_menu()
{
	std::vector<std::pair<std::string, Status>> buttons;
	buttons.push_back(std::make_pair("resume.png", Status::resume));
	buttons.push_back(std::make_pair("reset.png", Status::reset));
	buttons.push_back(std::make_pair("main_menu.png", Status::main_menu));
	m_menu.setup(buttons);
}
