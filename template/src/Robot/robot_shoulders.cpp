#include "robot_shoulders.hpp"

Texture RobotShoulders::robot_shoulder_texture;

bool RobotShoulders::init()
{
    if (!robot_shoulder_texture.is_valid())
    {
        if (!robot_shoulder_texture.load_from_file(textures_path("body_shoulder.png")))
        {
            fprintf(stderr, "Failed to load shoulder texture!");
            return false;
        }
    }

    texture = &robot_shoulder_texture;

    if (!init_sprite())
        return false;

    motion.position = { 0.f, 0.f };
    motion.velocity = { 0.f, 0.f };
    motion.acceleration = { 0.f , 0.f };
    motion.radians = 0.f;

    physics.scale = { 1.0f, 1.0f };

    return true;
}

void RobotShoulders::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteBuffers(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void RobotShoulders::update(float ms, vec2 goal)
{
    vec2 dist = sub(goal, motion.position);
    set_position(add(get_position(), { dist.x,  dist.y }));
}

void RobotShoulders::draw(const mat3& projection, const vec2& camera_shift)
{
    // Transformation code, see Rendering and Transformation in the template specification for more info
// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
    transform.begin();
    transform.translate(camera_shift);
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    if (!m_face_right)
        transform.scale({ -1.f, 1.f });
    transform.end();

    draw_sprite(projection);
}

vec2 RobotShoulders::get_position() const
{
    return motion.position;
}

void RobotShoulders::set_position(vec2 position)
{
    motion.position = position;
}

void RobotShoulders::set_scaling(vec2 scaling)
{
    physics.scale = scaling;
}

void RobotShoulders::set_direction(bool right)
{
    m_face_right = right;
}
