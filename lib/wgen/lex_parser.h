#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <set>
#include <regex>

struct lex_section {
    std::string number;
    std::string name;
    int start_row;
};

typedef std::unordered_map<std::string, lex_section> lex_section_index;

struct lex_class {
    std::string name;
    std::string description;
    lex_section_index sections;

    int start_row;
};

typedef std::unordered_map<std::string, lex_class> lex_index;
typedef std::unordered_map<std::string, std::set<std::string>> lex_verbs;

class lex_parser {
public:
    lex_parser(const std::string& lex_file) : out("lex_parser"), _index() {
        std::ifstream is(lex_file);
        if (!is.is_open()) {
            out("Error opening file: ", lex_file);
            exit(-1);
        }
        else {
            extract_indices(is);
        }
    }

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

    }

    std::string find_section() {

    }

private:
    output out;
    lex_index _index;
    lex_verbs _verbs;

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
        } current_iteration;
    } entry;

    void extract_class() {
        
    }

    void extract_indices(std::ifstream& ifs) {
        auto& iter = entry.current_iteration;
        iter.current_row = 0;

        auto& next_line = [&] {
            if (std::getline(ifs, iter.line)) {
                iter.current_row++;
                return true;
            }
            return false;
        };
        auto& cls = entry.current_class;
        auto& sec = entry.current_section;
        while (next_line()) {
            if (match_pat(std::regex(R"(CLASS\s+([IVXLCDM]+))"))) {
                cls.class_start_row = iter.current_row;
                cls.current_class = iter.line;
                next_line();
                cls.current_description = iter.line;
            } 
            else if (match_pat(std::regex(R"(SECTION\s+([IVXLCDM]+)\.)"))) {
                sec.section_start_row = iter.current_row;
                sec.section_number = iter.match[1];
                next_line();
                sec.section_name = iter.line;

                auto& it = _index.find(cls.current_class);
                if (it != _index.end()) {
                    const auto& [sec_num, sec_name, sec_row] = sec;
                    auto& sections = it->second.sections;
                    sections[sec_num] = { sec_num, sec_name, sec_row };
                }
                else {
                    auto& [sec_num, sec_name, sec_row] = sec;
                    auto& [cls_name, cls_desc, cls_row] = cls;

                    lex_section_index id_sec = { 
                        {
                            sec_num, {
                                sec_num, 
                                sec_name,
                                sec_row
                            }
                        }
                    };

                    lex_class cls = { cls_name, cls_desc, id_sec, cls_row };
                    _index[cls.name] = cls;
                }
            }
        }

        entry = { };
    }

    bool match_pat(const std::regex& pattern) {
        auto& [_, line, match] = entry.current_iteration;
        return std::regex_match(line, match, pattern);
    }
};