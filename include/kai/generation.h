#ifndef KAI_GENERATION_H
#define KAI_GENERATION_H
#include <kai/parser.h>
__KAI_BEGIN_API__

typedef struct {
	kai_Module* module;
	kai_Memory  memory;
	kai_Error*  error_info;
} kai_Program_Create_Info;

typedef struct kai_Program_Impl* kai_Program;

KAI_API(kai_result)
	kai_create_program(kai_Program_Create_Info* Info, kai_Program* out_Program);

KAI_API(void)
	kai_destroy_program(kai_Program Program);

// Example: `proc = kai_find_procedure(program, "main", (int, *str) -> int")`
KAI_API(void*)
	kai_find_procedure(kai_Program Program, char const* Name, char const* Type);

__KAI_END_API__
#endif//KAI_GENERATION_H