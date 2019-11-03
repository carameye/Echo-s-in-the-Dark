#pragma once

#include "common.hpp"
#include <vector>
#include <map>

extern int next_entity_id;
extern int next_ui_id;

struct MotionComponent
{
	vec2 position;
	vec2 velocity;
	vec2 acceleration;
	float radians;
};
extern std::map<int, MotionComponent*> s_motion_components;
extern std::map<int, MotionComponent*> s_ui_motion_components;

struct RenderComponent
{
	Texture* texture;
	Mesh mesh;
	Effect effect;
	Physics physics;
	Transform transform;
	bool render = true;
	float alpha;

	bool init_sprite();
	void draw_sprite_alpha(const mat3& projection, float alpha);
};
extern std::map<int, RenderComponent*> s_render_components;
extern std::map<int, RenderComponent*> s_ui_render_components;

extern void clear_level_components();
extern void clear_ui_components();