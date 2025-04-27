#include "config.h"

//! TODO: only panic in dev builds

Kai_vector3_u32 kai_get_version(void)
{
    return (Kai_vector3_u32) {
        .x = KAI_VERSION_MAJOR,
        .y = KAI_VERSION_MINOR,
        .z = KAI_VERSION_PATCH,
    };
}

Kai_str kai_get_version_string(void)
{
    return KAI_STRING("Kai Compiler v" KAI_VERSION_STRING);
}

Kai_bool kai_str_equals(Kai_str a, Kai_str b)
{
    if (a.count != b.count) return KAI_FALSE;
        for (Kai_int i = 0; i < a.count; ++i)
            if (a.data[i] != b.data[i]) return KAI_FALSE;
    return KAI_TRUE;
}

Kai_str kai_str_from_cstring(char const* string)
{
    Kai_u32 len = 0;
    while (string[len] != '\0') ++len;
    return (Kai_str) {.data = (Kai_u8*)string, .count = len};
}

#define kai__allocate(S) allocator->heap_allocate(allocator->user, 0, S, 0)
#define kai__free(P,S)   allocator->heap_allocate(allocator->user, P, 0, S)

void kai_destroy_error(Kai_Error* Error, Kai_Allocator* allocator)
{
    while (Error)
    {
        Kai_Error* next = Error->next;
        if (Error->memory.size)
        {
            kai__free(Error->memory.data, Error->memory.size);
        }
        Error = next;
    }
}

#include <stdio.h>
#include <stdlib.h>

void panic(void) {
    puts("\nPanic triggered. Now exiting...");
    exit(1);
}

static char const* kai__file(char const* cs)
{
	Kai_str s = kai_str_from_cstring(cs);
	Kai_u32 i = s.count - 1;
	while (i > 0)
	{
		if (cs[i] == '/' || cs[i] == '\\')
		{
			return cs + i + 1;
		}
		i -= 1;
	}
	return cs;
}

void kai__fatal_error(
	char const* Desc,
	char const* Message,
	char const* File,
	int         Line)
{
	printf("\x1b[91m%s\x1b[0m: \"\x1b[94m%s\x1b[0m\"\nin \x1b[92m%s:%i\x1b[0m",
		Desc, Message, kai__file(File), Line
	);
	exit(1);
}
