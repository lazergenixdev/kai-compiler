cmake_minimum_required(VERSION 3.10.0)

project(kai)

set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "*.c")
file(GLOB DEBUG_SOURCES "debug/*.c")

add_library(kai ${SOURCES} ${DEBUG_SOURCES})

target_include_directories(kai PUBLIC "../include/")
