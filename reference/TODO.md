# TODO
- [x] expression parser
- [x] type parser
- [x] procedure parser
- [x] statement parser
- [x] declaration parser
- [x] module/file parser (top level)
- [x] dependency graph generation (with small visualizer) (use stack of scopes)
- [ ] option for saving AST in `Kai_Program`
- [ ] add `#optional_export` and `#require_export` directives
- [ ] type checker + code generation (nothing for now, so just type checker)
- [ ] `#char` needs to do a utf8 decode and evaluate to the codepoint value
- [ ] add `then` keyword
- [ ] utility (function + commandline) to generate C/C++ bindings from script
- [ ] measure performance impact of `occupied` in hash table
- [ ] add multi-line string literals
- [ ] add tags (notes) to language parser
- [ ] figure out how to do multi declaration statements `a, b, c: int;`
- [ ] need some kind of import handler from host
- [ ] remove `;` from case statements?
- [ ] remove need for `destroy_syntax_tree` by passing in arena as input to `create_syntax_tree`
- [ ] errors should use an arena so that `destroy_error` can be removed
- [ ] `_type_of_expression` should return `bool`
- [ ] `_type_of_expression` should have an option to write out the types of subexpressions to remove need for separate typechecker

# Version 1
- [ ] AST interpreter (compile-time) -> compile time execution
- [ ] Source code to machine code (x86) -> host procedure execution
- [ ] No optimization (yet!)
