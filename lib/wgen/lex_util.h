#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <regex>

#define _REGEX(pattern) std::regex(##pattern, std::regex_constants::optimize)
#define DECL_REGEX(name, pattern) std::regex REG_##name(##pattern, std::regex_constants::optimize)

#define _PATTERN(name, pattern)                                  \
const auto& PATTERN_##name = ##pattern;

#define _REG(name, pattern)                                     \
_PATTERN(name, pattern)                                         \
const static DECL_REGEX(name, pattern)

#define _REG_C(name, pattern, constant)                         \
_PATTERN(name, pattern)                                         \
const static DECL_REGEX(name, pattern)

#define REPLACE(term, pattern, rep_char)                        \
std::regex_replace(##term, ##pattern, ##rep_char)

#define IF_MATCH(term, pattern)                                 \
if(std::regex_match(##term, ##pattern))

#define MATCH(line, match, pattern) std::regex_match(##line, ##match, ##pattern);

///Only works with strings
#define EMPTY(term) (##term.empty() && ##term.length() <= 1)

///Only works with strings
#define NOT_EMPTY(term) (!##term.empty() && ##term.length() > 1)

#define CURRENT_CLASS _index[c_class().current_class]
#define CURRENT_SECTION CURRENT_CLASS.sections[c_section().section_number]

_REG(CLASS, R"(CLASS\s+([IVXLCDM]+))");
_REG(SECTION, R"(SECTION\s+([IVXLCDM]+)\.)");
_REG(ENTRY_HEADER, R"(^(\d+)\.\s+[A-Z][A-Za-z, ]+$)");
_REG(ENTRY, R"((\d+)\.\s+(.*?)(?:\s*-\s*([A-Z]+\.?))?)");
_REG(SPLITTER, R"(([^,;\.]+))");
_REG(NUM_FILTER, R"(\d+)");
_REG(HEADING, R"(^(\d+)\.\s*(?:\[[^\]]*\]\.\s*)?(.+?)\s*[—\-]\s*([A-Za-z\.]+)\.?)");
_REG(NUM_REMOVE, R"(\b\d+\b)");
_REG_C(POS_REMOVE, R"(\b(?:adj|adv|n|v)\b\.?)", std::regex_constants::icase);

const static std::unordered_map<const std::regex*, const char*> pattern_map = {
    { &REG_CLASS, PATTERN_CLASS },
    { &REG_SECTION, PATTERN_SECTION },
    { &REG_ENTRY_HEADER, PATTERN_ENTRY_HEADER },
    { &REG_ENTRY, PATTERN_ENTRY }
};

const static std::vector<std::pair<std::regex, std::string>> cleaning_match = {
    { _REGEX(R"(\[.*?\])"), "" },
    { _REGEX(R"(&c)"), "" },
    { _REGEX(R"(\s*;\s*)"), "," }
};

const static std::vector<std::pair<std::regex, std::string>> replace_match = {
    { _REGEX(R"(\s{2,})"), " " },
    { _REGEX(R"(\(\s*[^)]+\s*\)\s*\d+)"), "" },
    { _REGEX(R"(\d+\.)"), "" },
    { _REGEX(R"(V\.\s*)"), "" },
    { _REGEX(R"(\b\d+\b)"), "" },
    { _REGEX(R"(\b(?:adj|adv|n|v)\b\.?)", std::regex_constants::icase), "" }
};

inline void trim(std::string& s) {
    auto is_trim_char = [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    };

    s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), is_trim_char));
    s.erase(std::find_if_not(s.rbegin(), s.rend(), is_trim_char).base(), s.end());
    //const auto str_begin = std::find_if_not(s.begin(), s.end(), is_trim_char);
    //if (str_begin == s.end()) return "";

    //const auto str_end = std::find_if_not(s.rbegin(), s.rend(), is_trim_char).base();
    //s = std::string(str_begin, str_end);
    //return s;
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

std::string& normalize_dashes(std::string& str) {
    static const std::vector<std::string_view> dashes = {
        "\xE2\x80\x93", // – EN DASH
        "\xE2\x80\x94", // — EM DASH
        "\xE2\x80\x95", // ‐ HORIZONTAL BAR
        "\xE2\x88\x92"  // − MINUS SIGN
    };
    static const std::string_view dash_chars = "\u2013\u2014\u2212\u2012\u2011";

    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size();) {
        unsigned char c = static_cast<unsigned char>(str.at(i));

        if (c < 0x80) {
            result += str[i++];
        }
        else {
            if (str.compare(i, 3, "\xE2\x80\x93") == 0 ||
                str.compare(i, 3, "\xE2\x80\x94") == 0 ||
                str.compare(i, 3, "\xE2\x88\x92") == 0 ||
                str.compare(i, 3, "\xE2\x80\x92") == 0 ||
                str.compare(i, 3, "\xE2\x80\x91") == 0) {
                result += '-';
                i += 3;
            }
            else {
                result += str[i++];
            }
        }
    }

    return str = result;
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