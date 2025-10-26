
# Assume ran from `tools/` if `build.c` is not found 
if (!(Test-Path -Path "build.c" -PathType Leaf)) {
	cd ../
}

# Debug flags: -g -DDEBUG
if (!(Test-Path -Path "build.exe" -PathType Leaf)) {
	clang.exe -o build.exe build.c
}

Set-Alias kai ./bin/kai
Set-Alias build ./build
Set-Alias newbuild ./newbuild
