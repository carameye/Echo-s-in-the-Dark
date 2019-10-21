#pragma once

#include "common.hpp"

class Text : public Entity
{
public:
	// Creates all the associated render resources and default transform
	bool init(std::string sign_text, vec2 position);

	void destroy();

	void draw(const mat3& projection, const vec2& camera_shift) override;

private:
	Texture m_text_texture;
};
