# TODO
- [x] `#char` needs to do a utf8 decode and evaluate to the codepoint value
- [x] check if node's dependencies are evaluated before we compile this node
- [x] need some way to only type-check without needing the value in `_value_of_expr`
- [x] need a writer that writes into a buffer (just a Arena wrapper)
- [x] change literal type to not use assignment expression
- [x] compilation unit for compound statements
- [x] add keywords `then`, (separator) `null`, `true`, `false` (Special)
- [ ] by default, we need to cache all types, and only create new types for ones we haven't seen (or marked "#distinct")
- [ ] should print type look for recursive types?
- [ ] better compilation for recusive types
- [ ] cached typed need to be per-scope?
- [ ] add statement tags {}
- [ ] add `#optional_export` and `#require_export` directives
- [ ] need some kind of import handler from host
- [ ] remove need for `destroy_syntax_tree` by passing in arena as input to `create_syntax_tree`
- [ ] rename `EXPR_ARRAY` to `EXPR_ARRAY_TYPE`
- [ ] fix parser so that `#array` and `#map` can be removed
- [ ] fix nested constant declarations
- [ ] fix crash on circular dependencies on types

# Optional / Extra
- [ ] utility (function + commandline) to generate C/C++ bindings from script
- [ ] add flag `KAI_FLAG_CODEPOINT` to number expressions that came from `#char`
- [ ] procedure arguments can have no type, then the type used will be the type of the next argument
- [ ] declarations should be able to declare multiple names
- [ ] errors should use an arena so that `destroy_error` can be removed
- [ ] remove unused `loop` keyword
- [ ] remove `;` from case statements?
- [ ] measure performance impact of `occupied` in hash table
- [ ] add multi-line string literals
- [ ] parser needs a recovery mode for syntax highlighting only??
- [ ] consider using `KAI_IMP` instead of `KAI_API` for implementation for grep purposes
- [ ] strings need to be handled better in parser/tokenizer
- [ ] use web workers for running wasm compiler code, to have proper syncronization

# Version 0.1.1
- [ ] type-check all of compiler source code

# Version 0.5
- [ ] AST interpreter (compile-time) -> compile time execution

# Version 1
- [ ] Source code to machine code (x86) + host procedure execution
- [ ] No optimization (yet!)

# LLVM Backend Options
- Add directly to `kai.h` and have a flag to enable linking with LLVM
- *New command line utility that will lower to LLVM IR and JIT compile + run
- New command line utility that will lower to LLVM IR and compile to executable binary
