cmake -S . -B build --toolchain=tools/wasm.cmake -DCMAKE_C_COMPILER="clang.exe" -G Ninja
function build { cmake --build build --clean-first }