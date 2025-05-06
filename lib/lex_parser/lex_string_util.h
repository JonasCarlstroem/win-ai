#pragma once
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>

struct range {
    size_t start;
    size_t end;
};

struct raw_class {
    std::string name;
    std::string description;
    std::string raw_class_text;
};

struct raw_section {
    std::string num;
    std::string name;
    std::string raw_section_text;
};

struct raw_entry_header {
    int num;
    std::string name;
};

struct raw_entry {
    raw_entry_header header;
    std::string raw_entry_text;
};


static constexpr std::string_view PREFIX_CLASS = "CLASS ";
static constexpr std::string_view PREFIX_SECTION = "SECTION ";

inline std::string trim_copy(const std::string& s);


inline std::string get_and_strip_class_name(std::string& str) {
    size_t name_start = str.find(PREFIX_CLASS);
    if (name_start == std::string::npos) return "";

    size_t name_end = str.find_first_of("\r\n", PREFIX_CLASS.size());
    if (name_end == std::string::npos) name_end = str.size();

    std::string class_name = str.substr(name_start, name_end - name_start);
    str = trim_copy(str.substr(class_name.size()));
    return class_name;
}

inline std::string get_and_strip_class_description(std::string& str) {
    size_t desc_start = str.find_first_not_of(" \r\n");
    if (desc_start == std::string::npos) return "";

    size_t desc_end = str.find_first_of("\r\n", desc_start + 1);
    if (desc_end == std::string::npos) desc_end = str.size();

    std::string class_desc = str.substr(desc_start, desc_end - desc_start);
    str = trim_copy(str.substr(class_desc.size()));
    return class_desc;
}

inline std::string get_and_strip_section_num(std::string& str) {
    size_t sec_start = str.find(PREFIX_SECTION);
    if (sec_start == std::string::npos) return "";

    size_t num_start = str.find_first_not_of(' ', sec_start + PREFIX_SECTION.size());
    if (num_start == std::string::npos) return "";

    size_t num_end = str.find('.');
    if (num_end == std::string::npos) return "";

    std::string sec_num = str.substr(num_start, num_end - num_start);
    str = trim_copy(str.substr(num_end + 1));
    return sec_num;
}

inline std::string get_and_strip_section_name(std::string& str) {
    size_t name_start = str.find_first_not_of(" \r\n");
    if (name_start == std::string::npos) return "";

    size_t name_end = str.find_first_of("\r\n", name_start + 1);
    if (name_end == std::string::npos) name_end = str.size();

    std::string sec_name = str.substr(name_start, name_end - name_start);
    str = trim_copy(str.substr(sec_name.size()));
    return sec_name;
}

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

bool is_line_sub_section(const std::string_view line) {
    size_t dot_pos = line.find('.');
    if (dot_pos == std::string_view::npos) return false;

    std::string_view num_part = line.substr(0, dot_pos);
    if (!std::all_of(num_part.begin(), num_part.end(), ::isdigit)) return false;

    size_t label_start = line.find_first_not_of(" ", dot_pos + 1);
    if (label_start == std::string_view::npos) return false;

    std::string_view label_part = line.substr(label_start);
    if (label_part.empty() || !std::all_of(label_part.begin(), label_part.end(), [](char c) {
        return std::isupper(c) || std::isspace(c) || std::ispunct(c);
    })) return false;

    return true;
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

inline std::string get_line(std::string str, size_t* pos = 0) {
    if (pos == nullptr) pos = new size_t{ 0 };
    size_t line_end = str.find_first_of("\r\n", *pos);

    if (line_end == 0) return "";
    if (line_end == std::string::npos) line_end = str.size();
    
    std::string line = str.substr(*pos, line_end);
    *pos = line.size();
    //str = trim_copy(str.substr(line.size()));
    return line;
}

inline raw_entry_header get_and_strip_entry_header(std::string& str) {
    std::string line = get_line(str);
    if (is_line_sub_section(line)) {
        str = trim_copy(str.substr(line.size()));
        line = get_line(str);
    }

    size_t dot_pos = line.find('.');
    std::string_view num_part = line.substr(0, dot_pos);
    size_t label_start = line.find_first_not_of(" ", dot_pos + 1);
    size_t label_end = line.find_first_of(" ", label_start + 1);
    std::string_view label_part = line.substr(label_start, label_end - label_start);

    int num; // = std::stoi(std::string(num_part));
    std::string name;
    if (is_line_entry_header(line,  num, name)) return { num, name };
    return {};
}

inline size_t get_entry_start_len(std::string& str, size_t start = 0) {
    bool found = false;
    size_t pos = start;
    while (!found) {
        std::string line = get_line(str, &pos);
        if (is_line_sub_section(line)) {
            line = get_line(str, &pos);
        }

        if (is_line_entry_header(line)) return pos;
    }

    return -1;
}

inline size_t get_entry_start(std::string& str, size_t start = 0) {
    bool found = false;
    size_t pos = start;
    while (!found) {
        std::string line = get_line(str, &pos);
        if (is_line_sub_section(line)) {
            line = get_line(str);
        }

        if (is_line_entry_header(line)) return pos - line.size();
    }

    return -1;
}

inline size_t get_entry_end(std::string& str, size_t start = 0) {
    bool found = false;
    size_t pos = start;
    while (!found) {
        std::string line = get_line(str, &pos);
        if (is_line_sub_section(line)) {
            line = get_line(str);
        }

        if (is_line_entry_header(line)) return pos;
    }

    return -1;
}

inline std::string get_entry_header(std::string& content) {
    bool found = false;
    size_t pos = 0;
    while (!found) {
        std::string line = get_line(content, &pos);

        if (is_line_entry_header(line)) {
            content.replace(pos, line.length(), "");
            return line;
        }

        if (pos == content.size()) break;
    }

    return "";
}

inline void strip_sub_section(std::string& str) {
    std::string line = get_line(str);
    if (is_line_sub_section(line)) {
        str = trim_copy(str.substr(line.size()));
    }
}

inline void strip_sub_sections(std::string& str) {
    size_t start = 0;
    int stripped = 0;
    while (start < str.size()) {
        size_t end = str.find_first_of("\r\n", start);
        if (end == std::string::npos) end = str.size();

        std::string line = str.substr(start, end - start);

        if (is_line_sub_section(line)) {
            size_t erase_len = (end < str.size() && str[end] == '\r' && end + 1 < str.size() && str[end + 1] == '\n') ? 2 :
                ((end < str.size() && (str[end] == '\n' || str[end] == '\r')) ? 1 : 0);

            //str = trim_copy(str.replace(start, pos, ""));
            str.erase(start, (end - start) + erase_len);
            stripped += 1;
        }
        else {
            start = end + 1;
        }
    }
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

inline void replace(std::string& s, char char_to_replace, char replace_with) {
    for (auto& c : s) {
        if (c == char_to_replace) {
            c = replace_with;
        }
    }
}

inline void replace(std::string& s, const char* str_to_replace, const char* replace_with) {
    size_t start = 0;
    size_t from_len = strlen(str_to_replace);
    size_t to_len = strlen(replace_with);

    if (from_len == 0) return;

    while ((start = s.find(str_to_replace, start)) != std::string::npos) {
        s.replace(start, from_len, replace_with);
        start += to_len;
    }
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