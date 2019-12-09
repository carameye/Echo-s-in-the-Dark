#pragma once

#include "menuentity.hpp"
#include "systems.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <vector>
#include <utility>

class Menu
{
public:
	// Creates a window, sets up events and begins the game
	bool init(GLFWwindow* window, vec2 screen);

	// Setup entities
	bool setup(std::vector<std::tuple<std::string, Status, vec2>> buttons);

	// Releases all associated resources
	void destroy();

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over() const;

	// Handle input
	bool handle_key_press(GLFWwindow*, int key, int, int action, int mod);
	void handle_mouse_move(GLFWwindow* window, double xpos, double ypos);
	Status handle_mouse_button(int button, int action);

private:
	// Window handle
	GLFWwindow* m_window;
	float m_screen_scale;
	GLuint m_frame_buffer;
	Texture m_screen_tex;

	// Menu entities
	std::vector<Button*> m_entities;

	// Rendering system
	RenderingSystem m_rs;

	// State
	bool m_is_over = false;
};