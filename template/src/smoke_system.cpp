#include "smoke_system.hpp"

#include <cmath>

namespace
{
	const size_t SPAWN_DELAY_MS = 25;
	const size_t SMOKE_COUNT = 3; // # of smokes generated at the same time
	const float SMOKE_WIDTH = 40.f;
}

Texture SmokeSystem::test_texture;

bool SmokeSystem::init()
{
	if (!test_texture.is_valid())
	{
		if (!test_texture.load_from_file(textures_path("smoke_large.png")))
		{
			fprintf(stderr, "Failed to load smoke texture!");
			return false;
		}
	}
	texture = &test_texture;

	if (!init_sprite())
		return false;

	m_smokes.clear();

	return true;
}

void SmokeSystem::destroy()
{
	for (auto& smoke : m_smokes)
		smoke.destroy();
	m_smokes.clear();
}

void SmokeSystem::update(float ms, vec2 robot_position, vec2 robot_velocity)
{
	m_next_spawn -= ms;
    if (m_started && m_next_spawn < 0.f) {
		vec2 smoke_position = robot_position;
		float x_interval = SMOKE_WIDTH / (SMOKE_COUNT - 1);
		smoke_position.x -= SMOKE_WIDTH / 2.f;
		for (unsigned i = 0; i < SMOKE_COUNT; i++) {
			Smoke smoke;
			if (smoke.init(smoke_position, robot_velocity)) {
				m_smokes.push_back(smoke);
			}
			smoke_position.x += x_interval;
		}
		m_next_spawn = SPAWN_DELAY_MS;
	}
	for (int i = m_smokes.size() - 1; i >= 0; i--) {
		if (m_smokes.at(i).should_destroy()) {
			m_smokes.erase(m_smokes.begin() + i);
		} else {
			m_smokes.at(i).update(ms);
		}
	}
}

void SmokeSystem::draw(const mat3& projection, const vec2& camera_shift)
{
    for (auto& smoke : m_smokes)
		smoke.draw(projection, camera_shift);

	transform.begin();
	transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.end();
}

void SmokeSystem::start_smoke()
{
	m_started = true;
}

void SmokeSystem::stop_smoke()
{
	m_started = false;
}
