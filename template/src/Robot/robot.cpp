// Header
#include "robot.hpp"

#include <cmath>

namespace {
    const float FLIGHT_ACCELERATION = 30.f;
    const float GRAVITY_ACCELERATION = 75.f;
    const float HORIZONTAL_ACCELERATION = 30.f;
    const float HORIZONTAL_DECELERATION = 90.f;
    const float MAX_HORIZONTAL_VELOCITY = 45.f;
    const float MAX_FLIGHT_VELOCITY = 45.f;
    const float MAX_FLIGHT_DURATION = 1500.f;
}

Texture Robot::robot_body_texture;
Texture Robot::robot_body_flying_texture;

bool Robot::init(int id, bool use_parts)
{
	m_id = id;

	if (!robot_body_texture.is_valid())
	{
		if (!robot_body_texture.load_from_file(textures_path("body_ball.png")))
		{
			fprintf(stderr, "Failed to load body texture!");
			return false;
		}
	}
	if (!robot_body_flying_texture.is_valid())
	{
		if (!robot_body_flying_texture.load_from_file(textures_path("body_ball_flying.png")))
		{
			fprintf(stderr, "Failed to load body flying texture!");
			return false;
		}
	}

	rc.texture = &robot_body_texture;

	if (!rc.init_sprite())
		return false;

    mc.position = { 0.f, 0.f };
    mc.velocity = { 0.f, 0.f };
    mc.acceleration = { 0.f , GRAVITY_ACCELERATION };
    mc.radians = 0.f;

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };

	if (!use_parts)
	{
		return true;
	}

	bool valid = m_shoulders.init(id + 1) && m_head.init(id + 2) && m_energy_bar.init(id + 3) && m_smoke_system.init(id + 4) && m_hat.init(id + 5);
	m_head.set_scaling(mc.physics.scale);
	m_shoulders.set_scaling(mc.physics.scale);
    m_energy_bar.set_scaling(mc.physics.scale);
    m_hat.set_scaling(mc.physics.scale);
    m_available_flight_time = 0;
    m_grounded = false;
	m_should_stop_smoke = true;
	m_is_accelerating_right = false;
	m_is_accelerating_left = false;
	m_is_flying = false;

	calculate_hitbox();

	return valid;
}

vec2 Robot::update_velocity(float ms) {

    float step = (ms / 1000);

    if (m_is_accelerating_right) {
        float new_velocity = mc.velocity.x + mc.acceleration.x * step;
        mc.velocity.x = new_velocity > MAX_HORIZONTAL_VELOCITY ? MAX_HORIZONTAL_VELOCITY : new_velocity;
    }
    if (m_is_accelerating_left) {
        float new_velocity = mc.velocity.x - mc.acceleration.x * step;
        mc.velocity.x = new_velocity < MAX_HORIZONTAL_VELOCITY * -1.f ? MAX_HORIZONTAL_VELOCITY * -1.f : new_velocity;
    }
    if (!m_is_accelerating_right && mc.velocity.x > 0) {
        float new_velocity = mc.velocity.x - mc.acceleration.x * step;
        mc.velocity.x = new_velocity < 0 ? 0 : new_velocity;
    }
    if (!m_is_accelerating_left && mc.velocity.x < 0) {
        float new_velocity = mc.velocity.x + mc.acceleration.x * step;
        mc.velocity.x = new_velocity > 0 ? 0 : new_velocity;
    }
    if (m_is_flying) {
        float new_velocity = mc.velocity.y - mc.acceleration.y * step;
        mc.velocity.y = new_velocity < MAX_FLIGHT_VELOCITY * -1.f ? MAX_FLIGHT_VELOCITY * -1.f : new_velocity;
    }
    if (!m_is_flying) {
        float new_velocity = mc.velocity.y + mc.acceleration.y * step;
        mc.velocity.y = new_velocity;
    }

    return {mc.velocity.x, mc.velocity.y};
}

void Robot::update(float ms)
{
	if (m_grounded) {
        m_available_flight_time = fmin(m_available_flight_time += (ms*2), MAX_FLIGHT_DURATION);
	    if (std::abs(mc.velocity.x) > TOLERANCE) {
            mc.radians += mc.velocity.x / 250.f;
        }
    }

	m_head.update(ms, add(mc.position, { 0.f, -48.f }));
    m_hat.update(ms, add(m_head.get_position(), { 0.f, -8.f }));
    m_shoulders.update(ms, add(mc.position, { 0.f, 0.f }));

    if (m_is_flying) {
        m_grounded = false;
        m_available_flight_time = (float)fmax(m_available_flight_time -= ms, 0);
        if (m_available_flight_time == 0) {
            stop_flying();
        }
    }
    m_energy_bar.update(ms, add(mc.position, { 0.f, -90.f }), (m_available_flight_time/MAX_FLIGHT_DURATION));

	if (mc.velocity.x != 0.f) {
        m_shoulders.set_direction(mc.velocity.x > 0.f);
    }

	m_smoke_system.update(ms, mc.position, mc.velocity);

	if (m_should_stop_smoke && mc.velocity.y >= 0) {
		m_smoke_system.stop_smoke();
	}

	m_energy_bar.set_status(MAX_FLIGHT_DURATION != m_available_flight_time);

}

vec2 Robot::get_position() const
{
	return mc.position;
}

vec2 Robot::get_velocity() const
{
	return mc.velocity;
}

vec2 Robot::get_acceleration() const
{
	return mc.acceleration;
}

vec2 Robot::get_next_position(float elapsed_ms)
{
    float step = elapsed_ms / 100.f;
    vec2 translation = {mc.velocity.x * step, mc.velocity.y * step};
    return {mc.position.x + translation.x, mc.position.y + translation.y};
}

bool Robot::is_grounded() const
{
    return m_grounded;
}

void Robot::set_position(vec2 position)
{
    vec2 translation;

    translation.x = position.x - mc.position.x;
    translation.y = position.y - mc.position.y;

	mc.position = position;

    m_hitbox.translate(translation);
}

void Robot::set_velocity(vec2 velocity)
{
	mc.velocity = velocity;
}

void Robot::set_acceleration(vec2 acceleration)
{
	mc.acceleration = acceleration;
}

void Robot::set_grounded()
{
	m_grounded = true;
}

void Robot::set_head_position(vec2 position)
{
	m_head.set_position(position);
	m_hat.set_position(position);
}

bool Robot::get_head_direction()
{
    return m_head.get_direction();
}

void Robot::set_shoulder_position(vec2 position)
{
    m_shoulders.set_position(position);
}


void Robot::set_energy_bar_position(vec2 position) {
    m_energy_bar.set_position(position);
}

Hitbox Robot::get_hitbox()
{
    return m_hitbox;
}

void Robot::calculate_hitbox()
{
    std::vector<Circle> circles(1);

    vec2 position = mc.position;

    int radius = (int)brick_size / 2;
    Circle circle(position, radius);
    circles[0] = circle;

    Hitbox hitbox(circles, {});
	m_hitbox = hitbox;
}

Hitbox Robot::get_head_hitbox()
{
    return m_head.get_hitbox();
}

void Robot::start_flying()
{
    if (m_available_flight_time == 0) {
        return;
    }
    m_is_flying = true;
    set_acceleration({mc.acceleration.x, FLIGHT_ACCELERATION});
	m_smoke_system.start_smoke();
	m_should_stop_smoke = false;
	rc.texture = &robot_body_flying_texture;
	mc.physics.scale.x *= 53.f / 45.f;
	mc.radians = 0.f;
    // start the rocket sound effect
    SoundSystem::get_system()->play_sound_effect(Sound_Effects::rocket, -1);
    // If we want made robot fall faster, reset vertical acceleration here.
}

void Robot::stop_flying()
{
    m_is_flying = false;
    set_acceleration({mc.acceleration.x, GRAVITY_ACCELERATION});
	// smoke will stop in update() when velocity.y is positive
	m_should_stop_smoke = true;
	rc.texture = &robot_body_texture;
	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };
    // stop the rocket sound effect
    SoundSystem::get_system()->stop_sound_effect(Sound_Effects::rocket, 1500);
	// If we want the robot to fall a bit faster, set vertical acceleration here. Positive number, make it a const
}

void Robot::set_is_accelerating_right(bool val) {
    m_is_accelerating_right = val;

    if (val) {
        set_acceleration({ HORIZONTAL_ACCELERATION , mc.acceleration.y });
    } else {
        set_acceleration({ HORIZONTAL_DECELERATION , mc.acceleration.y });
    }
}

void Robot::set_is_accelerating_left(bool val) {
    m_is_accelerating_left = val;

    if (val) {
        set_acceleration({ HORIZONTAL_ACCELERATION , mc.acceleration.y });
    } else {
        set_acceleration({ HORIZONTAL_DECELERATION , mc.acceleration.y });
    }

}

vec2 Robot::get_head_position() {
    return m_head.get_position();
}

vec2 Robot::get_next_head_position(vec2 next_body_position) {
	return m_head.get_next_position(add(next_body_position, { 0.f, -48.f }));
}

vec2 Robot::get_head_velocity() {
    return m_head.get_velocity();
}

void Robot::set_head_velocity(vec2 velocity) {
    m_head.set_velocity(velocity);
    m_hat.set_velocity(velocity);
}

void Robot::set_head_direction(bool b) {
    m_head.set_direction(b);
    m_hat.set_direction(b);
}

void Robot::destroy()
{
	m_smoke_system.destroy();
}


