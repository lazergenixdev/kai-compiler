#pragma once
#include <kai/generation.h>

struct kai_Program_Impl {
	void* executable_memory;
	kai_u64 executable_memory_size;

	void* procedure_table; // @TODO: procedure_table
};

struct Machine_Code {
	void* data;
	kai_u64 size;
};

extern kai_Program init_program(Machine_Code code);

extern Machine_Code generate_machine_code(void* bytecode, kai_u64 size);
