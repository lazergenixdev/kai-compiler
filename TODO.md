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
- [ ] remove `push` from `traverse_tree`
- [ ] remove `file` from fatal error function
- [ ] replace `panic_with_message` with `kai__todo`
- [ ] replace `debug_writer`
- [ ] Delete kai_dev.h
- [ ] refactor to use `Compiler_Context`
- [ ] Add Copy instruction to bytecode
- [ ] Add vector and matrix types to bytecode
- [ ] Add declaration and assignment to bytecode generation
- [ ] Scopes in bytecode generation
- [ ] Simple machine code generation (ARM64)
- [ ] Linear register allocation with spilling
- [ ] Evaluate Feasibility of using C++ reflection API with compiling scripts

## Website test compiler playground thing
- [x] Fix WASM cmake toolchain, somehow...
- [x] Get program creation working in WASM
- [x] Fix random use of stdio
- [ ] Fix corruption in error messages (WASM, Semantic Errors)
- [ ] Fix WASM page allocator (debug memory issues)

# NEED FOR VERSION 1
- Structs
- Vector + Matrix Types
- Type Checker
- Runtime safety
- Memory + Stack + Pointers
- Machine code generation (only arm64 or x64 is good enough)
- C interop
