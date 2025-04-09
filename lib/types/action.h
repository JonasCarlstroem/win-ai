#pragma once

#include <string>
#include <iostream>
#include <unordered_map>

class Action;
class ActionRegistry;

class action_type {
public:
    std::string name;
    std::string description;

    action_type(std::string name, std::string description) :
        name(name), description(description) {}

    void display() const {
        std::cout << name << ": " << description << std::endl;
    }
};

class ActionRegistry {
private:
    output out;
    std::unordered_map<std::string, std::unique_ptr<Action>> action_map;

    ActionRegistry() : out("Action registry") {}

public:

    template<typename T>
    void register_action(const std::string& action_name) {
        static_assert(std::is_base_of<Action, T>::value, "T must derive from Action");
        //std::unique_ptr<Action> ptr = std::make_unique<T>();
        //action_map.insert({ action_name, std::move(ptr) });
        action_map[action_name] = std::make_unique<T>(action_name);
        //action_map[action_name] = std::unique_ptr<T>(new T());
    }

    std::unique_ptr<Action> resolve(const string& action_name) {
        auto it = action_map.find(action_name);
        if (it != action_map.end()) {
            return std::move(it->second);
        }
        else {
            out.err("Could not resolve action for the specific intent..");
            return std::unique_ptr<Action>(nullptr);
        }
    }

    inline static ActionRegistry get_instance() {
        return registry;
    }

private:
    static ActionRegistry registry;
};

ActionRegistry ActionRegistry::registry;

class Action {
protected:
    std::string action_name;

    template<typename T>
    void register_action() {
        ActionRegistry::get_instance().register_action<T>(action_name);
    }

public:
    Action(std::string name) :
        action_name(name) {}

    //template<typename T>
    //Action(T type, std::string name) :
    //    action_name(name) {
    //    register_action<T>();
    //}

    virtual void execute(const std::string& parameters) = 0;
};

template<typename T>
class Templ_Action : public Action {
public:
    Templ_Action(std::string name) : Action(name) {
        static_assert(std::is_base_of<Action, T>::value, "T must derive from Action");
        register_action<T>();
    }
};

class Browser {
public:

    class OpenAction : public Templ_Action<OpenAction> {
    public:
        OpenAction() : 
            Templ_Action("open_browser") {}

        void execute(const std::string& parameters) override {

        }
    };

    static OpenAction open_action;
};

Browser::OpenAction Browser::open_action;