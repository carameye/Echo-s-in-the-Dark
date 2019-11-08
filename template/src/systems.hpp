#pragma once

#include <vector>
#include "components.hpp"

class RenderingSystem
{
private:
	std::vector<int> level_entities;
	std::vector<int> menu_entities;

public:
	void render(const mat3& projection, const vec2& camera_shift);
	void process(int min, int max);
	void destroy();
	void clear();
};
