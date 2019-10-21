#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "text.hpp"

class Sign : public Entity
{
	static Texture s_sign_texture;

public:
	// Creates all the associated render resources and default transform
	bool init(std::string text, vec2 position);

	void destroy();

	void draw(const mat3& projection, const vec2& camera_shift) override;

	Hitbox get_hitbox() const;

	void show_text();

	void hide_text();

private:
	Text* m_text;
	bool m_show_text = false;
};
