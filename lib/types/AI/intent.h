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
    intent_pair             command;
    std::unique_ptr<action> intent_action;

    intent(const intent_pair& pair, std::unique_ptr<action>&& action) :
        command(pair), intent_action(std::move(action)) {}

    void execute(const std::string& parameters = "") const {
        intent_action->execute(parameters);
    }
};