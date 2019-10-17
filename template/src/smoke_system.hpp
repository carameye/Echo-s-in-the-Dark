#pragma once

#include "common.hpp"
#include "smoke.hpp"
#include <vector>

class SmokeSystem : public Entity
{
	static Texture test_texture;

public:
	bool init();

    void destroy();

    void update(float ms, vec2 robot_position, vec2 robot_velocity);

    void draw(const mat3& projection, const vec2& camera_shift) override;

	void start_smoke();

	void stop_smoke();

private:
	std::vector<Smoke> m_smokes;
	bool m_started = false;
	float m_next_spawn;

	void create_smoke(vec2 robot_position, vec2 robot_velocity);
};