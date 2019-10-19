#pragma once

#include "common.hpp"
#include <vector>

class Smoke : public Entity
{
	static Texture smoke_texture_large;
	static Texture smoke_texture_small;

public:
	bool init();

	void activate(vec2 robot_position, vec2 robot_velocity);

	void destroy();
	
	void update(float ms);

	void draw(const mat3& projection, const vec2& camera_shift) override;

	bool should_destroy();

private:
	float m_alpha = 1.f;
	float m_size_mod_count = 0.f;
	vec2 m_original_scale;
};