cmake_minimum_required(VERSION 3.14)
project(lib C)
add_executable(${PROJECT_NAME} "lib.c")
target_link_libraries(${PROJECT_NAME} PRIVATE kai)

set_target_properties(lib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    SUFFIX ".wasm"
)