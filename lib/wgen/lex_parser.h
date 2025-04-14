#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <set>
#include <regex>
#include "../util.h"
#include "lex_types.h"

#define _REG(name, pattern)                       \
const auto& PATTERN_##name = ##pattern;           \
const std::regex REG_##name(PATTERN_##name);

_REG(CLASS, R"(CLASS\s+([IVXLCDM]+))");
_REG(SECTION, R"(SECTION\s+([IVXLCDM]+)\.)");
_REG(ENTRY_HEADER, R"(^(\d+)\.\s+[A-Z][A-Z, ]+$)");
//_REG(ENTRY, R"((\d+)\.\s+(.*?)\s*-\s*([A-Z]+\.?))");
_REG(ENTRY, R"((\d+)\.\s+(.*?)(?:\s*-\s*([A-Z]+\.?))?)");
//_REG(ENTRY, R"((\d*)\.\s*(.*?)(?:\s*-\s*([A-Z]+\.?))?)");
//_REG(ENTRY, R"((?:(\d+)\.\s+)?([\w\-']+)(?:\s*-\s*([A-Z]+\.?))?)");

std::unordered_map<const std::regex*, const char*> pattern_map = {
    { &REG_CLASS, PATTERN_CLASS },
    { &REG_SECTION, PATTERN_SECTION },
    { &REG_ENTRY_HEADER, PATTERN_ENTRY_HEADER },
    { &REG_ENTRY, PATTERN_ENTRY }
};

const std::vector<std::pair<std::regex, std::string>> cleaning_regex = {
    { std::regex(R"(\[.*?\])"), "" },
    { std::regex(R"(&c)"), "" },
    { std::regex(R"(\s*;\s*)"), "," }
};

class lex_parser {
private:
    struct {
        struct {
            std::string current_class;
            std::string current_description;
            int class_start_row = 0;
        } current_class;

        struct {
            std::string section_number;
            std::string section_name;
            int section_start_row = 0;
        } current_section;

        struct {
            int current_row = 0;
            std::string line;
            std::smatch match;
        } current_iteration_values;
    } entry;

    auto& c_class() { return entry.current_class; }
    auto& c_section() { return entry.current_section; }
    auto& c_iteration() { return entry.current_iteration_values; }

public:
    lex_parser(const std::string& lex_file, output& out = output("lex_parser")) 
        : out(out), _lex_file(lex_file), _index() {}

    void display_toc() {
        out("--- Table of content ---");
        out("");

        for (const auto& index : _index) {
            auto& lc = index.second;
            out(lc.name, " - ", lc.description, " (Row: ", lc.start_row, ")");
            for (const auto& pair : index.second.sections) {
                const auto& section = pair.second;
                out("\t", section.number, ": ", section.name, " (Row: ", section.start_row, ")");
            }
            out("");
        }
    }

    void display_section(int section) {
        std::string section_str = std::to_string(section);
        for (const auto& [class_name, lex_cls] : _index) {
            auto it = lex_cls.sections.find(section_str);
            if (it != lex_cls.sections.end()) {
                const auto& section = it->second;

                out("--- Section ", section.number, ": ", section.name, " ---");
                for (const auto& entry : section.entries) {
                    out(entry.number, ". ", join(entry.terms, ", "), " [", entry.pos, "]");
                }
                return;
            }
        }

        out("Section ", section, " not found.");
    }

    std::string find_section() {

    }

    void init() {
        std::ifstream file(_lex_file, std::ios::binary);

        if (!file.is_open()) {
            out.err("Error opening file: ", _lex_file);
            exit(-1);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        normalize_dashes(content);

        size_t start = 0, end;
        while ((end = content.find('\n', start)) != std::string::npos) {
            std::string line = content.substr(start, end - start);
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            _lines.emplace_back(line);
            start = end + 1;
        }

        if (start < content.size()) {
            std::string line = content.substr(start);
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            _lines.emplace_back(line);
        }
    }

    void extract_indices() {
        auto& [row, cline, _] = c_iteration();
        row = 0;

        std::smatch match;
        std::vector<std::string> entry_lines;
        bool extracting_entry = false;
        for (const auto& line : _lines) {
            row++;
            cline = line;
            

            if (match_class()) {
                if (extracting_entry) extract_entry(entry_lines);

                extracting_entry = false;
                extract_class();
            }
            else if (match_section()) {
                if (extracting_entry) extract_entry(entry_lines);

                extracting_entry = false;
                extract_section();
            }
            else if (match_entry_header()) {
                if (extracting_entry) extract_entry(entry_lines);

                extracting_entry = false;
                extract_entry_header();
            }
            else if (extracting_entry || match_entry()) {
                extracting_entry = true;
                entry_lines.emplace_back(cline);
            }
        }

        entry = { };
    }

private:
    output& out;

    std::string _lex_file;
    lex_class_index _index;
    lex_verbs_index _verbs;
    std::unordered_map<std::string, std::string> _entry_headers;

    std::istringstream _iss;
    std::vector<std::string> _lines;

    void normalize_dashes(std::string& str) {
        static const std::vector<std::string> dashes = {
            "\xE2\x80\x93", // – EN DASH
            "\xE2\x80\x94", // — EM DASH
            "\xE2\x80\x95", // ‐ HORIZONTAL BAR
            "\xE2\x88\x92"  // − MINUS SIGN
        };
        static const std::string dash_chars = "\u2013\u2014\u2212\u2012\u2011";

        std::string result;
        result.reserve(str.size());

        for (size_t i = 0; i < str.size();) {
            unsigned char c = static_cast<unsigned char>(str[i]);

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
        
        str = std::move(result);
    }

    bool next_line() {
        if (std::getline(_iss, entry.current_iteration_values.line)) {
            c_iteration().current_row++;
            return true;
        }
        return false;
    }

    std::string peek_next_line() {
        std::streampos current_pos = _iss.tellg();

        std::string next;
        if (std::getline(_iss, next)) {
            _iss.seekg(current_pos);
        }
        else {
            _iss.clear();
            _iss.seekg(current_pos);
        }

        return next;
    }

    std::string peek_next_line(int line) {
        const std::string& current_line = _lines[line];
        const std::string* next_line = (line + 1 < _lines.size()) ? &_lines[line + 1] : nullptr;

        if (next_line) {
            return *next_line;
        }
        else {
            return "";
        }
    }

    void extract_class() {
        auto& [cls_name, cls_desc, cls_row] = c_class();

        cls_row = c_iteration().current_row;
        cls_name = c_iteration().line;
        cls_desc = peek_next_line();

        lex_class class_entry = {
            cls_name,
            cls_desc,
            {},
            cls_row
        };

        _index[class_entry.name] = class_entry;
    }

    void extract_section() {
        auto& [sec_num, sec_name, sec_row] = c_section();
        auto& [cls_name, cls_desc, cls_row] = c_class();
        auto [current_row, line, match] = c_iteration();

        sec_row = c_iteration().current_row;
        sec_num = c_iteration().match[1];
        sec_name = peek_next_line();

        lex_section section = {
            sec_num,
            sec_name,
            {},
            sec_row
        };

        _index[cls_name].sections[section.number] = std::move(section);
    }

    void extract_entry_header() {
        auto [row, line, _] = c_iteration();
        std::string header_number = line.substr(0, line.find('.'));
        std::string header_text = line;

        _entry_headers[std::move(header_number)] = std::move(header_text);

        out("Header: ", line, " at row ", row);
    }

    void extract_entry() {
        auto& [row, line, match] = c_iteration();

        std::string entry_number = match[1];
        std::string entry_terms_raw = match[2];
        std::string pos = match[3];

        std::vector<std::string> terms = split(entry_terms_raw, ',');

        for (auto& term : terms) {
            term = trim(term);
        }

        out("Entry: ", entry_number, " | POS: ", pos, " | Terms: ", join(terms, ", "));

        lex_entry entry = {
            entry_number,
            line,
            pos,
            terms
        };

        auto& cls = _index[c_class().current_class];
        auto& section = cls.sections[c_section().section_number];
        section.entries.push_back(entry);

        _verbs[pos].push_back(std::move(entry));
    }

    void extract_entry(std::vector<std::string>& lines) {
        if (lines.empty()) return;

        std::smatch match;
        const std::string& heading = lines[0];
        if (std::regex_match(heading, match, std::regex(R"(^(\d+)\.\s+(.+?)(?:\s*[-—]\s*([A-Za-z\.]+))?$)"))) {
            std::string entry_number = match[1];
            std::string entry_name = match[2];
            std::string pos = match[3];

            entry_name = clean_name(entry_name);

            std::vector<std::string> terms;
            for (size_t i = 1; i < lines.size(); ++i) {
                std::string line = lines[i];

                //auto semicolon_pos = line.find(';');
                //if (semicolon_pos != std::string::npos) {
                //    line = line.substr(0, semicolon_pos);
                //}
                line = clean_term_annotations(line);

                std::istringstream iss(line);
                std::string term;
                while (std::getline(iss, term, ',')) {
                    trim(term);
                    if (!term.empty()) {
                        terms.emplace_back(term);
                    }
                }
            }
            
            lex_entry entry = {
                entry_number,
                entry_name,
                pos,
                terms
            };

            auto& cls = _index[c_class().current_class];
            auto& section = cls.sections[c_section().section_number];
            section.entries.push_back(entry);

            _verbs[pos].push_back(std::move(entry));
            lines.clear();
        }
    }

    bool match_class() {
        return match_pat(REG_CLASS);
    }

    bool match_section() {
        return match_pat(REG_SECTION);
    }

    bool match_entry_header() {
        return match_pat(REG_ENTRY_HEADER);
    }

    bool match_entry() {
        return match_pat(REG_ENTRY);
    }

    bool match_pat(const std::regex& pattern) {
        auto& [_, line, match] = c_iteration();
        //return std::regex_match(line, match, pattern);
        if (!std::regex_match(line, match, pattern)) {
            out("Regex \"", pattern_map[&pattern], "\" did not match line \"", line, "\"");
            return false;
        }
        return true;
    }

    template<size_t N>
    bool match_pat(const char(&pattern)[N]) {
        auto& [_, line, match] = c_iteration();
        if (!std::regex_match(line, match, std::regex(pattern))) {
            out("Regex \"", pattern, "\" did not match line \"", line, "\"");
            return false;
        }
        return true;
    }

    std::string clean_name(const std::string& name) {
        std::string cleaned_name = name;
        
        cleaned_name.erase(std::remove(cleaned_name.begin(), cleaned_name.end(), ','), cleaned_name.end());

        auto pos = cleaned_name.find('[');
        if (pos != std::string::npos) {
            cleaned_name = cleaned_name.substr(0, pos);
        }

        return cleaned_name;
    }

    std::string clean_term_annotations(const std::string& term) {
        std::string cleaned_term = term;

        for (const auto& pair : cleaning_regex) {
            const std::regex& reg = pair.first;
            const std::string& rep = pair.second;
            cleaned_term = std::regex_replace(cleaned_term, reg, rep);
        }

        trim(cleaned_term);
        return cleaned_term;
    }
};