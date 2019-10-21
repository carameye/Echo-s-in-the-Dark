#include "interactable.hpp"

bool Interactable::init()
{
    motion.position = { 0.f, 0.f };
    physics.scale = { brick_size / texture->width, brick_size / texture->height };
    return true;
}

void Interactable::destroy()
{
    // remove resources
    glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Interactable::draw(const mat3& projection, const vec2& camera_shift)
{
    transform.begin();
    transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.scale(physics.scale);
	transform.end();

	draw_sprite(projection);
}

void Interactable::set_position(vec2 pos)
{
	motion.position = add(pos, { 0.f, brick_size / 2.f + 6.f });
}
