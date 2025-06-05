cmake -S . -B build
Set-Alias kai ./build/bin/Debug/kai.exe
Set-Alias test ./build/bin/Debug/test.exe
function build { cmake --build build --clean-first -j }
