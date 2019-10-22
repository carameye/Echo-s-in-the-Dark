#include "interactable.hpp"

bool Interactable::init(int id)
{
	m_id = id;

    mc.position = { 0.f, 0.f };
    rc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

    return true;
}\

void Interactable::set_position(vec2 pos)
{
	mc.position = add(pos, { 0.f, brick_size / 2.f + 6.f });
}
