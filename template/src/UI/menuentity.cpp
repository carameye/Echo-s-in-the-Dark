#include "menuentity.hpp"

bool MenuEntity::init(int id, vec2 position)
{
	m_id = id;

	if (!m_rc.init_sprite())
		return false;

	m_mc.position = position;
	m_mc.velocity = { 0.f, 0.f };
	m_mc.acceleration = { 0.f , 0.f };
	m_mc.radians = 0.f;

	m_mc.physics.scale = { width / m_rc.texture->width, height / m_rc.texture->height };

	s_ui_render_components[id] = &m_rc;
	s_ui_motion_components[id] = &m_mc;

	return true;
}

void MenuEntity::set_size(vec2 size)
{
	width = size.x;
	height = size.y;
}

bool Button::init(int id, vec2 position)
{
	if (!button_texture.is_valid())
	{
		std::string path = textures_path("");
		path = path.append(m_texture_name);
		if (!button_texture.load_from_file(path.c_str()))
		{
			fprintf(stderr, "Failed to load button texture (%s)!", path.c_str());
			return false;
		}
	}

	m_rc.texture = &button_texture;

	return MenuEntity::init(id, position);
}

void Button::hover(vec2 p)
{
	vec2 tl = sub(m_mc.position, { width / 2.f, height / 2.f });
	vec2 br = add(m_mc.position, { width / 2.f, height / 2.f });
	if (p.x >= tl.x && p.x <= br.x && p.y >= tl.y && p.y <= br.y)
	{
		m_hover = true;
	}
	else
	{
		m_hover = false;
	}
}

bool Button::is_click()
{
	return m_hover;
}

Status Button::click()
{
	return m_status;
}

void Button::set_texture_name(std::string name)
{
	m_texture_name = name;
}

void Button::set_status(Status status)
{
	m_status = status;
}
