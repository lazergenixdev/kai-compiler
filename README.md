# Kai Scripting Language
AOT data-oriented scripting language designed for embedding, without sacrificing performance.

[Preview online now!](https://lazergenixdev.github.io/kai-compiler/playground/)

## Features
✅ Header only stb-style C library with no external dependencies \
✅ Strict static type checking \
✅ Modern programming features (slices, defer, ...)

# Usage
- Copy `kai.h` into your include directory
- Define `KAI_IMPLEMENTATION` in one c/cpp file

# Background
There are three main problems that this language is looking to solve, where at least one of these problems is present in every scripting language.
- Language boundary problem
- Slow performance
- Bloated runtimes

## Language boundary problem
TODO

... the amount of glue code in order to bind your application ...

This language the ... as such, when retrieving a function, what you get is literally just a pointer to a C function.

## Slow performance
Lots of people may say that it's fine if your scripting (embedded) language is slow, because the point of a scripting language it to connect higher level constructs that can, internally, be optimized. For example, in Python you should use numpy when working with large arrays. 

But this is an incorrect assumption to make, because this inherently limits how complex your script can be, and you must depend on libraries that can provide optimized code, or write this optimized code in a lower level language (like C). But even then, once you hit the a certain level of complexity in your program where there is a significant performance loss, you will need to switch languages to get performance back; and at the point at which you hit that complexity, then switching languages will come at a significant development cost.

There is no reason that a embedded language should be slow, just make it as fast as AOT compiled languages to begin with, and you will never need to pay the cost of switching languages.

## Bloated runtimes
C#


## Benefits of a scripting language
TODO

... programs become data ...

# Building

## TLDR
```bash
source tools/setup.sh
build
```

This language is written in itself, and the target for the build is a C header file `kai.h`, which is built via a build program `build.c`.

`build.c` does not require any fancy compiler input, just input `build.c` to any C compiler and run the resulting program to build the project. Alternatively, there are setup scripts in [tools/](https://github.com/lazergenixdev/kai-compiler/tree/main/tools) that will compile the build program and define simple aliases for running it's generated output to make development easier.

Once the build program is built, there is no need to recompile it, as it uses Go Rebuild Urself™ Technology to automatically recompile itself if needed.

# Credits
Special thanks to Tsoding for making `nob.h` ([github.com/tsoding/nob.h](https://github.com/tsoding/nob.h))
and Sean Barrett for making the stb libraries, which was a big inspiration for the API design.
