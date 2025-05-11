cmake_minimum_required(VERSION 3.10.0)

project(test)
set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE kai)
