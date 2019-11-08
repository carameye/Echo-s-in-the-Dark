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

bool FlightEnergyBar::init(int id)
{
	m_id = id;

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

    mc.position = { 0.f, 0.f };
    mc.velocity = { 0.f, 0.f };
    mc.acceleration = { 0.f , 0.f };
    mc.radians = 0.f;

	mc.physics.scale = { 1.0f, 1.0f };

	rc.texture = &fuel_full;

    if (!rc.init_sprite()) {
        return false;
    }

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

    return true;
}

void FlightEnergyBar::update(float ms, vec2 goal, float percent)
{
    if (percent > .8) {
		rc.texture = &fuel_full;
    } else if (percent > .6) {
		rc.texture = &fuel_4;
    } else if (percent > .4) {
		rc.texture = &fuel_3;
    } else if (percent > .2) {
		rc.texture = &fuel_2;
    } else {
		rc.texture = &fuel_empty;
    }
    vec2 dist = sub(goal, mc.position);
    set_position(add(get_position(), { dist.x,  dist.y }));
}

vec2 FlightEnergyBar::get_position() const
{
    return mc.position;
}

void FlightEnergyBar::set_position(vec2 position)
{
    mc.position = position;
}

void FlightEnergyBar::set_scaling(vec2 scaling)
{
	mc.physics.scale = scaling;
}

void FlightEnergyBar::set_status(bool visible)
{
	rc.render = visible;
}
