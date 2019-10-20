#include "level.hpp"

bool Level::init(std::string level)
{
    m_interactable = NULL;
	return parse_level(level) && m_light.init();
}

void Level::destroy()
{
    // clear all level-dependent resources
    for (auto& brick : m_bricks) {
        brick.destroy();
    }
    for (auto& interactable : m_interactables) {
		interactable.destroy();
    }
    for (auto& ghost : m_ghosts) {
        ghost.destroy();
    }
    m_robot.destroy();
    m_bricks.clear();
    m_ghosts.clear();
    m_interactables.clear();
}

void Level::draw_entities(const mat3& projection, const vec2& camera_shift)
{
	// Draw entities
	for (auto& brick : m_bricks) {
        brick.draw(projection, camera_shift);
    }
	for(auto& interactable : m_interactables) {
		interactable.draw(projection, camera_shift);
    }
	m_robot.draw(projection, camera_shift);
	for (auto& ghost : m_ghosts) {
        ghost.draw(projection, camera_shift);
    }
}

void Level::draw_light(const mat3& projection, const vec2& camera_shift)
{
	m_light.draw(projection, camera_shift);
}

void Level::update(float elapsed_ms)
{
	vec2 robot_pos = m_robot.get_position();

    m_robot.update_velocity(elapsed_ms);

    vec2 new_robot_vel = m_robot.get_velocity();
    vec2 new_robot_pos = m_robot.get_next_position();

	float translation = new_robot_vel.x;
	for (auto& i_brick : m_bricks) 
	{
		const auto& robot_hitbox_x = m_robot.get_hitbox({ translation, 0.f });
		Brick brick = i_brick;
		if (brick.get_hitbox().collides_with(robot_hitbox_x)) {
			m_robot.set_velocity({ 0.f, m_robot.get_velocity().y });

			float circle_width = brick_size / 2.f;
			if (abs(m_robot.get_position().y - brick.get_position().y) > brick_size / 2.f)
			{
				float param = abs(m_robot.get_position().y - brick.get_position().y) - brick_size / 2.f;
				float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
				if (dist_no_sqrt >= 0.f)
					circle_width = sqrt(dist_no_sqrt);
			}

			new_robot_pos.x = get_closest_point(robot_pos.x, brick.get_position().x, circle_width, brick_size / 2.f);
			translation = new_robot_pos.x - robot_pos.x;
		}
	}

	m_robot.set_position({ new_robot_pos.x, robot_pos.y });

	translation = new_robot_vel.y;
	for (auto& i_brick : m_bricks)
	{
		const auto& robot_hitbox_y = m_robot.get_hitbox({ 0.f, translation });
		Brick brick = i_brick;
		if (brick.get_hitbox().collides_with(robot_hitbox_y)) {
			m_robot.set_velocity({ m_robot.get_velocity().x, 0.f });

			float circle_width = brick_size / 2.f;
			if (abs(m_robot.get_position().x - brick.get_position().x) > brick_size / 2.f)
			{
				float param = abs(m_robot.get_position().x - brick.get_position().x) - brick_size / 2.f;
				float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
				if (dist_no_sqrt >= 0.f)
					circle_width = sqrt(dist_no_sqrt);
			}

			new_robot_pos.y = get_closest_point(robot_pos.y, brick.get_position().y, circle_width, brick_size / 2.f);
			translation = new_robot_pos.y -robot_pos.y;

			if (brick.get_position().y > new_robot_pos.y)
				m_robot.set_grounded();
		}
	}

	m_robot.set_position(new_robot_pos);
	m_robot.update(elapsed_ms);
	m_light.set_position(new_robot_pos);

	for (auto& ghost : m_ghosts)
	{
		ghost.set_goal(m_robot.get_position());
		ghost.update(elapsed_ms);
	}

	const Hitbox robot_hitbox = m_robot.get_hitbox({0.f, 0.f});
	// only check collision with interactable if there is no current interactable or if the current interactable
	// isn't being interacted with
	if (m_interactable == NULL || !m_interactable->get_hitbox().collides_with(robot_hitbox))
	{
		m_interactable = NULL;
		for (auto& interactable : m_interactables)
		{
			if (interactable.get_hitbox().collides_with(robot_hitbox)) {
				m_interactable = &interactable;
				break;
			}
		}
	}
}

vec2 Level::get_camera_position()
{
    return m_robot.get_position();
}

Robot* Level::get_player()
{
    return &m_robot;
}

void Level::interact()
{
    if (m_interactable != NULL) {
		m_interactable->perform_action(std::bind(&Level::parse_level, this, std::placeholders::_1));
	}
}

bool Level::parse_level(std::string level)
{
	fprintf(stderr, "parsing level\n");
	static std::map<char, vec3> colours;
	colours.clear();
	colours['B'] = { 1.f, 1.f, 1.f };
	colours['C'] = { 1.f, 0.f, 0.f };
	colours['M'] = { 0.f, 1.f, 0.f };
	colours['N'] = { 0.f, 0.f, 1.f };
	colours['Y'] = { 1.f, 1.f, 0.f };
	colours['Z'] = { 1.f, 0.f, 1.f };
	colours['L'] = { 0.f, 1.f, 1.f };

	std::string filename = level_path;
	filename.append(level);
	filename.append(".txt");
	std::ifstream file;
	file.open(filename);
	if (file.is_open())
	{
		fprintf(stderr, "Opened level file\n");

		// Destroy level-dependent resources
        destroy();

		float x = 0.f;
		float y = 0.f;
		std::string line;

		// Get ambient light level
		if (!getline(file, line))
			return false;
		m_light.set_ambient(std::stof(line));

		// Get the next levels file names
		if (!getline(file, line))
			return false;

		std::vector<std::string> doors;
		int door_i = 0;
		while (line.compare("enddoors") != 0)
		{
			doors.push_back(line);
			if (!getline(file, line))
				return false;
		}

		// Get the text from the signs in the order they will appear 
		// starting from top to bottom/left to right
		if (!getline(file, line))
			return false;

		std::vector<std::string> signs;
		int sign_i = 0;
		while (line.compare("endsigns") != 0)
		{
			signs.push_back(line);
			if (!getline(file, line))
				return false;
		}

		std::vector<std::string> brick_data;

		// Finally get the actual map data
		while (getline(file, line))
		{
			std::string row;

			for (x = 0.f; x < line.length(); x++)
			{
				vec2 position;
				position.x = x * brick_size;
				position.y = y * brick_size;
				switch (line[x])
				{
				case 'D':
					if (!spawn_door(position, doors[door_i++]))
						return false;
					row = row.append(" ");
					break;
				case 'G':
					if (!spawn_ghost(position))
						return false;
					row = row.append(" ");
					break;
				case 'R':
					if (!spawn_robot(position))
						return false;
					row = row.append(" ");
					break;
				case 'S':
					if (!spawn_sign(position, signs[sign_i++]))
						return false;
					row = row.append(" ");
					break;
				case 'T':
					m_light.add_torch(position);
					row = row.append(" ");
					break;
				case ' ':
					row = row.append(" ");
					break;
				default:
					row = row.append("B");
					if (colours.find(line[x]) == colours.end())
						return false;

					if (!spawn_brick(position, colours[line[x]]))
						return false;
				}
			}
			brick_data.push_back(row);
			y++;
		}

		fprintf(stderr, "Generating level graph\n");
		bool valid = m_graph.generate(brick_data);

		for (auto& g : m_ghosts)
		{
			g.set_level_graph(&m_graph);
		}

		return valid;
	}

	return false;
}

bool Level::spawn_door(vec2 position, std::string next_level)
{
	Door door;
	if (door.init())
	{
		door.set_position(position);
		door.set_destination(next_level);
		m_interactables.push_back(door);
		return true;
	}
	fprintf(stderr, "	door spawn at (%f, %f) failed\n", position.x, position.y);
	return false;
}

bool Level::spawn_ghost(vec2 position)
{
	Ghost ghost;
	if (ghost.init())
	{
		ghost.set_position(position);
		m_ghosts.push_back(ghost);
		return true;
	}
	return false;
}

bool Level::spawn_robot(vec2 position)
{
	if (m_robot.init())
	{
		m_robot.set_position(position);
		m_robot.set_head_position(position);
        m_robot.set_shoulder_position(position);
        if (m_light.init())
            m_light.set_position(m_robot.get_position());

		return true;
		// TODO: init light when robot is spawned
	}
	fprintf(stderr, "	robot spawn failed\n");
	return false;
}

bool Level::spawn_sign(vec2 position, std::string text)
{
	// TODO: add sign code
	fprintf(stderr, "	sign at (%f, %f) has text \"%s\"\n", position.x, position.y, text.c_str()); // remove once real code is done
	return true;
}

bool Level::spawn_brick(vec2 position, vec3 colour)
{
	// TODO: make bricks respond to different colours
	bool x = colour.x == 1.f;
	bool y = colour.y == 1.f;
	bool z = colour.z == 1.f;
	if (!x || !y || !z)
		fprintf(stderr, "	brick at (%f, %f)is coloured\n", position.x, position.y); // remove once real code is done

	Brick brick;
	if (brick.init())
	{
		brick.set_position(position);
		m_bricks.push_back(brick);
		return true;
	}
	fprintf(stderr, "	brick spawn failed\n");
	return false;
}
