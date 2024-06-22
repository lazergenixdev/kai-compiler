#ifndef KAI_KAI_H
#define KAI_KAI_H
#include "parser.h"
__KAI_BEGIN_API__

typedef struct {
	Kai_ptr address;
	Kai_str name;
	Kai_str signature;
} Kai_Native_Procedure;

typedef struct {
	Kai_AST*              trees;
	Kai_int               tree_count;
	Kai_Memory            memory;
	Kai_Error*            error;
	Kai_Native_Procedure* native_procedures;
	Kai_int               native_procedure_count;
} Kai_Program_Create_Info;

typedef struct Kai_Program_Impl* Kai_Program;

KAI_API(Kai_Result)
	kai_create_program(Kai_Program_Create_Info* Info, Kai_Program* out_Program);

KAI_API(void)
	kai_destroy_program(Kai_Program Program);

// Example: `proc = kai_find_procedure(program, "main", (int, *str) -> int")`
KAI_API(void*)
	kai_find_procedure(Kai_Program Program, char const* Name, char const* Type);

__KAI_END_API__
#endif//KAI_KAI_H
