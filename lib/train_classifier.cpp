#pragma once

#include <dlib/svm_threaded.h>
#include "classifier_data.h"

using namespace dlib;

int main() {
    using net_type = mlp::kernel_1a_c;

    std::vector<std::string> texts = {
        "open chrome", "start browser", "launch browser",
        "notepad", "open notepad",
        "show me the file", "read document",
        "chat with me", "talk", "gpt please"
    };

    std::vector<unsigned long> labels = {
        0, 0, 0,
        1, 1,
        2, 2,
        3, 3, 3
    };

    const long output_size = get_unique_label_count(labels);

    hot_labels<one_hot> one_hot_labels = get_one_hot_labels(labels, output_size);
    std::vector<matrix<double, 0, 1>> samples = get_samples(texts);

    if (samples.empty()) {
        std::cerr << "[Error] No samples extracted.\n";
    }

    const long input_size = samples[0].size();
    const long hidden_size = 10;
    const long second_hidden_size = 4;

    net_type net(input_size, hidden_size, second_hidden_size, output_size);

    try {
        for (int epoch = 0; epoch < 500; ++epoch) {
#ifdef _DEBUG
            std::cout << "Epoch " << epoch + 1 << "/500" << std::endl;
#endif
            for (size_t i = 0; i < samples.size(); ++i) {
                if (samples[i].size() != input_size) {
                    std::cerr << "Error: Input size mismatch at index: " << i << std::endl;
                    continue;
                }

                try {
                    net.train(samples[i], vector_to_sample_type(one_hot_labels[i]));
                }
                catch (const std::exception& e) {
                    std::ostringstream os;
                    os << "[Error] Exception thrown during training: \n\ti=" << i << "\n\tepoch=" << epoch << "\n\tsample size=" << samples[i].size() << std::endl;
                    os << e.what();
                    std::exception ex(os.str().c_str());
                    throw ex;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    serialize("mlp_command_classifier.dat") << net;
    std::cout << "[Info] Model trained and saved to mlp_command_classifier.dat\n";
    
    return 0;
}