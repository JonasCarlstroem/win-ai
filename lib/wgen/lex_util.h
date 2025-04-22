#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <regex>
#include "../macros.h"

_REG(CLASS, R"(CLASS\s+([IVXLCDM]+))");
_REG(SECTION, R"(SECTION\s+([IVXLCDM]+)\.)");
_REG(ENTRY_HEADER, R"(^(\d+)\.\s+[A-Z][A-Za-z, ]+$)");
_REG(ENTRY, R"((\d+)\.\s+(.*?)(?:\s*-\s*([A-Z]+\.?))?)");
_REG(SPLITTER, R"(([^,;\.]+))");
_REG(NUM_FILTER, R"(\d+)");
_REG(HEADING, R"(^(\d+)[a-z]?\.\s*(?:\[[^\]]*\]\.\s*)?(.+?)\s*[—\-]\s*([A-Za-z\.]+)\.?)");
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

template<typename... Args>
using matcher_fn = bool(*)(Args... args);

using find_match_fn = matcher_fn<const std::string_view>;
using class_section_fn = matcher_fn<const std::string_view, std::string&>;

template<typename Fn>
struct matcher {
    matcher(Fn f) : roman(), matchers() {
        matchers.emplace_back(f);
    }
    matcher(std::initializer_list<Fn> list) : roman(), matchers(list) {
        /*for (size_t i = 0; i < list.size(); ++i) {
        }*/
    }

    std::string roman;
    std::vector<Fn> matchers;
};