#pragma once
#include "action.h"
#include "object.h"

class intent {
public:
    action_type action;
    object_type object;

    intent(const action_type& action, const object_type& object) :
        action(action), object(object) {}

    void execute() const {
        std::cout << action.description << " " << object.description << std::endl;
    }
};