#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <set>
#include <regex>
#include "lex_util.h"
#include "lex_types.h"
#include "lex_string_util.h"
#include "../timer.h"
#include "thread_pool.h"

//#undef DEBUG_THREADS

#ifdef PROFILING && _defined(TIMER)
#endif

class lex_parser {
private:
    output& out;
    timer _timer;

    std::string _lex_file;
    lex_class_index _index;
    lex_class_index _classes;
    lex_verbs_index _verbs;
    std::unordered_map<std::string, std::string> _entry_headers;

    std::istringstream _iss;
    std::vector<std::string> _lines;
    //thread_pool _class_pool;
    thread_pool _section_pool;
    //thread_pool _entry_pool;

    std::vector<std::future<lex_class>> _class_futures;
    std::unordered_map<std::string, std::vector<std::future<lex_section>>> _section_futures;

    std::unordered_map<std::string, std::pair<std::string, std::string>> _raw_class_text;
    std::vector<raw_class> _raw_class_text2;
    
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
            size_t current_row = 0;
            std::string line;
            std::smatch match;
        } current_iteration_values;
    } entry;

    auto& c_class() { return entry.current_class; }
    auto& c_section() { return entry.current_section; }
    auto& c_iteration() { return entry.current_iteration_values; }

public:
    lex_parser(const std::string& lex_file, output& out = output("lex_parser")) 
        : out(out), _timer(), _lex_file(lex_file), _index(), _section_pool{} {}

    void display_toc() {
        out("--- Table of content ---");
        out("");

        for (const auto& index : _index) {
            auto& lc = index.second;
            out(lc.name, " - ", lc.description, " (Row: ", lc.start_row, ")");
            for (const auto& pair : index.second.sections) {
                const auto& section = pair.second;
                out("\t", 
                    section.number, ": ", section.name, 
                    " (Row: ", section.start_row, ")",
                    " (Entries: ", section.entries.size(), ")");
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

                int sec_num = roman_to_int(section.number);
                out("--- Section ", sec_num, ": ", section.name, " ---");
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

    void init2() {
        std::ifstream file(_lex_file, std::ios::binary);

        if (!file.is_open()) {
            out.err("Error opening file: ", _lex_file);
            exit(-1);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        normalize_dashes(content);
        size_t start = content.find(PREFIX_CLASS), end = 0;
        while (end < content.size()) {
            end = content.find(PREFIX_CLASS, start + 1);
            if (end == std::string::npos) end = content.size();
            std::string cls = content.substr(start, end - start);

            std::string class_name = get_and_strip_class_name(cls);
            std::string class_desc = get_and_strip_class_description(cls);

            /*std::string clsDesc = cls.substr(lbl_end + 2, cls.find_first_of("\r\n", lbl_end + 2));*/
            if (!cls.empty() && cls.back() == '\r') {
                cls.pop_back();
            }

            if (!cls.empty() && cls.size() > 1) {
                raw_class raw = { class_name, class_desc, cls };
                _raw_class_text2.emplace_back(std::move(raw));
                //auto& p = std::make_pair(clsDesc, cls);
                //_raw_class_text[clsName] = std::move(p);
            }

            start = end;
        }

        out("start=", start, "\r\nend=", end, "\r\ncontent size=", content.size());
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

            if(!line.empty() && line.size() > 1)
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

    void extract_classes() {
        std::vector<lex_class> classes;
        TIMER_START(indices4_timer, "extract_indices4");
        TIMER_OUT(indices4_timer, "Extracting indices");

        try {
            for (auto& raw_cls : _raw_class_text2) {
                lex_class cls = extract_class(raw_cls);
                classes.push_back(std::move(cls));
            }
        }
        catch (const std::exception& ex) {
            throw ex;
        }
    }

    void extract_indices3() {
        std::vector<lex_class> classes;
        range class_range;

        TIMER_START(indices3_timer, "extract_indices3");
        TIMER_OUT(indices3_timer, "Extracting indices");

        try {
            std::smatch match;
            std::string roman;
            matcher<class_section_fn> m(is_line_class);
            for (size_t i = 0; i < _lines.size(); ++i) {
                /*int search_result = search_any(_lines[i], match, REG_CLASS);*/
                int search_result = search_any(_lines[i], m);
                if (search_result > 0) {
                    class_range.start = i++;
                    class_range.end = get_end<find_match_fn>(i, is_line_class);

                    if (class_range.end == (size_t)-1) {
                        class_range.end = _lines.size() - 1;
                    }

                    lex_class cls = extract_class(class_range);
                    classes.push_back(std::move(cls));
                }
            }

            out("DEBUG");
        }
        catch (const std::exception& ex) {
            throw ex;
        }
        //for (auto& [class_name, section_list] : _section_futures) {
        //    for (auto& section_fut : section_list) {
        //        lex_section section = section_fut.get();
        //        SECTION(class_name, section.name) = std::move(section);
        //        //_classes[class_name].sections[section.name] = std::move(section);
        //    }
        //}
    }

    void extract_indices2() {
#ifdef DEBUG_THREADS
        lex_class_index classes;
#endif
        /*std::vector<std::future<lex_class>> class_futures;
        std::unordered_map<std::string, std::vector<std::future<lex_section>>> section_futures;*/

        std::smatch match;
        range class_range;

        TIMER_START(extraction, "Index Timer");
        TIMER_OUT(extraction, "Extracting indices...");

        for (size_t i = 0; i < _lines.size(); ++i) {
            int search_result = search_any_reg(_lines[i], match, REG_CLASS);
            //if(MATCH(_lines[i], match, REG_CLASS))
            if(search_result > 0) {
                class_range.start = i++;
                class_range.end = get_end_reg(i, REG_CLASS);

                if (class_range.end == (size_t)-1) {
                    class_range.end = _lines.size() - 1;
                }

#ifdef DEBUG_THREADS
                lex_class cls = extract_class(class_range);
                classes[cls.name] = std::move(cls);
#else
 /*               _class_futures.push_back(_class_pool.enqueue([&, class_range, match] {
                    return extract_class(class_range, match);
                }));*/
#endif 

                i = class_range.end - 1;
            }
        }

#ifndef DEBUG_THREADS
        for (auto& f : _class_futures) {
            f.wait();
        }

        for (auto& f : _class_futures) {
            lex_class cls = f.get();
            CLASS(cls.name) = std::move(cls);
        }

        for (auto& [class_name, section_futures] : _section_futures) {
            for (auto& f : section_futures) {
                f.wait();
            }
        }

        for (auto& [class_name, section_list] : _section_futures) {
            for (auto& section_fut : section_list) {
                lex_section section = section_fut.get();
                SECTION(class_name, section.name) = std::move(section);
                //_classes[class_name].sections[section.name] = std::move(section);
            }
        }
#endif

        TIMER_STOP(extraction);
        TIMER_TIME(extraction, "Extracted indices in ");
    }

    void extract_indices() {
        auto& [row, cline, _] = c_iteration();
        row = 0;

        std::smatch match;
        std::vector<std::string> entry_lines;
        bool extracting_entry = false;

        TIMER_START(extract_timer, "Extract Indices Timer");

        for(size_t i = 0; i < _lines.size(); ++i) {
            row = i;
            cline = _lines[i];

            find_matches_timer.start();
            if (match_class()) {
                if (extracting_entry) extract_entry(entry_lines);

                extracting_entry = false;
                extract_class(i);
            }
            else if (match_section()) {
                if (extracting_entry) extract_entry(entry_lines);

                extracting_entry = false;
                extract_section(i);
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
            find_matches_timer.stop();
            time_data.match_time.emplace_back(_time{ i, find_matches_timer.elapsed_seconds() });
            find_matches_timer.reset();
        }

        TIMER_STOP(extract_timer, "Extracted all indices in ");

        out("Total time extracting classes: ", time_data.total_class_time(), "\n",
            "Total time extracting sections: ", time_data.total_section_time(), "\n",
            "Total time extracting entry headers: ", time_data.total_entry_header_time(), "\n",
            "Total time extracting entries: ", time_data.total_entry_time());

        entry = { };
    }

private:
    timer find_matches_timer{ "Matches" };
    timer match_class_timer{ "Match Class" };
    timer match_section_timer{ "Match Section" };
    timer match_entry_header_timer{ "Match Entry Header" };
    timer match_entry_timer{ "Match Entry" };
    timer extract_class_timer{ "Extract Class" };
    timer extract_section_timer{ "Extract Section" };
    timer extract_entry_header_timer{ "Extract Entry Header" };
    timer extract_entry_timer{ "Extract Entry" };

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

    std::string peek_next_line(size_t line) {
        const std::string& current_line = _lines[line];
        const std::string* next_line = (line + 1 < _lines.size()) ? &_lines[line + 1] : nullptr;

        if (next_line) {
            return *next_line;
        }
        else {
            return "";
        }
    }

    template<bool condition = true>
    size_t get_end_reg(size_t start, const std::vector<std::regex>& patterns) {
        for (size_t end = start; end < _lines.size(); ++end) {
            for (const auto& pattern : patterns) {
                if (SEARCH(_lines[end], pattern) == condition) {
                    while (_lines[end - 1].empty()) --end;
                    return end - 1;
                }
            }
        }

        return -1;
    }

    size_t get_end_reg(size_t start, const std::regex& pattern) {
        for (size_t i = start; i < _lines.size(); ++i) {
            IF_MATCH(_lines[i], pattern) {
                while (_lines[i - 1].empty()) --i;
                return i - 1;
            }
        }

        return -1;
    }

    template<typename Fn>
    size_t get_end(size_t start, const Fn& f) {
        for (size_t i = start; i < _lines.size(); ++i) {
            if((*f)(_lines[i])) {
                while (_lines[i - 1].empty()) --i;
                return i - 1;
            }
        }

        return -1;
    }

    template<bool condition = true, typename... Args>
    size_t get_end(size_t start, const std::vector<matcher_fn<Args...>>& fs) {
        for (size_t i = start; i < _lines.size(); ++i) {
            for (size_t j = 0; j < fs.size(); ++j) {
                auto f = fs[j];
                if ((*f)(_lines[i])) {
                    while (_lines[i - 1].empty()) --i;
                    return i - 1;
                }
            }
        }

        return -1;
    }

    template<bool condition = true, typename... Args>
    size_t get_end(size_t start, const std::vector<matcher_fn<Args...>>& fs, Args... args) {
        for (size_t i = start; i < _lines.size(); ++i) {
            for (size_t j = 0; j < fs.size(); ++j) {
                auto f = fs[j];
                if ((*f)(_lines[i], args...)) {
                    while (_lines[i - 1].empty()) --i;
                    return i - 1;
                }
            }
        }

        return -1;
    }

    template<typename Fn>
    size_t get_end(size_t start, matcher<Fn>& fmatch) {
        for (size_t i = start; i < _lines.size(); ++i) {
            for (size_t j = 0; j < fmatch.matchers.size(); ++j) {
                auto f = fmatch.matchers[j];
                if ((this->*f)(_lines[i], fmatch.roman)) {
                    while (_lines[i - 1].empty()) --i;
                    return i - 1;
                }
            }
        }

        return -1;
    }

    lex_class extract_class(raw_class cls) {
        lex_class result;
        result.name = cls.name;
        result.description = cls.description;

        std::string content = cls.raw_class_text;
        size_t section_start = content.find(PREFIX_SECTION), section_end = 0;
        //range section_range = { content.find(PREFIX_SECTION), 0 };
        while (section_end < content.size()) {
            section_end = content.find(PREFIX_SECTION, section_start + 1);
            if (section_end == std::string::npos) section_end = content.size();
            std::string section = content.substr(section_start, section_end - section_start);

            std::string sec_num = get_and_strip_section_num(section);
            std::string sec_name = get_and_strip_section_name(section);

            if (!section.empty() && section.back() == '\r') {
                section.pop_back();
            }

            if (!section.empty() && section.size() > 1) {
                raw_section raw = { sec_num, sec_name, section };
                lex_section sec = extract_section(raw);

                result.sections[sec_name] = std::move(sec);
            }
        }

        return result;
    }

    lex_class extract_class(range class_range) {
        try {
            //static const std::vector<std::regex> end_patterns = {
            //    REG_CLASS,
            //    REG_SECTION
            //};

            static const std::vector<find_match_fn> end_fpatterns = {
                is_line_class,
                is_line_section
            };

            lex_class result;
            result.name = _lines[class_range.start];
            result.description = peek_next_line(class_range.start);
        

            TIMER_START(class_timer, result.name);
            TIMER_OUT(class_timer, "Extracting sections from ", result.name);

            std::vector<std::future<lex_section>> section_futures;
            range section_range;

            matcher<class_section_fn> m({ is_line_class, is_line_section });

            for (size_t i = class_range.start + 1; i < class_range.end; ++i) {
                int search_result = search_any(_lines[i], m);
                if (search_result > 0) {
                    section_range.start = i++;
                    section_range.end = get_end(i, end_fpatterns);

                    if (section_range.end == (size_t)-1) {
                        section_range.end = _lines.size() - 1;
                    }
#ifdef DEBUG_THREADS
                        lex_section section = extract_section(section_range, m.roman);
                        result.sections[section.name] = std::move(section);
#else
                        section_futures.push_back(_section_pool.enqueue([&, section_range, m] {
                            return extract_section(section_range, m.roman);
                        }));
#endif

                        i = section_range.end - 1;
                }
            }

            try {
                for (auto& section_fut : section_futures) {
                    if (!section_fut.valid()) {
                        out("DEBUG");
                    }

                    section_fut.wait();
                    //for (auto& section_fut : section_list)
                    //for (size_t i = 0; i < section_list.size(); ++i) {
                    //    auto& section_fut = section_list[i];
                    //    if (!section_fut.valid()) {
                    //        out("DEBUG");
                    //    }
                    //    section_fut.wait();
                    //}
                    //for (auto& section_fut : section_list) {
                    //    lex_section section = section_fut.get();
                    //    SECTION(class_name, section.name) = std::move(section);
                    //}
                }

                for (auto& section_fut : section_futures) {
                    lex_section section = section_fut.get();
                    result.sections[section.name] = std::move(section);
                }
            }
            catch (const std::exception& ex) {
                throw ex;
            }

            TIMER_STOP(class_timer);
            TIMER_OUT(class_timer, "Section extraction queued.");
            return result;
        }
        catch (const std::exception& ex) {
            throw ex;
        }
    }

    void extract_class(size_t line_num) {
        extract_class_timer.start();
        auto& [cls_name, cls_desc, cls_row] = c_class();

        cls_row = c_iteration().current_row;
        cls_name = c_iteration().line;
        cls_desc = peek_next_line(line_num);

        lex_class class_entry = {
            cls_name,
            cls_desc,
            {},
            cls_row
        };

        CURRENT_CLASS = std::move(class_entry);
        extract_class_timer.stop();
        time_data.extract_class_time.emplace_back(_time{ line_num, extract_class_timer.elapsed_seconds() });
        extract_class_timer.reset();
    }

    lex_section extract_section(raw_section section) {
        lex_section result;
        result.number = section.num;
        result.name = section.name;

        std::string content = section.raw_section_text;
        strip_sub_sections(content);
        trim(content);
        size_t entry_start = get_entry_start(content), entry_end = 0;
        while (entry_end < content.size()) {
            
        }
        return result;
    }

    lex_section extract_section(range section_range, std::string_view roman) {
        try {
            static const std::vector<std::regex> end_patterns = {
                REG_CLASS,
                REG_SECTION,
                REG_ENTRY_HEADER,
                REG_HEADING
            };

            static const std::vector<find_match_fn> end_fpatterns = {
                is_line_class,
                is_line_section,
                is_line_entry_header
            };

            lex_section result;
            result.number = roman;
            result.name = peek_next_line(section_range.start);

            TIMER_START(section_timer, result.name);
            TIMER_OUT(section_timer, "Extracting entries from section ", result.number);

            std::vector<std::future<lex_entry>> entry_futures;
            range entry_range;
            for (size_t i = section_range.start; i < section_range.end; ++i) {
                /*int search_result = search_any_reg(_lines[i], { REG_ENTRY_HEADER, REG_HEADING });*/
                int entry_num = 0;
                std::string entry_label;
                //int search_result = search_any(_lines[i], is_line_entry_header);
                if (is_line_sub_section(_lines[i])) {

                }

                bool s_res = is_line_entry_header(_lines[i]);
                if (s_res) {
                    /*if (search_result == 1)
                        entry_range.start = ++i;
                    else if (search_result == 2)*/
                    entry_range.start = ++i;

                    entry_range.end = get_end<true>(++i, end_fpatterns);

                    if (entry_range.end == (size_t)-1 || entry_range.end > _lines.size()) {
                        entry_range.end = _lines.size() - 1;
                    }

                    if (entry_range.start == 14800) {
                        out("Something breaks here...");
                    }

                    lex_entry entry = extract_entry(entry_range, result.name);
                    result.entries.push_back(entry);

                    i = entry_range.end;
                }
            }

            TIMER_STOP(section_timer);
            TIMER_TIME(section_timer, "Entries extracted in ");
            return result;
        }
        catch (const std::exception& ex) {
            throw ex;
        }
    }

    void extract_section(size_t line_num) {
        extract_section_timer.start();
        auto& [sec_num, sec_name, sec_row] = c_section();
        auto& [cls_name, cls_desc, cls_row] = c_class();

        sec_row = c_iteration().current_row;
        sec_num = c_iteration().match[1];
        sec_name = peek_next_line(line_num);

        lex_section section = {
            sec_num,
            sec_name,
            {},
            {},
            sec_row
        };

        CURRENT_SECTION = std::move(section);
        extract_section_timer.stop();
        time_data.extract_section_time.emplace_back(_time{ line_num, extract_section_timer.elapsed_seconds() });
        extract_section_timer.reset();
    }

    void extract_entry_header() {
        extract_entry_header_timer.start();
        auto [row, line, _] = c_iteration();
        std::string header_number = line.substr(0, line.find('.'));
        std::string header_text = line;

        _entry_headers[std::move(header_number)] = std::move(header_text);

        extract_entry_header_timer.stop();
        time_data.extract_entry_header_time.emplace_back(_time{ row, extract_entry_header_timer.elapsed_seconds() });
        extract_entry_header_timer.reset();
    }

    lex_entry extract_entry(range range, const std::string& section_name) {
        const std::string& heading = _lines[range.start];

        std::string::size_type dash_pos = heading.find("-");
        if (dash_pos == std::string::npos) return {};

        std::string::size_type pos_dot = heading.find('.', dash_pos);
        if (pos_dot == std::string::npos || pos_dot <= dash_pos) return {};

        std::string::size_type dot_pos = heading.find('.');
        std::string entry_num = trim_copy(heading.substr(0, dot_pos));

        std::string::size_type label_start = heading.find_first_not_of(" ", dot_pos + 1);
        std::string entry_name = trim_copy(heading.substr(label_start, dash_pos - label_start));

        std::string pos = trim_copy(heading.substr(dash_pos + 1, pos_dot - dash_pos - 1));

        std::string rest = trim_copy(heading.substr(pos_dot + 1));
        std::string raw_text;
        size_t total_size = rest.size() + 1;
        for (size_t i = range.start + 1; i < range.end; ++i) {
            total_size += _lines[i].size() + 1;
        }

        raw_text.reserve(total_size);
        raw_text += rest;
        raw_text += ' ';
        for (size_t i = range.start + 1; i < range.end; ++i) {
            raw_text += _lines[i];
            raw_text += ' ';
        }

        raw_text = clean_term_annotations(raw_text);

        raw_text = reduce_whitespace(raw_text);
        raw_text = remove_parenthesis_with_number(raw_text);
        raw_text = remove_number_dot(raw_text);
        raw_text = remove_v_dot(raw_text);
        raw_text = remove_isolated_numbers(raw_text);
        raw_text = remove_pos_tags(raw_text);

        std::vector<std::string> terms;
        size_t start = 0;
        while (start < raw_text.size()) {
            size_t end = raw_text.find_first_of(",;.", start);
            std::string term = raw_text.substr(start, end - start);
            trim(term);

            if (!term.empty() && !is_term_number(term) && !is_term_pos_tag(term)) {
                terms.emplace_back(std::move(term));
            }

            if (end == std::string::npos) break;
            start = end + 1;
        }

        return {
            entry_num,
            clean_name(entry_name),
            pos,
            terms
        };
    }

    //lex_entry extract_entry(range range, const std::string& section_name) {
    //    const std::string & heading = _lines[range.start];

    //    std::smatch match;
    //    if (!std::regex_search(heading, match, REG_HEADING)) return {};
    //    
    //    std::string entry_num = match[1];
    //    std::string entry_name = clean_name(match[2]);
    //    std::string pos = match[3];

    //    std::string raw_text;
    //    size_t total_size = 0;

    //    for (size_t i = range.start; i < range.end; ++i) {
    //        total_size += _lines[i].size() + 1;
    //    }

    //    raw_text.reserve(total_size);
    //    for (size_t i = range.start; i < range.end; ++i) {
    //        raw_text += _lines[i];
    //        raw_text += ' ';
    //    }

    //    if (raw_text.empty()) {
    //        return {};
    //    }
    //    raw_text = raw_text.substr(match[0].length());
    //    raw_text = clean_term_annotations(raw_text);

    //    for (const auto& [pat, rep] : replace_match) {
    //        raw_text = REPLACE(raw_text, pat, rep);
    //    }

    //    std::vector<std::string> terms;
    //    auto words_begin = std::sregex_iterator(raw_text.begin(), raw_text.end(), REG_SPLITTER);
    //    auto words_end = std::sregex_iterator();

    //    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    //        std::string term = i->str();
    //        trim(term);

    //        term = REPLACE(term, REG_NUM_REMOVE, "");
    //        term = REPLACE(term, REG_POS_REMOVE, "");

    //        trim(term);

    //        IF_MATCH(term, REG_NUM_FILTER) continue;
    //        if (NOT_EMPTY(term)) {
    //            terms.emplace_back(term);
    //        }
    //    }

    //    lex_entry result = {
    //        entry_num,
    //        entry_name,
    //        pos,
    //        terms
    //    };

    //    /*TIMER_STOP(entry_timer);
    //    TIMER_TIME(entry_timer, "Extracted entry in ");*/
    //    return result;
    //}

    void extract_entry(std::vector<std::string>& lines) {
        extract_entry_timer.start();
        if (lines.empty()) return;

        auto sec_name = c_section().section_name;
        if (sec_name == "POSSESSIVE RELATIONS" || sec_name == "PERSONAL AFFECTIONS") {
            out("DEBUG");
        }

        std::smatch match;
        const std::string& heading = lines[0];

        if (std::regex_search(heading, match, REG_HEADING)) {
            std::string entry_number = match[1];
            std::string entry_name = clean_name(match[2]);
            std::string pos = match[3];

            std::string raw_text;
            size_t total_size = 0;
            for (const auto& line : lines) {
                total_size += line.size() + 1;
            }
            raw_text.reserve(total_size);

            for(const auto& line : lines) {
                raw_text += line;
                raw_text += ' ';
            }

            raw_text = raw_text.substr(match[0].length());
            raw_text = clean_term_annotations(raw_text);

            for (const auto& [pat, rep] : replace_match) {
                raw_text = std::regex_replace(raw_text, pat, rep);
            }

            std::vector<std::string> terms;
            auto words_begin = std::sregex_iterator(raw_text.begin(), raw_text.end(), REG_SPLITTER);
            auto words_end = std::sregex_iterator();

            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::string term = i->str();
                trim(term);

                term = REPLACE(term, REG_NUM_REMOVE, "");
                term = REPLACE(term, REG_POS_REMOVE, "");

                trim(term);

                IF_MATCH(term, REG_NUM_FILTER) continue;

                if (NOT_EMPTY(term)) {
                    terms.emplace_back(term);
                }
            }
            
            lex_entry entry = {
                entry_number,
                entry_name,
                pos,
                terms
            };

            CURRENT_SECTION.entries.emplace_back(entry);
            _verbs[pos].emplace_back(std::move(entry));
        }

        lines.clear();
        extract_entry_timer.stop();
        time_data.extract_entry_time.emplace_back(_time{ c_iteration().current_row, extract_entry_timer.elapsed_seconds() });
        extract_entry_timer.reset();
    }

    bool match_class() {
        return match_pat(REG_CLASS);
    }

    bool match_class(const std::string& line, std::smatch& match) {
        return MATCH(line, match, REG_CLASS);
    }

    bool match_section() {
        return match_pat(REG_SECTION);
    }

    bool match_section(const std::string& line, std::smatch& match) {
        return MATCH(line, match, REG_SECTION);
    }

    bool match_entry_header() {
        return match_pat(REG_ENTRY_HEADER);
    }

    bool match_entry_header(const std::string& line, std::smatch& match) {
        return MATCH(line, match, REG_ENTRY_HEADER);
    }

    bool match_entry() {
        return match_pat(REG_ENTRY);
    }

    bool match_entry(const std::string& line, std::smatch& match) {
        return MATCH(line, match, REG_ENTRY);
    }

    bool match_pat(const std::regex& pattern) {
        auto& [_, line, match] = c_iteration();
        return MATCH(line, match, pattern);
    }

    bool match_any(const std::string& line, const std::vector<std::regex>& patterns) {
        for (const auto& pat : patterns) {
            if (std::regex_match(line, pat)) return true;
        }

        return false;
    }

    bool match_all(const std::string& line, const std::vector<std::regex>& patterns) {
        for (const auto& pat : patterns) {
            if (!std::regex_match(line, pat)) return false;
        }

        return true;
    }

    int search_any(const std::string& line, const std::vector<class_section_fn>& patterns, std::string& _out) {
        for (size_t i = 0; i < patterns.size(); ++i) {
            auto& f = patterns[i];
            if ((*f)(line, _out)) {
                return i + 1;
            }
        }

        return 0;
    }

    template<typename Fn, typename... Args>
    int search_any(const std::string& line, const Fn f, Args&... args) {
        if ((*f)(line, args...)) {
            return 1;
        }

        return 0;
    }

    template<typename Fn>
    int search_any(const std::string& line, matcher<Fn>& fmatch) {
        for (size_t i = 0; i < fmatch.matchers.size(); ++i) {
            auto f = fmatch.matchers[i];
            if ((*f)(line, fmatch.roman)) return 1;
        }

        return 0;
    }

    int search_any_reg(const std::string& line, std::smatch& match, const std::regex& pattern) {
        if (SEARCH_MATCH(line, match, pattern)) return 1;
        return 0;
    }

    int search_any_reg(const std::string& line, std::smatch& match, const std::vector<std::regex>& patterns) {
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (SEARCH_MATCH(line, match, patterns[i])) return i + 1;
        }

        return 0;
    }

    int search_any_reg(const std::string& line, const std::vector<std::regex>& patterns) {
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (SEARCH(line, patterns[i])) return i + 1;
        }

        return 0;
    }

    //bool is_class(const std::string_view line, std::string& roman_num_out) {
    //    constexpr std::string_view prefix = "CLASS ";
    //    if (starts_with(line, prefix)) {
    //        auto rest = line.substr(prefix.size());
    //        size_t i = 0;
    //        while (i < rest.size() && std::isupper(rest[i]) && std::string("IVXLCDM").find(rest[i]) != std::string::npos) {
    //            ++i;
    //        }

    //        if (i > 0) {
    //            roman_num_out = std::string(rest.substr(0, i));
    //            return true;
    //        }
    //    }

    //    return false;
    //}

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

        for (const auto& pair : cleaning_match) {
            const std::regex& reg = pair.first;
            const std::string& rep = pair.second;
            cleaned_term = std::regex_replace(cleaned_term, reg, rep);
        }

        trim(cleaned_term);
        return cleaned_term;
    }
};