
# WIP Experimental Scripting language made for real-time applications.

- Strict static type checking
- Compile-then-execute model (no JIT/Interpreter)
	- *there will be an bytecode interpreter for compile-time execution
- Compiler API written in C, and implemented in C++

Notes:
- only works on Windows (because costum allocator uses VirtualAlloc to reserve multiple pages at a time).
- will use re2c to parse keywords in lexer, I think it is better than using a hash table, but this will need to be tested.
- will use AsmJit for machine code generation, as it will speed up development time significantly.
- will update license to be GPL
