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
#include "systems.hpp"
#include "background.hpp"
#include "torch.hpp"

#include <vector>

enum class ObjectType { brick, torch, door, ghost, robot };

class MakerLevel
{
public:
	// Renders level
	// projection is the 2D orthographic projection matrix
	void draw_entities(const mat3& projection, const vec2& camera_shift);

	// Releases all level-associated resources
	void destroy();

	// Generate a large basic level
	vec2 generate_starter();

	// Load previously made level
	vec2 load_level();

	// Handle input
	void handle_key_press(int key, int action);
	void handle_mouse_click(double xpos, double ypos, vec2 camera, bool left);
	void handle_mouse_move(float xpos, float ypos, vec2 camera_pos);

	// Refresh the object that is hovering under the mouse
	void refresh_hover_object(float x, float y);

	// Generate JSON and shadow image for current level
	void process();

private:
	// Spawn entities
	bool spawn_robot(vec2 position);
	bool spawn_brick(vec2 position, vec3 colour);
	bool spawn_door(vec2 position, std::string next_level);
	bool spawn_ghost(vec2 position, vec3 colour);
	bool spawn_torch(vec2 position);
	bool delete_object(vec2 position);

	bool valid_robot_position(vec2 position);
	bool valid_torch_position(vec2 position);

	std::string m_level;
	float width = 64.f * 40.f, height = 64.f * 40.f;

	ObjectType m_lot = ObjectType::brick;
	ObjectType m_ot = ObjectType::brick;
	int m_ot_selection = 0;
	vec3 m_color = { 1.f, 1.f, 1.f };
	bool m_hover_object_is_spawned = false;
	vec2 m_hover_object_position;
	vec2 m_robot_position;

	// Systems
	int min;
	RenderingSystem m_rendering_system;

	// Level entities
	bool permanent[40][40];
	Entity* slots[40][40];

	Robot m_robot;
	std::vector<Brick*> m_bricks;
	std::vector<Ghost*> m_ghosts;
	std::vector<Door*> m_interactables;
	std::vector<Torch*> m_torches;
};
