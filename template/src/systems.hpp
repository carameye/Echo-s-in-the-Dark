#pragma once

#include <vector>
#include "components.hpp"

class RenderingSystem
{
private:
	std::vector<int> entities;

public:
	void render(const mat3& projection, const vec2& camera_shift);
	void process(int max);
	void destroy();
	void clear();
};

class UISystem
{
private:
	std::vector<int> entities;

public:
	void render(const mat3& projection);
	void process(int min, int max);
	void destroy();
	void clear();
};
