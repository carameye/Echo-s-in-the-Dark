#include "flight_energy_bar.hpp"


namespace
{
    const float MAX_FLIGHT_DURATION = 5000.f;
}

Texture FlightEnergyBar::fuel_full;
Texture FlightEnergyBar::fuel_4;
Texture FlightEnergyBar::fuel_3;
Texture FlightEnergyBar::fuel_2;
Texture FlightEnergyBar::fuel_empty;

bool FlightEnergyBar::init()
{
    if (!fuel_full.is_valid())
    {
        if (!fuel_full.load_from_file(textures_path("fuel_5.png")))
        {
            fprintf(stderr, "Failed to load full flight timer!");
            return false;
        }
    }
    if (!fuel_4.is_valid())
    {
        if (!fuel_4.load_from_file(textures_path("fuel_4.png")))
        {
            fprintf(stderr, "Failed to load green flight timer");
            return false;
        }
    }
    if (!fuel_3.is_valid())
    {
        if (!fuel_3.load_from_file(textures_path("fuel_3.png")))
        {
            fprintf(stderr, "Failed to load yellow flight timer!");
            return false;
        }
    }
    if (!fuel_2.is_valid())
    {
        if (!fuel_2.load_from_file(textures_path("fuel_2.png")))
        {
            fprintf(stderr, "Failed to load orange flight timer");
            return false;
        }
    }
    if (!fuel_empty.is_valid())
    {
        if (!fuel_empty.load_from_file(textures_path("fuel_1.png")))
        {
            fprintf(stderr, "Failed to load empty flight timer");
            return false;
        }
    }

    motion.position = { 0.f, 0.f };
    motion.velocity = { 0.f, 0.f };
    motion.acceleration = { 0.f , 0.f };
    motion.radians = 0.f;

    physics.scale = { 1.0f, 1.0f };

    texture = &fuel_full;

    if (!init_sprite()) {
        return false;
    }

    return true;
}

void FlightEnergyBar::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteBuffers(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void FlightEnergyBar::update(float ms, vec2 goal, float percent)
{
    if (percent > .8) {
        texture = &fuel_full;
    } else if (percent > .6) {
        texture = &fuel_4;
    } else if (percent > .4) {
        texture = &fuel_3;
    } else if (percent > .2) {
        texture = &fuel_2;
    } else {
        texture = &fuel_empty;
    }
    vec2 dist = sub(goal, motion.position);
    set_position(add(get_position(), { dist.x,  dist.y }));
}

void FlightEnergyBar::draw(const mat3& projection, const vec2& camera_shift)
{
    transform.begin();
    transform.translate(camera_shift);
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    transform.end();

    draw_sprite(projection);
}

vec2 FlightEnergyBar::get_position() const
{
    return motion.position;
}

void FlightEnergyBar::set_position(vec2 position)
{
    motion.position = position;
}

void FlightEnergyBar::set_scaling(vec2 scaling)
{
    physics.scale = scaling;
}