#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "level_graph.hpp"
#include "components.hpp"

class Ghost : public Entity
{
	static Texture s_ghost_texture;
	vec2 m_goal;
	std::vector<vec2> m_path;
	LevelGraph* m_level_graph;

	RenderComponent rc;
	MotionComponent mc;

public:
	// Creates all the associated render resources and default transform
	bool init(int id, vec3 colour, vec3 headlight_colour);

	// Updates the ghost
	void update(float ms);

	// Returns the current brick position
	vec2 get_position()const;

	// Sets the new ghost position
	void set_position(vec2 position);

	// Get the colour of the ghost
	vec3 get_colour();

	// Returns the bricks hitbox for collision detection
	Hitbox get_hitbox() const;

	// Tell the ghost where it wants to go
	void set_goal(vec2 position);

	// Tell the ghost how to navigate the map
	void set_level_graph(LevelGraph* graph);

	// Update whether ghost is currently visible
	void update_is_chasing(vec3 headlight_color);

	// Set path
	void set_path();

	float dist_from_goal();

private:
    vec3 m_colour;
    bool m_is_chasing;
    Hitbox m_hitbox;

    void calculate_hitbox();
	bool colour_is_white(vec3 colour);
};
