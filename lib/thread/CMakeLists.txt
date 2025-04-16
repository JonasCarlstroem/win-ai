cmake_minimum_required(VERSION 3.28.0)

project("thread")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

add_library(thread INTERFACE)
target_sources(thread INTERFACE thread_pool.h)
target_compile_features(thread INTERFACE cxx_std_17)