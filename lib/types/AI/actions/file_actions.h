#pragma once

#include "../action.h"
#include "../action_registry.h"

class open_file_action : public action {
public:
    void execute(const std::string& params) override {}
};

class close_file_action : public action {
public:
    void execute(const std::string& params) override {}
};

class read_file_action : public action {
public:
    void execute(const std::string& params) override {}
};

class write_file_action : public action {
public:
    void execute(const std::string& params) override {}
};

class remove_file_action : public action {
public:
    void execute(const std::string& params) override {}
};

inline bool _file_actions_registered = [] {
    action_registry& registry = action_registry::instance();
    registry.register_action_factory<open_file_action>("open_file");
    registry.register_action_factory<close_file_action>("close_file");
    registry.register_action_factory<read_file_action>("read_file");
    return true;
}