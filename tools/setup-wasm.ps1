cmake -S . -B build --toolchain=cmake/wasm.cmake -DCMAKE_C_COMPILER="clang.exe" -G Ninja
function build { cmake --build build --clean-first }