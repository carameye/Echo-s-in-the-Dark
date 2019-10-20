#pragma once

#include "common.hpp"

class FlightEnergyBar : public Entity
{
    static Texture fuel_full;
    static Texture fuel_4;
    static Texture fuel_3;
    static Texture fuel_2;
    static Texture fuel_empty;

public:

    // Initializes energy bar above the robots head
    bool init();

    // Destroys the health bar when the robot is detroyed
    void destroy();

    // Updates the Health bar bot position and percentage health
    void update(float ms, vec2 goal, float percent);

    // Renders the health bar
    void draw(const mat3& projection, const vec2& camera_shift) override;

    // Returns the current energy bar position
    vec2 get_position() const;

    // Sets the new energy bar position
    void set_position(vec2 position);

    // Sets the scaling
    void set_scaling(vec2 scaling);

private:

};