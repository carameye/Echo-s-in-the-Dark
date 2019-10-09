// Header
#include "ghost.hpp"

#include <cmath>

Texture Ghost::ghost_texture;

bool Ghost::init()
{
	if (!ghost_texture.is_valid())
	{
		if (!ghost_texture.load_from_file(textures_path("robot.png")))
		{
			fprintf(stderr, "Failed to load ghost texture!");
			return false;
		}
	}

	texture = &ghost_texture;

	if (!init_sprite())
		return false;

	motion.position = { 0.f, 0.f };
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f , 0.f };
	motion.radians = 0.f;

	physics.scale = { brick_size.x / texture->width, brick_size.y / texture->height };

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
	if (len(sub(goal, motion.position)) < 500.f)
	{
		last_seen = goal;
	}

	vec2 dif = sub(last_seen, motion.position);
	if (len(dif) != 0.f)
	{
		vec2 dir = normalize(dif);
		vec2 mov = mul(dir, 100.f * ms / 1000.f);
		if (len(mov) > len(dif))
			motion.position = last_seen;
		else
			motion.position = add(motion.position, mov);
	}
	fprintf(stderr, "(%f, %f)\n", motion.position.x, motion.position.y);
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
	last_seen = position;
}

Hitbox Ghost::get_hitbox() const
{
	std::vector<Square> squares(1);

	float width = brick_size.x;
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
	goal = position;
}
