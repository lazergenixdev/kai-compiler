# WASM Toolchain

# TODO: better solution for this 
set(WASM TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR wasm32)
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_C_FLAGS "-D__WASM__ -g --target=wasm32 -fno-builtin --no-standard-libraries -Wl,--no-entry -Wl,--allow-undefined -Wl,--export=kai_create_syntax_tree -Wl,--export=kai_get_version_string -Wl,--export=test")
#set(CMAKE_EXE_LINKER_FLAGS "--allow-undefined")
## Prevent default -arch on macOS
#set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "Do not set -arch flag")
set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
