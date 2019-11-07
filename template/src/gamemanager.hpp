#pragma once

#include "world.hpp"
#include "UI/menu.hpp"

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

	// Load menus
	void load_main_menu();
	void load_pause_menu();
	void load_loading_menu();
	void draw_loading_screen();

private:
	// Window information
	GLFWwindow* m_window;
	vec2 m_screen;

	// Information about the state
	bool m_world_valid = true;
	bool m_in_menu = false;

	// Game states
	World m_world;
	Menu* m_menu;
	Menu m_main_menu;
	Menu m_pause_menu;
	Menu m_load_menu;

	// Should end game
	bool m_is_over = false;
};