#define KAI_USE_DEBUG_API
#include "config.h"

void
kai_destroy_program(Kai_Program Program)
{
	UNIMPLEMENTED(Program);
}

void*
kai_find_procedure(Kai_Program Program, Kai_str Name, Kai_Type Type)
{
	UNIMPLEMENTED(Program, Name, Type);
    return NULL;
}

Kai_bool is_convertible(Kai_Type from, Kai_Type to) {
    switch (from->type) {
        case KAI_TYPE_TYPE: {
            return to->type == KAI_TYPE_TYPE;
        } break;

        case KAI_TYPE_INTEGER: {
            
            return KAI_FALSE;
        } break;

        case KAI_TYPE_FLOAT: {
            return KAI_FALSE;
        } break;

        case KAI_TYPE_POINTER: {
            return KAI_FALSE;
        } break;

        case KAI_TYPE_PROCEDURE: {
            return KAI_FALSE;
        } break;

        case KAI_TYPE_SLICE: {
            return KAI_FALSE;
        } break;

        case KAI_TYPE_STRING: {
            return KAI_FALSE;
        } break;

        default:
        case KAI_TYPE_STRUCT: {
            return KAI_FALSE;
        } break;
    }
}
