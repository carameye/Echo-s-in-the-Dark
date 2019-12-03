#pragma once

#include "common.hpp"
#include "components.hpp"
#include "torch.hpp"

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
    void draw(const mat3& projection, const vec2& camera_shift, const vec2& size, std::vector<Torch*> torches);

    void set_position(vec2 pos);

    void convert_mouse_pos_to_rad(vec2 coordinates, vec2 centre);

    float get_radians();

    void set_direction();

    bool get_direction();

    // Sets the ambient light level
    void set_ambient(float ambient);

    vec3 get_headlight_channel();

    void set_red_channel();
    void set_green_channel();
    void set_blue_channel();
    void set_next_light_channel();

    void set_prev_light_channel();

private:
    vec2 m_light_position;
    float ambient = 0.f;
    vec3 m_headlight_channel;

	RenderComponent rc;

	Mesh mesh;
	Effect effect;
	Motion motion;

	bool isBlue(vec3 color);
	bool isRed(vec3 color);
	bool isGreen(vec3 color);
	bool isWhite(vec3 color);
    void set_rotation(float radians);
};
