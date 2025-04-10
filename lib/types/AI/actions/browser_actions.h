#pragma once

#include "../action_registry.h"

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

template<typename T>
void reg(const std::string& name) {

}

inline bool _browser_actions_registered = [] {
    action_registry& registry = action_registry::instance();
    registry.register_action_factory<open_browser_action>("open_browser");
    registry.register_action_factory<close_browser_action>("close_browser");
    registry.register_action_factory<navigate_browser_action>("navigate_browser");

    return true;
}();

//void register_browser_actions(action_registry& registry) {
//    registry.register_action<open_browser_action>("open_browser");
//    registry.register_action<close_browser_action>("close_browser");
//    registry.register_action<navigate_browser_action>("navigate_browser");
//}