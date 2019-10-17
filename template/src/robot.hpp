#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "smoke_system.hpp"

class RobotHead : public Entity
{
	static Texture robot_head_texture;
	bool m_face_right = true;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();

	// Update robot
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms, vec2 goal);

	// Renders the robot
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection, const vec2& camera_shift) override;

	// Returns the current robot position
	vec2 get_position() const;

	// Sets the new robot position
	void set_position(vec2 position);

	// Sets the scaling
	void set_scaling(vec2 scaling);

	// Set the look direction
	void set_direction(bool right);
};

class RobotShoulders : public Entity
{
    static Texture robot_shoulder_texture;
    bool m_face_right = true;

public:
    // Creates all the associated render resources and default transform
    bool init();

    // Releases all the associated resources
    void destroy();

    // Update robot
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms, vec2 goal);

    // Renders the robot
    // projection is the 2D orthographic projection matrix
    void draw(const mat3& projection, const vec2& camera_shift) override;

    // Returns the current robot position
    vec2 get_position() const;

    // Sets the new robot position
    void set_position(vec2 position);

    // Sets the scaling
    void set_scaling(vec2 scaling);

    // Set the look direction
    void set_direction(bool right);
};

class Robot : public Entity
{
	static Texture robot_body_texture;

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
};