
:: Run script from ../

@ECHO OFF

IF NOT EXIST "build.exe" (
	:: /Zi /DDEBUG
	cl /nologo /utf-8 build.c
)

:: alias kai ./bin/kai
