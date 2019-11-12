#include "robot_head.hpp"

Texture RobotHat::robot_hat_texture;

bool RobotHat::init(int id)
{
    m_id = id;

    if (!robot_hat_texture.is_valid())
    {
        if (!robot_hat_texture.load_from_file(textures_path("hat.png")))
        {
            fprintf(stderr, "Failed to load hat texture!");
            return false;
        }
    }

    rc.texture = &robot_hat_texture;

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

void RobotHat::update(float ms, vec2 goal)
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
    set_position(add(get_position(), dist));
}

vec2 RobotHat::get_position() const
{
    return mc.position;
}

void RobotHat::set_position(vec2 position)
{
    mc.position = position;
}

void RobotHat::set_scaling(vec2 scaling)
{
    mc.physics.scale = scaling;
}

void RobotHat::set_direction(bool right)
{
    m_face_right = right;
}

bool RobotHat::get_direction()
{
    return m_face_right;
}

void RobotHat::set_velocity(vec2 velocity) {
    mc.velocity = velocity;
}
