#include <iostream>
#include <string>
#include "lib/classification/commands.h"
#include "lib/classification/classifier_data.h"
#include "lib/classification/intent_classifier.h"
#include "lib/classification/command_resolver.h"
#include "lib/types/definitions.h"
#include "lib/types/AI/intent.h"
#include <vector>

using namespace dlib;

int main() {
    action_registry registry;
    command_resolver resolver(registry);
    resolver.load_classifiers("intents");

    std::string input;
    while (true) {
        std::cout << "input> ";
        std::getline(std::cin, input);

        auto final_intent = resolver.resolve(input);
        std::cout << "Resolved intent: " << (std::string)final_intent.command << std::endl;

        final_intent.execute();
    }

    return 0;
}