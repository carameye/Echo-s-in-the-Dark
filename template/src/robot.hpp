#pragma once

#include "common.hpp"
#include "hitbox.hpp"

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

	// Returns the current robot velocity
	vec2 get_velocity() const;

	// Returns the current robot acceleration
	vec2 get_acceleration() const;

	// Sets the new robot position
	void set_position(vec2 position);

	// Sets the new robot velocity
	void set_velocity(vec2 velocity);

	// Sets the new robot acceleration
	void set_acceleration(vec2 acceleration);

	// Returns the robots hitbox for collision detection
	Hitbox get_hitbox(vec2 translation) const;
};