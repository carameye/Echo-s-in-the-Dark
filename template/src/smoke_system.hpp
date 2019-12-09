#pragma once

#include "common.hpp"
#include "smoke.hpp"
#include <vector>

class SmokeSystem : public Entity
{
	static Texture test_texture;

public:
	bool init(int id);

    void update(float ms, vec2 robot_position, vec2 robot_velocity);

	void start_smoke();

	void stop_smoke();

	void destroy();

private:
	std::vector<Smoke*> m_active_smokes;
	std::vector<Smoke*> m_inactive_smokes;
	bool m_started = false;
	float m_next_spawn;
};