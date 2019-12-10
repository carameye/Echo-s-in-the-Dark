#pragma once

#include "common.hpp"
#include "brick.hpp"
#include "Robot/robot.hpp"
#include "ghost.hpp"
#include "level_graph.hpp"
#include "Interactables/door.hpp"
#include "light.hpp"
#include "sign.hpp"
#include "json.hpp"
#include <vector>
#include <unordered_map>
#include "systems.hpp"
#include "background.hpp"
#include "torch.hpp"
#include "sound_system.hpp"

class Level
{
    public:
    // Renders level
    // projection is the 2D orthographic projection matrix
	void draw_entities(const mat3& projection, const vec2& camera_shift);
    void draw_light(const mat3& projection, const vec2& camera_shift);

    // Releases all level-associated resources
	void destroy();

    // Update level entites, returns name of audio file to be played if something happens on update
    void update(float elapsed_ms);

	// Updates the background
	void update_background(float elapsed_ms, vec2 pos_diff);

    // Gets the camera position when the level first starts
    vec2 get_starting_camera_position() const;

	// Gets the position of the player
	vec2 get_player_position() const;

    // Interact with the current level interactable
    std::string interact();

	// Resets the level
	void reset_level();

	// Generate a level from a text file
	bool parse_level(std::string level, std::vector<std::string> unlocked, vec2 start_pos);

	// Handle input
	std::string handle_key_press(int key, int action, std::unordered_map<int, int> &input_states);
	void handle_mouse_move(double xpos, double ypos, vec2 camera);
	void handle_mouse_click(int button, int action);

	// Get the name of the current level
	std::string get_current_level();

    void handle_mouse_scroll(double yoffset);

	// Get the proper music to play for the level
	Music get_level_music();

	vec2 get_size();

private:
    // Spawn entities
	bool spawn_robot(vec2 position);
	bool spawn_brick(vec2 position, vec3 colour);
	bool spawn_door(vec2 position, std::string next_level);
	bool spawn_ghost(vec2 position, vec3 colour);
	bool spawn_sign(vec2 position, std::string text);
	bool spawn_background();
	bool spawn_torch(vec2 position);

	// get the closest ghost to the robot
	float get_min_ghost_distance();
	Music prev_bgm = Music::standard;

	// returns the square of bricks around and at pos. Used for collision checking
	std::vector<vec2> get_brick_positions_around_pos(vec2 pos) const;

	// For resetting the level
	void save_level();

	std::string m_level;
	float width, height;

	// Systems
	RenderingSystem m_rendering_system;

	// Light effect
	Light m_light;

	// Data structure for unordered_map, using vec2 as key
	struct vec2Hash {
		std::size_t operator()(const vec2& v) const
		{
			return std::hash<std::string>()(std::to_string(v.x) + "," + std::to_string(v.y));
		}
	};

    // Level entities
    Robot m_robot;
	std::unordered_map<vec2, Brick*, vec2Hash> m_brick_map;
	std::vector<Ghost*> m_ghosts;
    std::vector<Door*> m_interactables;
	std::vector<Sign*> m_signs;
	std::vector<Background*> m_backgrounds;
	std::vector<Torch*> m_torches;

	vec2 m_starting_camera_pos;

    LevelGraph* m_graph;
    LevelGraph m_white_graph;
    LevelGraph m_red_graph;
    LevelGraph m_green_graph;
    LevelGraph m_blue_graph;
    Door* m_interactable;

    bool m_has_colour_changed = true;

    std::vector<vec2> reset_positions;

	double m_scroll_amount = 0;
	bool m_scroll_down = false;
};
