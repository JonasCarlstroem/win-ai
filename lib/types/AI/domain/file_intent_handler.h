#pragma once
#include "../intent_handler.h"
#include "../intent_handler_registry.h"

class file_intent_handler : public intent_handler {
public:
    void handle(const std::string& action, const std::string& entity) override {

    }
};

inline bool _file_handler_registered = [] {
    intent_handler_registry::instance().register_handler("file", [] {
        return std::make_unique<file_intent_handler>();
    });
    return true;
}