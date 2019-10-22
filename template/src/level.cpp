#include <iostream>
#include "level.hpp"

using json = nlohmann::json;
static int next_entity_id = 0;

bool Level::init(std::string level)
{
    m_interactable = NULL;
	s_motion_components.clear();
	s_render_components.clear();
	m_rendering_system.clear();
	return parse_level(level) && m_light.init();
}

void Level::destroy()
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
	for (auto& sign : m_signs) {
		delete sign;
	}
	s_render_components.clear();
	s_motion_components.clear();
	m_rendering_system.clear();
	m_interactable = NULL;
    m_bricks.clear();
    m_ghosts.clear();
    m_interactables.clear();
	m_signs.clear();
	m_light.clear_torches();
}

void Level::draw_entities(const mat3& projection, const vec2& camera_shift)
{
	m_rendering_system.render(projection, camera_shift);
}

void Level::draw_light(const mat3& projection, const vec2& camera_shift)
{
	m_light.draw(projection, camera_shift, { width, height });
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
		Brick brick = *i_brick;
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
		Brick brick = *i_brick;
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


    // TODO: init light when robot is spawned
    bool isHeadFacingRight = m_robot.get_head_direction();
    bool isLightFacingRight = m_light.get_direction();
    // if head and light are facing different directions
    if (isHeadFacingRight != isLightFacingRight) {
        m_light.set_direction();
    }

	Hitbox new_robot_hitbox = m_robot.get_hitbox({ 0.f, 0.f });

	for (auto& ghost : m_ghosts)
	{
		ghost->set_goal(m_robot.get_position());
		ghost->update(elapsed_ms);
		if (ghost->get_hitbox().collides_with(new_robot_hitbox))
		{
			reset_level();
		}
	}

	for (auto& sign : m_signs)
	{
		 if (sign->get_hitbox().collides_with(new_robot_hitbox))
			sign->show_text();
		 else
			sign->hide_text();
	}

	const Hitbox robot_hitbox = m_robot.get_hitbox({0.f, 0.f});
	// only check collision with interactable if there is no current interactable or if the current interactable
	// isn't being interacted with
	if (m_interactable == NULL || !m_interactable->get_hitbox().collides_with(robot_hitbox))
	{
		m_interactable = NULL;
		for (auto& interactable : m_interactables)
		{
			if (interactable->get_hitbox().collides_with(robot_hitbox)) {
				m_interactable = interactable;
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
	// Construct file name with path
	std::string filename = level_path;
	filename.append(level);
	filename.append(".json");

	// Open file
	std::ifstream file(filename);
	if (!file.is_open())
	{
		return false;
	}
	fprintf(stderr, "Opened level file\n");

	// clear all level-dependent resources
	destroy();

	// Parse the json
	json j = json::parse(file);

	width = j["size"]["width"];
	height = j["size"]["height"];

	// Get ambient light level
	m_light.set_ambient(j["ambient_light"]);

	// Get the doors
	fprintf(stderr, "	getting doors\n");
	for (json door : j["doors"])
	{
		vec2 pos = { door["pos"]["x"], door["pos"]["y"] };
		spawn_door(to_pixel_position(pos), door["next_level"]);
	}

	fprintf(stderr, "   getting torches\n");
	for (json torch : j["torches"])
    {
	    vec2 pos = {torch["pos"]["x"], torch["pos"]["y"]};
	    m_light.add_torch(to_pixel_position(pos));
    }

	// Get the signs
	fprintf(stderr, "	getting signs\n");
	for (json sign : j["signs"])
	{
		vec2 pos = { sign["pos"]["x"], sign["pos"]["y"] };
		spawn_sign(to_pixel_position(pos), sign["text"]);
	}

	// Get the ghosts
	fprintf(stderr, "	getting ghosts\n");
	for (json ghost : j["ghosts"])
	{
		vec2 pos = { ghost["pos"]["x"], ghost["pos"]["y"] };
		spawn_ghost(to_pixel_position(pos));
	}

	// Get the bricks
	fprintf(stderr, "	getting bricks\n");
	std::vector<vec2> potential_cp;
	std::vector<vec2> diffs = { { -1.f, -1.f }, { 1.f, -1.f }, { -1.f, 1.f }, { 1.f, 1.f } };

	std::vector<bool> empty(width, false);
	std::vector<std::vector<bool>> bricks(height, empty);

	for (json brick : j["bricks"])
	{
		vec2 pos = { brick["pos"]["x"], brick["pos"]["y"] };
		vec3 colour = { brick["colour"]["r"], brick["colour"]["g"], brick["colour"]["b"] };

		// Set brick here
		bricks[pos.y][pos.x] = true;

		// Add brick to critical points if not already cancelled
		for (vec2 diff : diffs)
		{
			vec2 pot = add(pos, diff);
			if (pot.x >= 0.f && pot.x < width && pot.y >= 0.f && pot.y < height)
			{
				potential_cp.push_back(pot);
			}
		}

		spawn_brick(to_pixel_position(pos), colour);
	}

	fprintf(stderr, "	built world with %ld doors, %ld ghosts, and %ld bricks\n",
		m_interactables.size(), m_ghosts.size(), m_bricks.size());

	// Generate the graph
	m_graph.generate(potential_cp, bricks, width, height);

	// Spawn the robot
	vec2 pos = { j["spawn"]["pos"]["x"], j["spawn"]["pos"]["y"] };
	spawn_robot(to_pixel_position(pos));

	save_level();

	m_rendering_system.process(next_entity_id);

	return true;
}

bool Level::spawn_door(vec2 position, std::string next_level)
{
	Door *door = new Door();
	if (door->init(next_entity_id++))
	{
		door->set_position(position);
		door->set_destination(next_level);
		m_interactables.push_back(door);
		return true;
	}
	fprintf(stderr, "	door spawn at (%f, %f) failed\n", position.x, position.y);
	return false;
}

bool Level::spawn_ghost(vec2 position)
{
	Ghost *ghost = new Ghost();
	if (ghost->init(next_entity_id++))
	{
		ghost->set_position(position);
		ghost->set_level_graph(&m_graph);
		m_ghosts.push_back(ghost);
		return true;
	}
	return false;
}

bool Level::spawn_robot(vec2 position)
{
	if (m_robot.init(next_entity_id))
	{
		next_entity_id += 104;
		m_robot.set_position(position);
		m_robot.set_head_position(position);
        m_robot.set_shoulder_position(position);
        if (m_light.init()) {
            m_light.set_position(m_robot.get_position());

//            // TODO: init light when robot is spawned
//            bool isHeadFacingRight = m_robot.get_head_direction();
//            bool isLightFacingRight = m_light.get_direction();
//            // if head and light are facing different directions
//            if (isHeadFacingRight != isLightFacingRight) {
//                std::cout << 1 << std::endl;
//                m_light.set_direction();
//            }
        }
        return true;
	}
	fprintf(stderr, "	robot spawn failed\n");
	return false;
}

bool Level::spawn_sign(vec2 position, std::string text)
{
	Sign *sign = new Sign();
	if (sign->init(next_entity_id, text, position))
	{
		next_entity_id += 2;
		m_signs.push_back(sign);
		return true;
	}
	fprintf(stderr, "	sign spawn failed\n");
	return false;
}

bool Level::spawn_brick(vec2 position, vec3 colour)
{
	// TODO: make bricks respond to different colours
	bool x = colour.x == 1.f;
	bool y = colour.y == 1.f;
	bool z = colour.z == 1.f;
	if (!x || !y || !z)
		fprintf(stderr, "	brick at (%f, %f)is coloured\n", position.x, position.y); // remove once real code is done

	Brick *brick = new Brick();
	if (brick->init(next_entity_id++))
	{
		brick->set_position(position);
		m_bricks.push_back(brick);
		return true;
	}
	fprintf(stderr, "	brick spawn failed\n");
	return false;
}

void Level::save_level()
{
	reset_positions.clear();
	reset_positions.push_back(m_robot.get_position());
	for (auto ghost : m_ghosts)
	{
		reset_positions.push_back(ghost->get_position());
	}
}

void Level::reset_level()
{
	int pos_i = 0;
	m_robot.set_position(reset_positions[pos_i++]);
	for (auto& ghost : m_ghosts)
	{
		ghost->set_position(reset_positions[pos_i++]);
	}
}

Light* Level::get_light() {
    return &m_light;
}