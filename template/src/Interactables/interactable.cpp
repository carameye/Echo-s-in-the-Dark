#include "interactable.hpp"

bool Interactable::init(int id, vec2 position)
{
	m_id = id;
	
	position = add(position, { 0.f, brick_size / 2.f + 6.f });

    mc.position = position;
    mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

    return true;
}
