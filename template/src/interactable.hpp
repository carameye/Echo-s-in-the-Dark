#pragma once

#include "common.hpp"
#include "hitbox.hpp"

#include <vector>
#include <cmath>

class Interactable: public Entity
{
    public:
        bool init();

        void destroy();

        // Renders interactable
	    // projection is the 2D orthographic projection matrix
	    void draw(const mat3& projection, const vec2& camera_shift) override;

        // Set the position of the interactable
        void set_position(vec2 pos);

        void set_can_perform_action(bool perform);

        virtual Hitbox get_hitbox() const = 0;

        // perform_action is abstract, as implementation is dependent on child classes
        virtual bool perform_action() = 0;

    protected:
        bool m_can_perform_action;
};
