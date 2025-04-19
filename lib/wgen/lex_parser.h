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
#include "../timer.h"
#include "thread_pool.h"

#undef DEBUG_THREADS

#ifdef PROFILING && _defined(TIMER)
#endif

struct _time {
    size_t row;
    double seconds;
};

struct timer_data {
    std::vector<_time> match_time{};
    std::vector<_time> extract_class_time{};
    std::vector<_time> extract_section_time{};
    std::vector<_time> extract_entry_header_time{};
    std::vector<_time> extract_entry_time{};

    double total_class_time() const {
        double total = 0;
        for (const auto& it : extract_class_time) {
            total += it.seconds;
        }

        return total;
    }

    double total_section_time() const {
        double total = 0;
        for (const auto& it : extract_section_time) {
            total += it.seconds;
        }

        return total;
    }

    double total_entry_header_time() const {
        double total = 0;
        for (const auto& it : extract_entry_header_time) {
            total += it.seconds;
        }

        return total;
    }

    double total_entry_time() const {
        double total = 0;
        for (const auto& it : extract_entry_time) {
            total += it.seconds;
        }

        return total;
    }
} time_data;

struct line_range {
    size_t start;
    size_t end;
};

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
    thread_pool _class_pool;
    thread_pool _section_pool;
    thread_pool _entry_pool;

    std::vector<std::future<lex_class>> _class_futures;
    std::unordered_map<std::string, std::vector<std::future<lex_section>>> _section_futures;
    
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
        : out(out), _timer(), _lex_file(lex_file), _index(), _class_pool{4}, _section_pool{4} {}

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

    void extract_indices3() {
        std::vector<lex_class> classes;
        line_range class_range;

        TIMER_START(indices3_timer, "extract_indices3");
        TIMER_OUT(indices3_timer, "Extracting indices");

        try {
            std::smatch match;
            for (size_t i = 0; i < _lines.size(); ++i) {
                int search_result = search_any(_lines[i], match, REG_CLASS);
                if (search_result > 0) {
                    class_range.start = i++;
                    class_range.end = get_end(i, REG_CLASS);

                    if (class_range.end == (size_t)-1) {
                        class_range.end = _lines.size() - 1;
                    }

                    lex_class cls = extract_class(class_range, match);
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
        //lex_class_index classes;
        /*std::vector<std::future<lex_class>> class_futures;
        std::unordered_map<std::string, std::vector<std::future<lex_section>>> section_futures;*/

        std::smatch match;
        line_range class_range;

        TIMER_START(extraction, "Index Timer");
        TIMER_OUT(extraction, "Extracting indices...");

        for (size_t i = 0; i < _lines.size(); ++i) {
            int search_result = search_any(_lines[i], match, REG_CLASS);
            //if(MATCH(_lines[i], match, REG_CLASS))
            if(search_result > 0) {
                class_range.start = i++;
                class_range.end = get_end(i, REG_CLASS);

                if (class_range.end == (size_t)-1) {
                    class_range.end = _lines.size() - 1;
                }

#ifdef DEBUG_THREADS
                lex_class cls = extract_class(class_range, match);
                classes.push_back(cls);
#else
                _class_futures.push_back(_class_pool.enqueue([&, class_range, match] {
                    return extract_class(class_range, match);
                }));
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
    size_t get_end(size_t start, const std::vector<std::regex>& patterns) {
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

    size_t get_end(size_t start, const std::regex& pattern) {
        for (size_t i = start; i < _lines.size(); ++i) {
            IF_MATCH(_lines[i], pattern) {
                while (_lines[i - 1].empty()) --i;
                return i - 1;
            }
        }

        return -1;
    }

    lex_class extract_class(line_range range, std::smatch match) {
        try {
            static const std::vector<std::regex> end_patterns = {
                REG_CLASS,
                REG_SECTION
            };
            lex_class result;
            result.name = _lines[range.start];
            result.description = peek_next_line(range.start);
        

            TIMER_START(class_timer, result.name);
            TIMER_OUT(class_timer, "Extracting sections from ", result.name);

            std::vector<std::future<lex_section>> section_futures;
            line_range section_range;
            for (size_t i = range.start + 1; i < range.end; ++i) {
                int search_result = search_any(_lines[i], match, end_patterns);
                if (search_result > 0) {
                    section_range.start = i++;
                    section_range.end = get_end<true>(i, end_patterns);

                    if (section_range.end == (size_t)-1) {
                        section_range.end = _lines.size() - 1;
                    }
                    try {
#ifdef DEBUG_THREADS
                        lex_section section = extract_section(section_range, match);
                        result.sections[section.name] = std::move(section);
#else
                        _section_futures[result.name].push_back(_section_pool.enqueue([&, section_range, match] {
                            return extract_section(section_range, match);
                        }));
#endif

                        i = section_range.end - 1;
                    }
                    catch (const std::exception& ex) {
                        throw ex;
                    }
                }
            }

            for (auto& [class_name, section_list] : _section_futures) {
                for (auto& section_fut : section_list) {
                    section_fut.wait();
                }
                for (auto& section_fut : section_list) {
                    lex_section section = section_fut.get();
                    SECTION(class_name, section.name) = std::move(section);
                }
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

    lex_section extract_section(line_range range, std::smatch match) {
        try {
            static const std::vector<std::regex> end_patterns = {
                REG_CLASS,
                REG_SECTION,
                REG_ENTRY_HEADER,
                REG_HEADING
            };

            lex_section result;
            result.number = match[1];
            result.name = peek_next_line(range.start);

            TIMER_START(section_timer, result.name);
            TIMER_OUT(section_timer, "Extracting entries from section ", result.number);

            std::vector<std::future<lex_entry>> entry_futures;
            line_range entry_range;
            for (size_t i = range.start; i < range.end; ++i) {
                int search_result = search_any(_lines[i], { REG_ENTRY_HEADER, REG_HEADING });
                if (search_result > 0) {
                    if (search_result == 1)
                        entry_range.start = ++i;
                    else if (search_result == 2)
                        entry_range.start = i;

                    entry_range.end = get_end<true>(++i, end_patterns);

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

    lex_entry extract_entry(line_range range, const std::string& section_name) {
        const std::string & heading = _lines[range.start];

        std::smatch match;
        if (!std::regex_search(heading, match, REG_HEADING)) return {};
        
        std::string entry_num = match[1];
        std::string entry_name = clean_name(match[2]);
        std::string pos = match[3];

        std::string raw_text;
        size_t total_size = 0;

        for (size_t i = range.start; i < range.end; ++i) {
            total_size += _lines[i].size() + 1;
        }

        raw_text.reserve(total_size);
        for (size_t i = range.start; i < range.end; ++i) {
            raw_text += _lines[i];
            raw_text += ' ';
        }

        if (raw_text.empty()) {
            return {};
        }
        raw_text = raw_text.substr(match[0].length());
        raw_text = clean_term_annotations(raw_text);

        for (const auto& [pat, rep] : replace_match) {
            raw_text = REPLACE(raw_text, pat, rep);
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

        lex_entry result = {
            entry_num,
            entry_name,
            pos,
            terms
        };

        /*TIMER_STOP(entry_timer);
        TIMER_TIME(entry_timer, "Extracted entry in ");*/
        return result;
    }

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

    int search_any(const std::string& line, const std::vector<std::regex>& patterns) {
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (SEARCH(line, patterns[i])) return i + 1;
        }

        return 0;
    }

    int search_any(const std::string& line, std::smatch& match, const std::regex& pattern) {
        if (SEARCH_MATCH(line, match, pattern)) return 1;
        return 0;
    }

    int search_any(const std::string& line, std::smatch& match, const std::vector<std::regex>& patterns) {
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (SEARCH_MATCH(line, match, patterns[i])) return i + 1;
        }

        return 0;
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

        for (const auto& pair : cleaning_match) {
            const std::regex& reg = pair.first;
            const std::string& rep = pair.second;
            cleaned_term = std::regex_replace(cleaned_term, reg, rep);
        }

        trim(cleaned_term);
        return cleaned_term;
    }
};