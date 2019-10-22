#include "systems.hpp"

void RenderingSystem::render(const mat3& projection, const vec2& camera_shift)
{
	for (auto& entity : entities)
	{
		RenderComponent* rc = s_render_components[entity];
		MotionComponent* mc = s_motion_components[entity];

		if (!rc->render)
		{
			continue;
		}

		// Transformation code, see Rendering and Transformation in the template specification for more info
		// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
		rc->transform.begin();
		rc->transform.translate(camera_shift);
		rc->transform.translate(mc->position);
		rc->transform.rotate(mc->radians);
		rc->transform.scale(rc->physics.scale);
		rc->transform.end();

		rc->draw_sprite_alpha(projection, rc->alpha);
	}
}

void RenderingSystem::process(int max)
{
	for (int i = 0; i < max; i++)
	{
		if (s_render_components.find(i) != s_render_components.end() &&
			s_motion_components.find(i) != s_motion_components.end())
		{
			entities.push_back(i);
		}
	}
}

void RenderingSystem::destroy()
{
	for (auto& entity : entities)
	{
		RenderComponent* rc = s_render_components[entity];

		glDeleteBuffers(1, &rc->mesh.vbo);
		glDeleteBuffers(1, &rc->mesh.ibo);
		glDeleteBuffers(1, &rc->mesh.vao);

		glDeleteShader(rc->effect.vertex);
		glDeleteShader(rc->effect.fragment);
		glDeleteShader(rc->effect.program);
	}
}

void RenderingSystem::clear()
{
	entities.clear();
}