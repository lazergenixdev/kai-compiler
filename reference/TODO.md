# TODO
- [x] expression parser
- [x] type parser
- [x] procedure parser
- [x] statement parser
- [x] declaration parser
- [x] module/file parser (top level)
- [x] dependency graph generation (with small visualizer) (use stack of scopes)
- [?] builtin type table, expose api to set what the builtin types should be
- [ ] type checker + code generation (nothing for now, so just type checker)
- [ ] `#char` needs to do a utf8 decode and evaluate to the codepoint value
- [ ] add `then` keyword
- [ ] utility (function + commandline) to generate C/C++ bindings from host script file

# Version 1
- [ ] AST interpreter (compile-time)
- [ ] Source code to machine code (x86)
- [ ] No optimization (yet!)
