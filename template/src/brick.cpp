// Header
#include "brick.hpp"

#include <cmath>

Texture Brick::brick_texture;
RenderComponent Brick::rc;

bool Brick::init(int id)
{
	m_id = id;

	if (!brick_texture.is_valid())
	{
		if (!brick_texture.load_from_file(textures_path("tile_brick.png")))
		{
			fprintf(stderr, "Failed to load brick texture!");
			return false;
		}

		rc.texture = &brick_texture;

		if (!rc.init_sprite())
			return false;
	}

	mc.position = { 0.f, 0.f };
	mc.velocity = { 0.f, 0.f };
	mc.acceleration = { 0.f , 0.f };
	mc.radians = 0.f;
	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };
	m_colour = {1.f, 1.f, 1.f};
	m_is_visible = m_colour.x == 1.f && m_colour.y == 1.f && m_colour.z == 1.f;
	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

    s_render_components[id]->can_be_hidden = 1.f;
    s_render_components[id]->colour = m_colour;

	return true;
}

void Brick::update(vec3 hl_colour)
{
    if (m_colour.x == 1.f && m_colour.y == 1.f && m_colour.z == 1.f) {
        m_is_visible = true;
        return;
    }

    m_is_visible = m_colour.x == hl_colour.x && m_colour.y == hl_colour.y && m_colour.z == hl_colour.z;
}

vec2 Brick::get_position()const
{
	return mc.position;
}

void Brick::set_position(vec2 position)
{
	mc.position = position;
}

void Brick::set_colour(vec3 colour)
{
    m_colour = colour;
}

Hitbox Brick::get_hitbox() const
{
    std::vector<Square> squares(1);
    float width = brick_size;
    vec2 position = mc.position;
    position.x -= width / 2;
    position.y += width / 2;
    Square square(position, width);
    squares[0] = square;
    Hitbox hitbox({}, squares);
    return hitbox;
}

bool Brick::get_is_visible() {
    return m_is_visible;
}
