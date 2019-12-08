#include <iostream>
#include "maker_level.hpp"
#include "bitmap_image.hpp"

using json = nlohmann::json;

static bool within_range(vec2 p1, vec2 p2, float range)
{
	bool x = p1.x > p2.x - range && p1.x < p2.x + range;
	bool y = p1.y > p2.y - range && p1.y < p2.y + range;
	return x && y;
}

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

vec2 MakerLevel::generate_starter()
{
	m_ot = ObjectType::brick;
	m_ot_selection = 0;

	min = next_id;

	spawn_robot({ 6.f * 64.f, height - 5 * 64.f });

	for (float x = 0.f; x < width; x += 64.f)
	{
		for (float y = 0.f; y < height; y += 64.f)
		{
			slots[(int)(x / 64.f)][(int)(y / 64.f)] = nullptr;
			if ((x == 0.f || x == width - 64.f) || (y == 0.f || y == height - 64.f))
			{
				spawn_brick({ x, y }, { 1.f, 1.f, 1.f });
				permanent[(int)(x / 64.f)][(int)(y / 64.f)] = true;
			}
			else
			{
				permanent[(int)(x / 64.f)][(int)(y / 64.f)] = false;
			}
		}
	}


	m_rendering_system.process(min, next_id);

	return m_robot.get_position();
}

vec2 MakerLevel::load_level()
{
	m_ot = ObjectType::brick;
	m_ot_selection = 0;

	min = next_id;

	for (float x = 0.f; x < width; x += 64.f)
	{
		for (float y = 0.f; y < height; y += 64.f)
		{
			if ((x == 0.f || x == width - 64.f) || (y == 0.f || y == height - 64.f))
			{
				permanent[(int)(x / 64.f)][(int)(y / 64.f)] = true;
			}
			else
			{
				permanent[(int)(x / 64.f)][(int)(y / 64.f)] = false;
			}
			slots[(int)(x / 64.f)][(int)(y / 64.f)] = nullptr;
		}
	}
	std::ifstream file(maker_file);
	if (!file.is_open()) {
		destroy();
		generate_starter();
		return { 0.f, 0.f };
	}
	fprintf(stderr, "Opened level file\n");

	// clear all level-dependent resources
	destroy();

	// Parse the json
	json j = json::parse(file);

	width = (float) j["size"]["width"] * 64.f;
	height = (float) j["size"]["height"] * 64.f;

	// Get first entity in this group
	int min = next_id;

	// Get the doors
	fprintf(stderr, "	getting doors\n");
	for (int i = 0; i < j["doors"].size(); i++) {
		json door = j["doors"][i];
		vec2 pos = { door["pos"]["x"], door["pos"]["y"] };
		spawn_door(to_pixel_position(pos), door["next_level"]);
	}

	fprintf(stderr, "   getting torches\n");
	for (json torch : j["torches"])
	{
		vec2 pos = { torch["pos"]["x"], torch["pos"]["y"] };
		spawn_torch(to_pixel_position(pos));
	}

	// Get the ghosts
	fprintf(stderr, "	getting ghosts\n");
	for (json ghost : j["ghosts"]) {
		vec2 pos = { ghost["pos"]["x"], ghost["pos"]["y"] };
		vec3 colour = { ghost["colour"]["r"], ghost["colour"]["g"], ghost["colour"]["b"] };
		spawn_ghost(to_pixel_position(pos), colour);
	}

	// Get the bricks
	fprintf(stderr, "	getting bricks\n");
	std::vector<vec2> potential_cp;
	std::vector<vec2> diffs = { {-1.f, -1.f},
							   {1.f,  -1.f},
							   {-1.f, 1.f},
							   {1.f,  1.f} };

	std::vector<bool> empty((int)width, false);
	std::vector<std::vector<bool>> bricks((int)height, empty);
	std::vector<std::vector<bool>> white_bricks((int)height, empty);
	std::vector<std::vector<bool>> red_bricks((int)height, empty);
	std::vector<std::vector<bool>> green_bricks((int)height, empty);
	std::vector<std::vector<bool>> blue_bricks((int)height, empty);

	for (json brick : j["bricks"]) {
		vec2 pos = { brick["pos"]["x"], brick["pos"]["y"] };
		vec3 colour = { brick["colour"]["r"], brick["colour"]["g"], brick["colour"]["b"] };

		// Set brick here
		bricks[(int)pos.y][(int)pos.x] = true;

		if (colour.x == 1.f && colour.y == 1.f && colour.z == 1.f) {
			white_bricks[(int)pos.y][(int)pos.x] = true;
			red_bricks[(int)pos.y][(int)pos.x] = true;
			green_bricks[(int)pos.y][(int)pos.x] = true;
			blue_bricks[(int)pos.y][(int)pos.x] = true;
		}
		else if (colour.x == 1.f && colour.y == 0.f && colour.z == 0.f) {
			red_bricks[(int)pos.y][(int)pos.x] = true;
		}
		else if (colour.x == 0.f && colour.y == 1.f && colour.z == 0.f) {
			green_bricks[(int)pos.y][(int)pos.x] = true;
		}
		else if (colour.x == 0.f && colour.y == 0.f && colour.z == 1.f) {
			blue_bricks[(int)pos.y][(int)pos.x] = true;
		}

		// Add brick to critical points if not already cancelled
		for (vec2 diff : diffs) {
			vec2 pot = add(pos, diff);
			if (pot.x >= 0.f && pot.x < width && pot.y >= 0.f && pot.y < height) {
				potential_cp.push_back(pot);
			}
		}

		spawn_brick(to_pixel_position(pos), colour);
	}

	fprintf(stderr, "	built world with %lu doors, %lu ghosts, and %lu bricks\n",
		(long unsigned int)m_interactables.size(), (long unsigned int)m_ghosts.size(), 
		(long unsigned int)m_bricks.size());

	// Spawn the robot
	vec2 robot_pos = { j["spawn"]["pos"]["x"], j["spawn"]["pos"]["y"] };
	spawn_robot(to_pixel_position(robot_pos));

	m_rendering_system.process(min, next_id);

	return m_robot.get_position();
}

void MakerLevel::draw_entities(const mat3& projection, const vec2& camera_shift) 
{
	m_rendering_system.render(projection, camera_shift, { 1.f, 1.f, 1.f });
}

void MakerLevel::handle_key_press(int key, int action)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {
		m_ot_selection = (m_ot_selection + 1) % 5;
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
}

void MakerLevel::handle_mouse_click(double xpos, double ypos, vec2 camera, bool left)
{
	float x = (float)xpos + camera.x - 600.f + brick_size / 2.f;
	float y = (float)ypos + camera.y - 400.f + brick_size / 2.f;
	vec2 position = { x - fmod(x, 64.f) , y - fmod(y, 64.f) };

	if (position.x < 0.f || position.x > width || position.y < 0.f || position.y > height)
	{
		return;
	}

	int start = next_id;

	if (m_hover_object_is_spawned && left)
	{
		m_hover_object_is_spawned = false;
		return;
	}

	if (left) 
	{
		switch (m_ot)
		{
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
		case ObjectType::robot:
		{
			if (valid_robot_position(position))
			{
				slots[(int)(m_robot_position.x / 64.f)][(int)(m_robot_position.y / 64.f)] = nullptr;
				m_robot_position = position;
				slots[(int)(m_robot_position.x / 64.f)][(int)(m_robot_position.y / 64.f)] = &m_robot;
			}
			break;
		}
		default:
			break;
		}

		for (int i = start; i < next_id; i++)
		{
			m_rendering_system.add(i);
		}
	}
	else if (!m_hover_object_is_spawned)
	{
		delete_object(position);
	}
}

void MakerLevel::handle_mouse_move(float xpos, float ypos, vec2 camera_pos)
{
	float x = xpos + camera_pos.x - 600.f + brick_size / 2.f;
	float y = ypos + camera_pos.y - 400.f + brick_size / 2.f;
	vec2 position = { x - fmod(x, 64.f) , y - fmod(y, 64.f) };
	refresh_hover_object(position.x, position.y);
}

void MakerLevel::refresh_hover_object(float x, float y)
{
	vec2 position = { x, y };

	if (m_hover_object_is_spawned)
	{
		delete_object(m_hover_object_position);
	}

	m_robot.set_position(m_robot_position);
	m_hover_object_position = position;

	switch (m_ot)
	{
	case ObjectType::brick:
		m_hover_object_is_spawned = spawn_brick(position, m_color);
		break;
	case ObjectType::torch:
		m_hover_object_is_spawned = spawn_torch(position);
		break;
	case ObjectType::door:
		m_hover_object_is_spawned = spawn_door(position, "complete");
		break;
	case ObjectType::ghost:
		m_hover_object_is_spawned = spawn_ghost(position, m_color);
		break;
	case ObjectType::robot:
		m_hover_object_is_spawned = false;	
		if (valid_robot_position(position))
		{
			m_robot.set_position(position);
		}
	default:
		break;
	}

	m_lot = m_ot;

	if (m_hover_object_is_spawned)
	{
		m_rendering_system.add(next_id - 1);
	}
}

void MakerLevel::process()
{
	if (m_hover_object_is_spawned)
	{
		delete_object(m_hover_object_position);
	}

	json j = {};
	j["size"]["width"] = (int)(width / 64.f);
	j["size"]["height"] = (int)(height / 64.f);
	j["ambient_light"] = 0.0;
	j["spawn"]["pos"]["x"] = (int)(m_robot.get_position().x / 64.f);
	j["spawn"]["pos"]["y"] = (int)(m_robot.get_position().y / 64.f);
	j["signs"] = json::array();
	std::vector<json> doors;
	std::vector<json> ghosts;
	std::vector<json> bricks;
	std::vector<json> torches;

	for (auto i : m_interactables)
	{
		json door;
		door["pos"]["x"] = (int)(i->get_position().x / 64.f);
		door["pos"]["y"] = (int)(i->get_position().y / 64.f);
		door["next_level"] = i->get_destination();
		doors.push_back(door);
	}

	for (auto i : m_ghosts)
	{
		json ghost;
		ghost["pos"]["x"] = (int)(i->get_position().x / 64.f);
		ghost["pos"]["y"] = (int)(i->get_position().y / 64.f);
		ghost["colour"]["r"] = i->get_colour().x;
		ghost["colour"]["g"] = i->get_colour().y;
		ghost["colour"]["b"] = i->get_colour().z;
		ghosts.push_back(ghost);
	}

	for (auto i : m_bricks)
	{
		json brick;
		brick["pos"]["x"] = (int)(i->get_position().x / 64.f);
		brick["pos"]["y"] = (int)(i->get_position().y / 64.f);
		brick["colour"]["r"] = i->get_colour().x;
		brick["colour"]["g"] = i->get_colour().y;
		brick["colour"]["b"] = i->get_colour().z;
		bricks.push_back(brick);
	}

	for (auto i : m_torches)
	{
		json torch;
		torch["pos"]["x"] = (int)(i->get_position().x / 64.f);
		torch["pos"]["y"] = (int)(i->get_position().y / 64.f);
		torches.push_back(torch);
	}

	j["doors"] = json(doors);
	j["ghosts"] = json(ghosts);
	j["bricks"] = json(bricks);
	j["torches"] = json(torches);

	std::ofstream o(maker_file);
	if (o.is_open())
	{
		o << j.dump() << std::endl;
		o.close();
	}

	bitmap_image image(40 * 64, 40 * 64);
	image.set_all_channels(255, 255, 255);
	rgb_t black = make_colour(0, 0, 0);

	for (auto b : m_bricks)
	{
		vec3 colour = b->get_colour();
		if (colour.x == 0.f || colour.y == 0.f || colour.z == 0.f)
		{
			continue;
		}

		int startx = (int)b->get_position().x;
		int starty = (int)b->get_position().y;
		for (int i = 0; i < 64; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				image.set_pixel(startx + i, starty + j, black);
			}
		}
	}

	image.save_image(maker_shadow);
	image.clear();
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
	bool found = false;

	if (e == &m_robot)
	{
		return true;
	}

	if (!found && !m_ghosts.empty()) {
		if (e == m_ghosts.back())
		{
			m_ghosts.pop_back();
			found = true;
		}
		else
		{
			auto it_g = std::find(m_ghosts.begin(), m_ghosts.end(), e);
			if (it_g != m_ghosts.end())
			{
				m_ghosts.erase(it_g);
				found = true;
			}
		}
	}

	if (!found && !m_interactables.empty())
	{
		if (e == m_interactables.back())
		{
			m_interactables.pop_back();
			found = true;
		}
		else
		{
			auto it_i = std::find(m_interactables.begin(), m_interactables.end(), e);
			if (it_i != m_interactables.end())
			{
				m_interactables.erase(it_i);
				found = true;
			}
		}
	}

	if (!found && !m_torches.empty())
	{
		if (e == m_torches.back())
		{
			clean = false;
			m_torches.pop_back();
			found = true;
		}
		else
		{
			auto it_t = std::find(m_torches.begin(), m_torches.end(), e);
			if (it_t != m_torches.end())
			{
				clean = false;
				m_torches.erase(it_t);
				found = true;
			}
		}
	}

	if (!found && !m_bricks.empty())
	{
		if (e == m_bricks.back())
		{
			clean = false;
			m_bricks.pop_back();
			found = true;
		}
		else
		{
			auto it_b = std::find(m_bricks.begin(), m_bricks.end(), e);
			if (it_b != m_bricks.end())
			{
				clean = false;
				m_bricks.erase(it_b);
				found = true;
			}
		}
	}

	if (found)
	{
		m_rendering_system.remove(id, clean);
	}

	return true;
}

bool MakerLevel::valid_robot_position(vec2 position)
{
	int x = (int)(position.x / 64.f);
	int y = (int)(position.y / 64.f);
	for (int i = x - 3; i < x + 4; i++)
	{
		for (int j = y - 3; j < y + 4; j++)
		{
			if (i >= 0 && i < width && j >= 0 && j < height && slots[i][j] != nullptr &&
				(std::find(m_bricks.begin(), m_bricks.end(), slots[i][j]) != m_bricks.end() ||
					std::find(m_ghosts.begin(), m_ghosts.end(), slots[i][j]) != m_ghosts.end()))
			{
				return false;
			}
		}
	}

	return true;
}

bool MakerLevel::valid_torch_position(vec2 position)
{
	int x = (int)(position.x / 64.f);
	int y = (int)(position.y / 64.f);
	for (int i = x - 7; i < x + 8; i++)
	{
		for (int j = y - 7; j < y + 8; j++)
		{
			if (i >= 0 && i < width && j >= 0 && j < height && slots[i][j] != nullptr &&
				std::find(m_torches.begin(), m_torches.end(), slots[i][j]) != m_torches.end())
			{
				return false;
			}
		}
	}

	return true;
}

bool MakerLevel::spawn_door(vec2 position, std::string next_level)
{
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr ||
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f) + 1] != nullptr ||
		position.x < 0.f || position.x > width || position.y < 0.f || position.y > height)
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
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr ||
		position.x < 0.f || position.x > width || position.y < 0.f || position.y > height ||
		within_range(position, m_robot.get_position(), 4.f * 64.f) || m_ghosts.size() > 4)
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
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr ||
		position.x < 0.f || position.x > width || position.y < 0.f || position.y > height)
	{
		return false;
	}

	if (m_robot.init(next_id, false))
	{
		m_robot_position = position;
		next_id += 104;
		m_robot.set_position(position);
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] = &m_robot;
		return true;
	}
	fprintf(stderr, "	robot spawn failed\n");
	return false;
}

bool MakerLevel::spawn_torch(vec2 position) 
{
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr ||
		position.x < 0.f || position.x > width || position.y < 0.f || position.y > height ||
		!valid_torch_position(position))
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

bool MakerLevel::spawn_brick(vec2 position, vec3 colour) 
{
	if (slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] != nullptr ||
		position.x < 0.f || position.x > width || position.y < 0.f || position.y > height ||
		within_range(position, m_robot.get_position(), 4.f * 64.f))
	{
		return false;
	}

	Brick* brick = new Brick();
	if (brick->init(next_id++, colour))
	{
		brick->set_position(position);
		m_bricks.push_back(brick);
		slots[(int)(position.x / 64.f)][(int)(position.y / 64.f)] = brick;
		brick->update(colour);
		return true;
	}
	fprintf(stderr, "	brick spawn failed\n");
	return false;
}
