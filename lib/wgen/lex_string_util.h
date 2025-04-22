#pragma once
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>

//using matcher_rom_fn = bool(*)(std::string_view, std::string&);

static constexpr std::string_view PREFIX_CLASS = "CLASS ";
static constexpr std::string_view PREFIX_SECTION = "SECTION ";

inline bool starts_with(const std::string_view str, const std::string_view prefix) {
    return str.size() >= prefix.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}

bool is_line_class(const std::string_view line) {
    return starts_with(line, PREFIX_CLASS);
}

bool is_line_class(const std::string_view line, std::string& roman_num_out) {
    if (is_line_class(line)) {
        auto rest = line.substr(PREFIX_CLASS.size());
        size_t i = 0;
        while (i < rest.size() && std::isupper(rest[i]) && std::string("IVXLCDM").find(rest[i]) != std::string::npos) {
            ++i;
        }

        if (i > 0) {
            roman_num_out = std::string(rest.substr(0, i));
            return true;
        }
    }

    return false;
}

bool is_line_section(const std::string_view line) {
    return starts_with(line, PREFIX_SECTION);
}

bool is_line_section(const std::string_view line, std::string& roman_num_out) {
    if (is_line_section(line)) {
        auto rest = line.substr(PREFIX_SECTION.size());
        size_t i = 0;
        while (i < rest.size() && std::isupper(rest[i]) && std::string("IVXLCDM").find(rest[i]) != std::string::npos) {
            ++i;
        }

        if (i > 0) {
            roman_num_out = std::string(rest.substr(0, i));
            return true;
        }
    }

    return false;
}

bool is_line_entry_header(const std::string_view line) {
    size_t dot_pos = line.find('.');
    if (dot_pos == std::string_view::npos) return false;

    std::string_view num_part = line.substr(0, dot_pos);
    if (!std::all_of(num_part.begin(), num_part.end(), ::isdigit)) return false;

    size_t label_start = line.find_first_not_of(" ", dot_pos + 1);
    if (label_start == std::string_view::npos) return false;

    std::string_view label_part = line.substr(label_start);
    if (label_part.empty() || !std::isupper(label_part[0])) return false;

    return true;
}

bool is_line_entry_header(const std::string_view line, int& entry_num_out, std::string& label_out) {
    if (!is_line_entry_header(line)) return false;

    size_t dot_pos = line.find('.');
    std::string_view num_part = line.substr(0, dot_pos);
    size_t label_start = line.find_first_not_of(" ", dot_pos + 1);
    std::string_view label_part = line.substr(label_start);

    if (label_part.empty() || !std::isupper(label_part[0])) return false;

    entry_num_out = std::stoi(std::string(num_part));
    label_out = std::string(label_part);
    return true;
}

bool is_term_number(const std::string_view term) {
    return std::all_of(term.begin(), term.end(), ::isdigit);
}

bool is_term_pos_tag(const std::string_view term) {
    static const std::unordered_set<std::string> tags = { "adj", "adv", "n", "v" };
    std::string lower;
    lower.reserve(term.size());
    for (char ch : term) lower += std::tolower(ch);
    return tags.count(lower);
}

std::string reduce_whitespace(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    bool prev_space = false;

    for (char ch : input) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!prev_space) {
                result += ' ';
                prev_space = true;
            }
        }
        else {
            result += ch;
            prev_space = false;
        }
    }

    return result;
}

std::string remove_parenthesis_with_number(const std::string& input) {
    std::string result;
    result.reserve(input.size());

    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == '(') {
            size_t end = input.find(')', i);
            if (end != std::string::npos) {
                size_t num_start = end + 1;
                while (num_start < input.size() && std::isspace(input[num_start])) ++num_start;

                size_t num_end = num_start;
                while (num_end < input.size() && std::isdigit(input[num_end])) ++num_end;

                if (num_end > num_start) {
                    i = num_end;
                    continue;
                }
            }
        }
        result += input[i++];
    }

    return result;
}

std::string remove_number_dot(const std::string& input) {
    std::string result;
    result.reserve(input.size());

    size_t i = 0;
    while (i < input.size()) {
        if (std::isdigit(static_cast<unsigned char>(input[i]))) {
            size_t start = i;
            while (i < input.size() && std::isdigit(static_cast<unsigned char>(input[i]))) ++i;
            if (i < input.size() && input[i] == '.') {
                ++i;
                continue;
            }
            result.append(input.begin() + start, input.begin() + i);
        }
        else {
            result += input[i++];
        }
    }

    return result;
}

std::string remove_v_dot(const std::string& input) {
    std::string result;
    result.reserve(input.size());

    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == 'V') {
            size_t j = i + 1;
            while (j < input.size() && std::isspace(input[j])) ++j;
            if (j < input.size() && input[j] == '.') {
                ++j;
                while (j < input.size() && std::isspace(input[j])) ++j;
                i = j;
                continue;
            }
        }
        result += input[i++];
    }

    return result;
}

std::string remove_isolated_numbers(const std::string& input) {
    std::string result;
    std::stringstream ss(input);
    std::string word;
    while (ss >> word) {
        if (!std::all_of(word.begin(), word.end(), ::isdigit)) {
            result += word + " ";
        }
    }
    return result;
}

std::string remove_pos_tags(const std::string& input) {
    std::string result;
    std::stringstream ss(input);
    std::string word;
    static const std::unordered_set<std::string> tags = { "n", "v", "adj", "adv" };
    while (ss >> word) {
        std::string lower;
        for (char ch : word) lower += std::tolower(ch);
        if (lower.back() == '.') lower.pop_back();
        if (!tags.count(lower)) {
            result += word + " ";
        }
    }
    return result;
}

inline void trim(std::string& s) {
    auto is_trim_char = [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    };

    s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), is_trim_char));
    s.erase(std::find_if_not(s.rbegin(), s.rend(), is_trim_char).base(), s.end());

}

inline std::string trim_copy(const std::string& s) {
    static auto is_trim_char = [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    };

    std::string res(s);
    res.erase(res.begin(), std::find_if_not(res.begin(), res.end(), is_trim_char));
    res.erase(std::find_if_not(res.rbegin(), res.rend(), is_trim_char).base(), res.end());
    return res;
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