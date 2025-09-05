# Kai Scripting Language
[Preview online now!](https://lazergenixdev.github.io/kai-compiler/playground/)
## Background
todo
## Features
- Header only C library with no external dependencies
- Strict static type checking

# Usage
- Copy `kai.h` into your include directory
- Define `KAI_IMPLEMENTATION` in one .c file

# Building
The build system used is `nob.h` ([github.com/tsoding/nob.h](https://github.com/tsoding/nob.h)) so to build this project (which also generates `kai.h`) then you need to compile `build.c` and run it. There are scripts included that will do this for you.
## Windows
```bash
./tools/build.bat   # MSVC
./tools/build.ps1   # clang
```
## Linux/Mac
```bash
./tools/build.sh    # clang
./tools/build.fish  # clang
```
