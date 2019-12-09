#include "robot_head.hpp"

Texture RobotHead::robot_head_texture;

bool RobotHead::init(int id)
{
	m_id = id;

    if (!robot_head_texture.is_valid())
    {
        if (!robot_head_texture.load_from_file(textures_path("head.png")))
        {
            fprintf(stderr, "Failed to load head texture!");
            return false;
        }
    }

    rc.texture = &robot_head_texture;

    if (!rc.init_sprite())
        return false;

    mc.position = { 0.f, 0.f };
    mc.velocity = { 0.f, 0.f };
    mc.acceleration = { 0.f , 0.f };
    mc.radians = 0.f;

	mc.physics.scale = { 1.0f, 1.0f };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

	calculate_hitbox();

    return true;
}

void RobotHead::update(float ms, vec2 goal)
{
	if (m_face_right)
	{
		mc.physics.scale.x = abs(mc.physics.scale.x);
	}
	else
	{
		mc.physics.scale.x = -abs(mc.physics.scale.x);
	}
}

Hitbox RobotHead::get_hitbox()
{
    return m_hitbox;
}

vec2 RobotHead::get_position() const
{
    return mc.position;
}

void RobotHead::set_position(vec2 position)
{
    vec2 translation;

    translation.x = position.x - mc.position.x;
    translation.y = position.y - mc.position.y;

    mc.position = position;

    m_hitbox.translate(translation);
}

void RobotHead::set_scaling(vec2 scaling)
{
	mc.physics.scale = scaling;
}

void RobotHead::set_direction(bool right)
{
    m_face_right = right;
}

bool RobotHead::get_direction()
{
    return m_face_right;
}

vec2 RobotHead::get_next_position(vec2 goal) {
	vec2 dist = sub(goal, mc.position);
    return add(get_position(), dist);
}

vec2 RobotHead::get_velocity() {
    return mc.velocity;
}

void RobotHead::set_velocity(vec2 velocity) {
    mc.velocity = velocity;
}

void RobotHead::calculate_hitbox() {
    std::vector<Circle> circles(1);

    vec2 position = mc.position;

    int radius = rc.texture->height/2;
    Circle circle(position, radius);
    circles[0] = circle;

    Hitbox hitbox(circles, {});
    m_hitbox = hitbox;
}
