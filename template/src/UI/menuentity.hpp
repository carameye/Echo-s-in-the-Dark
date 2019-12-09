#pragma once

#include "common.hpp"
#include "components.hpp"

class MenuEntity : public Entity
{
public:
	// Initialize
	virtual bool init(int id, vec2 position);

	// Set the size
	void set_size(vec2 size);

protected:
	// ID for ECS
	int m_id;

	// Size for mouse checking
	float width, height;

	// Components for rendering
	RenderComponent m_rc;
	MotionComponent m_mc;

	// Name of image
	std::string m_texture_name;
};

class Button : public MenuEntity
{
public:
	// Initialize
	bool init(int id, vec2 position) override;

	// Set whether mouse is hovering over
	void hover(vec2 position);

	// Return true if clicking on this button
	bool is_click();

	// Returns the status from clicking this button
	Status click();

	// Set name of image
	void set_texture_name(std::string name);

	// Set return status
	void set_status(Status status);

private:
	Status m_status;
	bool m_hover = false;
	Texture button_texture;
};