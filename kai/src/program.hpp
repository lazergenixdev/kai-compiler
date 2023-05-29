#pragma once
#include <kai/generation.h>

struct kai_Program_Impl {
	void* executable_memory;
	kai_u64 executable_memory_size;

	void* procedure_table; // @TODO: procedure_table
};

extern kai_Program init_program(void* raw_machine_code, kai_u64 size);
