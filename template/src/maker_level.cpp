#include <iostream>
#include "maker_level.hpp"

using json = nlohmann::json;

void MakerLevel::destroy()
{
	// clear all level-dependent resources
	for (auto& brick : m_bricks) {
		delete brick;
	}
	for (auto& interactable : m_interactables) {
		delete interactable;
	}
	for (auto& ghost : m_ghosts) {
		delete ghost;
	}
	for (auto& torch : m_torches) {
		delete torch;
	}

	clear_level_components();
	m_rendering_system.clear();
	m_bricks.clear();
	m_ghosts.clear();
	m_interactables.clear();
	m_torches.clear();
	m_rendering_system.destroy();
}

void MakerLevel::generate_starter()
{
	m_j = {};
	for (int i = 0; i < 40; i++)
	{
		for (int j = 0; j < 40; j++)
		{
			permanent[i][j] = false;
			slots[i][j] = nullptr;
		}
	}

	min = next_id;

	spawn_door({ 4.f * 64.f, height - 3 * 64.f }, "quit");
	permanent[4][37] = true;
	permanent[4][38] = true;
	spawn_robot({ 6.f * 64.f, height - 4 * 64.f });
	permanent[6][36] = true;

	for (float x = 0.f; x < width; x += 64.f)
	{
		for (float y = 0.f; y < height; y += 64.f)
		{
			if ((x == 0.f || x == width - 64.f) || (y == 0.f || y == height - 64.f))
			{
				spawn_brick({ x, y }, { 1.f, 1.f, 1.f });
				permanent[(int)(x / 64.f)][(int)(y / 64.f)] = true;
			}
		}
	}

	m_rendering_system.process(min, next_id);
}

void MakerLevel::draw_entities(const mat3& projection, const vec2& camera_shift) 
{
	m_rendering_system.render(projection, camera_shift, { 1.f, 1.f, 1.f });
}

std::string MakerLevel::handle_key_press(int key, int action)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_E) {
		m_ot_selection = (m_ot_selection + 1) % 5;
		m_ot = (ObjectType)m_ot_selection;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_Q) {
		m_ot_selection = (m_ot_selection - 1) % 5;
		m_ot = (ObjectType)m_ot_selection;
	}

	// light toggle
	if (action == GLFW_PRESS && key == GLFW_KEY_1) {
		if (m_color.x == 0.f) {
			m_color = { 1.f, 0.f, 0.f };
		} else if (m_color.y == 0.f) {
			m_color = { 1.f, 1.f, 1.f };
		} else {
			m_color = { 1.f, 0.f, 0.f };
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_2) {
		if (m_color.y == 0.f) {
			m_color = { 0.f, 1.f, 0.f };
		} else if (m_color.x == 0.f) {
			m_color = { 1.f, 1.f, 1.f };
		} else {
			m_color = { 0.f, 1.f, 0.f };
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_3) {
		if (m_color.z == 0.f) {
			m_color = { 0.f, 0.f, 1.f };
		} else if (m_color.y == 0.f) {
			m_color = { 1.f, 1.f, 1.f };
		} else {
			m_color = { 0.f, 0.f, 1.f };
		}
	}

	return "";
}

void MakerLevel::handle_mouse_click(double xpos, double ypos, vec2 camera)
{
	float x = xpos + camera.x - 600.f + brick_size / 2.f;
	float y = ypos + camera.y - 400.f + brick_size / 2.f;
	vec2 position = { x - fmod(x, 64.f) , y - fmod(y, 64.f) };

	if (position.x < 0.f || position.x > width || position.y < 0.f || position.y > height)
	{
		return;
	}

	int start = next_id;

	switch (m_ot)
	{
	case ObjectType::del:
		delete_object(position);
		break;
	case ObjectType::brick:
		spawn_brick(position, m_color);
		break;
	case ObjectType::torch:
		spawn_torch(position);
		break;
	case ObjectType::door:
		spawn_door(position, "complete");
		break;
	case ObjectType::ghost:
		spawn_ghost(position, m_color);
		break;
	default:
		break;
	}
	
	for (int i = start; i < next_id; i++)
	{
		m_rendering_system.add(i);
	}
}

bool MakerLevel::spawn_door(vec2 position, std::string next_level)
{
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr ||
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f) + 1] != nullptr)
	{
		return false;
	}

	Door* door = new Door();
	if (door->init(next_id++, position))
	{
		door->set_destination(next_level);
		m_interactables.push_back(door);
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] = door;
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f) + 1] = door;
		return true;
	}
	fprintf(stderr, "	door spawn at (%f, %f) failed\n", position.x, position.y);
	return false;
}

bool MakerLevel::spawn_ghost(vec2 position, vec3 colour)
{
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr)
	{
		return false;
	}

	Ghost* ghost = new Ghost();
	if (ghost->init(next_id++, colour, colour))
	{
		ghost->set_position(position);
		m_ghosts.push_back(ghost);
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] = ghost;
		return true;
	}
	return false;
}

bool MakerLevel::spawn_robot(vec2 position)
{
	if (m_robot.init(next_id))
	{
		next_id += 104;
		m_robot.set_position(position);
		m_robot.set_head_position(position);
		m_robot.set_shoulder_position(position);
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] = &m_robot;
		return true;
	}
	fprintf(stderr, "	robot spawn failed\n");
	return false;
}

bool MakerLevel::spawn_torch(vec2 position) 
{
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr)
	{
		return false;
	}

	Torch* torch = new Torch();
	if (torch->init(next_id++))
	{
		torch->set_position(position);
		m_torches.push_back(torch);
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] = torch;
		return true;
	}
	fprintf(stderr, "	torch spawn failed\n");
	return false;
}

bool MakerLevel::delete_object(vec2 position)
{
	int x = (int)(position.x / 64.f);
	int y = (int)(position.y / 64.f);

	Entity* e = slots[x][y];

	if (permanent[x][y] || e == nullptr)
	{
		return false;
	}

	int id = e->m_id;
	slots[x][y] = nullptr;

	if (slots[x][y - 1] == e)
	{
		slots[x][y - 1] = nullptr;
	}
	if (slots[x][y + 1] == e)
	{
		slots[x][y + 1] = nullptr;
	}

	bool clean = true;

	auto it_b = std::find(m_bricks.begin(), m_bricks.end(), e);
	if (it_b != m_bricks.end())
	{
		clean = false;
		m_bricks.erase(it_b);
	}

	auto it_g = std::find(m_ghosts.begin(), m_ghosts.end(), e);
	if (it_g != m_ghosts.end())
	{
		m_ghosts.erase(it_g);
	}

	auto it_i = std::find(m_interactables.begin(), m_interactables.end(), e);
	if (it_i != m_interactables.end())
	{
		m_interactables.erase(it_i);
	}

	auto it_t = std::find(m_torches.begin(), m_torches.end(), e);
	if (it_t != m_torches.end())
	{
		m_torches.erase(it_t);
	}

	m_rendering_system.remove(id, clean);

	return true;
}

bool MakerLevel::spawn_brick(vec2 position, vec3 colour) 
{
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr)
	{
		return false;
	}

	Brick* brick = new Brick();
	if (brick->init(next_id++, colour))
	{
		brick->set_position(position);
		m_bricks.push_back(brick);
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] = brick;
		return true;
	}
	fprintf(stderr, "	brick spawn failed\n");
	return false;
}
