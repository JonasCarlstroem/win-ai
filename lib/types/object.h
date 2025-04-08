#pragma once

class object_type {
public:
    std::string name;
    std::string description;

    object_type(std::string name, std::string description) :
        name(name), description(description) {}
    
    void display() const {
        std::cout << name << ": " << description << std::endl;
    }
};