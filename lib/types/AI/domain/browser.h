#pragma once
#include "../action.h"

class browser {
public:

    class open_action : public action {
    public:
        void execute(const std::string& parameters) override {
            std::cout << "Executing OpenAction on Browser" << std::endl;
        }
    };

    class close_action : public action {
    public:
        void execute(const std::string& parameters) override {
            std::cout << "Executing CloseAction on Browser" << std::endl;
        }
    };

    class navigate_action : public action {
    public:
        void execute(const std::string& parameters) override {
            std::cout << "Executing NavigateAction on Browser" << std::endl;
        }
    };

    inline static void register_actions(action_registry& registry) {
        registry.register_action<open_action>("open_browser");
        registry.register_action<close_action>("close_browser");
        registry.register_action<navigate_action>("navigate_browser");
    }
};