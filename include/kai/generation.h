#ifndef KAI_GENERATION_H
#define KAI_GENERATION_H
#include <kai/parser.h>
__KAI_BEGIN_API__

typedef struct {
	kai_Module* module;
	kai_Error_Info* error_info;
} kai_Program_Create_Info;

typedef struct kai_Program_Impl* kai_Program;

KAI_API kai_result kai_create_program(kai_Program_Create_Info* Info, kai_Program* out_Program);

KAI_API void kai_destroy_program(kai_Program program);

KAI_API void* kai_find_procedure(kai_Program program, char const* name, char const* signature);

__KAI_END_API__
#ifdef  KAI_CPP_API

#endif//KAI_CPP_API
#endif//KAI_GENERATION_H