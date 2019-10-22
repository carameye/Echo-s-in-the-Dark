#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "components.hpp"

#include <vector>
#include <cmath>
#include <functional>

class Interactable: public Entity
{
protected: 
	typedef std::function<bool(std::string)> actionFunction;

	RenderComponent rc;
	MotionComponent mc;
public:
    bool init(int id);

    // Set the position of the interactable
    void set_position(vec2 pos);

    virtual Hitbox get_hitbox() const = 0;

    // perform_action is abstract, as implementation is dependent on child classes
    virtual bool perform_action(actionFunction action) = 0;
};
