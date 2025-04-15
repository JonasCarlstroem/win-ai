#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

struct lex_entry {
    std::string number;
    std::string name;
    //std::string raw_line;
    std::string pos;
    std::vector<std::string> terms;
};

struct lex_section {
    std::string number;
    std::string name;
    std::vector<lex_entry> entries;
    std::vector<lex_section> sub_sections;

    int start_row;
};

typedef std::unordered_map<std::string, lex_section> lex_section_index;

struct lex_class {
    std::string name;
    std::string description;
    lex_section_index sections;

    int start_row;
};

typedef std::unordered_map<std::string, lex_class> lex_class_index;
typedef std::vector<lex_entry> lex_verbs;
typedef std::unordered_map<std::string, lex_verbs> lex_verbs_index;