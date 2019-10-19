#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "smoke_system.hpp"
#include "robot_head.hpp"
#include "robot_shoulders.hpp"

class Robot : public Entity
{
	static Texture robot_body_texture;
	static Texture robot_body_flying_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();
	
	// Update robot
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Update robots velocity based on its current movement direction and it's acceleration
	void update_velocity(float ms);

	// Renders the robot
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection, const vec2& camera_shift) override;

	// Returns the current robot position
	vec2 get_position() const;

	// Returns the current robot velocity
	vec2 get_velocity() const;

	// Returns the current robot acceleration
	vec2 get_acceleration() const;

	// Calculates and return next position with out updating robot
	vec2 get_next_position();

	// Sets the new robot position
	void set_position(vec2 position);

	// Sets the new robot velocity
	void set_velocity(vec2 velocity);

	// Sets the new robot acceleration
	void set_acceleration(vec2 acceleration);

	// Sets whether robot is accelerating right
	void set_is_accelerating_right(bool val);

	// Sets whether robot is accelerating left
    void set_is_accelerating_left(bool val);

	// Sets grounded to true
	void set_grounded();

	// Set position of head, for spawning only
    void set_head_position(vec2 position);

    // Set position of head, for spawning only
    void set_shoulder_position(vec2 position);

	// Returns the robots hitbox for collision detection
	Hitbox get_hitbox(vec2 translation) const;

	// Starts smoke system and changes to flying sprite
	void start_flying();

	// Stops smoke system and changes to normal sprite
	void stop_flying();

private:
	RobotHead m_head;
	RobotShoulders m_shoulders;
	SmokeSystem m_smoke_system;
	bool m_grounded = false;
	bool m_should_stop_smoke = false;
	bool m_is_accelerating_right = false;
	bool m_is_accelerating_left = false;
	bool m_is_accelerating_up = false;
};