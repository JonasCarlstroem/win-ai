#pragma once
#include <string>

class intent_handler {
public:
    virtual ~intent_handler() = default;
    virtual void handle(const std::string& action, const std::string& entity) = 0;
};