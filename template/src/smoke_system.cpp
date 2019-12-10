#include "smoke_system.hpp"

#include <cmath>

namespace
{
	const size_t SPAWN_DELAY_MS = 25;
	const size_t SMOKE_COUNT = 3; // # of smokes generated at the same time
	const size_t MAX_ACTIVE_SMOKE = 100; // # of smokes to be used
	const float SMOKE_WIDTH = 40.f;
}

Texture SmokeSystem::test_texture;

bool SmokeSystem::init(int id)
{
	m_active_smokes.clear();
	m_inactive_smokes.clear();

	for (unsigned i = 0; i < MAX_ACTIVE_SMOKE; i++) {
		Smoke *smoke = new Smoke();
		if (smoke->init(id + i)) {
			m_inactive_smokes.push_back(smoke);
		}
	}
	return true;
}

void SmokeSystem::update(float ms, vec2 robot_position, vec2 robot_velocity)
{
	m_next_spawn -= ms;
    if (m_started && m_next_spawn < 0.f && m_inactive_smokes.size() >= SMOKE_COUNT) {
		vec2 smoke_position = robot_position;
		float x_interval = SMOKE_WIDTH / (SMOKE_COUNT - 1);
		smoke_position.x -= SMOKE_WIDTH / 2.f;
		for (unsigned i = 0; i < SMOKE_COUNT; i++) {
			m_inactive_smokes.at(i)->activate(smoke_position, robot_velocity);
			m_active_smokes.push_back(m_inactive_smokes.at(i));
			m_inactive_smokes.erase(m_inactive_smokes.begin() + i);
			smoke_position.x += x_interval;
		}
	}
	for (int i = (int)m_active_smokes.size() - 1; i >= 0; i--) {
		if (m_active_smokes.at(i)->should_destroy()) {
			m_inactive_smokes.push_back(m_active_smokes.at(i));
			m_active_smokes.erase(m_active_smokes.begin() + i);
		} else {
			m_active_smokes.at(i)->update(ms);
		}
	}
}

void SmokeSystem::start_smoke()
{
	m_started = true;
}

void SmokeSystem::stop_smoke()
{
	m_started = false;
}

void SmokeSystem::destroy()
{
	for (auto& smoke : m_active_smokes)
	{
		delete smoke;
	}
	for (auto& smoke : m_inactive_smokes)
	{
		delete smoke;
	}
	m_active_smokes.clear();
	m_inactive_smokes.clear();
}
