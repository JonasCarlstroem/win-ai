cmake_minimum_required (VERSION 3.28)

project ("win-ai")
set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE SOURCES "lib/**/*.h")
file(GLOB_RECURSE AI_SOURCES "lib/types/AI/**/*.h")

include_directories("G:/Dev/GIT/win-ai/deps/dlib-19.24.8")

if(MSVC) 
	add_compile_options(/bigobj)
endif()

add_subdirectory("lib/wgen")

file(COPY "${CMAKE_SOURCE_DIR}/lib/intents" DESTINATION "${CMAKE_BINARY_DIR}")

add_executable(trainer "train_classifier.cpp" "lib/classification/classifier_data.h" "lib/process.h")
add_executable(win-ai "main.cpp" ${AI_SOURCES})

set(INTENTS_OUT_DIR "${CMAKE_BINARY_DIR}/intents")
set(INTENTS_SRC_DIR "${CMAKE_SOURCE_DIR}/lib/intents")

message(STATUS "INTENTS_OUT_DIR=${INTENTS_OUT_DIR}")
message(STATUS "INTENTS_SRC_DIR=${INTENTS_SRC_DIR}")

add_custom_command(TARGET trainer PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${INTENTS_OUT_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${INTENTS_OUT_DIR}
	COMMENT "Clearing intents folder")

add_custom_command(TARGET trainer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${INTENTS_SRC_DIR}
	${INTENTS_OUT_DIR})

add_custom_target(clear_folder ALL DEPENDS ${INTENTS_OUT_DIR}/cleared_marker.txt)

target_sources(trainer 
	PRIVATE "G:/Dev/GIT/win-ai/deps/dlib-19.24.8/dlib/all/source.cpp")

target_sources(win-ai 
	PRIVATE "G:/Dev/GIT/win-ai/deps/dlib-19.24.8/dlib/all/source.cpp")

