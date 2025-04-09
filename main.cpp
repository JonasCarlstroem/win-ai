#include <iostream>
#include <string>
#include "lib/classification/commands.h"
#include "lib/classification/classifier_data.h"
#include "lib/classification/intent_classifier.h"
#include "lib/classification/command_resolver.h"
#include "lib/types/definitions.h"
#include <vector>

using namespace dlib;
//using df_type = one_vs_one_decision_function<one_vs_one_trainer<any_trainer<sample_type>>>;

df_type& load_model(const std::string& model_filename) {
    df_type df; //(input_size, hidden_size, sec_hidden_size, 4l);
    deserialize(model_filename) >> df;

    return df;
}

int classify_input(df_type* df, const std::string& input_text, const std::set<std::string>& vocabulary) {
    std::vector<double> features = extract_features(input_text);

    if (features.size() != 20) return -1;

    sample_type sample(features.size(), 1);
    for (size_t i = 0; i < features.size(); ++i) {
        sample(i) = features[i];
    }

    std::vector<sample_type> test_samples = preprocess_text({ input_text }, vocabulary);

    try {
        return (*df)(test_samples[0]);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what();
        return -1;
    }
}

int main() {
    command_resolver resolver;
    resolver.load_classifiers("intents");

    std::string input;
    while (true) {
        std::cout << "input> ";
        std::getline(std::cin, input);

        auto [action, object] = resolver.resolve(input);
        std::cout << "Resolved intent: " << action << " + " << object << std::endl;

        resolver.dispatch({ action, object });
    }
    //df_type df; // = load_model("mlp_command_classifier.dat");

    //try {
    //    deserialize("mlp_command_classifier.dat") >> df;
    //}
    //catch (const std::exception& e) {
    //    std::cerr << e.what();
    //    return -1;
    //}

    //std::set<std::string> vocabulary = load_vocabulary();
    //std::string user_input;

    //while (true) {
    //    std::cout << "input> ";
    //    std::getline(std::cin, user_input);

    //    int command = classify_input(&df, user_input, vocabulary);

    //    std::string intent_name;
    //    switch (command) {
    //        case 0:
    //        intent_name = "open_browser";
    //        break;
    //        case 1:
    //        intent_name = "open_notepad";
    //        break;
    //        case 2:
    //        intent_name = "read_file";
    //        break;
    //        case 3:
    //        intent_name = "chat";
    //        break;
    //        default:
    //        intent_name = "unknown";
    //        break;
    //    }

    //    execute_intent(intent_name);
    //}

    return 0;
}