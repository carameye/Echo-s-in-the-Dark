#include "sign.hpp"

Texture Sign::s_sign_texture;

bool Sign::init(std::string sign_text, vec2 position)
{
	if (!s_sign_texture.is_valid())
	{
		if (!s_sign_texture.load_from_file(textures_path("sign.png")))
		{
			fprintf(stderr, "Failed to load sign texture!");
			return false;
		}
	}

	texture = &s_sign_texture;

	if (!init_sprite())
		return false;

	motion.position = position;
	physics.scale = { brick_size / texture->width, brick_size / texture->height };
	m_text = new Text();
	return m_text->init(sign_text, position);
}

void Sign::destroy()
{
	m_text->destroy();
	delete m_text;
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

Hitbox Sign::get_hitbox() const
{
	std::vector<Square> squares(2);

	float width = brick_size;
	vec2 position = motion.position;
	position.x -= width / 2;
	position.y += width / 2;
	Square top(position, width);
	Square bot(add(position, { 0.f, width }), width);
	squares[0] = top;
	squares[1] = bot;

	Hitbox hitbox({}, squares);
	return hitbox;
}

void Sign::show_text()
{
	m_show_text = true;
}

void Sign::hide_text()
{
	m_show_text = false;
}

void Sign::draw(const mat3& projection, const vec2& camera_shift)
{
	transform.begin();
	transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.scale(physics.scale);
	transform.end();
	draw_sprite(projection);
	if (m_show_text)
		m_text->draw(projection, camera_shift);
}