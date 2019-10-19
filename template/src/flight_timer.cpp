#include "flight_timer.hpp";


namespace
{
    const float MAX_FLIGHT_DURATION = 5000.f;
}

Texture FlightTimer::energy_full;
Texture FlightTimer::energy_green;
Texture FlightTimer::energy_yellow;
Texture FlightTimer::energy_orange;
Texture FlightTimer::energy_red;

bool FlightTimer::init(vec2 robot_position, vec2 robot_velocity)
{
    if (!energy_full.is_valid())
    {
        if (!energy_full.load_from_file(textures_path("full_energy.png")))
        {
            fprintf(stderr, "Failed to load full flight timer!");
            return false;
        }
    }
    if (!energy_green.is_valid())
    {
        if (!energy_green.load_from_file(textures_path("green_energy.png")))
        {
            fprintf(stderr, "Failed to load green flight timer");
            return false;
        }
    }
    if (!energy_yellow.is_valid())
    {
        if (!energy_yellow.load_from_file(textures_path("yellow_energy.png")))
        {
            fprintf(stderr, "Failed to load yellow flight timer!");
            return false;
        }
    }
    if (!energy_orange.is_valid())
    {
        if (!energy_orange.load_from_file(textures_path("orange_energy.png")))
        {
            fprintf(stderr, "Failed to load orange flight timer");
            return false;
        }
    }
    if (!energy_red.is_valid())
    {
        if (!energy_red.load_from_file(textures_path("red_energy.png")))
        {
            fprintf(stderr, "Failed to load red flight timer");
            return false;
        }
    }

    motion.velocity.x = robot_velocity.x;
    motion.velocity.y = robot_velocity.y;
    motion.position = { robot_position.x, robot_position.y + 70.f };

}

void FlightTimer::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteBuffers(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void FlightTimer::update(float ms)
{
    float time_factor = ms / 1000.f;
    motion.position.x += motion.velocity.x * time_factor;
    motion.position.y += motion.velocity.y * time_factor;
}

void FlightTimer::draw(const mat3& projection, const vec2& camera_shift)
{
    transform.begin();
    transform.translate(camera_shift);
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    transform.end();

    draw_sprite_alpha(projection);
}