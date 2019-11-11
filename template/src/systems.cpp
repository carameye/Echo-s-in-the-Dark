#include "systems.hpp"

void RenderingSystem::render(const mat3& projection, const vec2& camera_shift, vec3 headlight_channel)
{
	for (auto& entity : level_entities)
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
		rc->transform.scale(mc->physics.scale);
		rc->transform.end();

		rc->draw_sprite_alpha(projection, rc->alpha, headlight_channel);
	}
}

void RenderingSystem::render_ui(const mat3& projection, const vec2& camera_shift)
{
    for (auto& entity : menu_entities)
    {
        RenderComponent* rc = s_ui_render_components[entity];
        MotionComponent* mc = s_ui_motion_components[entity];

        if (!rc->render)
        {
            continue;
        }

        // Transformation code, see Rendering and Transformation in the template specification for more info
        // Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
        rc->transform.begin();
        rc->transform.translate(mc->position);
        rc->transform.rotate(mc->radians);
        rc->transform.scale(mc->physics.scale);
        rc->transform.end();

        rc->draw_ui_sprite_alpha(projection, rc->alpha);
    }
}

void RenderingSystem::process(int min, int max)
{
	for (int i = min; i < max; i++)
	{
		if (s_render_components.find(i) != s_render_components.end() &&
			s_motion_components.find(i) != s_motion_components.end())
		{
			level_entities.push_back(i);
		}

		if (s_ui_render_components.find(i) != s_ui_render_components.end() &&
			s_ui_motion_components.find(i) != s_ui_motion_components.end())
		{
			menu_entities.push_back(i);
		}
	}
}

void RenderingSystem::destroy()
{
	for (auto& entity : level_entities)
	{
		RenderComponent* rc = s_render_components[entity];

		glDeleteBuffers(1, &rc->mesh.vbo);
		glDeleteBuffers(1, &rc->mesh.ibo);
		glDeleteVertexArrays(1, &rc->mesh.vao);

		glDeleteShader(rc->effect.vertex);
		glDeleteShader(rc->effect.fragment);
		glDeleteShader(rc->effect.program);
	}

	for (auto& entity : menu_entities)
	{
		RenderComponent* rc = s_ui_render_components[entity];

		glDeleteBuffers(1, &rc->mesh.vbo);
		glDeleteBuffers(1, &rc->mesh.ibo);
		glDeleteVertexArrays(1, &rc->mesh.vao);

		glDeleteShader(rc->effect.vertex);
		glDeleteShader(rc->effect.fragment);
		glDeleteShader(rc->effect.program);
	}
}

void RenderingSystem::clear()
{
	level_entities.clear();
	menu_entities.clear();
}