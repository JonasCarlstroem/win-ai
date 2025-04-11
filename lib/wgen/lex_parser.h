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

#define _R(reg) std::regex(R##reg)
#define REG_CLASS _R("(CLASS\s+([IVXLCDM]+))")
#define REG_SECTION _R("(SECTION\s+([IVXLCDM]+)\.)")
#define REG_ENTRY_HEADER _R("(^(\d+)\.\s+[A-Z][A-Z, ]+$)")
#define REG_ENTRY _R("(^(\d+)\.\s+(.*?)\s+[-—–]\s+([A-Z]+)\.?)")

char REG_CLS[] = "(CLASS\s+([IVXLCDM]+))";
char REG_SEC[] = "(SECTION\s+([IVXLCDM]+)\.)";
char REG_ENT_H[] = "(^(\d+)\.\s+[A-Z][A-Z, ]+$)";
char REG_ENT[] = "(^(\d+)\.\s+(.*?)\s+[-—–]\s+([A-Z]+)\.?)";

class lex_parser {
public:
    lex_parser(const std::string& lex_file, output* out) : out(out), _index(), _ifs(lex_file) {
        if (!_ifs.is_open()) {
            (*out)("Error opening file: ", lex_file);
            exit(-1);
        }
        else {
            extract_indices();
        }
    }

    void display_toc() {
        (*out)("--- Table of content ---");
        (*out)("");

        for (const auto& index : _index) {
            auto& lc = index.second;
            (*out)(lc.name, " - ", lc.description, " (Row: ", lc.start_row, ")");
            for (const auto& pair : index.second.sections) {
                const auto& section = pair.second;
                (*out)("\t", section.number, ": ", section.name, " (Row: ", section.start_row, ")");
            }
            (*out)("");
        }
    }

    void display_section(int section) {
        std::string section_str = std::to_string(section);
        for (const auto& [class_name, lex_cls] : _index) {
            auto it = lex_cls.sections.find(section_str);
            if (it != lex_cls.sections.end()) {
                const auto& section = it->second;

                (*out)("--- Section ", section.number, ": ", section.name, " ---");
                for (const auto& entry : section.entries) {
                    (*out)(entry.number, ". ", join(entry.terms, ", "), " [", entry.pos, "]");
                }
                return;
            }
        }

        (*out)("Section ", section, " not found.");
    }

    std::string find_section() {

    }

private:
    output* out;

    lex_class_index _index;
    lex_verbs_index _verbs;
    std::unordered_map<std::string, std::string> _entry_headers;

    std::ifstream _ifs;

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

    bool next_line() {
        if (std::getline(_ifs, entry.current_iteration_values.line)) {
            c_iteration().current_row++;
            return true;
        }
        return false;
    }

    std::string peek_next_line() {
        std::streampos current_pos = _ifs.tellg();

        std::string next;
        if (std::getline(_ifs, next)) {
            _ifs.seekg(current_pos);
        }
        else {
            _ifs.clear();
            _ifs.seekg(current_pos);
        }

        return next;
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

        if (out) {
            (*out)("Header: ", line, " at row ", row);
        }
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

        if (out) {
            (*out)("Entry: ", entry_number, " | POS: ", pos, " | Terms: ", join(terms, ", "));
        }

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

    void extract_indices() {
        c_iteration().current_row = 0;

        while (next_line()) {
            if (match_class()) {
                extract_class();
            } 
            else if (match_section()) {
                extract_section();
            }
            else if (match_entry_header()) {
                extract_entry_header();
            }
            else if (match_entry()) {
                extract_entry();
            }
        }

        entry = { };
    }

    bool match_class() {
        return match_pat(REG_CLS);
    }

    bool match_section() {
        return match_pat(REG_SEC);
    }

    bool match_entry_header() {
        return match_pat(REG_ENT_H);
    }

    bool match_entry() {
        return match_pat(REG_ENT);
    }

    //bool match_pat(const std::regex& pattern) {
    //    auto& [_, line, match] = c_iteration();
    //    if (!std::regex_match(line, match, pattern)) {
    //        (*out)("Regex \"", pattern, "\" did not match line \"", line, "\"");
    //        return false;
    //    }
    //    return true;
    //}

    template<size_t N>
    bool match_pat(const char(&pattern)[N]) {
        auto& [_, line, match] = c_iteration();
        if (!std::regex_match(line, match, std::regex(pattern))) {
            (*out)("Regex \"", pattern, "\" did not match line \"", line, "\"");
            return false;
        }
        return true;
    }
};