#include "text.hpp"

// TODO: render actual text
bool Text::init(int id, std::string sign_text, vec2 position)
{
	m_id = id;

	if (!m_text_texture.is_valid())
	{
		const char* path;
		if (sign_text == "roll")
			path = textures_path("text_roll.png");
		else if (sign_text == "pan")
			path = textures_path("text_pan.png");
		else if (sign_text == "fly")
			path = textures_path("text_fly.png");
		else if (sign_text == "ghosts")
			path = textures_path("text_ghosts.png");
		else if (sign_text == "interact")
			path = textures_path("text_interact.png");
		else if (sign_text == "locked")
			path = textures_path("text_locked.png");
		else if (sign_text == "mouse")
			path = textures_path("text_mouse.png");
		else if (sign_text == "color")
			path = textures_path("text_color.png");
		else if (sign_text == "colored_bricks")
			path = textures_path("text_bricks.png");
		else if (sign_text == "freeze_ghosts")
			path = textures_path("text_freeze.png");
        else if (sign_text == "story1")
            path = textures_path("story_1.png");
        else if (sign_text == "story2")
            path = textures_path("story_2.png");
        else if (sign_text == "story3")
            path = textures_path("story_3.png");
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
