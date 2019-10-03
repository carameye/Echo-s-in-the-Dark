#pragma once

#include "common.hpp"

// Salmon food
class Robot : public Entity
{
	static Texture robot_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();
	
	// Update robot
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the robot
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection, const vec2& camera_shift) override;

	// Returns the current robot position
	vec2 get_position() const;

	// Sets the new robot position
	void set_position(vec2 position);

	// Returns the robots bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;
};