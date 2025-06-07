cmake_minimum_required(VERSION 3.10.0)

project(kai)
set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/..")
