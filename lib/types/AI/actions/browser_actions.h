#pragma once

#include "../action.h"

class open_browser_action : public action {
public:
    void execute(const std::string& params) override {}
};

class close_browser_action : public action {
public:
    void execute(const std::string& params) override {}
};

class navigate_browser_action : public action {
public:
    void execute(const std::string& params) override {}
};

void register_browser_actions(action_registry& registry) {
    registry.register_action<open_browser_action>("open_browser");
    registry.register_action<close_browser_action>("close_browser");
    registry.register_action<navigate_browser_action>("navigate_browser");
}