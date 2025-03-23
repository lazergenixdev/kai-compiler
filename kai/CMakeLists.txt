cmake_minimum_required(VERSION 3.10.0)

project(kaic)
set(CMAKE_C_STANDARD 99)
add_executable(kaic "main.c")
set_target_properties(kaic PROPERTIES OUTPUT_NAME "kai")
target_link_libraries(kaic PRIVATE kai)
