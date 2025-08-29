:: Run script from ../

@ECHO OFF

IF NOT EXIST "build.exe" (
	:: -g -DDEBUG
	clang.exe -o build.exe build.c
)

.\build.exe
