#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "../types/definitions.h"

namespace fs = std::filesystem;

class intent_trainer {
private:
    std::vector<std::string> vocabulary;

public:
    std::string base_path;

    intent_trainer(std::string base_path) : 
        base_path(base_path) {}

    void build_vocabulary() {
        vocabulary.clear();
        for (const auto& dir_entry : fs::directory_iterator(base_path)) {
            if (dir_entry.path().extension() == ".txt") {
                std::ifstream file(dir_entry.path());
                std::string word;
                while (std::getline(file, word)) {
                    if (!word.empty()) {
                        vocabulary.push_back(word);
                    }
                }
            }
        }
    }

    std::vector<sample_type> encode_samples(const std::vector<std::string>& lines) {
        std::vector<sample_type> samples;
        for (const auto& line : lines) {
            sample_type sample(vocabulary.size());
            sample = 0;
            for (size_t i = 0; i < vocabulary.size(); ++i) {
                if (line.find(vocabulary[i]) != std::string::npos) {
                    sample(i) = 1.0;
                }
            }
            samples.push_back(sample);
        }

        return samples;
    }

    void train(const std::string& output_file) {
        std::vector<sample_type> samples;
        std::vector<label_type> labels;
        std::map<std::string, int> label_map;

        label_type label_index = 0;
        //std::vector<std::string> vocabulary = build_vocabulary(base_path);

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

                auto encoded = encode_samples(lines);
                for (auto& sample : encoded) {
                    samples.push_back(sample);
                    labels.push_back(label_index);
                }

                ++label_index;
            }
        }

        trainer_type trainer;
        ovo_trainer o_trainer;
        o_trainer.set_trainer(trainer);

        df_type df = o_trainer.train(samples, labels);

        serialize(output_file) << df;
        serialize(output_file + ".vocabulary") << vocabulary;

        std::cout << "Trained and saved classifier to: " << output_file << std::endl;
    }
};