cmake_minimum_required (VERSION 3.28)

project ("win-ai")
set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE SOURCES "lib/*.h")

include_directories("G:/Dev/GIT/win-ai/deps/dlib-19.24.8")

if(MSVC) 
	add_compile_options(/bigobj)
endif()

add_executable(trainer "lib/train_classifier.cpp" "lib/classifier_data.h" "lib/commands.h" "lib/process.h")
add_executable(win-ai "main.cpp")

target_sources(trainer 
	PRIVATE "G:/Dev/GIT/win-ai/deps/dlib-19.24.8/dlib/all/source.cpp")

target_sources(win-ai 
	PRIVATE "G:/Dev/GIT/win-ai/deps/dlib-19.24.8/dlib/all/source.cpp")

