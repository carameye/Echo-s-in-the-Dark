#pragma once

#include "common.hpp"

class FlightTimer : public Entity
{
    static Texture energy_full;
    static Texture energy_green;
    static Texture energy_yellow;
    static Texture energy_orange;
    static Texture energy_red;

public:

    // Initializes energy bar above the robots head
    bool init(vec2 robot_position, vec2 robot_velocity);

    // Destroys the health bar when the robot is detroyed
    void destroy();

    // Updates the Health bar bot position and percentage health
    void update(float ms);

    // Renders the health bar
    void draw(const mat3& projection, const vec2& camera_shift) override;


private:

};