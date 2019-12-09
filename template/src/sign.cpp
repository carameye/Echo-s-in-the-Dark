#include "sign.hpp"

Texture Sign::s_sign_texture;

bool Sign::init(int id, std::string sign_text, vec2 position)
{
	m_id = id;

	if (!s_sign_texture.is_valid())
	{
		if (!s_sign_texture.load_from_file(textures_path("sign.png")))
		{
			fprintf(stderr, "Failed to load sign texture!");
			return false;
		}
	}

	rc.texture = &s_sign_texture;

	if (!rc.init_sprite())
		return false;

	mc.position = position;
	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

	hide_text();

	calculate_hitbox();

	return m_text.init(id + 1, sign_text, position);
}

Hitbox Sign::get_hitbox() const
{
    return m_hitbox;
}

void Sign::show_text()
{
	m_text.set_status(true);
}

void Sign::hide_text()
{
	m_text.set_status(false);
}

void Sign::calculate_hitbox() {
    std::vector<Square> squares(2);

    float width = brick_size;
    vec2 position = mc.position;
    position.x -= width / 2;
    position.y += width / 2;
    Square top(position, (int)width);
    Square bot(add(position, { 0.f, width }), (int)width);
    squares[0] = top;
    squares[1] = bot;

    Hitbox hitbox({}, squares);
    m_hitbox = hitbox;
}
