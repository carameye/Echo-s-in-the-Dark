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

static void load()
{
	gm->draw_loading_screen();
}

static void exit()
{
	gm->back_to_maker_menu();
}

static void success()
{
    gm->to_success_menu();
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
    auto mouse_scroll_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((GameManager*)glfwGetWindowUserPointer(wnd))->on_scroll(wnd, _0, _1); };
	glfwSetKeyCallback(m_window, key_redirect);
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(m_window, mouse_button_redirect);
	glfwSetScrollCallback(m_window, mouse_scroll_redirect);

	m_sound_system = SoundSystem::get_system();

	m_title_menu.init(m_window, screen);
	load_title_menu();

	m_main_menu.init(m_window, screen);
	load_main_menu();

	m_story_menu.init(m_window, screen);
	load_story_menu();

    m_introduction_1_menu.init(m_window, screen);
    load_introduction_1_menu();

    m_introduction_2_menu.init(m_window, screen);
    load_introduction_2_menu();

    m_introduction_3_menu.init(m_window, screen);
    load_introduction_3_menu();

    m_introduction_4_menu.init(m_window, screen);
    load_introduction_4_menu();

    m_success_menu.init(m_window, screen);
    load_success_menu();

    m_credits_menu.init(m_window, screen);
    load_credits_menu();

	m_maker_menu.init(m_window, screen);
	load_maker_menu();

	m_world_pause_menu.init(m_window, screen);
	load_world_pause_menu();

	m_maker_pause_menu.init(m_window, screen);
	load_maker_pause_menu();

	m_maker_help_menu.init(m_window, screen);
	load_maker_help_menu();

	m_load_menu.init(m_window, screen);
	load_loading_menu();

	m_settings_menu.init(m_window, screen);
	load_settings_menu();

	m_maker_instructions_menu.init(m_window, screen);
	load_maker_instructions_menu();

	m_in_menu = true;
	m_menu = &m_title_menu;

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
	m_title_menu.destroy();
	m_main_menu.destroy();
	m_world_pause_menu.destroy();
	m_maker_pause_menu.destroy();
	m_maker_help_menu.destroy();
	m_maker_instructions_menu.destroy();
	m_load_menu.destroy();
	m_world.destroy();
	m_maker.destroy();
	m_sound_system->free_sounds();

	glfwDestroyWindow(m_window);
}

void GameManager::on_key(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	if (m_in_menu)
	{
		if (!m_menu->handle_key_press(window, key, scancode, action, mod))
		{
			m_in_menu = false;
			if (!m_in_maker)
			{
				m_sound_system->play_bgm(m_world.get_background_music());
				m_sound_system->resume_all_sound_effects();
			}
		}
	}
	else if (m_in_maker)
	{
		if (!m_maker.handle_key_press(window, key, action))
		{
			m_in_menu = true;
			m_menu = &m_maker_pause_menu;
			m_sound_system->play_bgm(Music::menu);
		}
	}
	else
	{
		if (!m_world.handle_key_press(window, key, action))
		{
			m_in_menu = true;
			m_menu = &m_world_pause_menu;
			m_sound_system->pause_all_sound_effects();
			m_sound_system->play_bgm(Music::menu);
		}
	}
}

void GameManager::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	m_title_menu.handle_mouse_move(window, xpos, ypos);
	m_main_menu.handle_mouse_move(window, xpos, ypos);
	m_story_menu.handle_mouse_move(window, xpos, ypos);
	m_introduction_1_menu.handle_mouse_move(window, xpos, ypos);
	m_introduction_2_menu.handle_mouse_move(window, xpos, ypos);
	m_introduction_3_menu.handle_mouse_move(window, xpos, ypos);
	m_introduction_4_menu.handle_mouse_move(window, xpos, ypos);
    m_success_menu.handle_mouse_move(window, xpos, ypos);
    m_credits_menu.handle_mouse_move(window, xpos, ypos);
	m_maker_menu.handle_mouse_move(window, xpos, ypos);
	m_world_pause_menu.handle_mouse_move(window, xpos, ypos);
	m_maker_pause_menu.handle_mouse_move(window, xpos, ypos);
	m_maker_help_menu.handle_mouse_move(window, xpos, ypos);
	m_load_menu.handle_mouse_move(window, xpos, ypos);
	m_settings_menu.handle_mouse_move(window, xpos, ypos);
	m_maker_instructions_menu.handle_mouse_move(window, xpos, ypos);

	if (m_in_maker)
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
	if (m_in_menu)
	{
		if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
			return;
		}

		Status s = m_menu->handle_mouse_button(button, action);
		fprintf(stderr, "%d\n", s);

		switch (s)
		{
		case Status::nothing:
			return;
			break;
		case Status::resume:
			m_in_menu = false;
			if (!m_in_maker)
			{
				m_sound_system->play_bgm(m_world.get_background_music());
				m_sound_system->resume_all_sound_effects();
				m_world.poll_keys(window);
			} else {
				m_sound_system->play_bgm(Music::level_builder);
				m_maker.poll_keys(window);
			}
			break;
		case Status::new_game:
			m_in_menu = false;
			m_in_maker = false;
			m_world.destroy();
			m_world.init(m_window, m_screen);
			m_world.set_pl_functions(load, exit, success);
			m_world.start_level(true);
			m_sound_system->play_bgm(m_world.get_background_music());
			m_sound_system->resume_all_sound_effects();
			break;
		case Status::load_game:
			m_in_menu = false;
			m_in_maker = false;
			m_world.destroy();
			m_world.init(m_window, m_screen);
			m_world.set_pl_functions(load, exit, success);
			m_world.start_level(false);
			m_sound_system->play_bgm(m_world.get_background_music());
			m_sound_system->resume_all_sound_effects();
			break;
		case Status::title_menu:
			m_menu = &m_title_menu;
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
			m_in_menu = false;
			if (m_in_maker)
			{
				m_maker.destroy();
				m_maker.init(m_window, m_screen);
				m_maker.set_load_trigger(load);
				m_maker.generate_starter();
				m_sound_system->play_bgm(Music::level_builder);
			}
			else
			{
				m_world.reset();
				m_sound_system->play_bgm(m_world.get_background_music());
				m_sound_system->resume_all_sound_effects();
				m_world.poll_keys(window);
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
		case Status::go_to_intro_1:
		    m_menu = &m_introduction_1_menu;
		    break;
        case Status::go_to_intro_2:
            m_menu = &m_introduction_2_menu;
            break;
        case Status::go_to_intro_3:
            m_menu = &m_introduction_3_menu;
            break;
        case Status::go_to_intro_4:
            m_menu = &m_introduction_4_menu;
            break;
	    case Status::go_to_credits:
	        m_menu = &m_credits_menu;
	        break;
		case Status::make_level:
			fprintf(stderr, "making level\n");
			m_in_menu = false;
			m_in_maker = true;
			m_maker.destroy();
			m_maker.init(m_window, m_screen);
			m_maker.set_load_trigger(load);
			m_maker.generate_starter();
			m_sound_system->play_bgm(Music::level_builder);
			break;
		case Status::play_level:
			m_maker.save();
			m_in_menu = false;
			m_in_maker = false;
			m_world.destroy();
			m_world.init(m_window, m_screen);
			m_world.set_pl_functions(load, exit, success);
			if (!m_world.start_maker_level()) {
				m_world.destroy();
				m_in_menu = true;
			}
			m_sound_system->play_bgm(m_world.get_background_music());
			m_sound_system->resume_all_sound_effects();
			break;
		case Status::load_level:
			m_in_menu = false;
			m_in_maker = true;
			m_maker.destroy();
			m_maker.init(m_window, m_screen);
			m_maker.set_load_trigger(load);
			m_maker.load();
			m_sound_system->play_bgm(Music::level_builder);
			break;
		case Status::help:
			m_menu = &m_maker_help_menu;
			break;
		case Status::ret_pause:
			m_menu = &m_maker_pause_menu;
			break;
		case Status::settings:
			m_menu = &m_settings_menu;
			break;
		case Status::dec_sens:
			if (scroll_sensitivity < 16.f)
			{
				scroll_sensitivity *= 2.f;
			}
			break;
		case Status::inc_sens:
			if (scroll_sensitivity > 1.f)
			{
				scroll_sensitivity /= 2.f;
			}
			break;
		case Status::maker_instructions:
			m_menu = &m_maker_instructions_menu;
		default:
			break;
		}
	} else if (m_in_maker)
	{
		if ((button != GLFW_MOUSE_BUTTON_LEFT && button != GLFW_MOUSE_BUTTON_RIGHT) ||
			(action != GLFW_PRESS && action != GLFW_RELEASE)) {
			return;
		}
		m_maker.handle_mouse_click(window, button, action, mods);
	} else
    {
		if (action != GLFW_PRESS) {
			return;
		}
        m_world.handle_mouse_click(button, action);
    }
}

void GameManager::on_scroll(GLFWwindow *window, double xoffset, double yoffset) {
    if (m_in_menu)
    {
        return;
    }
    else if (m_in_maker)
    {
        return;
    }
    else
    {
        m_world.handle_mouse_scroll(yoffset);
    }
}

void GameManager::load_title_menu()
{
	vec2 button_size = { 1200.f, 800.f };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("title.png", Status::main_menu, button_size));
	m_title_menu.setup(buttons);
}

void GameManager::load_main_menu()
{
	vec2 button_size = { 8.f * brick_size, 2.f * brick_size };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("story_mode.png", Status::story_mode, button_size));
	buttons.push_back(std::make_tuple("maker_mode.png", Status::maker_mode, button_size));
	buttons.push_back(std::make_tuple("settings.png", Status::settings, button_size));
	buttons.push_back(std::make_tuple("exit.png", Status::exit, button_size));
	m_main_menu.setup(buttons);
}

void GameManager::load_story_menu()
{
    vec2 button_size = { 8.f * brick_size, 2.f * brick_size };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("new_game.png", Status::go_to_intro_1, button_size));
	buttons.push_back(std::make_tuple("load_game.png", Status::load_game, button_size));
	buttons.push_back(std::make_tuple("main_menu.png", Status::main_menu, button_size));
	m_story_menu.setup(buttons);
}

void GameManager::load_introduction_1_menu()
{
    vec2 button_size = { 1200.f, 800.f };
    std::vector<std::tuple<std::string, Status, vec2>> buttons;
    buttons.push_back(std::make_tuple("intro_1_full.png", Status::go_to_intro_2, button_size));
    m_introduction_1_menu.setup(buttons);
}

void GameManager::load_introduction_2_menu()
{
    vec2 button_size = { 1200.f, 800.f };
    std::vector<std::tuple<std::string, Status, vec2>> buttons;
    buttons.push_back(std::make_tuple("intro_2_full.png", Status::go_to_intro_3, button_size));
    m_introduction_2_menu.setup(buttons);
}

void GameManager::load_introduction_3_menu()
{
    vec2 button_size = { 1200.f, 800.f };
    std::vector<std::tuple<std::string, Status, vec2>> buttons;
    buttons.push_back(std::make_tuple("intro_3_full.png", Status::go_to_intro_4, button_size));
    m_introduction_3_menu.setup(buttons);
}

void GameManager::load_introduction_4_menu()
{
    vec2 button_size = { 1200.f, 800.f };
    std::vector<std::tuple<std::string, Status, vec2>> buttons;
    buttons.push_back(std::make_tuple("intro_4_full.png", Status::new_game, button_size));
    m_introduction_4_menu.setup(buttons);
}

void GameManager::load_success_menu()
{
    vec2 button_size = { 1200.f, 800.f };
    std::vector<std::tuple<std::string, Status, vec2>> buttons;
    buttons.push_back(std::make_tuple("ending_1.png", Status::go_to_credits, button_size));
    m_success_menu.setup(buttons);
}

void GameManager::load_credits_menu()
{
    vec2 button_size = { 1200.f, 800.f };
    std::vector<std::tuple<std::string, Status, vec2>> buttons;
    buttons.push_back(std::make_tuple("ending_2.png", Status::main_menu, button_size));
    m_credits_menu.setup(buttons);
}

void GameManager::load_maker_menu()
{
	vec2 button_size = { 8.f * brick_size, 2.f * brick_size };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("make_level.png", Status::maker_instructions, button_size));
	buttons.push_back(std::make_tuple("load_level.png", Status::load_level, button_size));
	buttons.push_back(std::make_tuple("main_menu.png", Status::main_menu, button_size));
	m_maker_menu.setup(buttons);
}

void GameManager::load_world_pause_menu()
{
	vec2 button_size = { 8.f * brick_size, 2.f * brick_size };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("resume.png", Status::resume, button_size));
	buttons.push_back(std::make_tuple("reset.png", Status::reset, button_size));
	buttons.push_back(std::make_tuple("save_game.png", Status::save_game, button_size));
	buttons.push_back(std::make_tuple("main_menu.png", Status::main_menu, button_size));
	m_world_pause_menu.setup(buttons);
}

void GameManager::load_maker_pause_menu()
{
	vec2 button_size = { 4.f * brick_size, 1.f * brick_size };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("resume.png", Status::resume, button_size));
	buttons.push_back(std::make_tuple("reset.png", Status::reset, button_size));
	buttons.push_back(std::make_tuple("play_level.png", Status::play_level, button_size));
	buttons.push_back(std::make_tuple("help_button.png", Status::help, button_size));
	buttons.push_back(std::make_tuple("save_game.png", Status::save_game, button_size));
	buttons.push_back(std::make_tuple("main_menu.png", Status::main_menu, button_size));
	m_maker_pause_menu.setup(buttons);
}

void GameManager::load_maker_help_menu()
{
	vec2 button_size = { 1200.f, 800.f };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("maker_help_screen.png", Status::ret_pause, button_size));
	m_maker_help_menu.setup(buttons);
}

void GameManager::load_maker_instructions_menu()
{
	vec2 button_size = { 1200.f, 800.f };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("maker_instructions.png", Status::make_level, button_size));
	m_maker_instructions_menu.setup(buttons);
}

void GameManager::load_loading_menu()
{
	vec2 button_size = { 8.f * brick_size, 2.f * brick_size };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("loading.png", Status::nothing, button_size));
	m_load_menu.setup(buttons);
}

void GameManager::load_settings_menu()
{
	vec2 button_size = { 8.f * brick_size, 2.f * brick_size };
	std::vector<std::tuple<std::string, Status, vec2>> buttons;
	buttons.push_back(std::make_tuple("inc_sens.png", Status::inc_sens, button_size));
	buttons.push_back(std::make_tuple("dec_sens.png", Status::dec_sens, button_size));
	buttons.push_back(std::make_tuple("main_menu.png", Status::main_menu, button_size));
	m_settings_menu.setup(buttons);
}

void GameManager::draw_loading_screen()
{
	m_load_menu.draw();
}

void GameManager::back_to_maker_menu()
{
	m_world.destroy();
	m_in_maker = false;
	m_in_menu = true;
	m_menu = &m_maker_menu;
	m_sound_system->play_bgm(Music::menu);
}

void GameManager::to_success_menu()
{
    m_world.destroy();
    m_in_maker = false;
    m_in_menu = true;
    m_menu = &m_success_menu;
    m_sound_system->play_bgm(Music::menu);
}
