# Kai Scripting Language

## Background
todo
## Features
- todo

# Usage
- Copy `kai.h` into your include directory
- Define `KAI_IMPLEMENTATION` in one .c file

# Building
The build system used is `nob.h` ([github.com/tsoding/nob.h](https://github.com/tsoding/nob.h)) so to build this project (which also generates `kai.h`) then you need to compile `build.c` and run it. There are scripts included that will do this with `clang` C compiler.
## Windows
```bash
./tools/build.bat
```
## Linux/Mac
```bash
./tools/build.sh
```

# Miscellaneous Notes
Changes to `nob.h`
```c
1513	fprintf(stderr, "[\e[94mINFO\e[0m] ");
1516	fprintf(stderr, "[\e[93mWARNING\e[0m] ");
1519	fprintf(stderr, "[\e[91mERROR\e[0m] ");
```
