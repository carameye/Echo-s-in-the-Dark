// Header
#include "brick.hpp"

#include <cmath>

Texture Brick::brick_texture;
RenderComponent Brick::rc;
RenderComponent Brick::rrc;
RenderComponent Brick::grc;
RenderComponent Brick::brc;
RenderComponent Brick::irc;

bool Brick::init(int id, vec3 colour)
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
        rrc.texture = &brick_texture;
        grc.texture = &brick_texture;
        brc.texture = &brick_texture;
        irc.texture = &brick_texture;

		if (!rc.init_sprite())
			return false;

        if (!rrc.init_sprite())
            return false;

        if (!grc.init_sprite())
            return false;

        if (!brc.init_sprite())
            return false;

        if (!irc.init_sprite())
            return false;
	}

	mc.position = { 0.f, 0.f };
	mc.velocity = { 0.f, 0.f };
	mc.acceleration = { 0.f , 0.f };
	mc.radians = 0.f;
	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };
	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };

    m_colour = colour;
    m_is_collidable = (m_colour.x == 1.f && m_colour.y == 1.f && m_colour.z == 1.f)
            || (m_colour.x == 0.f && m_colour.y == 0.f && m_colour.z == 0.f);

    if (colour.x == 1.f && colour.y == 0.f && colour.z == 0.f) {
        rrc.can_be_hidden = 1;
        rrc.colour = m_colour;
        s_render_components[id] = &rrc;
    } else if (colour.x == 0.f && colour.y == 1.f && colour.z == 0.f) {
        grc.can_be_hidden = 1;
        grc.colour = m_colour;
        s_render_components[id] = &grc;
    } else if (colour.x == 0.f && colour.y == 0.f && colour.z == 1.f) {
        brc.can_be_hidden = 1;
        brc.colour = m_colour;
        s_render_components[id] = &brc;
    } else if (colour.x == 0.f && colour.y == 0.f && colour.z == 0.f) {
        irc.is_invisible = 1;
        irc.colour = m_colour;
        s_render_components[id] = &irc;
    } else {
        s_render_components[id] = &rc;
    }
	s_motion_components[id] = &mc;

	return true;
}

void Brick::update(vec3 hl_colour)
{
    if ((m_colour.x == 1.f && m_colour.y == 1.f && m_colour.z == 1.f)
    || (m_colour.x == 0.f && m_colour.y == 0.f && m_colour.z == 0.f)) {
        m_is_collidable = true;
        return;
    }

    m_is_collidable = m_colour.x == hl_colour.x && m_colour.y == hl_colour.y && m_colour.z == hl_colour.z;
}

vec2 Brick::get_position()const
{
	return mc.position;
}

void Brick::set_position(vec2 position)
{
	mc.position = position;
	calculate_hitbox();
}

void Brick::calculate_hitbox()
{
    std::vector<Square> squares(1);
    float width = brick_size;
    vec2 position = mc.position;
    position.x -= width / 2;
    position.y += width / 2;
    Square square(position, (int)width);
    squares[0] = square;
    Hitbox hitbox({}, squares);
    m_hitbox = hitbox;
}

Hitbox Brick::get_hitbox() const
{
    return m_hitbox;
}

bool Brick::get_is_collidable() {
    return m_is_collidable;
}

vec3 Brick::get_colour() {
    return m_colour;
}
