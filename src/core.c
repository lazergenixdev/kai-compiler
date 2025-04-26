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

#include <stdio.h>
#include <stdlib.h>

void panic(void) {
    puts("\nPanic triggered. Now exiting...");
    exit(1);
}

void kai__fatal_error(
	char const* Desc,
	char const* Message,
	char const* File,
	int         Line)
{
	printf("\x1b[91m%s\x1b[0m: \"\x1b[94m%s\x1b[0m\"\nin \x1b[92m%s:%i\x1b[0m",
		Desc, Message, File, Line
	);
	exit(1);
}
