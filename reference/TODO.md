# TODO
- [ ] need `_value_of_expr` to always return the value of constant expressions (i.e. Numbers)
- [ ] need constant folding in `_value_of_expr`
- [ ] only use `u64` and `s64` for integer values
- [ ] should print type look for recursive types?
- [ ] better compilation for recusive types
- [ ] cached typed need to be per-scope?
- [ ] add `#optional_export` and `#require_export` directives
- [ ] need some kind of import handler from host
- [ ] remove need for `destroy_syntax_tree` by passing in arena as input to `create_syntax_tree`
- [ ] rename `EXPR_ARRAY` to `EXPR_ARRAY_TYPE`
- [ ] fix parser so that `#array` and `#map` can be removed
- [ ] fix nested constant declarations
- [ ] need a flag to tell `compile_program` to keep the AST around
- [ ] all control paths must return a value

# Optional / Extra
- [ ] add statement tags `{}`
- [ ] utility (function + commandline) to generate C/C++ bindings from script
- [ ] add flag `KAI_FLAG_CODEPOINT` to number expressions that came from `#char` (`KAI_FLAG_MULTI`)
- [ ] procedure arguments can have no type, then the type used will be the type of the next argument (Ex: `a, b: int`)
- [ ] declarations should be able to declare multiple names also
- [ ] errors should use an arena so that `destroy_error` can be removed
- [ ] remove `;` from case statements?
- [ ] add multi-line string literals
- [ ] parser needs a recovery mode for syntax highlighting only??
- [ ] consider using `KAI_IMP` instead of `KAI_API` for implementation for grep purposes
- [ ] strings need to be handled better in parser/tokenizer
- [ ] use web workers for running wasm compiler code, to have proper syncronization?
- [ ] write procedure should return number of characters written
- [ ] use word "free" instead of "destroy"?
- [ ] create new types for types marked `#distinct`
- [ ] use `next` field in expressions to store multiple declarations (Ex: `a, b: int;` -> Stmt_Decl)

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
