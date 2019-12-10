#pragma once

#include "world.hpp"
#include "UI/menu.hpp"
#include "maker.hpp"
#include "sound_system.hpp"

#include <stack>

class GameManager
{
public:
	// Initialize the game
	bool init(vec2 size);

	// Update the game
	void update(float elapsed_ms);

	// Draw the game
	void draw();

	// Is the game over
	bool game_over();

	// Destroy all assets
	void destroy();

	// Handle input
	void on_key(GLFWwindow*, int key, int, int action, int mod);
	void on_mouse_move(GLFWwindow* window, double xpos, double ypos);
	void on_click(GLFWwindow* window, int button, int action, int mods);
    void on_scroll(GLFWwindow* window, double xoffset, double yoffset);

	// Load menus
	void load_title_menu();
	void load_main_menu();
	void load_story_menu();
    void load_introduction_1_menu();
    void load_introduction_2_menu();
    void load_introduction_3_menu();
    void load_introduction_4_menu();
    void load_success_menu();
    void load_credits_menu();
	void load_maker_menu();
	void load_world_pause_menu();
	void load_maker_pause_menu();
	void load_maker_help_menu();
	void load_loading_menu();
	void load_settings_menu();
	void draw_loading_screen();
	void back_to_maker_menu();
	void load_maker_instructions_menu();
    void to_success_menu();

private:
	// Sound System
	SoundSystem* m_sound_system;

	// Window information
	GLFWwindow* m_window;
	vec2 m_screen;

	// Information about the state
	bool m_world_valid = true;
	bool m_in_menu = false;
	bool m_in_maker = false;

	// Game states
	World m_world;
	Maker m_maker;
	Menu* m_menu;
	Menu m_title_menu;
	Menu m_main_menu;
	Menu m_story_menu;
	Menu m_introduction_1_menu;
    Menu m_introduction_2_menu;
    Menu m_introduction_3_menu;
    Menu m_introduction_4_menu;
    Menu m_success_menu;
    Menu m_credits_menu;
	Menu m_maker_menu;
	Menu m_world_pause_menu;
	Menu m_maker_pause_menu;
	Menu m_maker_help_menu;
	Menu m_load_menu;
	Menu m_settings_menu;
	Menu m_maker_instructions_menu;

	// Should end game
	bool m_is_over = false;
};