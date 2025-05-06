cmake_minimum_required(VERSION 3.28)

project("wgen")
set(CMAKE_CXX_STANDARD 17)

add_executable(wgen "wgen.cpp" "../lex_parser/lex_types.h" "../lex_parser/lex_parser.h" "../lex_parser/lex_util.h" "../timer.h" "../lex_parser/lex_string_util.h")
add_compile_options(/utf-8)