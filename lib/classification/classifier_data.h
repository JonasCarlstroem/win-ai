#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <filesystem>
#include <set>
#include "../types/definitions.h"

using namespace dlib;
namespace fs = std::filesystem;

std::set<std::string> build_vocabulary(const std::vector<std::string>& texts) {
    std::set<std::string> vocabulary;
    for (const auto& text : texts) {
        std::istringstream iss(text);
        std::string word;
        while (iss >> word) {
            vocabulary.insert(word);
        }
    }

    return vocabulary;
}

std::vector<std::string> build_vocabulary(const std::string& base_path) {
    std::vector<std::string> vec;
    for (const auto& dir_entry : fs::directory_iterator(base_path)) {
        if (dir_entry.path().extension() == ".txt") {
            std::ifstream file(dir_entry.path());
            std::string word;
            while (std::getline(file, word)) {
                if (!word.empty()) {
                    vec.push_back(word);
                }
            }
        }
    }

    return vec;
}

std::vector<sample_type> encode_samples(const std::vector<std::string>& lines, const std::vector<std::string>& vocab) {
    std::vector<sample_type> samples;
    for (const auto& line : lines) {
        sample_type sample(vocab.size());
        sample = 0;
        for (size_t i = 0; i < vocab.size(); ++i) {
            if (line.find(vocab[i]) != std::string::npos) {
                sample(i) = 1.0;
            }
        }
        samples.push_back(sample);
    }

    return samples;
}

void train_classifier(const std::string& base_path, const std::string& output_file) {
    std::vector<sample_type> samples;
    std::vector<label_type> labels;
    std::map<std::string, int> label_map;

    label_type label_index = 0;
    std::vector<std::string> vocab = build_vocabulary(base_path);

    for (const auto& file : fs::directory_iterator(base_path)) {
        if (file.path().extension() == ".txt") {
            std::string label = file.path().stem().string();
            label_map[label] = label_index;

            std::ifstream f(file.path());
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(f, line)) {
                if (!line.empty()) {
                    lines.push_back(line);
                }
            }

            auto encoded = encode_samples(lines, vocab);
            for (auto& sample : encoded) {
                samples.push_back(sample);
                labels.push_back(label_index);
            }

            ++label_index;
        }
    }

    svm_c_trainer<linear_kernel<sample_type>> trainer;
    ovo_trainer o_trainer;
    o_trainer.set_trainer(trainer);

    df_type df = o_trainer.train(samples, labels);

    serialize(output_file) << df;
    serialize(output_file + ".vocab") << vocab;

    std::cout << "Trained and saved classifier to: " << output_file << std::endl;
}

void save_vocabulary(const std::set<std::string>& vocabulary) {
    std::ofstream file("vocabulary.txt");
    for (const auto& word : vocabulary) {
        file << word << std::endl;
    }
}

std::set<std::string> load_vocabulary() {
    std::set<std::string> vocabulary;
    std::ifstream file("vocabulary.txt");
    std::string word;
    while (std::getline(file, word)) {
        vocabulary.insert(word);
    }

    return vocabulary;
}

std::vector<sample_type> preprocess_text(const std::vector<std::string>& texts, const std::set<std::string>& vocabulary) {
    std::vector<sample_type> samples;

    for (const auto& text : texts) {
        sample_type sample;
        sample.set_size(vocabulary.size());

        std::map<std::string, size_t> word_index;
        size_t index = 0;
        for (const auto& word : vocabulary) {
            word_index[word] = index++;
        }

        std::istringstream iss(text);
        std::string word;
        while (iss >> word) {
            if (word_index.find(word) != word_index.end()) {
                sample(word_index.at(word)) = 1.0;
            }
        }

        samples.push_back(sample);
    }

    return samples;
}

std::vector<text_label> load_label_mapping(const std::string& directory) {
    fs::path action_path(directory + "/action.txt");
    std::vector<text_label> samples;
    
    unsigned long id = 0;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::ifstream file(entry.path());
            std::string line;

            label_type label = id++;
            while (std::getline(file, line)) {
                for (auto& c : line) {
                    if (std::isalpha(c)) {
                        c = static_cast<char>(std::tolower(c));
                    }
                }

                samples.push_back({ line, label });
            }
        }
    }

    return samples;
}

std::vector<double> extract_features(const std::string& input, size_t max_len = 20) {
    std::vector<double> features;
    for (char c : input) {
        if (isalpha(c)) {
            features.push_back(static_cast<double>(tolower(c) - 'a') / 25.0f);
        }
        if (features.size() >= max_len) break;
    }

    while (features.size() < max_len) {
        features.push_back(0.0f);
    }

    return features;
}

unsigned long get_unique_label_count(const std::vector<unsigned long> labels) {
    std::set<unsigned long> unique_labels(labels.begin(), labels.end());
    return unique_labels.size();
}

matrix<double, 0, 1> vector_to_sample_type(const std::vector<double>& vec) {
    matrix<double, 0, 1> m(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        m(i) = vec[i];
    }

    return m;
}

std::vector<std::vector<double>> get_one_hot_labels(const std::vector<ulong> labels, int output_size = 4) {
    std::vector<std::vector<double>> hot_labels;
    for (const auto& label : labels) {
        std::vector<double> one_h(output_size, 0.0f);
        one_h[label] = 1.0f;
        
        hot_labels.push_back(one_h);
    }

    return hot_labels;
}

std::vector<sample_type> get_samples(const std::vector<std::string>& texts) {
    std::vector<sample_type> samples;
    for (const auto& text : texts) {
        auto vec = extract_features(text);
        
        matrix<double, 0, 1> m(vec.size());
        for (size_t i = 0; i < vec.size(); ++i) m(i) = vec[i];
        samples.push_back(m);
    }

    return samples;
}