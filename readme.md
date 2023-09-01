
# WIP Experimental Scripting language made for real-time applications.

- Strict static type checking
- Compile-then-execute model (no JIT/Interpreter)
	- *there will be an bytecode interpreter for compile-time execution
- Compiler API written in C, and implemented in C++
- Runtime Safety will be achieved with runtime checks (will be able to be toggled off for faster performance)
- Kai is a place-holder name, I an have no idea what to call this language
- My first compiler, so don't expect anything crazy

Notes:
- only works on Windows (because costum allocator uses VirtualAlloc to reserve multiple pages at a time).
- will use re2c to parse keywords in lexer, I think it is better than using a hash table, but this will need to be tested.
- will use AsmJit for machine code generation, as it will speed up development time significantly.
- will need dyncall for compile-time execution of bytecode

## TODO
- [ ] add "type slot" to declarations
- [ ] add Expression type-checker
- [ ] add Expression evaluator (compile-time)
- [ ] use base address when generating machine code
- [ ] put license information in source files
# How do I want the API to look?
```C++
kai_Error error;
kai_AST tree;
tree.source_filename = KAI_STR("main.kai");

kai_result result;
// Create AST
{
	kai_Syntax_Tree_Create_Info info;
	info.source = source_code;
	info.memory = tree_memory;
	info.error  = &error;
	result = kai_create_syntax_tree(&info, &tree);
}

if (result != kai_Result_Success) handle_error(&error);

// Now that we have the AST, we can now modify the AST as we please

// Compile into a program we can run directly
kai_Program program;
{
	kai_Program_Create_Info info;
	info.trees      = &tree;
	tree.tree_count = 1;
	info.memory     = program_memory;
	info.error      = &error;
	info.flags      = 0;
	result = kai_create_program(&info, &program);
}

if (result != kai_Result_Success) handle_error(&error);

// Now we get the main procedure from our script
using main_proc_type = kai_int(kai_int, kai_str*);
auto main_proc = (main_proc_type*)kai_find_procedure(program, "main");

if (main_proc == nullptr) error("main not found!");

// AST can be freed now
kai_destroy_memory(&tree_memory);

// We now use "main_proc" as if it is just a C function!
kai_str args[] = { KAI_STR("main.kai"), KAI_STR("script test") };
kai_int r = main_proc(2, args);
std::cout << "main returned " << r << '\n';

// uninit
kai_destroy_memory(&program_memory);
kai_destroy_program(program);
```
