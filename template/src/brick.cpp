// Header
#include "brick.hpp"

#include <cmath>

Texture Brick::brick_texture;
RenderComponent Brick::rc;

bool Brick::init(int id)
{
	m_id = id;

	if (!brick_texture.is_valid())
	{
		if (!brick_texture.load_from_file(textures_path("tile_brick.png")))
		{
			fprintf(stderr, "Failed to load brick texture!");
			return false;
		}

		rc.texture = &brick_texture;

		if (!rc.init_sprite())
			return false;
	}

	mc.position = { 0.f, 0.f };
	mc.velocity = { 0.f, 0.f };
	mc.acceleration = { 0.f , 0.f };
	mc.radians = 0.f;
	mc.physics.scale = { brick_size / rc.texture->width, brick_size / rc.texture->height };

	s_render_components[id] = &rc;
	s_motion_components[id] = &mc;

	return true;
}

void Brick::update(float ms)
{
	// probably don't really need much here...
}

vec2 Brick::get_position()const
{
	return mc.position;
}

void Brick::set_position(vec2 position)
{
	mc.position = position;
}

Hitbox Brick::get_hitbox() const
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