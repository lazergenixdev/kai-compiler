# TODO
- [x] Rename `Kai_String_Writer` to `Kai_Writer`
- [x] Add to outputs of nodes (kai__node_add_use)
- [x] Create global stack of symbol tables
- [x] Push (pop) symbol tables on scope entry (exit)
- [x] Variable Declarations + Assignments
- [ ] `KAI_INTERNAL` for internal functions
- [ ] Deallocate nodes correctly
- [ ] Manage list of killed nodes, so we can reuse

## Website test compiler playground thing
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
