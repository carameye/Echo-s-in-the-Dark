#pragma once

#include "common.hpp"
#include "components.hpp"

class Torch : public Entity
{
    static Texture torch_texture;
    static RenderComponent rc;
    MotionComponent mc;

public:
    // Creates all the associated render resources and default transform
    bool init(int id);

    // Update torch
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms);

    // Returns the current torch position
    vec2 get_position()const;

    // Sets the new torch position
    void set_position(vec2 position);

};