#include "smoke.hpp";

#include <math.h>

namespace
{
	const size_t FADE_OUT_MS = 750;
	const float MAX_SCALE = 3.f;
	const float MIN_SCALE = 1.5f;
	const size_t SIZE_MOD_MS = 200; // MS for one size modulation cycle
	const float SIZE_MOD_AMPLITUDE = 0.7f;
	const float VELOCITY_Y = 50.f;
	const float PI = 3.14159265f;
}

Texture Smoke::smoke_texture_large;
Texture Smoke::smoke_texture_small;

bool Smoke::init(vec2 robot_position, vec2 robot_velocity)
{
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
		texture = &smoke_texture_large;
	} else {
		texture = &smoke_texture_small;
	}

    motion.velocity.x = robot_velocity.x * -1.f / 3.f;
    motion.velocity.y = robot_velocity.y * -1.f / 2.f;
	if (motion.velocity.y < 0.f) {
		motion.velocity.y = VELOCITY_Y;
	}
	motion.position = { robot_position.x, robot_position.y + 35.f };

	float scale = MIN_SCALE + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(MAX_SCALE-MIN_SCALE)));
    m_original_scale = { scale, scale };
	physics.scale = m_original_scale;

	motion.radians = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * PI)));

	if (!init_sprite())
		return false;
    return true;
}

void Smoke::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Smoke::update(float ms)
{
	float time_factor = ms / 1000.f;
	motion.position.x += motion.velocity.x * time_factor;
	motion.position.y += motion.velocity.y * time_factor;
	m_alpha -= ms / FADE_OUT_MS;
	m_size_mod_count += ms / SIZE_MOD_MS * PI;
	if (m_size_mod_count > PI) {
		m_size_mod_count = 0;
	}
	physics.scale.x = m_original_scale.x + sin(m_size_mod_count) * SIZE_MOD_AMPLITUDE;
	physics.scale.y = m_original_scale.y + sin(m_size_mod_count) * SIZE_MOD_AMPLITUDE;
}

void Smoke::draw(const mat3& projection, const vec2& camera_shift)
{
	transform.begin();
	transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.rotate(motion.radians);
    transform.scale(physics.scale);
	transform.end();

	draw_sprite_alpha(projection, m_alpha);
}

bool Smoke::should_destroy() {
	return m_alpha < 0.f;
}