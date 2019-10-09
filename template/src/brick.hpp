#pragma once

#include "common.hpp"
#include "hitbox.hpp"

class Brick : public Entity
{
	static Texture brick_texture;

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
};