#pragma once
#include <string>
#include <functional>
#include <memory>
#include "intent_handler.h"
#include "../../util.h"

using intent_handler_factory = std::function<std::unique_ptr<intent_handler>()>;

class intent_handler_registry {
public:
    static intent_handler_registry& instance() {
        static intent_handler_registry instance;
        return instance;
    }

    template<typename T>
    void register_intent_handler(const std::string& name) {

    }

    void register_handler(const std::string& name, intent_handler_factory factory) {
        intent_handler_factories[name] = std::move(factory);
    }

    std::unique_ptr<intent_handler> create(const std::string& name) {
        auto it = intent_handler_factories.find(name);
        if (it != intent_handler_factories.end()) {
            return (it->second)();
        }
        return nullptr;
    }

    intent_handler_registry(const intent_handler_registry&) = delete;
    intent_handler_registry(const intent_handler_registry&&) = delete;

private:
    output out;
    std::unordered_map<std::string, intent_handler_factory> intent_handler_factories;

    intent_handler_registry() : out("intent_handler_registry"), intent_handler_factories() {}
};