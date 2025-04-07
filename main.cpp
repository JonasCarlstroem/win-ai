#include <iostream>
#include <string>
#include "lib/commands.h"
#include "lib/classifier_data.h"
#include <dlib/svm_threaded.h>
#include <dlib/mlp.h>
#include <vector>

using namespace dlib;
//typedef matrix<double, 0, 1> sample_type;
typedef mlp::kernel_1a_c net_type;

const long input_size = 20;
const long hidden_size = 10;
const long sec_hidden_size = 4;

net_type& load_model(const std::string& model_filename) {
    net_type net(input_size, hidden_size, sec_hidden_size, 4l);
    deserialize(model_filename) >> net;
    return net;
}

int classify_input(net_type& net, const std::string& input_text) {
    std::vector<double> features = extract_features(input_text);

    if (features.size() != 20) return -1;

    matrix<double, 0, 1> sample(features.size(), 1);
    for (size_t i = 0; i < features.size(); ++i) {
        sample(i) = features[i];
    }

    if (sample.nr() != net.input_layer_nodes()) {
        std::cerr << "[Error] sample.nr() != net.input_layer_nodes()" << std::endl;
        return -1;
    }

    if (sample.nc() != 1) {
        std::cerr << "[Error] sample.nc() != 1" << std::endl;
    }

    try {
        auto predict = net.operator()(sample);
        auto prediction = net(sample);
        //the rest of this part we can disregard for now.
        //return net(sample);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what();
        return -1;
    }
}

int main() {
    net_type& net = load_model("mlp_command_classifier.dat");

    std::string user_input;

    while (true) {
        std::cout << "input> ";
        std::getline(std::cin, user_input);

        int command = classify_input(net, user_input);

        std::string intent_name;
        switch (command) {
            case 0:
            intent_name = "open_browser";
            break;
            case 1:
            intent_name = "open_notepad";
            break;
            case 2:
            intent_name = "read_file";
            break;
            case 3:
            intent_name = "chat";
            break;
            default:
            intent_name = "unknown";
            break;
        }

        execute_intent(intent_name);
    }

    return 0;
}