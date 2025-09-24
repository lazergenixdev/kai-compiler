# TODO
- [x] add tags (notes) to language parser
- [x] add expression tags ()
- [ ] add statement tags {}
- [ ] add struct type generation to compiler
- [ ] add multi-line string literals
- [ ] create function to parse function call arguments (for use in parsing tags)
- [ ] option for AST output in `Kai_Program` (need to place ast's somewhere)
- [ ] add `#optional_export` and `#require_export` directives
- [ ] `#char` needs to do a utf8 decode and evaluate to the codepoint value
- [ ] add `then` keyword
- [ ] utility (function + commandline) to generate C/C++ bindings from script
- [ ] measure performance impact of `occupied` in hash table
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
