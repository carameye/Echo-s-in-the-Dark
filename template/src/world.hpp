#pragma once

// internal
#include "common.hpp"
#include "level.hpp"

// stlib
#include <vector>
#include <random>
#include <unordered_map>

#define SDL_MAIN_HANDLED
#include <SDL.h>

// Container for all our entities and game logic. Individual rendering / update is 
// deferred to the relative update() methods
class World
{
public:
	World();
	~World();

	// Creates a window, sets up events and begins the game
	bool init(GLFWwindow *window, vec2 screen);

	// Set pause and load functions
	void set_pl_functions(void (*l)(), void (*e)(),  void (*s)());

	// Releases all associated resources
	void destroy();

	// Steps the game ahead by ms milliseconds
	void update(float ms);

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over() const;

	// Handle input
	bool handle_key_press(GLFWwindow* window, int key, int action);
	void handle_mouse_move(GLFWwindow* window, double xpos, double ypos);
    void handle_mouse_click(int button, int action);
    void handle_mouse_scroll(double yoffset);

	// Begin level
	void start_level(bool new_game);

	// Begin level made in echo maker
	bool start_maker_level();

	// Reset the level
	void reset();

	// Save the game to save file
	void save();

	// Poll the state of the world's input keys
	void poll_keys(GLFWwindow* window);

	// Gets the proper music to play
	Music get_background_music();

private:
	// Draw loading screen, parse level, set camera pos
	void load_level(std::string level);

	// Plays game introduction. Should only be called if player selects new game
    void play_intro();

	// Load state from save file
	void load();

	// Load menu access
	void (*m_load)();
	void (*m_exit)();
    void (*m_success)();

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
	float camera_offset;
	bool is_level_load_pan = false; // true while showing the path to the door on initial level load
	float on_load_delay = 0.f; // keeps the player looking at level goal for delay ms

	Level m_level;
	vec2 m_robot_ls_pos;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist; // default 0..1

	// Saved level data
	std::vector<std::string> m_unlocked;

	// used track the input states of keys
	std::unordered_map<int, int> key_input_states;
};
