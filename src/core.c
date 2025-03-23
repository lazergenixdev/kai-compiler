#include "config.h"

//! TODO: only panic in dev builds

void
kai_get_version(Kai_u32* major, Kai_u32* minor, Kai_u32* patch) {
    *major = KAI_VERSION_MAJOR;
    *minor = KAI_VERSION_MINOR;
    *patch = KAI_VERSION_PATCH;
}

Kai_str
kai_get_version_string(void) {
    return KAI_STRING("Kai Compiler " KAI_VERSION_STRING_V);
}

Kai_bool
kai_string_equals(Kai_str a, Kai_str b) {
    if (a.count != b.count) return KAI_FALSE;
        for (Kai_int i = 0; i < a.count; ++i)
            if (a.data[i] != b.data[i]) return KAI_FALSE;
    return KAI_TRUE;
}

Kai_str
kai_str_from_c(char const* string) {
    Kai_int len = 0;
    while (string[len] != '\0') ++len;
    return (Kai_str) { .data = (Kai_u8*)string, .count = len };
}

#include <stdio.h>
#include <stdlib.h>

void panic(void) {
    puts("\nPanic triggered.\nNow exiting...");
    exit(1);
}
