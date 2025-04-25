cmake_minimum_required(VERSION 3.10.0)

project(kai)

set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "*.c")
file(GLOB DEBUG_SOURCES "debug/*.c")

add_library(${PROJECT_NAME} ${SOURCES} ${DEBUG_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "../include/")

### Extra Warning Messages ##############################################

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall")
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/W4")
endif()

#########################################################################
