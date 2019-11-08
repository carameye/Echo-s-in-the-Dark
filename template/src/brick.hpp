#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "components.hpp"

class Brick : public Entity
{
	static Texture brick_texture;

	RenderComponent rc;
	MotionComponent mc;

public:
	// Creates all the associated render resources and default transform
	bool init(int id);

	// Update brick
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(vec3 hl_colour);

	// Returns the current brick position
	vec2 get_position()const;

	// Sets the new turtle position
	void set_position(vec2 position);

	// Returns the bricks hitbox for collision detection
	Hitbox get_hitbox() const;

    void set_colour(vec3 colour);

private:
    vec3 m_colour;
    bool m_is_visible;
};