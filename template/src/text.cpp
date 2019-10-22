#include "text.hpp"

// TODO: render actual text
bool Text::init(int id, std::string sign_text, vec2 position)
{
	m_id = id;

	if (!m_text_texture.is_valid())
	{
		const char* path;
		if (sign_text == "Use A and D (or left and right) to roll")
			path = textures_path("text_1.png");
		else if (sign_text == "Use W and S (or up and down) to pan the camera")
			path = textures_path("text_2.png");
		else if (sign_text == "Use space to fly")
			path = textures_path("text_3.png");
		else if (sign_text == "You can only fly for a short time")
			path = textures_path("text_4.png");
		else if (sign_text == "watch out for ghosts!")
			path = textures_path("text_5.png");
		else if (sign_text == "Use F to interact with objects")
			path = textures_path("text_6.png");
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
	rc.physics.scale = { 1.5f, 1.5f };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

	return true;
}

void Text::set_status(bool enabled)
{
	rc.render = enabled;
}
