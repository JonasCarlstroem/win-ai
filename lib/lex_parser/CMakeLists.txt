cmake_minimum_required(VERSION 3.28)

project(lex_parser)
set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE LEX_SOURCES "lex_*.h")

add_library(lex_parser INTERFACE)
target_sources(lex_parser INTERFACE ${LEX_SOURCES})