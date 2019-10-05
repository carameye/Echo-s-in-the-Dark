#pragma once

#include "common.hpp"


class Light : public Entity
{
public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the water
	void draw(const mat3& projection, const vec2& camera_shift)override;

    void set_position(vec2 pos);


private:
    vec2 light_position;
};
