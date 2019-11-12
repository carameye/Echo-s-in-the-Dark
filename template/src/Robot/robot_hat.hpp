#include <common.hpp>
#include "components.hpp"
#include <math.h>

class RobotHat : public Entity
{
    static Texture robot_hat_texture;
    bool m_face_right = true;

    RenderComponent rc;
    MotionComponent mc;

public:
    // Creates all the associated render resources and default transform
    bool init(int id);

    // Update robot
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms, vec2 goal);

    // Returns the current robot position
    vec2 get_position() const;

    // Sets the new robot position
    void set_position(vec2 position);

    // Sets the scaling
    void set_scaling(vec2 scaling);

    // Set the look direction
    void set_direction(bool right);

    bool get_direction();

    void set_velocity(vec2 vec2);
};