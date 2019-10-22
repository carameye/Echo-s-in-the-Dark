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

	rc.physics.scale = { 1.0f, 1.0f };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

    return true;
}

void RobotHead::update(float ms, vec2 goal)
{
    vec2 dist = sub(goal, mc.position);
	if (m_face_right)
	{
		rc.physics.scale.x = abs(rc.physics.scale.x);
	}
	else
	{
		rc.physics.scale.x = -abs(rc.physics.scale.x);
	}
    set_position(add(get_position(), { 0.65f * dist.x, 0.65f * dist.y }));
}

vec2 RobotHead::get_position() const
{
    return mc.position;
}

void RobotHead::set_position(vec2 position)
{
    mc.position = position;
}

void RobotHead::set_scaling(vec2 scaling)
{
	rc.physics.scale = scaling;
}

void RobotHead::set_direction(bool right)
{
    m_face_right = right;
}

bool RobotHead::get_direction()
{
    return m_face_right;
}