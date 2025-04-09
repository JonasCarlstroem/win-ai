#pragma once
#include "action.h"

struct intent_pair {
    std::string action;
    std::string object;

    operator std::string() {
        return action + "_" + object;
    }
};

class intent {
public:
    intent_pair command;
    action*     intent_action;

    intent(const intent_pair& pair, action* action) :
        command(pair), intent_action(action) {}

    void execute(const std::string& parameters = "") const {
        intent_action->execute(parameters);
    }
};