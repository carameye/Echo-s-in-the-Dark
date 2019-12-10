#include "background.hpp"

Texture Background::s_background_texture;

bool Background::init(int id, float scale, float alpha)
{
	m_id = id;

	if (!s_background_texture.is_valid())
	{
		if (!s_background_texture.load_from_file(textures_path("bg_city.png")))
		{
			fprintf(stderr, "Failed to load background texture!");
			return false;
		}
	}

	rc_first.texture = &s_background_texture;
	rc_second.texture = &s_background_texture;
	rc_third.texture = &s_background_texture;

	if (!rc_first.init_sprite() || !rc_second.init_sprite() || !rc_third.init_sprite())
		return false;

	mc_first.position = { 0.f, 0.f };
	mc_first.physics.scale = { scale , scale };

	mc_second.position = { 0.f, 0.f };
	mc_second.physics.scale = { scale , scale };

	mc_third.position = { 0.f, 0.f };
	mc_third.physics.scale = { scale , scale };

	s_render_components[id] = &rc_first;
	s_motion_components[id] = &mc_first;

	s_render_components[id + 1] = &rc_second;
	s_motion_components[id + 1] = &mc_second;

	s_render_components[id + 2] = &rc_third;
	s_motion_components[id + 2] = &mc_third;

	// rc_first.alpha = alpha;
	// rc_second.alpha = alpha;
	// rc_third.alpha = alpha;

	m_texture_width = s_background_texture.width * scale;

	float color = 1.3f - scale;
	rc_first.colour = { color, color, color };
	rc_second.colour = { color, color, color };
	rc_third.colour = { color, color, color };

	return true;
}

void Background::set_position(vec2 position)
{
	float add_y = 150 - 600 * (1 - mc_first.physics.scale.x);
	mc_first.position = position;
	mc_first.position.x -= m_texture_width;
	mc_first.position.y += add_y;
	mc_second.position = position;
	mc_second.position.y += add_y;
	mc_third.position = position;
	mc_third.position.x += m_texture_width;
	mc_third.position.y += add_y;
}

void Background::update(float elapsed_ms, vec2 robot_movement)
{
	float factor = 1.25f - mc_first.physics.scale.x;
	vec2 texture_movement = { robot_movement.x * factor, robot_movement.y * factor / 5.f };
	mc_first.position = add(mc_first.position, texture_movement);
	mc_second.position = add(mc_second.position, texture_movement);
	mc_third.position = add(mc_third.position, texture_movement);

	m_robot_displacement += robot_movement.x - texture_movement.x;

	if (m_robot_displacement >= m_texture_width)
		swap_left();
	if (m_robot_displacement <= m_texture_width * -1.f)
		swap_right();
}

void Background::swap_left()
{
	if (m_left_most == 0)
		mc_first.position.x += m_texture_width * 3;
	else if (m_left_most == 1)
		mc_second.position.x += m_texture_width * 3;
	else if (m_left_most == 2)
		mc_third.position.x += m_texture_width * 3;
	int temp = m_left_most;
	m_left_most = m_middle;
	m_middle = m_right_most;
	m_right_most = temp;
	m_robot_displacement -= m_texture_width;
}

void Background::swap_right()
{
	if (m_right_most == 0)
		mc_first.position.x -= m_texture_width * 3;
	else if (m_right_most == 1)
		mc_second.position.x -= m_texture_width * 3;
	else if (m_right_most == 2)
		mc_third.position.x -= m_texture_width * 3;
	int temp = m_right_most;
	m_right_most = m_middle;
	m_middle = m_left_most;
	m_left_most = temp;
	m_robot_displacement += m_texture_width;
}