cmake_minimum_required (VERSION 3.28)

project ("win-ai")
set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE SOURCES "lib/**/*.h")

include_directories("G:/Dev/GIT/win-ai/deps/dlib-19.24.8")

if(MSVC) 
	add_compile_options(/bigobj)
endif()

file(COPY "${CMAKE_SOURCE_DIR}/lib/intents" DESTINATION "${CMAKE_BINARY_DIR}")

set(AI_SOURCES "lib/classification/intent_classifier.h" "lib/classification/command_resolver.h")
add_executable(trainer "train_classifier.cpp" "lib/classification/classifier_data.h" "lib/process.h")
add_executable(win-ai "main.cpp")

add_custom_command(TARGET trainer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/lib/intents"
	"${CMAKE_BINARY_DIR}/intents")

target_sources(trainer 
	PRIVATE "G:/Dev/GIT/win-ai/deps/dlib-19.24.8/dlib/all/source.cpp")

target_sources(win-ai 
	PRIVATE "G:/Dev/GIT/win-ai/deps/dlib-19.24.8/dlib/all/source.cpp")

