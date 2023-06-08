
# WIP Experimental Scripting language made for real-time applications.

- Strict static type checking
- Compile-then-execute model (no JIT/Interpreter)
	- *there will be an bytecode interpreter for compile-time execution
- Compiler API written in C, and implemented in C++
- Kai is a place-holder name, I an have no idea what to call this language
- My first compiler, so don't expect anything crazy

Notes:
- only works on Windows (because costum allocator uses VirtualAlloc to reserve multiple pages at a time).
- will use re2c to parse keywords in lexer, I think it is better than using a hash table, but this will need to be tested.
- will use AsmJit for machine code generation, as it will speed up development time significantly.
- will need dyncall for compile-time execution of bytecode

# How do I want the API to look?
```C++
kai_Error_Info error;
kai_result result;
kai_Module mod; // contains the AST

// Create AST
{
	kai_Syntax_Tree_Create_Info info;
	info.source     = source_code;
	info.module     = &mod;
	info.filename   = kai_static_string("main.kai");
	info.error_info = &error;
	result = kai_create_syntax_tree(&info);
}

if( result != kai_Result_Success ) handle_error(&error);

// Now that we have the AST, we can now modify the AST as we please

kai_Program program;

// Compile into a program we can run directly
{
	kai_Program_Create_Info info;
	info.module     = &mod;
	info.error_info = &error;
	result = kai_create_program(&info, &program);
}

if( result != kai_Result_Success ) handle_error(&error);

// Now we get the main procedure from our script
using main_proc_type = kai_int(kai_int, kai_str*);
auto main_proc = (main_proc_type*)kai_find_procedure(program, "main", "(int, *str) -> int");

if( main_proc == nullptr ) error("main not found!");

// We now use "main_proc" as if it is just a C function!
kai_str args[] = { "main.kai", "script test" };
kai_int r = main_proc(2, args);
std::cout << "main returned " << r << '\n';
```

# TODO
typing:
- add value dependency generation
- add type dependency generation
- add circular dependency check
- add Expression type deduction
- add Expression evaluator (compile-time)
- add bytecode generation for procedures
- add "type slot" to declarations
- add Expression type-checker

errors:
- better Error_Info struct (linked list)

other
- put license information in source files