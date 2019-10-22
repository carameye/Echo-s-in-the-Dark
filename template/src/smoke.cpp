#include "smoke.hpp"

#include <math.h>

namespace
{
	const size_t FADE_OUT_MS = 500;
	const float MAX_SCALE = 3.f;
	const float MIN_SCALE = 1.5f;
	const size_t SIZE_MOD_MS = 200; // MS for one size modulation cycle
	const float SIZE_MOD_AMPLITUDE = 0.7f;
	const float VELOCITY_Y = 50.f;
	const float PI = 3.14159265f;
}

Texture Smoke::smoke_texture_large;
Texture Smoke::smoke_texture_small;

bool Smoke::init(int id)
{
	m_id = id;

	if (!smoke_texture_large.is_valid())
	{
		if (!smoke_texture_large.load_from_file(textures_path("smoke_large.png")))
		{
			fprintf(stderr, "Failed to load smoke texture large!");
			return false;
		}
	}
	if (!smoke_texture_small.is_valid())
	{
		if (!smoke_texture_small.load_from_file(textures_path("smoke_small.png")))
		{
			fprintf(stderr, "Failed to load smoke texture small");
			return false;
		}
	}
	if (rand() % 2 == 0) {
		rc.texture = &smoke_texture_large;
	} else {
		rc.texture = &smoke_texture_small;
	}

	float scale = MIN_SCALE + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(MAX_SCALE-MIN_SCALE)));
    m_original_scale = { scale, scale };
	rc.physics.scale = m_original_scale;

	mc.radians = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * PI)));

	if (!rc.init_sprite())
		return false;

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

    return true;
}

void Smoke::activate(vec2 robot_position, vec2 robot_velocity)
{
	mc.velocity.x = robot_velocity.x * -1.f / 3.f;
	mc.velocity.y = robot_velocity.y * -1.f / 2.f;
	if (mc.velocity.y < 0.f) {
		mc.velocity.y = VELOCITY_Y;
	}
	mc.position = { robot_position.x, robot_position.y + 25.f };
	m_alpha = 1.f;
}

void Smoke::update(float ms)
{
	float time_factor = ms / 1000.f;
	mc.position.x += mc.velocity.x * time_factor;
	mc.position.y += mc.velocity.y * time_factor;
	m_alpha -= ms / FADE_OUT_MS;
	m_size_mod_count += ms / SIZE_MOD_MS * PI;
	if (m_size_mod_count > PI) {
		m_size_mod_count = 0;
	}
	rc.physics.scale.x = m_original_scale.x + sin(m_size_mod_count) * SIZE_MOD_AMPLITUDE;
	rc.physics.scale.y = m_original_scale.y + sin(m_size_mod_count) * SIZE_MOD_AMPLITUDE;
}

bool Smoke::should_destroy() {
	return m_alpha < 0.f;
}