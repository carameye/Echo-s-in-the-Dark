#pragma once

#include "common.hpp"
#include "hitbox.hpp"
#include "components.hpp"
#include "sound_system.hpp"

#include <vector>
#include <cmath>
#include <functional>

class Interactable: public Entity
{
protected: 
	std::string action;

	RenderComponent rc;
	MotionComponent mc;
public:
    bool init(int id, vec2 position);

    virtual Hitbox get_hitbox() const = 0;

    // perform_action is abstract, as implementation is dependent on child classes
    virtual std::string perform_action() = 0;
};
