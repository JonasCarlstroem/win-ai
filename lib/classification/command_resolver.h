#pragma once
#include <dlib/svm_threaded.h>
#include "intent_classifier.h"
#include "commands.h"
#include "../util.h"
#include "../types/action.h"

struct intent_pair {
    std::string action;
    std::string object;

    operator std::string () {
        return action + "_" + object;
    }
};

class command_resolver {
    output out;
    intent_classifier action_classifier;
    intent_classifier object_classifier;
    Browser browser_domain;

public:
    command_resolver() : 
        out("Resolver"), 
        action_classifier("action"), 
        object_classifier("object"),
        browser_domain() {}

    void load_classifiers(const std::string& base_path) {
        if (!action_classifier.load_model() || !object_classifier.load_model()) {
            out.err("Failed to load models...");
            return;
        }
        /*action_classifier.load_intents(base_path + "/action");
        object_classifier.load_intents(base_path + "/object");*/
        //action_classifier.load_vocabulary();
        //action_classifier.load_labels();

        //action_classifier.train_model("action_classifier.dat");
        //object_classifier.train_model("object_classifier.dat");
    }

    intent_pair resolve(const std::string& input) {
        std::string action = action_classifier(input);
        std::string object = object_classifier(input);

        intent_pair pair = { action, object };
        std::unique_ptr<Action> act = ActionRegistry::get_instance().resolve(pair);
        return { action, object };
    }

    void dispatch(const intent_pair& pair) {
        std::string intent_name = pair.action + "_" + pair.object;
        execute_intent(intent_name);
    }
};