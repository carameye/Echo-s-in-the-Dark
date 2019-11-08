#pragma once

#include "common.hpp"
#include <vector>
#include "components.hpp"

class Smoke : public Entity
{
	static Texture smoke_texture_large;
	static Texture smoke_texture_small;

	static RenderComponent rc_large;
	static RenderComponent rc_small;

	MotionComponent mc;

public:
	bool init(int id);

	void activate(vec2 robot_position, vec2 robot_velocity);

	void update(float ms);

	bool should_destroy();

private:
	float m_alpha = 1.f;
	float m_size_mod_count = 0.f;
	vec2 m_original_scale;
};