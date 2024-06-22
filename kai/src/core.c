#include <kai/core.h>

void kai_get_version(Kai_Version* version) {
    version->major = KAI_VERSION_MAJOR;
    version->minor = KAI_VERSION_MINOR;
    version->patch = KAI_VERSION_PATCH;
}

Kai_str kai_get_version_string() {
    return KAI_STR("Kai Compiler " KAI_VERSION_STRING_V);
}

Kai_bool kai_string_equals(Kai_str a, Kai_str b) {
    if (a.count != b.count) return KAI_FALSE;
        for (Kai_int i = 0; i < a.count; ++i)
            if (a.data[i] != b.data[i]) return KAI_FALSE;
    return KAI_TRUE;
}

#include <stdio.h>
#include <stdlib.h>

void panic(void) {
    puts("\nPanic triggered.\nNow exiting...");
    exit(1);
}
