#include "text.hpp"

// TODO: render actual text
bool Text::init(std::string sign_text, vec2 position)
{
	if (!m_text_texture.is_valid())
	{
		const char* path;
		if (sign_text == "Use A and D (or left and right) to roll")
			path = textures_path("text_1.png");
		else if (sign_text == "Use W and S (or up and down) to pan the camera")
			path = textures_path("text_2.png");
		else if (sign_text == "Use space to fly")
			path = textures_path("text_3.png");
		else if (sign_text == "You can only fly for a short time")
			path = textures_path("text_4.png");
		else if (sign_text == "watch out for ghosts!")
			path = textures_path("text_5.png");
		else if (sign_text == "Use F to interact with objects")
			path = textures_path("text_6.png");
		else
			return false;
		if (!m_text_texture.load_from_file(path))
		{
			fprintf(stderr, "Failed to load text texture!");
			return false;
		}
	}
	texture = &m_text_texture;

	if (!init_sprite())
		return false;

	motion.position = position;
	motion.position.y -= 130.f;
	physics.scale = { 1.5f, 1.5f };

	return true;
}

void Text::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Text::draw(const mat3& projection, const vec2& camera_shift)
{
	transform.begin();
	transform.translate(camera_shift);
	transform.translate(motion.position);
	transform.scale(physics.scale);
	transform.end();
	draw_sprite(projection);
}