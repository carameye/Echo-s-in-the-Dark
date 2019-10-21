#pragma once

#include "common.hpp"
#include <vector>

class Light : public Entity
{
public:
    // Creates all the associated render resources and default transform
    bool init();

    // Releases all associated resources
    void destroy();

    // Renders the water
    void draw(const mat3& projection, const vec2& camera_shift)override;

    void set_position(vec2 pos);

    void set_radians(float radians);

    float get_radians();

    void set_direction();

    bool get_direction();

    // Sets the ambient light level
    void set_ambient(float ambient);

    // Clear torches
    void clear_torches();

    // Add a torch at a specific location
    void add_torch(vec2 torch);

private:
    vec2 light_position;
    float ambient = 0.f;
    std::vector<vec2> torches;
};
