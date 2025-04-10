#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>
#include "../../util.h"
#include "action.h"

//class action;
using action_factory = std::function<std::unique_ptr<action>()>;

class action_registry {
public:
    static action_registry& instance() {
        static action_registry instance;
        return instance;
    }

    //template<typename T>
    //void register_action(const std::string& action_name) {
    //    static_assert(std::is_base_of<action, T>::value, "T must derive from Action");
    //    action_map[action_name] = std::make_unique<T>();
    //}

    void register_action(const std::string& action_name, action_factory factory) {
        action_factories[action_name] = std::move(factory);
    }

    template<typename T>
    void register_action_factory(const std::string& action_name) {
        action_factories[action_name] = std::move([] {
            return std::make_unique<T>();
        });
    }

    std::unique_ptr<action> resolve(const std::string& action_name) {
        auto it = action_factories.find(action_name);
        if (it != action_factories.end()) {
            return it->second();
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

private:
    output out;
    //std::unordered_map<std::string, std::unique_ptr<action>> action_map;
    std::unordered_map<std::string, action_factory> action_factories;

    action_registry() : out("Action registry"), action_factories() {}
};