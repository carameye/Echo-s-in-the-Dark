#include <iostream>
#include "level.hpp"
#include "torch.hpp"

using json = nlohmann::json;

namespace
{
    const size_t GHOST_DANGER_DIST = 500;
    const size_t COLLISION_SOUND_MIN_VEL = 5;
}

void Level::destroy()
{
	// clear all level-dependent resources
	for (auto& brick_element : m_brick_map) {
		delete brick_element.second;
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
    for (auto& torch : m_torches) {
        delete torch;
    }
	for (auto& bg : m_backgrounds) {
		delete bg;
	}

	clear_level_components();
	m_rendering_system.clear();
	m_interactable = NULL;
    m_brick_map.clear();
    m_ghosts.clear();
    m_interactables.clear();
    m_signs.clear();
    m_torches.clear();
	m_backgrounds.clear();
    m_rendering_system.destroy();
	m_light.destroy();
	m_robot.destroy();
}

void Level::draw_entities(const mat3 &projection, const vec2 &camera_shift) {
    vec3 headlight_channel = m_light.get_headlight_channel();
    m_rendering_system.render(projection, camera_shift, headlight_channel);
}

void Level::draw_light(const mat3 &projection, const vec2 &camera_shift) {
    m_light.draw(projection, camera_shift, {width, height}, m_torches);
}

void Level::update(float elapsed_ms) {

    SoundSystem* sound_system = SoundSystem::get_system();

    vec2 robot_pos = m_robot.get_position();
    vec2 robot_head_pos = m_robot.get_head_position();

    m_robot.update_velocity(elapsed_ms);

    if (m_has_colour_changed) {
        vec3 headlight_channel = m_light.get_headlight_channel();
        if (headlight_channel.x == 1.f && headlight_channel.y == 1.f && headlight_channel.z == 1.f) {
            m_graph = &m_white_graph;
        }
        if (headlight_channel.x == 1.f && headlight_channel.y == 0.f && headlight_channel.z == 0.f) {
            m_graph = &m_red_graph;
        }
        if (headlight_channel.x == 0.f && headlight_channel.y == 1.f && headlight_channel.z == 0.f) {
            m_graph = &m_green_graph;
        }
        if (headlight_channel.x == 0.f && headlight_channel.y == 0.f && headlight_channel.z == 1.f) {
            m_graph = &m_blue_graph;
        }
        for (auto &brick_element : m_brick_map) {
            Brick* i_brick = brick_element.second;
            i_brick->update(headlight_channel);
        }

        for (auto &i_ghost : m_ghosts) {
            i_ghost->set_level_graph(m_graph);
			i_ghost->set_path();
            i_ghost->update_is_chasing(headlight_channel);
        }
        m_has_colour_changed = false;
    }

    vec2 new_robot_pos = m_robot.get_next_position(elapsed_ms);
    vec2 new_robot_head_pos = m_robot.get_next_head_position(new_robot_pos);

    float translation = new_robot_pos.x - robot_pos.x;
    float translation_head = new_robot_head_pos.x - robot_head_pos.x;
    // get possible brick collision points after trying to move in x dir
    std::vector<vec2> possible_brick_collisions = get_brick_positions_around_pos({new_robot_pos.x, robot_pos.y});

    for (auto& pos : possible_brick_collisions) {
        Hitbox robot_hitbox_x = m_robot.get_hitbox();
        robot_hitbox_x.translate({translation, 0.f});
        Hitbox robot_head_hitbox_x = m_robot.get_head_hitbox();
        robot_head_hitbox_x.translate({ translation_head, 0.f});
        if (m_brick_map.find(pos) == m_brick_map.end()) {
            // pos was not in the brick map. Therefore, there is no brick at pos, so no collision possible
            continue;
        }
        Brick brick = *m_brick_map[pos];
        bool should_check_collisions = brick.get_is_collidable();
        if (should_check_collisions) {
            if (brick.get_hitbox().collides_with(robot_hitbox_x)) {
                vec2 vel = m_robot.get_velocity();
                if (std::abs(vel.x) >= COLLISION_SOUND_MIN_VEL) {
                    sound_system->play_sound_effect(Sound_Effects::collision);
                }
                m_robot.set_velocity({0.f, vel.y});

                float circle_width = brick_size / 2.f;
                if (abs(m_robot.get_position().y - brick.get_position().y) > brick_size / 2.f) {
                    float param = abs(m_robot.get_position().y - brick.get_position().y) - brick_size / 2.f;
                    float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
                    if (dist_no_sqrt >= 0.f)
                        circle_width = sqrt(dist_no_sqrt);
                }

                new_robot_pos.x = get_closest_point(robot_pos.x, brick.get_position().x, circle_width,
                                                    brick_size / 2.f);
                translation = new_robot_pos.x - robot_pos.x;
            }


            if (brick.get_hitbox().collides_with(robot_head_hitbox_x)) {
                vec2 head_vel = m_robot.get_head_velocity();
                if (std::abs(head_vel.x) >= COLLISION_SOUND_MIN_VEL) {
                    sound_system->play_sound_effect(Sound_Effects::collision);
                }
                m_robot.set_head_velocity({0.f, head_vel.y});

                float circle_width = brick_size / 2.f;
                if (abs(m_robot.get_head_position().y - brick.get_position().y) > brick_size / 2.f) {
                    float param = abs(m_robot.get_head_position().y - brick.get_position().y) - brick_size / 2.f;
                    float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
                    if (dist_no_sqrt >= 0.f)
                        circle_width = sqrt(dist_no_sqrt);
                }

                new_robot_head_pos.x = get_closest_point(robot_head_pos.x, brick.get_position().x, circle_width,
                                                         21);
                translation_head = new_robot_head_pos.x - robot_head_pos.x;
            }
        }
    }

    m_robot.set_position({new_robot_pos.x, robot_pos.y});
    m_robot.set_head_position({new_robot_head_pos.x, robot_head_pos.y});

    translation = new_robot_pos.y - robot_pos.y;
    translation_head = new_robot_head_pos.y - robot_head_pos.y;
    // get possible brick collision points after trying to move in y dir
    possible_brick_collisions = get_brick_positions_around_pos({robot_pos.x, new_robot_pos.y});

    for (auto& pos : possible_brick_collisions) {
        Hitbox robot_hitbox_y = m_robot.get_hitbox();
        robot_hitbox_y.translate({0.f, translation});
        Hitbox robot_head_hitbox_y = m_robot.get_head_hitbox();
        robot_head_hitbox_y.translate({0.f, translation_head });
        if (m_brick_map.find(pos) == m_brick_map.end()) {
            // pos was not in the brick map. Therefore, there is no brick at pos, so no collision possible
            continue;
        }
        Brick brick = *m_brick_map[pos];
        bool should_check_collisions = brick.get_is_collidable();
        if (should_check_collisions) {
            if (brick.get_hitbox().collides_with(robot_hitbox_y)) {
                vec2 vel = m_robot.get_velocity();
                if (std::abs(vel.y) >= COLLISION_SOUND_MIN_VEL) {
                    sound_system->play_sound_effect(Sound_Effects::collision);
                }
                m_robot.set_velocity({vel.x, 0.f});

                float circle_width = brick_size / 2.f;
                if (abs(m_robot.get_position().x - brick.get_position().x) > brick_size / 2.f) {
                    float param = abs(m_robot.get_position().x - brick.get_position().x) - brick_size / 2.f;
                    float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
                    if (dist_no_sqrt >= 0.f)
                        circle_width = sqrt(dist_no_sqrt);
                }

                new_robot_pos.y = get_closest_point(robot_pos.y, brick.get_position().y, circle_width,
                                                    brick_size / 2.f);
                translation = new_robot_pos.y - robot_pos.y;
                if (brick.get_position().y > new_robot_pos.y) {
                    m_robot.set_grounded();
                }
            }

            if (brick.get_hitbox().collides_with(robot_head_hitbox_y)) {
                vec2 head_vel = m_robot.get_head_velocity();
                if (std::abs(head_vel.y) >= COLLISION_SOUND_MIN_VEL) {
                    sound_system->play_sound_effect(Sound_Effects::collision);
                }
                m_robot.set_head_velocity({head_vel.x, 0.f});

                float circle_width = brick_size / 2.f;
                if (abs(m_robot.get_head_position().x - brick.get_position().x) > brick_size / 2.f) {
                    float param = abs(m_robot.get_head_position().x - brick.get_position().x) - brick_size / 2.f;
                    float dist_no_sqrt = pow(brick_size / 2.f, 2.f) - pow(param, 2.f);
                    if (dist_no_sqrt >= 0.f)
                        circle_width = sqrt(dist_no_sqrt);
                }

                new_robot_head_pos.y = get_closest_point(robot_head_pos.y, brick.get_position().y, circle_width,
                                                         21);
                translation_head = new_robot_head_pos.y - robot_head_pos.y;
            }
        }
        Music level_bgm = get_level_music();
        if (level_bgm != prev_bgm) {
            sound_system->play_bgm(level_bgm);
            prev_bgm = level_bgm;
        }
    }

    m_robot.set_position(new_robot_pos);
    m_robot.set_head_position(new_robot_head_pos);
    m_robot.update(elapsed_ms);
    m_light.set_position(new_robot_head_pos);

    m_robot.set_head_direction(m_light.get_direction());

    Hitbox new_robot_hitbox = m_robot.get_hitbox();

    for (auto &ghost : m_ghosts) {
        ghost->set_goal(m_robot.get_position());
        ghost->update(elapsed_ms);
        if (ghost->get_hitbox().collides_with(new_robot_hitbox)) {
            sound_system->play_sound_effect(Sound_Effects::robot_hurt);
            reset_level();
        }
    }

    for (auto &sign : m_signs) {
        if (sign->get_hitbox().collides_with(new_robot_hitbox))
            sign->show_text();
        else
            sign->hide_text();
    }

    const Hitbox robot_hitbox = m_robot.get_hitbox();
    // only check collision with interactable if there is no current interactable or if the current interactable
    // isn't being interacted with
    if (m_interactable == NULL || !m_interactable->get_hitbox().collides_with(robot_hitbox)) {
        m_interactable = NULL;
        for (auto &interactable : m_interactables) {
            if (interactable->get_hitbox().collides_with(robot_hitbox)) {
                m_interactable = interactable;
                break;
            }
        }
    }
}

void Level::update_background(float elapsed_ms, vec2 pos_diff)
{
	// Update background
	for (auto& background : m_backgrounds) {
		background->update(elapsed_ms, pos_diff);
	}
}

std::vector<vec2> Level::get_brick_positions_around_pos(vec2 pos) const {
    // round the pos to the nearest brick position
    pos = {floor(pos.x / brick_size) * brick_size, floor(pos.y / brick_size) * brick_size};

    // get the square of bricks around and at pos
    std::vector<vec2> brick_positions{
        pos,
        add(pos, {0, brick_size}), // brick under pos
        sub(pos, {0, brick_size}), // brick above pos
        add(pos, {brick_size, 0}), // brick to right of pos
        sub(pos, {brick_size, 0}), // brick to left of pos
        add(pos, {brick_size, brick_size}), // brick diagonal (down + right)
        sub(pos, {brick_size, brick_size}), // brick diagonal (up + left)
        add(pos, {brick_size, -brick_size}), // brick diagonal (up + right)
        add(pos, {-brick_size, brick_size}), // brick diagonal (down + left)
    };

    return brick_positions;
}

vec2 Level::get_starting_camera_position() const {
    return m_starting_camera_pos;
}

vec2 Level::get_player_position() const {
	return m_robot.get_position();
}

std::string Level::interact()
{
    if (m_interactable != NULL) {
        return m_interactable->perform_action();
    }

    return "";
}

bool Level::parse_level(std::string level, std::vector<std::string> unlocked, vec2 start_pos)
{
    m_level = level;

    // Construct file name with path
    std::string filename = level_path;
    filename.append(level);
    filename.append(".json");

    // Open file
    std::ifstream file(filename);
    if (!file.is_open()) {
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

    // Get first entity in this group
    int min = next_id;

	// Spawn background
	spawn_background();

    // Get the doors
    fprintf(stderr, "	getting doors\n");
    for (int i = 0; i < j["doors"].size(); i++) {
        json door = j["doors"][i];
        vec2 pos = {door["pos"]["x"], door["pos"]["y"]};
        if (i == 0) {
            m_starting_camera_pos = to_pixel_position(pos);
        }
        spawn_door(to_pixel_position(pos), door["next_level"]);
    }
    if (m_level == "level_select")
    {
        for (auto& d : m_interactables)
        {
            if (find(unlocked.begin(), unlocked.end(), d->get_destination()) == unlocked.end())
            {
                d->lock();
            }
        }
    }

    fprintf(stderr, "   getting torches\n");
    for (json torch : j["torches"])
    {
        vec2 pos = {torch["pos"]["x"], torch["pos"]["y"]};
        spawn_torch(to_pixel_position(pos));
    }

    // Get the signs
    fprintf(stderr, "	getting signs\n");
    for (json sign : j["signs"]) {
        vec2 pos = {sign["pos"]["x"], sign["pos"]["y"]};
        spawn_sign(to_pixel_position(pos), sign["text"]);
    }

    // Get the ghosts
    fprintf(stderr, "	getting ghosts\n");
    for (json ghost : j["ghosts"]) {
        vec2 pos = {ghost["pos"]["x"], ghost["pos"]["y"]};
        vec3 colour = {ghost["colour"]["r"], ghost["colour"]["g"], ghost["colour"]["b"]};
        spawn_ghost(to_pixel_position(pos), colour);
    }

    // Get the bricks
    fprintf(stderr, "	getting bricks\n");
    std::vector<vec2> potential_cp;
    std::vector<vec2> diffs = {{-1.f, -1.f},
                               {1.f,  -1.f},
                               {-1.f, 1.f},
                               {1.f,  1.f}};

    std::vector<bool> empty((int)width, false);
    std::vector<std::vector<bool>> bricks((int)height, empty);
    std::vector<std::vector<bool>> white_bricks((int)height, empty);
    std::vector<std::vector<bool>> red_bricks((int)height, empty);
    std::vector<std::vector<bool>> green_bricks((int)height, empty);
    std::vector<std::vector<bool>> blue_bricks((int)height, empty);

    for (json brick : j["bricks"]) {
        vec2 pos = {brick["pos"]["x"], brick["pos"]["y"]};
        vec3 colour = {brick["colour"]["r"], brick["colour"]["g"], brick["colour"]["b"]};

        // Set brick here
        bricks[(int)pos.y][(int)pos.x] = true;

        if (colour.x == 1.f && colour.y == 1.f && colour.z == 1.f) {
            white_bricks[(int)pos.y][(int)pos.x] = true;
            red_bricks[(int)pos.y][(int)pos.x] = true;
            green_bricks[(int)pos.y][(int)pos.x] = true;
            blue_bricks[(int)pos.y][(int)pos.x] = true;
        } else if (colour.x == 1.f && colour.y == 0.f && colour.z == 0.f) {
            red_bricks[(int)pos.y][(int)pos.x] = true;
        } else if (colour.x == 0.f && colour.y == 1.f && colour.z == 0.f) {
            green_bricks[(int)pos.y][(int)pos.x] = true;
        } else if (colour.x == 0.f && colour.y == 0.f && colour.z == 1.f) {
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
		(long unsigned int)m_brick_map.size());

    // Generate the graph
    if (m_ghosts.size() > 0)
    {
        m_white_graph.generate(potential_cp, white_bricks, (int)width, (int)height);
        m_red_graph.generate(potential_cp, red_bricks, (int)width, (int)height);
        m_green_graph.generate(potential_cp, green_bricks, (int)width, (int)height);
        m_blue_graph.generate(potential_cp, blue_bricks, (int)width, (int)height);
    }

    // Level graph initially set to be the default white
    m_graph = &m_white_graph;

    // Spawn the robot
    vec2 robot_pos = {j["spawn"]["pos"]["x"], j["spawn"]["pos"]["y"]};
	if (level == "level_select" && start_pos.x > -1.f && start_pos.y > -1.f) {
		robot_pos = to_grid_position(start_pos);
	}

	if (level == "level_select") {
		m_starting_camera_pos = to_pixel_position(robot_pos);
	}
    spawn_robot(to_pixel_position(robot_pos));

	for (auto& background : m_backgrounds) {
		background->set_position(to_pixel_position(robot_pos));
	}

    save_level();

    m_rendering_system.process(min, next_id);

	m_has_colour_changed = true;

    return true;
}

std::string Level::handle_key_press(int key, int action, std::unordered_map<int, int> &input_states)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
        input_states[key] = action;
		m_robot.start_flying();
	}
	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)) {
        input_states[key] = action;
		m_robot.set_is_accelerating_left(true);
	}
	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)) {
        input_states[key] = action;
		m_robot.set_is_accelerating_right(true);
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE) {
        input_states[key] = action;
        m_robot.stop_flying();
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)) {
        input_states[key] = action;
		m_robot.set_is_accelerating_left(false);
	}
	if (action == GLFW_RELEASE && (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)) {
        input_states[key] = action;
		m_robot.set_is_accelerating_right(false);
	}

    if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
        return interact();
    }

    // headlight toggle
    if (action == GLFW_PRESS && key == GLFW_KEY_1) {
        m_light.set_red_channel();
        m_has_colour_changed = true;
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_2) {
        m_light.set_green_channel();
        m_has_colour_changed = true;
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_3) {
        m_light.set_blue_channel();
        m_has_colour_changed = true;
    }

    return "";
}

void Level::handle_mouse_move(double xpos, double ypos, vec2 camera_pos)
{
    float mouse_x = (float) xpos;
    float mouse_y = (float) ypos;
	vec2 top_left = sub(camera_pos, { 600.f, 400.f });
	m_light.convert_mouse_pos_to_rad({ mouse_x, mouse_y }, sub(m_robot.get_head_position(), top_left));
}

void Level::handle_mouse_click(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        m_light.set_next_light_channel();
        m_has_colour_changed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        m_light.set_prev_light_channel();
        m_has_colour_changed = true;
    }
}

void Level::handle_mouse_scroll(double yoffset) {
    if (yoffset > 0) {
		if (!m_scroll_down) {
			m_scroll_down = true;
			m_scroll_amount = 0;
		}
		m_scroll_amount += yoffset;
		if (m_scroll_amount >= scroll_sensitivity) {
			m_scroll_amount = 0;
			m_light.set_next_light_channel();
			m_has_colour_changed = true;
		}
    } else {
		if (m_scroll_down) {
			m_scroll_down = false;
			m_scroll_amount = 0;
		}
		m_scroll_amount -= yoffset;
		if (m_scroll_amount >= scroll_sensitivity) {
			m_scroll_amount = 0;
			m_light.set_prev_light_channel();
			m_has_colour_changed = true;
		}
    }
}

std::string Level::get_current_level()
{
    return m_level;
}

bool Level::spawn_door(vec2 position, std::string next_level)
{
	Door *door = new Door();
	if (door->init(next_id++, position))
	{
		door->set_destination(next_level);
		m_interactables.push_back(door);
		return true;
	}
	fprintf(stderr, "	door spawn at (%f, %f) failed\n", position.x, position.y);
	return false;
}

bool Level::spawn_ghost(vec2 position, vec3 colour)
{
    Ghost *ghost = new Ghost();
    vec3 headlight_channel = m_light.get_headlight_channel();
    if (ghost->init(next_id++, colour, headlight_channel))
    {
        ghost->set_position(position);
        ghost->set_level_graph(m_graph);
        m_ghosts.push_back(ghost);
        return true;
    }
    return false;
}

bool Level::spawn_robot(vec2 position)
{
    if (m_robot.init(next_id, true))
    {
        next_id += 104;
        m_robot.set_position(position);
        m_robot.set_head_position(position);
        m_robot.set_shoulder_position(position);
        if (m_light.init(m_level)) {
            m_light.set_position(m_robot.get_head_position());
        }
        return true;
    }
    fprintf(stderr, "	robot spawn failed\n");
    return false;
}

bool Level::spawn_torch(vec2 position) {
    Torch *torch = new Torch();
    if (torch->init(next_id++))
    {
        torch->set_position(position);
        m_torches.push_back(torch);
        return true;
    }
    fprintf(stderr, "	torch spawn failed\n");
    return false;
}

bool Level::spawn_background()
{
	for (int i = 0; i < 4; i++) {
		float scale = 0.25f * i + 0.25f;
		Background* background = new Background();
		if (!background->init(next_id++, scale, scale))
		{
			return false;
		}
		// background uses total of 3 components
		m_backgrounds.push_back(background);
		next_id += 2;
	}
	return true;
}

bool Level::spawn_sign(vec2 position, std::string text)
{
    Sign *sign = new Sign();
    if (sign->init(next_id, text, position))
    {
        next_id += 2;
        m_signs.push_back(sign);
        return true;
    }
    fprintf(stderr, "	sign spawn failed\n");
    return false;
}

bool Level::spawn_brick(vec2 position, vec3 colour) {
    Brick *brick = new Brick();
    if (brick->init(next_id++, colour))
    {
        brick->set_position(position);
        m_brick_map.insert({position, brick});
        return true;
    }
    fprintf(stderr, "	brick spawn failed\n");
    return false;
}

void Level::save_level() {
    reset_positions.clear();
    reset_positions.push_back(m_robot.get_position());
    for (auto ghost : m_ghosts) {
        reset_positions.push_back(ghost->get_position());
    }
}

void Level::reset_level() {
    int pos_i = 0;
    m_robot.set_position(reset_positions[pos_i++]);
    for (auto &ghost : m_ghosts) {
        ghost->set_position(reset_positions[pos_i++]);
    }
}

float Level::get_min_ghost_distance() {
    float min_ghost_dist = INFINITY;
    for(auto& ghost : m_ghosts) {
        float dist = ghost->dist_from_goal();
        if (dist < min_ghost_dist) {
            min_ghost_dist = dist;
        }
    }
    return min_ghost_dist;
}

Music Level::get_level_music()
{
    // if the robot is near to a ghost, play the ghost bgm, otherwise, play the standard music
    float dist_from_ghost = get_min_ghost_distance();

    if (dist_from_ghost <= GHOST_DANGER_DIST) {
        return Music::ghost_approach;
    }
    return Music::standard;
}

vec2 Level::get_size()
{
	return { width, height };
}
