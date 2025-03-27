#define KAI_USE_DEBUG_API
#include "config.h"

void
kai_destroy_program(Kai_Program Program)
{
	UNIMPLEMENTED(Program);
}

void*
kai_find_procedure(Kai_Program Program, char const* Name, char const* Type)
{
//    Kai_Type type = kai__parse_type(Type);
//    return kai__program_find(Program, Name, type);
    return NULL;
}

void
kai_debug_write_type(Kai_Debug_String_Writer* Writer, Kai_Type Type)
{
	UNIMPLEMENTED(Writer, Type);
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

#if 0
void __branching_examples() {
    int EXPR;
    // if (EXPR) { A } else { B }
    if (~EXPR) goto else__2;
    {
        // A
    }
    goto endif__2;
else__2:
    {
        // B
    }
endif__2:
    // BYTECODE_OP_COMPARE
    // BYTECODE_OP_BRANCH
    // ...
    // BYTECODE_OP_JUMP
    // ...

    // while (EXPR) { A }
while__0:
    if (~EXPR) goto endwhile__0;
    {
        // A
    }
    goto while__0;
endwhile__0:
    // BYTECODE_OP_COMPARE
    // BYTECODE_OP_BRANCH
    // ...
    // BYTECODE_OP_JUMP
    (void)0;
}
#endif