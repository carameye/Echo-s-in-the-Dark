// Header
#include "brick.hpp"

#include <cmath>

Texture Brick::brick_texture;

bool Brick::init()
{
	if (!brick_texture.is_valid())
	{
		if (!brick_texture.load_from_file(textures_path("tile_brick.png")))
		{
			fprintf(stderr, "Failed to load brick texture!");
			return false;
		}
	}

	texture = &brick_texture;

	if (!init_sprite())
		return false;

	motion.position = { 0.f, 0.f };
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f , 0.f };
	motion.radians = 0.f;

	physics.scale = { brick_size / texture->width, brick_size / texture->height };

	return true;
}

// Releases all graphics resources
void Brick::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Brick::update(float ms)
{
	// probably don't really need much here...
}

void Brick::draw(const mat3& projection, const vec2& camera_shift)
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

vec2 Brick::get_position()const
{
	return motion.position;
}

void Brick::set_position(vec2 position)
{
    // TODO: not really sure if we will aver need something like this but for now I am going to leave it
	motion.position = position;
}

Hitbox Brick::get_hitbox() const
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