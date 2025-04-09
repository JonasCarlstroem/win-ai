#pragma once
#include <dlib/svm_threaded.h>
#include "intent_classifier.h"
#include "commands.h"
#include "../util.h"
#include "../types/AI/action.h"
#include "../types/AI/domain/browser.h"
#include "../types/AI/intent.h"

class command_resolver {
    output out;
    intent_classifier action_classifier;
    intent_classifier object_classifier;
    browser browser_domain;
    action_registry& registry;

public:
    command_resolver(action_registry& registry) :
        out("Resolver"), 
        action_classifier("action"), 
        object_classifier("object"),
        browser_domain(),
        registry(registry) {
        browser_domain.register_actions(registry);
    }

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
        action* intent_action = registry.resolve(pair);
        return { pair, intent_action };
    }
};