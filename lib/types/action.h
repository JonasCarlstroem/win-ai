#pragma once

#include <string>
#include <iostream>

class action_type {
public:
    std::string name;
    std::string description;

    action_type(std::string name, std::string description) :
        name(name), description(description) {}

    void display() const {
        std::cout << name << ": " << description << std::endl;
    }
};