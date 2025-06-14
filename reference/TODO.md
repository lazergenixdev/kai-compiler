# TODO
- [x] Make todo list
- [x] Fix a bunch of compile warnings
- [x] Fix procedure location in bytecode generation
- [x] Header only
- [x] Rename `KAI_USE_DEBUG_API`
- [x] use `kai__interp_create` / `kai__interp_destroy`
- [x] better seperation between public and private API
- [x] "rewrite" parser to be more modular
- [x] add structs to parser
- [x] let parser hold pointer to error
- [x] proper type parsing (arrays, slices, matrix)
- [x] place default `fatal_error` in main header (`KAI_HAVE_FATAL_ERROR`)
- [x] remove `push` from `traverse_tree`
- [x] remove `file` from fatal error function (theres only one file lol)
- [x] replace `panic_with_message` with `kai__todo`
- [x] replace `debug_writer` with `kai__debug` (welllll...)
- [x] Delete kai_dev.h
- [x] refactor to use `Compiler_Context`
- [x] create an API to emit raw x86 instructions
- [ ] create an x86 assembler context to keep track of register use (linear)
- [ ] Machine code generation for add test (mov + add + calling conv)
- [ ] Add declaration and assignment to bytecode generation
- [ ] Add Copy instruction to bytecode
- [ ] test copy instruction on assignments
- [ ] Add vector and matrix types to bytecode
- [ ] Scopes in bytecode generation
- [ ] Simple machine code generation (ARM64)
- [ ] Evaluate Feasibility of using C++ reflection API with compiling scripts

## Website test compiler playground thing
- [x] Fix WASM cmake toolchain, somehow...
- [x] Get program creation working in WASM
- [x] Fix random use of stdio
- [ ] Move WASM memory allocator out of header
- [ ] Fix corruption in error messages (WASM, Semantic Errors)
- [ ] Fix WASM page allocator (debug memory issues)

# NEED FOR VERSION 1
- [ ] Struct Types
- [ ] Vector & Matrix Types
- [ ] Type Checker
- [ ] Memory & Stack & Pointers
- [ ] Machine code generation (only arm64 or x64 is good enough)
- [ ] C interop
- [ ] Runtime safety
