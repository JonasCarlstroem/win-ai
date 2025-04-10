#pragma once
#include <dlib/svm_threaded.h>
#include "intent_classifier.h"
#include "commands.h"
#include "../util.h"
#include "../types/AI/action.h"
#include "../types/AI/domain/browser_intent_handler.h"
#include "../types/AI/intent.h"

class intent_resolver {
    output out;
    intent_classifier action_classifier;
    intent_classifier object_classifier;

public:
    intent_resolver() :
        out("Resolver"), 
        action_classifier("action"), 
        object_classifier("object") { }

    void load_classifiers(const std::string& base_path) {
        if (!action_classifier.load_model() || !object_classifier.load_model()) {
            out.err("Failed to load models...");
            return;
        }
    }

    intent resolve(const std::string& input) {
        std::string action_name = action_classifier(input);
        std::string object_name = object_classifier(input);
        intent_pair pair = { action_name, object_name };

        action_registry& a_reg = action_registry::instance();
        intent_handler_registry& ih_reg = intent_handler_registry::instance();

        std::unique_ptr<action> intent_action = action_registry::instance().resolve(pair);
        std::unique_ptr<intent_handler> handler = intent_handler_registry::instance().create(action_name);
        //std::unique_ptr<action> intent_action = registry.resolve(pair);
        return { pair, std::move(intent_action) };
    }
};