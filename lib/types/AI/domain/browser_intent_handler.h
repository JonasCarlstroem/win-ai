#pragma once
#include "../action.h"
#include "../intent_handler.h"
#include "../intent_handler_registry.h"
#include "../actions/browser_actions.h"

class browser_intent_handler : public intent_handler {
public:
    void handle(const std::string& action, const std::string& entity) override {

    }
};

inline bool _browser_handler_registered = [] {
    intent_handler_registry::instance().register_handler("browser", [] {
        return std::make_unique<browser_intent_handler>();
    });
    return true;
}();