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

RenderComponent Smoke::rc_large;
RenderComponent Smoke::rc_small;

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

		rc_large.texture = &smoke_texture_large;

		if (!rc_large.init_sprite())
			return false;

		rc_large.render = true;
	}
	if (!smoke_texture_small.is_valid())
	{
		if (!smoke_texture_small.load_from_file(textures_path("smoke_small.png")))
		{
			fprintf(stderr, "Failed to load smoke texture small");
			return false;
		}

		rc_small.texture = &smoke_texture_small;

		if (!rc_small.init_sprite())
			return false;

		rc_small.render = true;
	}

	float scale = MIN_SCALE + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MAX_SCALE - MIN_SCALE)));
	m_original_scale = { scale, scale };
	mc.radians = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * PI)));
	mc.physics.scale = m_original_scale;

	if (rand() % 2 == 0) 
	{
		s_render_components[id] = &rc_large;
	}
	else 
	{
		s_render_components[id] = &rc_small;
	}
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
	if (m_alpha < 0.f)
	{
		mc.position = { -1000.f, -1000.f };
	}
	m_size_mod_count += ms / SIZE_MOD_MS * PI;
	if (m_size_mod_count > PI) {
		m_size_mod_count = 0;
	}
	mc.physics.scale.x = m_original_scale.x + sin(m_size_mod_count) * SIZE_MOD_AMPLITUDE;
	mc.physics.scale.y = m_original_scale.y + sin(m_size_mod_count) * SIZE_MOD_AMPLITUDE;
}

bool Smoke::should_destroy() {
	return m_alpha < 0.f;
}