# TODO
- [x] `#char` needs to do a utf8 decode and evaluate to the codepoint value
- [x] check if node's dependencies are evaluated before we compile this node
- [x] need some way to only type-check without needing the value in `_value_of_expr`
- [x] need a writer that writes into a buffer (just a Arena wrapper)
- [ ] by default, we need to cache all types, and only create new types for ones we haven't seen (or marked "#distinct")
- [ ] cached typed need to be per-scope
- [ ] add keywords `then`, (separator) `null`, `true`, `false` (Special)
- [ ] add statement tags {}
- [ ] add `#optional_export` and `#require_export` directives
- [ ] need some kind of import handler from host
- [ ] remove need for `destroy_syntax_tree` by passing in arena as input to `create_syntax_tree`
- [ ] ??? `_type_of_expression` should have an option to write out the types of subexpressions to remove need for separate typechecker
- [ ] rename `EXPR_ARRAY` to `EXPR_ARRAY_TYPE`
- [ ] fix parser so that `#array` and `#map` can be removed
- [ ] fix nested constant declarations

# Optional / Later
- [ ] utility (function + commandline) to generate C/C++ bindings from script
- [ ] errors should use an arena so that `destroy_error` can be removed
- [ ] figure out how to do multi declaration statements `a, b, c: int;` (in AST)
- [ ] remove `;` from case statements?
- [ ] measure performance impact of `occupied` in hash table
- [ ] add multi-line string literals
- [ ] parser needs a recovery mode for syntax highlighting only??
- [ ] consider using `KAI_IMP` instead of `KAI_API` for implementation for grep purposes
- [ ] strings need to be handled better in parser/tokenizer

# Version 0.5
- [ ] AST interpreter (compile-time) -> compile time execution

# Version 1
- [ ] Source code to machine code (x86) + host procedure execution
- [ ] No optimization (yet!)

