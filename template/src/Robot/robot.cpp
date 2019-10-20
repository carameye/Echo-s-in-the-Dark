// Header
#include "robot.hpp"

#include <cmath>
#include <ntdef.h>

namespace {
    const float VERTICAL_ACCELERATION = 20.f;
    const float HORIZONTAL_ACCELERATION = 20.f;
    const float HORIZONTAL_DECELERATION = 30.f;
    const float MAX_HORIZONTAL_VELOCITY = 20.f;
    const float MAX_VERTICAL_VELOCITY = 30.f;
    const float MAX_FLIGHT_DURATION = 4000.f;
}

Texture Robot::robot_body_texture;
Texture Robot::robot_body_flying_texture;

bool Robot::init()
{
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

	texture = &robot_body_texture;

	if (!init_sprite())
		return false;

    motion.position = { 0.f, 0.f };
    motion.velocity = { 0.f, 0.f };
    motion.acceleration = { 0.f , VERTICAL_ACCELERATION };
    motion.radians = 0.f;

	physics.scale = { brick_size / texture->width, brick_size / texture->height };
	bool valid = m_head.init() && m_shoulders.init() && m_smoke_system.init() && m_energy_bar.init() && m_energy_bar.init();
	m_head.set_scaling(physics.scale);
	m_shoulders.set_scaling(physics.scale);
    m_energy_bar.set_scaling(physics.scale);

    m_available_flight_time = MAX_FLIGHT_DURATION;

	return valid;
}

// Releases all graphics resources
void Robot::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);

	m_shoulders.destroy();
	m_head.destroy();
	m_smoke_system.destroy();
}

void Robot::update_velocity(float ms) {

    float step = (ms / 1000);

    if (m_is_accelerating_right) {
        float new_velocity = motion.velocity.x + motion.acceleration.x * step;
        motion.velocity.x = new_velocity > MAX_HORIZONTAL_VELOCITY ? MAX_HORIZONTAL_VELOCITY : new_velocity;
    }
    if (m_is_accelerating_left) {
        float new_velocity = motion.velocity.x - motion.acceleration.x * step;
        motion.velocity.x = new_velocity < MAX_HORIZONTAL_VELOCITY * -1.f ? MAX_HORIZONTAL_VELOCITY * -1.f : new_velocity;
    }
    if (!m_is_accelerating_right && motion.velocity.x > 0) {
        float new_velocity = motion.velocity.x - motion.acceleration.x * step;
        motion.velocity.x = new_velocity < 0 ? 0 : new_velocity;
    }
    if (!m_is_accelerating_left && motion.velocity.x < 0) {
        float new_velocity = motion.velocity.x + motion.acceleration.x * step;
        motion.velocity.x = new_velocity > 0 ? 0 : new_velocity;
    }
    if (m_is_flying) {
        float new_velocity = motion.velocity.y - motion.acceleration.y * step;
        motion.velocity.y = new_velocity < MAX_VERTICAL_VELOCITY * -1.f ? MAX_VERTICAL_VELOCITY * -1.f : new_velocity;
    }
    if (!m_is_flying) {
        float new_velocity = motion.velocity.y + motion.acceleration.y * step;
        motion.velocity.y = new_velocity > MAX_VERTICAL_VELOCITY ? MAX_VERTICAL_VELOCITY : new_velocity;
    }
}

void Robot::update(float ms)
    {
	if (m_grounded) {
        m_available_flight_time = fmin(m_available_flight_time += (ms/2), MAX_FLIGHT_DURATION);
	    if (std::abs(motion.velocity.x) > TOLERANCE) {
            motion.radians += motion.velocity.x / 50;
        }
    }

	m_grounded = false;
	m_head.update(ms, add(motion.position, { 0.f, -48.f }));
    m_shoulders.update(ms, add(motion.position, { 0.f, 0.f }));


    if (m_is_flying) {
        m_available_flight_time = fmax(m_available_flight_time -= ms, 0);
        if (m_available_flight_time == 0) {
            stop_flying();
        }
    }
    m_energy_bar.update(ms, add(motion.position, { 0.f, -90.f }), (m_available_flight_time/MAX_FLIGHT_DURATION));

	if (motion.velocity.x != 0.f) {
        m_head.set_direction(motion.velocity.x > 0.f);
        m_shoulders.set_direction(motion.velocity.x > 0.f);
    }

	m_smoke_system.update(ms, motion.position, motion.velocity);

	if (m_should_stop_smoke && motion.velocity.y >= 0) {
		m_smoke_system.stop_smoke();
	}
}

void Robot::draw(const mat3& projection, const vec2& camera_shift)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform.begin();
	transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.rotate(motion.radians);
	transform.scale(physics.scale);
	transform.end();

	draw_sprite(projection);

    m_shoulders.draw(projection, camera_shift);
	m_head.draw(projection, camera_shift);
	if (m_available_flight_time != MAX_FLIGHT_DURATION) {
        m_energy_bar.draw(projection, camera_shift);
	}
	m_smoke_system.draw(projection, camera_shift);
}

vec2 Robot::get_position() const
{
	return motion.position;
}

vec2 Robot::get_velocity() const
{
	return motion.velocity;
}

vec2 Robot::get_acceleration() const
{
	return motion.acceleration;
}

vec2 Robot::get_next_position()
{
    return {motion.position.x + motion.velocity.x, motion.position.y + motion.velocity.y};
}

void Robot::set_position(vec2 position)
{
	motion.position = position;
}

void Robot::set_velocity(vec2 velocity)
{
	motion.velocity = velocity;
}

void Robot::set_acceleration(vec2 acceleration)
{
	motion.acceleration = acceleration;
}

void Robot::set_grounded()
{
	m_grounded = true;
}

void Robot::set_head_position(vec2 position)
{
	m_head.set_position(position);
}

void Robot::set_shoulder_position(vec2 position)
{
    m_shoulders.set_position(position);
}


void Robot::set_energy_bar_position(vec2 position) {
    m_energy_bar.set_position(position);
}

Hitbox Robot::get_hitbox(vec2 translation) const
{
	std::vector<Circle> circles(1);

	vec2 position = motion.position;

	position.x += translation.x;
	position.y += translation.y;

	int radius = brick_size / 2;
	Circle circle(position, radius);
	circles[0] = circle;

	Hitbox hitbox(circles, {});
	// TODO: figure out why this line is not working
	// hitbox.translate(translation);
	return hitbox;
}

void Robot::start_flying()
{
    m_is_flying = true;
	m_smoke_system.start_smoke();
	m_should_stop_smoke = false;
	texture = &robot_body_flying_texture;
	physics.scale.x *= 53.f / 45.f;
	motion.radians = 0.f;
    // If we want made robot fall faster, reset vertical acceleration here.
}

void Robot::stop_flying()
{
    m_is_flying = false;
	// smoke will stop in update() when velocity.y is positive
	m_should_stop_smoke = true;
	texture = &robot_body_texture;
	physics.scale = { brick_size / texture->width, brick_size / texture->height };
	// If we want the robot to fall a bit faster, set vertical acceleration here. Positive number, make it a const
}

void Robot::set_is_accelerating_right(bool val) {
    m_is_accelerating_right = val;

    if (val) {
        set_acceleration({ HORIZONTAL_ACCELERATION , motion.acceleration.y });
    } else {
        set_acceleration({ HORIZONTAL_DECELERATION , motion.acceleration.y });
    }
}

void Robot::set_is_accelerating_left(bool val) {
    m_is_accelerating_left = val;

    if (val) {
        set_acceleration({ HORIZONTAL_ACCELERATION , motion.acceleration.y });
    } else {
        set_acceleration({ HORIZONTAL_DECELERATION , motion.acceleration.y });
    }

}
