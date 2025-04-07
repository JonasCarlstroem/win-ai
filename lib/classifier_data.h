#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <dlib/mlp.h>

using namespace dlib;
//typedef matrix<double, 0, 1> matrix<double, 0, 1>;
typedef std::vector<double> one_hot;
template<typename T>
using hot_labels = std::vector<T>;
typedef std::vector<std::vector<double>> one_hot_labels;
typedef unsigned long ulong;

std::vector<double> extract_features(const std::string& input, size_t max_len = 20) {
    std::vector<double> features;
    for (char c : input) {
        if (isalpha(c)) {
            //features.push_back(tolower(c) - 'a');
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

hot_labels<one_hot> get_one_hot_labels(const std::vector<ulong> labels, int output_size = 4) {
    hot_labels<one_hot> hot_labels;
    for (const auto& label : labels) {
        one_hot one_h(output_size, 0.0f);
        one_h[label] = 1.0f;
        
        hot_labels.push_back(one_h);
    }

    return hot_labels;
}

std::vector<matrix<double, 0, 1>> get_samples(const std::vector<std::string>& texts) {
    std::vector<matrix<double, 0, 1>> samples;
    for (const auto& text : texts) {
        auto vec = extract_features(text);
        
        matrix<double, 0, 1> m(vec.size());
        for (size_t i = 0; i < vec.size(); ++i) m(i) = vec[i];
        samples.push_back(m);
    }

    return samples;
}