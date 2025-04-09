#pragma once

#include <string>
#include <iostream>
#include <unordered_map>

class action;

class action_registry {
private:
    output out;
    std::unordered_map<std::string, std::unique_ptr<action>> action_map;

public:
    action_registry() : out("Action registry") {}

    template<typename T>
    void register_action(const std::string& action_name) {
        static_assert(std::is_base_of<action, T>::value, "T must derive from Action");
        action_map[action_name] = std::make_unique<T>();
    }

    action* resolve(const string& action_name) {
        auto it = action_map.find(action_name);
        if (it != action_map.end()) {
            return it->second.get();
        }
        else {
            out.err("Could not resolve action for the specific intent..");
            return nullptr;
        }
    }

    action_registry(const action_registry&) = delete;
    action_registry& operator=(const action_registry&) = delete;
    action_registry(action_registry&&) = delete;
    action_registry& operator=(action_registry&&) = delete;
};