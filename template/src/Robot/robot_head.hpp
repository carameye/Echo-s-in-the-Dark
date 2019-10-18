#include <common.hpp>

class RobotHead : public Entity
{
    static Texture robot_head_texture;
    bool m_face_right = true;

public:
    // Creates all the associated render resources and default transform
    bool init();

    // Releases all the associated resources
    void destroy();

    // Update robot
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms, vec2 goal);

    // Renders the robot
    // projection is the 2D orthographic projection matrix
    void draw(const mat3& projection, const vec2& camera_shift) override;

    // Returns the current robot position
    vec2 get_position() const;

    // Sets the new robot position
    void set_position(vec2 position);

    // Sets the scaling
    void set_scaling(vec2 scaling);

    // Set the look direction
    void set_direction(bool right);
};
