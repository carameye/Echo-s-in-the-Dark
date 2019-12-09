#pragma once

// internal
#include "common.hpp"
#include "maker_level.hpp"

// stlib
#include <vector>
#include <random>
#include <unordered_map>

#define SDL_MAIN_HANDLED
#include <SDL.h>

// Container for all our entities and game logic. Individual rendering / update is 
// deferred to the relative update() methods
class Maker
{
public:
	Maker();
	~Maker();

	// Creates a window, sets up events and begins the game
	bool init(GLFWwindow* window, vec2 screen);

	// Set load trigger
	void set_load_trigger(void (*l)());

	// Generates a large basic world
	void generate_starter();

	// Releases all associated resources
	void destroy();

	// Renders our scene
	void draw();

	// Update
	void update(float ms);

	// Should the game be over ?
	bool is_over() const;

	// Handle input
	bool handle_key_press(GLFWwindow*, int key, int action);
	void handle_mouse_move(GLFWwindow* window, double xpos, double ypos);
	void handle_mouse_click(GLFWwindow* window, int button, int action, int mods);

	// Save the level to save file
	void save();

	// Load level from save file
	void load();

	// Poll the state of the maker's input keys
	void poll_keys(GLFWwindow* window);

private:
	// Load screen event trigger
	void (*m_load)();

	// Window handle
	GLFWwindow* m_window;
	vec2 m_screen;
	float m_screen_scale; // Screen to pixel coordinates scale factor

	// Screen texture
	// The draw loop first renders to this texture, then it is used for the light shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;

	// Camera position
	vec2 camera_pos;

	// Mouse position
	vec2 mouse_pos;
	bool mouse_moved = false;
	bool m_left_click_hold = false;
	bool m_right_click_hold = false;

	// Camera movement direction
	bool movement[4] = { false, false, false, false };
	bool faster = false;

	// Maker level
	MakerLevel m_maker_level;

	// track the state of input keys
	std::unordered_map<int, int> input_key_states;
};
