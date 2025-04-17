#pragma once

#define DEBUG_THREADS
#define VERBOSE
#define PROFILING

#ifdef PROFILING
    #define TIMER(var_name, disp_name)                          \
    timer var_name { disp_name }                                

#ifdef VERBOSE
    #define TIMER_OUT(var_name, ...)                            \
    var_name.out(##__VA_ARGS__)
#else
    #define TIMER_OUT(var_name, ...)
#endif
    #define TIMER_TIME(var_name, ...)                           \
    var_name.display_time(##__VA_ARGS__)

    #define TIMER_START(var_name, disp_name)                    \
    TIMER(var_name, disp_name);                                 \
    var_name.start()

    #define TIMER_STOP(var_name)                                \
    var_name.stop();                                            
#else
    #define TIMER(var_name, disp_name)
    #define TIMER_OUT(var_name, ...)
    #define TIMER_START(var_name, disp_name, ...)
    #define TIMER_STOP(var_name, ...)
#endif

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

#define IS_MATCH(term, pattern) std::regex_match(##term, ##pattern)

#define SEARCH_MATCH(term, match, pattern) std::regex_search(##term, ##match, ##pattern)

#define SEARCH(term, pattern) std::regex_search(##term, ##pattern)

#define MATCH(line, match, pattern) std::regex_match(##line, ##match, ##pattern)

///Only works with strings
#define EMPTY(term) (##term.empty() && ##term.length() <= 1)

///Only works with strings
#define NOT_EMPTY(term) (!##term.empty() && ##term.length() > 1)

#define CURRENT_CLASS _index[c_class().current_class]
#define CURRENT_SECTION CURRENT_CLASS.sections[c_section().section_number]

#define CLASS(name) _classes[name]
#define SECTION(cls, name) CLASS(cls).sections[name]

#define TRY                                     \
try {

#define CATCH                                   \
} catch(const std::exception& ex)