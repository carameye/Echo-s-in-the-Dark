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
#include "systems.hpp"

class Level
{
    public:
    // Creates all the associated render resources and default transform
	bool init(std::string level);

    // Renders level
    // projection is the 2D orthographic projection matrix
	void draw_entities(const mat3& projection, const vec2& camera_shift);

    void draw_light(const mat3& projection, const vec2& camera_shift);

    // Releases all level-associated resources
	void destroy();

    // Update level entites
    void update(float elapsed_ms);

    // Gets the camera position
    vec2 get_camera_position();

    // Returns the player character
    Robot* get_player();

    // Interact with the current level interactable
    void interact();

    // Light effect
	Light m_light;

	Light* get_light();

private:
    // Generate a level from a text file
	bool parse_level(std::string level);

    // Spawn entities
	bool spawn_robot(vec2 position);
	bool spawn_brick(vec2 position, vec3 colour);
	bool spawn_door(vec2 position, std::string next_level);
	bool spawn_ghost(vec2 position);
	bool spawn_sign(vec2 position, std::string text);

	// For resetting the level
	void save_level();
	void reset_level();

private:
	float width, height;

	// Systems
	RenderingSystem m_rendering_system;

    // Level entities
    Robot m_robot;
    std::vector<Brick*> m_bricks;
	std::vector<Ghost*> m_ghosts;
    std::vector<Door*> m_interactables;
	std::vector<Sign*> m_signs;

    LevelGraph m_graph;
    Door* m_interactable;

    std::vector<vec2> reset_positions;
};
