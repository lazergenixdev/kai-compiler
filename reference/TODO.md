# TODO
- [x] expression parser
- [x] type parser
- [x] procedure parser
- [x] statement parser
- [x] declaration parser
- [x] module/file parser (top level)
- [x] dependency graph generation (with small visualizer) (use stack of scopes)
- [ ] add `#optional_export` and `#require_export` directives
- [ ] type checker + code generation (nothing for now, so just type checker)
- [ ] `#char` needs to do a utf8 decode and evaluate to the codepoint value
- [ ] add `then` keyword
- [ ] utility (function + commandline) to generate C/C++ bindings from script
- [ ] measure performance hit of occupied array in hash table
- [ ] add structs to compilation
- [ ] add strings to compilation
- [ ] add multi-line string literals
- [ ] add tags (notes) to language parser
- [ ] figure out how to do multi declaration statements `a, b, c: int;`
- [ ] need some kind of import handler from host
- [ ] remove `;` from case statements?
- [ ] remove need for `destroy_syntax_tree` by passing in arena as input to `create_syntax_tree`
- [ ] errors should use an arena so that `destroy_error` can be removed

# Version 1
- [ ] AST interpreter (compile-time) -> compile time execution
- [ ] Source code to machine code (x86) -> host procedure execution
- [ ] No optimization (yet!)
