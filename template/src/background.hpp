#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "text.hpp"

class Background : public Entity
{
	static Texture s_background_texture;

	RenderComponent rc_first;
	RenderComponent rc_second;
	RenderComponent rc_third;
	MotionComponent mc_first;
	MotionComponent mc_second;
	MotionComponent mc_third;

public:
	// Creates all the associated render resources and default transform
	bool init(int id, float scale, float alpha);

	void set_position(vec2 position);

	void update(float elapsed_ms, vec2 robot_velocity);

private:
	int m_left_most = 0;
	int m_middle = 1;
	int m_right_most = 2;

	// how much the robot has moved since the last swap
	float m_robot_displacement = 0.f;
	float m_texture_width;

	void swap_left();
	void swap_right();
};
