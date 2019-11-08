// Header
#include "ghost.hpp"

#include <cmath>

Texture Ghost::s_ghost_texture;

bool Ghost::init(int id)
{
	m_id = id;

	if (!s_ghost_texture.is_valid())
	{
		if (!s_ghost_texture.load_from_file(textures_path("ghost.png")))
		{
			fprintf(stderr, "Failed to load ghost texture!");
			return false;
		}
	}

	rc.texture = &s_ghost_texture;

	if (!rc.init_sprite())
		return false;

	mc.position = { 0.f, 0.f };
	mc.velocity = { 0.f, 0.f };
	mc.acceleration = { 0.f , 0.f };
	mc.radians = 0.f;

	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };
	mc.physics.scale.x *= 47.f / 41.f;

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

	return true;
}

void Ghost::update(float ms)
{
	if (len(sub(m_goal, mc.position)) < 800.f)
	{
		if (m_path.size() == 0 || len(sub(m_path.back(), m_goal)) > TOLERANCE)
		{
			m_path = m_level_graph->get_path(mc.position, m_goal);
		}
	}

	if (m_path.size() > 0)
	{
		float allowed_move = 100.f * ms / 1000.f;
		vec2 next_pos = m_path[0];

		while (allowed_move > TOLERANCE)
		{
			vec2 disp = sub(next_pos, mc.position);
			if (disp.x != 0.f)
			{
				mc.physics.scale.x = -abs(mc.physics.scale.x) * disp.x / abs(disp.x);
			}
			float dist = len(disp);

			if (allowed_move < dist)
			{
				vec2 dir = normalize(disp);
				mc.position = add(mc.position, mul(dir, allowed_move));
				allowed_move = 0.f;
			}
			else
			{
				mc.position = next_pos;
				allowed_move -= dist;
				m_path.erase(m_path.begin() + 0);

				if (m_path.size() > 0)
					next_pos = m_path[0];
				else
					allowed_move = 0.f;
			}
		}
	}
}

vec2 Ghost::get_position()const
{
	return mc.position;
}

void Ghost::set_position(vec2 position)
{
	mc.position = position;
	m_path.clear();
}

Hitbox Ghost::get_hitbox() const
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

void Ghost::set_goal(vec2 position)
{
	m_goal = position;
}

void Ghost::set_level_graph(LevelGraph* graph)
{
	m_level_graph = graph;
}
