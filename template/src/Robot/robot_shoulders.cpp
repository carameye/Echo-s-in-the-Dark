#include "robot_shoulders.hpp"

Texture RobotShoulders::robot_shoulder_texture;

bool RobotShoulders::init(int id)
{
	m_id = id;

    if (!robot_shoulder_texture.is_valid())
    {
        if (!robot_shoulder_texture.load_from_file(textures_path("body_shoulder.png")))
        {
            fprintf(stderr, "Failed to load shoulder texture!");
            return false;
        }
    }

	rc.texture = &robot_shoulder_texture;

    if (!rc.init_sprite())
        return false;

    mc.position = { 0.f, 0.f };
    mc.velocity = { 0.f, 0.f };
    mc.acceleration = { 0.f , 0.f };
    mc.radians = 0.f;

	mc.physics.scale = { 1.0f, 1.0f };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

    return true;
}

void RobotShoulders::update(float ms, vec2 goal)
{
    vec2 dist = sub(goal, mc.position);
	if (m_face_right)
	{
		mc.physics.scale.x = abs(mc.physics.scale.x);
	}
	else
	{
		mc.physics.scale.x = -abs(mc.physics.scale.x);
	}
    set_position(add(get_position(), { dist.x,  dist.y }));
}

vec2 RobotShoulders::get_position() const
{
    return mc.position;
}

void RobotShoulders::set_position(vec2 position)
{
    mc.position = position;
}

void RobotShoulders::set_scaling(vec2 scaling)
{
    mc.physics.scale = scaling;
}

void RobotShoulders::set_direction(bool right)
{
    m_face_right = right;
}
