#pragma once

#include "common.hpp"
#include "components.hpp"

#include <vector>
#include <map>

class Light : public Entity
{
	static std::map<std::string, Texture> brickmap_textures;

public:
    // Creates all the associated render resources and default transform
    bool init(std::string level);

    // Releases all associated resources
    void destroy();

    // Renders the water
    void draw(const mat3& projection, const vec2& camera_shift, const vec2& size);

    void set_position(vec2 pos);

    void convert_mouse_pos_to_rad(vec2 coordinates, vec2 centre);

    float get_radians();

    void set_direction();

    bool get_direction();

    // Sets the ambient light level
    void set_ambient(float ambient);

    // Clear torches
    void clear_torches();

    // Add a torch at a specific location
    void add_torch(vec2 torch);

    vec3 get_headlight_channel();

    void set_red_channel();
    void set_green_channel();
    void set_blue_channel();

private:
    vec2 m_light_position;
    float ambient = 0.f;
    vec3 m_headlight_channel;
    std::vector<vec2> torches;

	RenderComponent rc;

	Mesh mesh;
	Effect effect;
	Motion motion;

    void set_rotation(float radians);
};
