#pragma once

// internal
#include "common.hpp"
#include "brick.hpp"
#include "Robot/robot.hpp"
#include "light.hpp"
#include "ghost.hpp"
#include "level_graph.hpp"
#include "door.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is 
// deferred to the relative update() methods
class World
{
public:
	World();
	~World();

	// Creates a window, sets up events and begins the game
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	// Steps the game ahead by ms milliseconds
	bool update(float ms);

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over()const;

	// Generate a level from a text file
	bool parse_level(std::string level);

private:
	// !!! INPUT CALLBACK FUNCTIONS
	void on_key(GLFWwindow*, int key, int, int action, int mod);
	void on_mouse_move(GLFWwindow* window, double xpos, double ypos);

	// Spawn objects
	bool spawn_robot(vec2 position);
	bool spawn_brick(vec2 position, vec3 colour);
	bool spawn_door(vec2 position, std::string next_level);
	bool spawn_ghost(vec2 position);
	bool spawn_sign(vec2 position, std::string text);

private:
	// Window handle
	GLFWwindow* m_window;
	float m_screen_scale; // Screen to pixel coordinates scale factor

	// Screen texture
	// The draw loop first renders to this texture, then it is used for the water shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;

	// Light effect
	Light m_light;

	// Camera position
	vec2 camera_pos;
	float camera_offset;

	// Game entities
	std::vector<Brick> m_bricks;
	std::vector<Ghost> m_ghosts;
	std::vector<Door> m_doors;
	Robot m_robot;

	LevelGraph m_graph;
	Door* m_interactable_door;
	
	Mix_Music* m_background_music;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist; // default 0..1
};
