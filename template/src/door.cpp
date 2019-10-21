#include "door.hpp"

Texture Door::s_door_texture;

bool Door::init()
{

    if (!s_door_texture.is_valid())
	{
        if (!s_door_texture.load_from_file(textures_path("body_ball.png")))
		{
			std::fprintf(stderr, "Failed to load door texture!");
			return false;
		}
	}

    texture = &s_door_texture;

    if (!init_sprite())
        return false;

    if (!Interactable::init())
        return false;

    m_destination = "";
    m_locked = false;
    return true;
}

void Door::set_destination(std::string dest)
{
    m_destination = dest;
}

std::string Door::get_destination()
{
    return m_destination;
}

Hitbox Door::get_hitbox() const
{
	std::vector<Square> squares(2);
	
	float width = brick_size;
	vec2 position = motion.position;
	position.x -= width / 2;
	position.y += width / 2;
    Square top(position, width);
	Square bot(add(position, {0.f, width}), width);
	squares[0] = top;
	squares[1] = bot;

    Hitbox hitbox({}, squares);
    return hitbox;
}

bool Door::perform_action()
{
    if (!m_locked && m_destination.length() > 0 && m_can_perform_action) {
        // go to destination
        set_can_perform_action(false);
        return true;
    }
    // otherwise, door is locked or no destination specified
    // so, stay at current location
    return false;
};
