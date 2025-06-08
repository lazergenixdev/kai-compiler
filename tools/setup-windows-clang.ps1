# . ./tools/setup-windows-clang.ps1
if (Test-Path "build") {
    Remove-Item "build" -Recurse -Force
}
cmake -S . -B build -DCMAKE_C_COMPILER="clang.exe" -G Ninja
Set-Alias kai ./build/bin/kai
Set-Alias test ./build/bin/test
function build { cmake --build build --clean-first -j }