# TODO
- [x] add tags (notes) to language parser
- [x] option for AST output in `Kai_Program` (need to place ast's somewhere)
- [x] add expression tags ()
- [x] add struct type generation to compiler
- [ ] add statement tags {}
- [ ] pass in `debug_writer` to program_create to enable internal debug printing
- [ ] create function to parse function call arguments (for use in parsing tags)
- [ ] add `#optional_export` and `#require_export` directives
- [ ] `#char` needs to do a utf8 decode and evaluate to the codepoint value
- [ ] need some kind of import handler from host
- [ ] remove need for `destroy_syntax_tree` by passing in arena as input to `create_syntax_tree`
- [ ] `_type_of_expression` should return `bool`
- [ ] `_type_of_expression` should have an option to write out the types of subexpressions to remove need for separate typechecker

# Version 1
- [ ] AST interpreter (compile-time) -> compile time execution
- [ ] Source code to machine code (x86) -> host procedure execution
- [ ] No optimization (yet!)

# Optional / Later
- [ ] utility (function + commandline) to generate C/C++ bindings from script
- [ ] errors should use an arena so that `destroy_error` can be removed
- [ ] figure out how to do multi declaration statements `a, b, c: int;`
- [ ] remove `;` from case statements?
- [ ] measure performance impact of `occupied` in hash table
- [ ] add `then` keyword
- [ ] add multi-line string literals
