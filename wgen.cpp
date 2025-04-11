#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <set>
#include <filesystem>
#include "lib/util.h"
#include "lib/wgen/lex_parser.h"

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
    output out("wgen");

    bool index = true;
    bool parse = false;

    if (!fs::exists("thesaurus.txt")) {
        out.err("File not found...");
        return 1;
    }

    lex_parser parser("thesaurus.txt");
    parser.display_toc();

    //if (index) {
    //    std::unordered_map<std::string, lex_class> index = extract_section_index(lex);
    //    out("--- Table of content ---");
    //    out("");

    //    for (const auto& id : index) {
    //        lex_class lc = id.second;
    //        out("\t", lc.class_name, " - ", lc.class_description, " (Row: ", lc.start_row, ")");
    //        for (const auto& sec : id.second.sections) {
    //            out("\t\t", sec.section_number, ": ", sec.section_name, " (Row: ", sec.start_row, ")");
    //        }
    //    }
    //}

    //if (parse) {
    //    std::unordered_map<int, std::string> section_map = {
    //        { 66, "open" },
    //        { 151, "event" }
    //    };

    //    std::unordered_map<std::string, std::set<std::string>> extracted_verbs;

    //    std::string line;
    //    int current_section = -1;
    //    int current_row = 0;

    //    out("Parsing file...");
    //    while (std::getline(lex, line)) {
    //        current_row++;
    //        line = trim(line);

    //        std::smatch match;
    //        if (std::regex_search(line, match, std::regex(R"((\d+)\.\s+(.*))"))) {
    //            current_section = std::stoi(match[1].str());
    //            continue;
    //        }

    //        if (section_map.count(current_section) && line.find("V.") == 0) {
    //            std::string word_line = line.substr(2);
    //            auto words = split_words(word_line);
    //            for (const auto& word : words) {
    //                extracted_verbs[section_map[current_section]].insert(word);
    //            }
    //        }

    //        if (current_row % 100 == 0) {
    //            out("Current row: ", current_row);
    //        }
    //    }

    //    out("Finished parsing file");

    //    for (const auto& [label, words] : extracted_verbs) {
    //        out(label, ": ");

    //        for (const auto& word : words) {
    //            out("\t\t", word);
    //        }
    //    }
    //}

    return 0;
}