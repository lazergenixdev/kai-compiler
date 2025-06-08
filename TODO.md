# TODO
- [x] Make todo list
- [x] Fix a bunch of compile warnings
- [x] Fix procedure location in bytecode generation
- [x] Header only
- [x] Rename `KAI_USE_DEBUG_API`
- [x] use `kai__interp_create` / `kai__interp_destroy`
- [ ] better seperation between public and private API
- [ ] add structs to language
- [ ] replace `panic_with_message`
- [ ] Delete kai_dev.h, just have a `KAI__VERBOSE` macro that includes stdio
- [ ] Add vector and matrix types to bytecode
- [ ] Add Copy instruction to bytecode
- [ ] Fix corruption in error messages (WASM, Semantic Errors)
- [ ] Add declaration and assignment to bytecode generation
- [ ] Scopes in bytecode generation
- [ ] Simple machine code generation (ARM64)
- [ ] Linear register allocation with spilling
- [ ] Evaluate Feasibility of using C++ reflection API with compiling scripts
## Website test compiler playground thing
- [x] Fix WASM cmake toolchain, somehow...
- [x] Get program creation working in WASM
- [x] Fix random use of stdio
- [ ] Fix WASM page allocator (debug memory issues)

# NEED FOR VERSION 1
- Structs
- Type Checker
- Vector + Matrix Types
- Runtime safety
- Memory + Stack + Pointers
- Machine code generation (only arm64 or x64 is good enough)
- C interop
