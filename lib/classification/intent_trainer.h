#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "../util.h"
#include "../types/definitions.h"

namespace fs = std::filesystem;

class intent_trainer {
    output out;
    std::vector<std::string> vocabulary;
    df_type df_trainer;

public:
    std::string base_path;

    intent_trainer(std::string base_path) : 
        out("Trainer"),
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

    void train() {
        std::vector<sample_type> samples;
        std::vector<label_type> labels;
        std::map<std::string, int> label_map;

        out("Training model");
        label_type label_index = 0;
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

        df_trainer = o_trainer.train(samples, labels);
        out("Model trained successfully.");
    }

    void save(const std::string& output_file) {
        fs::path out_path = fs::path(output_file);
        fs::directory_entry parent_dir = fs::directory_entry(out_path.parent_path());
        if (!parent_dir.exists()) {
            fs::create_directory(out_path.parent_path());
        }

        if (!parent_dir.exists()) {
            out.err("Error creating directory: ", out_path.parent_path());
            return;
        }

        out("Saving model data to file: ", output_file);
        out("\tModel file: ", output_file, ".dat");
        out("\tVocabulary file: ", output_file, ".vocab");

        serialize(output_file + ".dat") << df_trainer;

        std::ofstream out_file(output_file + ".vocab");
        if (!out_file.is_open()) {
            out.err("Failed to create vocabulary file...");
            return;
        }

        for (const auto& word : vocabulary) {
            out_file << word << std::endl;
        }

        out_file.close();
        out("Saved classifier to: ", output_file);
    }
};