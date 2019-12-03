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

static GameManager* gm;

static void pause()
{
	gm->load_pause_menu();
}

static void load()
{
	gm->draw_loading_screen();
}

static void exit()
{
	gm->back_to_maker_menu();
}

bool GameManager::init(vec2 screen)
{
	gm = this;
	m_screen = screen;

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

	m_main_menu.init(m_window, screen);
	load_main_menu();

	m_main_menu.start_music();

	m_story_menu.init(m_window, screen);
	load_story_menu();

	m_maker_menu.init(m_window, screen);
	load_maker_menu();

	m_pause_menu.init(m_window, screen);
	load_pause_menu();

	m_load_menu.init(m_window, screen);
	load_loading_menu();

	m_in_menu = true;
	m_menu = &m_main_menu;

	// Setting window title
	std::stringstream title_ss;
	title_ss << "ECHO's in the Dark";
	glfwSetWindowTitle(m_window, title_ss.str().c_str());

	return true;
}

void GameManager::update(float elapsed_ms)
{
	if (!m_in_menu)
	{
		if (m_in_maker)
		{
			m_maker.update(elapsed_ms);
		}
		else
		{
			m_world.update(elapsed_ms);
		}
	}
}

void GameManager::draw()
{
	if (game_over())
	{
		return;
	}

	if (m_in_menu)
	{
		m_menu->draw();
	}
	else if (m_in_maker)
	{
		m_maker.draw();
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
		return m_menu->is_over();
	}
	else if (m_in_maker)
	{
		return m_maker.is_over();
	}
	else 
	{
		return m_world.is_over();
	}
}

void GameManager::destroy()
{
	m_main_menu.destroy();
	m_pause_menu.destroy();
	m_load_menu.destroy();
	m_world.destroy();
	m_maker.destroy();

	glfwDestroyWindow(m_window);
}

void GameManager::on_key(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	if (m_in_menu)
	{
		if (!m_menu->handle_key_press(window, key, scancode, action, mod))
		{
			m_menu->stop_music();
			m_in_menu = false;
			if (!m_in_maker)
			{
				m_world.start_sounds();
			}
		}
	}
	else if (m_in_maker)
	{
		if (!m_maker.handle_key_press(window, key, scancode, action, mod))
		{
			m_in_menu = true;
			m_menu = &m_pause_menu;
			m_menu->start_music();
		}
	}
	else
	{
		if (!m_world.handle_key_press(window, key, scancode, action, mod))
		{
			m_world.stop_sounds();
			m_in_menu = true;
			m_menu = &m_pause_menu;
			m_menu->start_music();
		}
	}
}

void GameManager::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	if (m_in_menu)
	{
		m_menu->handle_mouse_move(window, xpos, ypos);
	}
	else if (m_in_maker)
	{
		m_maker.handle_mouse_move(window, xpos, ypos);
	}
	else 
	{
		m_world.handle_mouse_move(window, xpos, ypos);
	}
}

void GameManager::on_click(GLFWwindow* window, int button, int action, int mods)
{
	if (action != GLFW_PRESS)
	{
		return;
	}

	if (m_in_menu)
	{
		Status s = m_menu->handle_mouse_button(button, action);

		switch (s)
		{
		case Status::nothing:
			return;
			break;
		case Status::resume:
			m_menu->stop_music();
			m_in_menu = false;
			if (!m_in_maker)
			{
				m_world.start_sounds();
			}
			break;
		case Status::new_game:
			m_menu->stop_music();
			m_in_menu = false;
			m_in_maker = false;
			m_world.destroy();
			m_world.init(m_window, m_screen);
			m_world.set_pl_functions(load, exit);
			m_world.start_sounds();
			m_world.start_level(true);
			break;
		case Status::load_game:
			m_menu->stop_music();
			m_in_menu = false;
			m_in_maker = false;
			m_world.destroy();
			m_world.init(m_window, m_screen);
			m_world.set_pl_functions(load, exit);
			m_world.start_level(false);
			m_world.start_sounds();
			break;
		case Status::main_menu:
			m_menu = &m_main_menu;
			m_world.destroy();
			break;
		case Status::save_game:
			if (m_in_maker)
			{
				m_maker.save();
			}
			else
			{
				m_world.save();
			}
			break;
		case Status::reset:
			m_menu->stop_music();
			m_in_menu = false;
			if (m_in_maker)
			{
				m_maker.destroy();
				m_maker.init(m_window, m_screen);
				m_maker.set_load_trigger(load);
				m_maker.generate_starter();
			}
			else
			{
				m_world.reset();
				m_world.start_sounds();
			}
			break;
		case Status::exit:
			m_is_over = true;
			break;
		case Status::story_mode:
			m_menu = &m_story_menu;
			break;
		case Status::maker_mode:
			m_menu = &m_maker_menu;
			break;
		case Status::make_level:
			m_menu->stop_music();
			m_in_menu = false;
			m_in_maker = true;
			m_maker.destroy();
			m_maker.init(m_window, m_screen);
			m_maker.set_load_trigger(load);
			m_maker.generate_starter();
			break;
		case Status::play_level:
			m_menu->stop_music();
			m_in_menu = false;
			m_in_maker = false;
			m_world.destroy();
			m_world.init(m_window, m_screen);
			m_world.set_pl_functions(load, exit);
			if (!m_world.start_maker_level()) {
				m_world.destroy();
				m_in_menu = true;
				m_menu->start_music();
			}
			m_world.start_sounds();
			break;
		case Status::load_level:
			m_menu->stop_music();
			m_in_menu = false;
			m_in_maker = true;
			m_maker.destroy();
			m_maker.init(m_window, m_screen);
			m_maker.set_load_trigger(load);
			m_maker.load();
			break;
		default:
			break;
		}
	} else if (m_in_maker)
	{
		m_maker.handle_mouse_click(window, button, action, mods);
	} else
    {
        m_world.handle_mouse_click(button, action);
    }
}

void GameManager::load_main_menu()
{
	std::vector<std::pair<std::string, Status>> buttons;
	buttons.push_back(std::make_pair("story_mode.png", Status::story_mode));
	buttons.push_back(std::make_pair("maker_mode.png", Status::maker_mode));
	buttons.push_back(std::make_pair("exit.png", Status::exit));
	m_main_menu.setup(buttons);
}

void GameManager::load_story_menu()
{
	std::vector<std::pair<std::string, Status>> buttons;
	buttons.push_back(std::make_pair("new_game.png", Status::new_game));
	buttons.push_back(std::make_pair("load_game.png", Status::load_game));
	buttons.push_back(std::make_pair("main_menu.png", Status::main_menu));
	m_story_menu.setup(buttons);
}

void GameManager::load_maker_menu()
{
	std::vector<std::pair<std::string, Status>> buttons;
	buttons.push_back(std::make_pair("play_level.png", Status::play_level));
	buttons.push_back(std::make_pair("make_level.png", Status::make_level));
	buttons.push_back(std::make_pair("load_level.png", Status::load_level));
	buttons.push_back(std::make_pair("main_menu.png", Status::main_menu));
	m_maker_menu.setup(buttons);
}

void GameManager::load_pause_menu()
{
	std::vector<std::pair<std::string, Status>> buttons;
	buttons.push_back(std::make_pair("resume.png", Status::resume));
	buttons.push_back(std::make_pair("reset.png", Status::reset));
	buttons.push_back(std::make_pair("save_game.png", Status::save_game));
	buttons.push_back(std::make_pair("main_menu.png", Status::main_menu));
	m_pause_menu.setup(buttons);
}

void GameManager::load_loading_menu()
{
	std::vector<std::pair<std::string, Status>> buttons;
	buttons.push_back(std::make_pair("loading.png", Status::nothing));
	m_load_menu.setup(buttons);
}

void GameManager::draw_loading_screen()
{
	m_load_menu.draw();
}

void GameManager::back_to_maker_menu()
{
	m_world.stop_music();
	m_world.destroy();
	m_in_maker = false;
	m_in_menu = true;
	m_menu = &m_maker_menu;
	m_menu->start_music();
}
