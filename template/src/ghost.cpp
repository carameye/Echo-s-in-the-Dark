// Header
#include "ghost.hpp"

#include <cmath>

Texture Ghost::s_ghost_texture;

bool Ghost::init()
{
	if (!s_ghost_texture.is_valid())
	{
		if (!s_ghost_texture.load_from_file(textures_path("ghost.png")))
		{
			fprintf(stderr, "Failed to load ghost texture!");
			return false;
		}
	}

	texture = &s_ghost_texture;

	if (!init_sprite())
		return false;

	motion.position = { 0.f, 0.f };
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f , 0.f };
	motion.radians = 0.f;

	physics.scale = { brick_size / texture->width, brick_size / texture->height };
	physics.scale.x *= 47.f / 41.f;

	return true;
}

// Releases all graphics resources
void Ghost::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Ghost::update(float ms)
{
	if (len(sub(m_goal, motion.position)) < 500.f)
	{
		if (m_path.size() == 0 || len(sub(m_path.back(), m_goal)) > brick_size / 2.f )
		{
			m_path = m_level_graph->get_path(motion.position, m_goal);
		}
	}

	if (m_path.size() > 0)
	{
		float allowed_move = 3.f;
		vec2 next_pos = m_path[0];

		while (allowed_move > TOLERANCE)
		{
			vec2 disp = sub(next_pos, motion.position);
			float dist = len(disp);

			if (allowed_move < dist)
			{
				vec2 dir = normalize(disp);
				motion.position = add(motion.position, mul(dir, allowed_move));
				allowed_move = 0.f;
			}
			else
			{
				motion.position = next_pos;
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

void Ghost::draw(const mat3& projection, const vec2& camera_shift)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform.begin();
	transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.rotate(motion.radians);
	transform.scale(physics.scale);
	if (motion.position.x < m_goal.x) {
		transform.scale({ -1.f, 1.f });
	}
	transform.end();

	draw_sprite(projection);
}

vec2 Ghost::get_position()const
{
	return motion.position;
}

void Ghost::set_position(vec2 position)
{
	motion.position = position;
}

Hitbox Ghost::get_hitbox() const
{
	std::vector<Square> squares(1);

	float width = brick_size;
	vec2 position = motion.position;
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
