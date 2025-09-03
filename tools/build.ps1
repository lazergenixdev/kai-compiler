# Run script from ../

# -g -DDEBUG
if (!(Test-Path -Path "build.exe" -PathType Leaf)) {
	clang.exe -o build.exe build.c
}

Set-Alias kai ./bin/kai

.\build.exe
