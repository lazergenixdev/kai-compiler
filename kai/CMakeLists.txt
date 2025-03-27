cmake_minimum_required(VERSION 3.10.0)

project(kai)

set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "src/*.c")

add_executable(kaic  ${SOURCES})
target_link_libraries(kaic PRIVATE kai)

set_target_properties(kaic PROPERTIES OUTPUT_NAME "kai")
set_target_properties(kaic PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/..")
