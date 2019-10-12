#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "level_graph.hpp"

class Ghost : public Entity
{
	static Texture s_ghost_texture;
	vec2 m_goal;
	std::vector<vec2> m_path;
	LevelGraph* m_level_graph;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();

	// Update brick
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the brick
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection, const vec2& camera_shift) override;

	// Returns the current brick position
	vec2 get_position()const;

	// Sets the new turtle position
	void set_position(vec2 position);

	// Returns the bricks hitbox for collision detection
	Hitbox get_hitbox() const;

	// Tell the ghost where it wants to go
	void set_goal(vec2 position);

	// Tell the ghost how to navigate the map
	void set_level_graph(LevelGraph* graph);
};