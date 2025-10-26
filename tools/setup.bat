@ECHO OFF

:: Assume ran from `tools/` if `build.c` is not found 
IF NOT EXIST "build.c" (
	cd ..
)

:: Debug flags: "/Zi /DDEBUG"
IF NOT EXIST "build.exe" (
	cl /nologo /utf-8 build.c
)

:: alias kai ./bin/kai
