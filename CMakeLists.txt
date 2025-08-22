cmake_minimum_required(VERSION 3.10.0)

project(kai)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories("${CMAKE_SOURCE_DIR}")

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

if (WASM)
    add_subdirectory("web")
else ()
    add_subdirectory("kai")   # Compiler
    add_subdirectory("tests")
endif ()
