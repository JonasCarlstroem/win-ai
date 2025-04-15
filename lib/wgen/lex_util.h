#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <regex>

#define GET_MACRO(_0, _1, _2, NAME, ...) NAME
#define TEST(...) GET_MACRO(_0, ##__VA_ARGS__,TEST2, TEST1, TEST0)(__VA_ARGS__)

#define _PATTERN(name, pattern)                                  \
const auto& PATTERN_##name = ##pattern;

#define _REG(name, pattern)                                     \
_PATTERN(name, pattern)                                         \
const std::regex REG_##name(PATTERN_##name);

#define _REG_C(name, pattern, constant)                         \
_PATTERN(name, pattern)                                         \
const std::regex REG_##name(PATTERN_##name, ##constant)

#define REPLACE(term, pattern, rep_char)                        \
std::regex_replace(##term, ##pattern, ##rep_char)

#define IF_MATCH(term, pattern)                                 \
if(std::regex_match(##term, ##pattern))

///Only works with strings
#define EMPTY(term) (##term.empty() && ##term.length() <= 1)

///Only works with strings
#define NOT_EMPTY(term) (!##term.empty() && ##term.length() > 1)

_REG(CLASS, R"(CLASS\s+([IVXLCDM]+))");
_REG(SECTION, R"(SECTION\s+([IVXLCDM]+)\.)");
_REG(ENTRY_HEADER, R"(^(\d+)\.\s+[A-Z][A-Za-z, ]+$)");
_REG(ENTRY, R"((\d+)\.\s+(.*?)(?:\s*-\s*([A-Z]+\.?))?)");
_REG(SPLITTER, R"(([^,;\.]+))");
_REG(NUM_FILTER, R"(\d+)");
_REG(HEADING, R"(^(\d+)\.\s*(?:\[[^\]]*\]\.\s*)?(.+?)\s*[—\-]\s*([A-Za-z\.]+)\.?)");
_REG(NUM_REMOVE, R"(\b\d+\b)")
_REG_C(POS_REMOVE, R"(\b(?:adj|adv|n|v)\b\.?)", std::regex_constants::icase);

std::unordered_map<const std::regex*, const char*> pattern_map = {
    { &REG_CLASS, PATTERN_CLASS },
    { &REG_SECTION, PATTERN_SECTION },
    { &REG_ENTRY_HEADER, PATTERN_ENTRY_HEADER },
    { &REG_ENTRY, PATTERN_ENTRY }
};

const std::vector<std::pair<std::regex, std::string>> cleaning_match = {
    { std::regex(R"(\[.*?\])"), "" },
    { std::regex(R"(&c)"), "" },
    { std::regex(R"(\s*;\s*)"), "," }
};

const std::vector<std::pair<std::regex, std::string>> replace_match = {
    { std::regex(R"(\s{2,})"), " " },
    { std::regex(R"(\(\s*[^)]+\s*\)\s*\d+)"), "" },
    { std::regex(R"(\d+\.)"), "" },
    { std::regex(R"(V\.\s*)"), "" },
    { std::regex(R"(\b\d+\b)"), "" },
    { std::regex(R"(\b(?:adj|adv|n|v)\b\.?)", std::regex_constants::icase), "" }
};

inline std::string trim(std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    auto end = s.find_last_not_of(" \t\n\r");

    s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    return s;
}

inline std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

inline std::string join(const std::vector<std::string>& parts, const std::string& sep) {
    std::ostringstream os;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) os << sep;
        os << parts[i];
    }

    return os.str();
}

inline int roman_to_int(const std::string& roman) {
    static std::unordered_map<char, int> values = {
        { 'I', 1}, { 'V', 5 }, { 'X', 10 }, { 'L', 50 },
        { 'C', 100 }, { 'D', 500 }, { 'M', 1000 }
    };

    int total = 0;
    int prev = 0;

    for (int i = roman.size() - 1; i >= 0; --i) {
        int current = values[roman[i]];
        if (current < prev)
            total -= current;
        else
            total += current;

        prev = current;
    }

    return total;
}