cmake_minimum_required(VERSION 3.10.0)

project(kai)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory("src")   # Core Library

if (NOT WASM)
    add_subdirectory("kai")   # Compiler
    add_subdirectory("tests")
else ()
    add_subdirectory("web/src")
endif ()
