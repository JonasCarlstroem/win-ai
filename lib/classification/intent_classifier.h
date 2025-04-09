#pragma once

#include <unordered_map>
#include <filesystem>
#include "../types/definitions.h"
#include "../util.h"

namespace fs = std::filesystem;

template<typename... T>
std::string _f(const T&... args) {
    std::ostringstream oss;
    ((oss << args), ...);
    std::string str = oss.str();

    return str;
}

class intent_classifier {
public:
    intent_classifier(const std::string& name) :
        out("Classifier"),
        type(name) {}

    void load_vocabulary() {
        vocab.clear();

        std::string vocab_file = model_vocabulary();

        std::ifstream in(model_vocabulary());
        std::string word;
        while (std::getline(in, word)) {
            vocab.push_back(word);
        }
    }

    void load_intents() {
        int label_index = 0;
        for (const auto& entry : fs::directory_iterator(intents_path())) {
            if (entry.path().extension() == ".txt") {
                std::string label = entry.path().stem().string();
                label_to_index[label] = label_index;
                index_to_label[label_index] = label;
                ++label_index;
            }
        }
    }

    void load_labels() {
        for (const auto& file : fs::directory_iterator(intents_path())) {
            if (file.path().extension() == ".txt") {
                labels.push_back(file.path().stem().string());
            }
        }
    }

    bool load_model() {
        load_vocabulary();
        load_labels();

        try {
            deserialize(model_file()) >> df;
        }
        catch (const std::exception& e) {
            std::cerr << "[Error] " << e.what() << std::endl;
            return false;
        }
    }

    void train_model(const std::string& file) {
        deserialize(file) >> df;
    }

    int classify(const string& input) const {
        sample_type sample(vocab.size());
        sample = 0;

        std::istringstream iss(input);
        std::string word;
        while (iss >> word) {
            auto it = find(vocab.begin(), vocab.end(), word);
            if (it != vocab.end()) {
                int idx = distance(vocab.begin(), it);
                sample(idx) += 1.0;
            }
        }

        return df(sample);
    }

    std::string label_for(int index) const {
        return (index >= 0 && index < labels.size()) ? labels[index] : "unknown";
    }

    std::string operator()(const std::string& input) {
        int id = classify(input);
        return label_for(id);
    }

    //int classify(const std::string& input) const {
    //    sample_type sample = text_to_sample(input);
    //    return df(sample);
    //    //sample_type sample(vocabulary.size());
    //    //sample = 0;
    //    //for (size_t i = 0; i < vocabulary.size(); ++i) {
    //    //    if (input.find(vocabulary[i]) != std::string::npos)
    //    //        sample(i) = 1.0;
    //    //}

    //    //try {
    //    //    int label_idx = df(sample);
    //    //    return index_to_label.at(label_idx);
    //    //}
    //    //catch (...) {
    //    //    return "unknown";
    //    //}
    //}

private:
    output out;
    std::string type;
    std::unordered_map<std::string, int> label_to_index;
    std::unordered_map<int, std::string> index_to_label;
    //std::unordered_map<std::string, int> vocabulary;
    std::vector<std::string> vocab;
    std::vector<std::string> labels;
    df_type df;

    inline static std::string model_base_path = "models/";
    inline static std::string intents_base_path = "intents/";

    const std::string classifier_name() const {
        return type;
    }

    std::string model_name() const {
        return _f(model_base_path, type, "_classifier");
    }

    std::string model_file() const {
        return _f(model_name(), ".dat");
    }

    std::string model_vocabulary() const {
        return _f(model_name(), ".vocab");
    }

    const std::string intents_path() const {
        return _f(intents_base_path, type);
    }

    //void load_vocabulary(const std::string& vocab_file) {
    //    vocabulary.clear();
    //    std::ifstream in(vocab_file);
    //    std::string word;

    //    int index = 0;
    //    while (std::getline(in, word)) {
    //        if (!word.empty()) {
    //            vocabulary[word] = index++;
    //        }
    //    }
    //}

    //sample_type text_to_sample(const std::string& input) const {
    //    sample_type sample;
    //    sample.set_size(vocab.size());
    //    sample = 0;

    //    std::istringstream iss(input);
    //    std::string word;
    //    while (iss >> word) {
    //        auto it = vocab.find(word);
    //        if (it != vocab.end()) {
    //            sample(it->second) += 1.0;
    //        }
    //    }

    //    return sample;
    //}
};