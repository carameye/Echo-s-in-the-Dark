#include <common.hpp>
#include "components.hpp"
#include <math.h>
#include <hitbox.hpp>
#include <brick.hpp>

#include "robot_hat.hpp"

class RobotHead : public Entity
{
    static Texture robot_head_texture;
    bool m_face_right = true;

	RenderComponent rc;
	MotionComponent mc;

public:
    // Creates all the associated render resources and default transform
    bool init(int id);

    // Update robot
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms, vec2 goal);

    // Returns the robots hitbox for collision detection
    Hitbox get_hitbox();

    // Returns the current robot position
    vec2 get_position() const;

    // Sets the new robot position
    void set_position(vec2 position);

    // Sets the scaling
    void set_scaling(vec2 scaling);

    // Set the look direction
    void set_direction(bool right);

    bool get_direction();

    vec2 get_next_position(vec2 goal);

    vec2 get_velocity();

    void set_velocity(vec2 vec2);

private:
    Hitbox m_hitbox;

    void calculate_hitbox();
};
