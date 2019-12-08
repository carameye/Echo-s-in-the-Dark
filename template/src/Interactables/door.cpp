#include "door.hpp"

Texture Door::s_door_texture;

bool Door::init(int id, vec2 position)
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

    if (!Interactable::init(id, position))
        return false;

	mc.physics.scale = { 1.5f, 1.5f };

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

vec2 Door::get_position()
{
	return mc.position;
}

Hitbox Door::get_hitbox() const
{
	std::vector<Square> squares(2);
	
	float width = brick_size;
	vec2 position = mc.position;
	position.x -= width / 2;
	position.y += width / 2;
    Square top(position, (int)width);
	Square bot(add(position, {0.f, width}), (int)width);
	squares[0] = top;
	squares[1] = bot;

    Hitbox hitbox({}, squares);
    return hitbox;
}

std::string Door::perform_action()
{
    SoundSystem* sound_system = SoundSystem::get_system();

    if (!m_locked && action.length() > 0) {
        // go to destination
        sound_system->play_sound_effect(Sound_Effects::open_door);
        return action;
    }
    if (m_locked) {
        sound_system->play_sound_effect(Sound_Effects::door_locked);
        return "locked";
    }
    // otherwise, no destination specified
    // so, stay at current location
    return "";
}

void Door::lock()
{
	m_locked = true;
}
