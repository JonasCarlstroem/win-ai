#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include "action_registry.h"

class action {
public:
    virtual void execute(const std::string& parameters) = 0;
};