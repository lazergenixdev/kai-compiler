cmake_minimum_required(VERSION 3.10.0)

project(kaic)
set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE kai)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "kai")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/..")
