#include <kai/kai.h>
#include <kai/debug.h>
#include "config.h"

void
kai_destroy_program(Kai_Program Program) {
    (void)Program;
	UNIMPLEMENTED();
}

void*
kai_find_procedure(Kai_Program Program, char const* Name, char const* Type) {
    (void)Program;
    (void)Name;
    (void)Type; // need to parse type :(
	UNIMPLEMENTED();
    return NULL;
}

void
kai_debug_write_type(Kai_Debug_String_Writer* Writer, Kai_Type Type) {
    (void)Writer;
    (void)Type;
	UNIMPLEMENTED();
}
