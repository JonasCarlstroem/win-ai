#pragma once
#include <string>
#include <functional>
#include <memory>
#include "intent_handler.h"

using handler_factory = std::function<std::unique_ptr<intent_handler>()>;

class intent_handler_registry {
public:
    static intent_handler_registry& instance() {
        static intent_handler_registry instance;
        return instance;
    }

    void register_handler(const std::string& name, handler_factory factory) {
        factories[name] = std::move(factory);
    }

    std::unique_ptr<intent_handler> create(const std::string& name) {
        auto it = factories.find(name);
        if (it != factories.end()) {
            return (it->second)();
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, handler_factory> factories;
};