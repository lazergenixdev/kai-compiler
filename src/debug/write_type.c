#define KAI_USE_DEBUG_API
#include "../config.h"

void kai_debug_write_type(Kai_Debug_String_Writer* writer, Kai_Type Type)
{
    switch (Type->type) {
        default:                 { kai__write("[Unknown]"); } break;
        case KAI_TYPE_TYPE:      { kai__write("Type");      } break;
        case KAI_TYPE_INTEGER:   { kai__write("Integer");   } break;   
        case KAI_TYPE_FLOAT:     { kai__write("Float");     } break; 
        case KAI_TYPE_POINTER:   { kai__write("Pointer");   } break;   
        case KAI_TYPE_PROCEDURE: { kai__write("Procedure"); } break;     
        case KAI_TYPE_SLICE:     { kai__write("Slice");     } break; 
        case KAI_TYPE_STRING:    { kai__write("String");    } break;  
        case KAI_TYPE_STRUCT:    { kai__write("Struct");    } break;  
    }
}