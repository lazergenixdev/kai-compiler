#define KAI_USE_DEBUG_API
#include "config.h"

void*
kai_find_procedure(Kai_Program Program, Kai_str Name, Kai_Type Type)
{
	//UNIMPLEMENTED(Program, Name, Type);
    return NULL;
}

#ifndef __WASM__
void debug_dump_memory(void* data, Kai_u32 count)
{
    Kai_u8* bytes = data;
    int k = 0;
    for (;;)
    {
        for (int i = 0; i < 16; ++i)
        {
            if (k >= count)
            {
                return;
            }
            printf("%02X ", bytes[k++]);
        }
        printf("\n");
    }
}
#endif

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
