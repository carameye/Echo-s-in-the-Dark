#pragma once

#include <brick.hpp>
#include "common.hpp"
#include "hitbox.hpp"
#include "smoke_system.hpp"
#include "robot_head.hpp"
#include "robot_shoulders.hpp"
#include "flight_energy_bar.hpp"
#include "sound_system.hpp"

class Robot : public Entity
{
	static Texture robot_body_texture;
	static Texture robot_body_flying_texture;

	RenderComponent rc;
	MotionComponent mc;

public:
	// Creates all the associated render resources and default transform
	bool init(int id, bool use_parts);
	
	// Update robot
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Update robots velocity based on its current movement direction and it's acceleration
	vec2 update_velocity(float ms);

	// Returns the current robot position
	vec2 get_position() const;

	// Returns the current robot velocity
	vec2 get_velocity() const;

	// Returns the current robot acceleration
	vec2 get_acceleration() const;

    // Calculates and return next position with out updating robot
    vec2 get_next_position(float elapsed_ms);

	bool is_grounded() const;

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

    bool get_head_direction();

    // Set position of head, for spawning only
    void set_shoulder_position(vec2 position);

    // Set position of energy bar, for spawning only
    void set_energy_bar_position(vec2 position);

	// Returns the robots hitbox for collision detection
	Hitbox get_hitbox();

    // Returns the robots head hitbox for collision detection
    Hitbox get_head_hitbox();

	// Starts smoke system and changes to flying sprite
	void start_flying();

	// Stops smoke system and changes to normal sprite
	void stop_flying();

    vec2 get_head_position();

    vec2 get_next_head_position(vec2 next_body_position);

    vec2 get_head_velocity();

    void set_head_velocity(vec2 velocity);

    void set_head_direction(bool b);

	void destroy();

private:
	RobotHead m_head;
	RobotShoulders m_shoulders;
	SmokeSystem m_smoke_system;
	FlightEnergyBar m_energy_bar;
    RobotHat m_hat;
    Hitbox m_hitbox;
	bool m_grounded;
	bool m_should_stop_smoke;
	bool m_is_accelerating_right;
	bool m_is_accelerating_left;
	bool m_is_flying;
	float m_available_flight_time;

    void calculate_hitbox();
};