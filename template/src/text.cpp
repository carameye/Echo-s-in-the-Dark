#include "text.hpp"

// TODO: render actual text
bool Text::init(int id, std::string sign_text, vec2 position)
{
	m_id = id;

	if (!m_text_texture.is_valid())
	{
		const char* path;
		if (sign_text == "roll")
			path = textures_path("text_1.png");
		else if (sign_text == "pan")
			path = textures_path("text_2.png");
		else if (sign_text == "fly")
			path = textures_path("text_3.png");
		else if (sign_text == "fly_duration")
			path = textures_path("text_4.png");
		else if (sign_text == "ghosts")
			path = textures_path("text_5.png");
		else if (sign_text == "interact")
			path = textures_path("text_6.png");
		else if (sign_text == "locked")
			path = textures_path("text_7.png");
		else if (sign_text == "mouse")
			path = textures_path("text_8.png");
		else if (sign_text == "color")
			path = textures_path("text_9.png");
		else if (sign_text == "colored_bricks")
			path = textures_path("text_10.png");
		else if (sign_text == "freeze_ghosts")
			path = textures_path("text_11.png");
		else
			return false;
		if (!m_text_texture.load_from_file(path))
		{
			fprintf(stderr, "Failed to load text texture!");
			return false;
		}
	}
	rc.texture = &m_text_texture;

	if (!rc.init_sprite())
		return false;

	mc.position = position;
	mc.position.y -= 130.f;
	mc.physics.scale = { 1.5f, 1.5f };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

	return true;
}

void Text::set_status(bool enabled)
{
	rc.render = enabled;
}
