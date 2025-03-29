cmake_minimum_required(VERSION 3.10.0)

project(kai)

# TODO: should I keep this?
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory("src")   # Core Library
add_subdirectory("kai")   # Compiler
add_subdirectory("tests")
