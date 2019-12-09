#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "text.hpp"

class Sign : public Entity
{
	static Texture s_sign_texture;

	RenderComponent rc;
	MotionComponent mc;

public:
	// Creates all the associated render resources and default transform
	bool init(int id, std::string text, vec2 position);

	Hitbox get_hitbox() const;

	void show_text();

	void hide_text();

private:
	Text m_text;
	Hitbox m_hitbox;

	void calculate_hitbox();
};
