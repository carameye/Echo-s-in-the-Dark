#include "door.hpp"

Texture Door::s_door_texture;

bool Door::init(int id)
{
    if (!s_door_texture.is_valid())
	{
        if (!s_door_texture.load_from_file(textures_path("door.png")))
		{
			std::fprintf(stderr, "Failed to load door texture!");
			return false;
		}
	}

    rc.texture = &s_door_texture;

    if (!rc.init_sprite())
        return false;

    if (!Interactable::init(id))
        return false;

	rc.physics.scale = { 1.5f, 1.5f };

	action = "";
    m_locked = false;
    return true;
}

void Door::set_destination(std::string dest)
{
    action = dest;
}

std::string Door::get_destination()
{
    return action;
}

Hitbox Door::get_hitbox() const
{
	std::vector<Square> squares(2);
	
	float width = brick_size;
	vec2 position = mc.position;
	position.x -= width / 2;
	position.y += width / 2;
    Square top(position, width);
	Square bot(add(position, {0.f, width}), width);
	squares[0] = top;
	squares[1] = bot;

    Hitbox hitbox({}, squares);
    return hitbox;
}

std::string Door::perform_action()
{
    if (!m_locked && action.length() > 0) {
        // go to destination
        return action;
    }
    // otherwise, door is locked or no destination specified
    // so, stay at current location
    return "";
}

void Door::lock()
{
	m_locked = true;
}
