
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

# Example Usage
```C
void example() {
	Kai_str source_code = ...;
	Kai_Result result;

    Kai_Memory memory;
    kai_create_memory(&memory);

    Kai_Error error = {0};

    Kai_Syntax_Tree tree;
    {
		Kai_Syntax_Tree_Create_Info info = {
			.error = &error,
			.memory = memory,
			.source_code = source_code,
		};
		result = kai_create_syntax_tree(&info, &tree);
	}

    if (KAI_FAILED(result)) handle_error(&error);

	Kai_Program program;
	{
		Kai_Program_Create_Info info = {
			.trees      = &tree;
			.tree_count = 1;
			.memory     = memory;
			.error      = &error;
		}
		result = kai_create_program(&info, &program);
	}

    if (KAI_FAILED(result)) handle_error(&error);

    kai_destroy_memory(&memory);

	// Now we get the main procedure from our script
	typedef Kai_int(*Proc)(Kai_int, Kai_ptr);
	Proc script_main = kai_find_procedure(program, "main", "(int, *u8) -> int");

	if (script_main == NULL) error("main not found!");

	Kai_int r = script_main(6, "hello!");
	printf("script_main returned %i\n", r);

	kai_destroy_memory(&memory);
	kai_destroy_program(program);
```
