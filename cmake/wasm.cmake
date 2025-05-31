# wasm-toolchain.cmake

set(WASM TRUE)

# Indicate we're cross-compiling to WebAssembly
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

# Set the compiler
if (MACOS)
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
else ()
set(CMAKE_C_COMPILER "clang.exe")
endif ()

message("Using compiler: " ${CMAKE_C_COMPILER})

# Force CMake to use the correct compiler target
set(CMAKE_C_COMPILER_TARGET wasm32 CACHE STRING "Target for Clang")

# Base C flags for WebAssembly
set(CMAKE_C_FLAGS_INIT "--target=wasm32 -D__WASM__ -fno-builtin -nostdlib")

# Linker flags to control WebAssembly output
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--no-entry -Wl,--allow-undefined -Wl,--export-dynamic")

# Disable standard libraries if you're targeting a fully freestanding environment
set(CMAKE_C_STANDARD_LIBRARIES "")

# Optional: avoid trying to link test binaries
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
