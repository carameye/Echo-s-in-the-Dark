#pragma once

#include "common.hpp"

class Menu
{
public:
	// Creates a window, sets up events and begins the game
	bool init(GLFWwindow* window, vec2 screen);

	// Releases all associated resources
	void destroy();

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over() const;

	// Handle input
	void handle_key_press(GLFWwindow*, int key, int, int action, int mod);
	void handle_mouse_move(GLFWwindow* window, double xpos, double ypos);

private:
	bool m_is_over = false;
};