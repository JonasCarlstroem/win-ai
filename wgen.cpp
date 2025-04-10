#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <set>

namespace fs = std::filesystem;

std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    const auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split_words(const std::string& line) {
    std::vector<std::string> words;
    std::stringstream ss(line);

    std::string token;
    while (std::getline(ss, token, ',')) {
        auto semi_pos = token.find(";");
        if (semi_pos != std::string::npos) token = token.substr(0, semi_pos);
        token = trim(token);
        if (!token.empty() && token.find('&') == std::string::npos && token.find("[") == std::string::npos) {
            words.push_back(token);
        }
    }

    return words;
}

int main() {
    std::ifstream lex("thesaurus.txt");
    if (!lex.is_open()) {
        std::cerr << "Failed to open file...\n";
        return 1;
    }

    std::unordered_map<int, std::string> section_map = {
        { 66, "open" },
        { 151, "event" }
    };

    std::unordered_map<std::string, std::set<std::string>> extracted_verbs;


    return 0;
}