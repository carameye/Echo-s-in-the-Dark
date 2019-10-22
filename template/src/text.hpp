#pragma once

#include "common.hpp"
#include "components.hpp"

class Text : public Entity
{
public:
	// Creates all the associated render resources and default transform
	bool init(int id, std::string sign_text, vec2 position);

	void set_status(bool enabled);

private:
	Texture m_text_texture;

	RenderComponent rc;
	MotionComponent mc;
};
