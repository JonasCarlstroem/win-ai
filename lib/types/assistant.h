#pragma once
#include <map>
#include <fstream>
#include "action.h"
#include "object.h"
#include "intent.h"

class ai_assistant {
private:
    std::map<std::string, action_type> actions_map;
    std::map<std::string, object_type> objects_map;

public:
    void load_config(const std::string& actions_file, const std::string& objects_file) {
        std::ifstream actions(actions_file);
        std::ifstream objects(objects_file);

        if (!actions.is_open() || !objects.is_open()) {
            std::cerr << "Error opening configuration files..." << std::endl;
            return;
        }

        std::string line;

        while (std::getline(actions, line)) {
            size_t separator = line.find(':');

            if (separator != std::string::npos) {
                std::string name = line.substr(0, separator);
                std::string description = line.substr(separator + 1);

                actions_map[name] = action_type(name, description);
            }
        }

        while (std::getline(objects, line)) {
            size_t separator = line.find(':');

            if (separator != std::string::npos) {
                std::string name = line.substr(0, separator);
                std::string description = line.substr(separator + 1);

                objects_map[name] = object_type(name, description);
            }
        }

        actions.close();
        objects.close();
    }

    std::string detect_action(const std::string& input) {
        for (const auto& action : actions_map) {
            if (input.find(action.first) != std::string::npos) {
                return action.first;
            }
        }

        return "unknown";
    }

    std::string detect_object(const std::string& input) {
        for (const auto& object : objects_map) {
            if (input.find(object.first) != std::string::npos) {
                return object.first;
            }
        }

        return "unknown";
    }

    void execute_intent(const std::string& action_name, const std::string& object_name) {
        if (actions_map.find(action_name) != actions_map.end() &&
            objects_map.find(object_name) != objects_map.end()) {
            
            intent intent_instance(actions_map[action_name], objects_map[object_name]);
            intent_instance.execute();
        }
        else {
            std::cout << "Unknown action or object" << std::endl;
        }
    }

    void display_config() {
        std::cout << "Actions: \n";
        for (const auto& action : actions_map) {
            action.second.display();
        }

        std::cout << "Objects: \n";
        for (const auto& object : objects_map) {
            object.second.display();
        }
    }
};