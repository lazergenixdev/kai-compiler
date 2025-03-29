#include <stdio.h>
#define KAI_USE_MEMORY_API
#define KAI_USE_DEBUG_API
#include "kai/kai.h"

#define test ++test_count, pass_count +=

extern int bytecode();
extern int parser();
extern int hash_table();
extern int compile_simple_add();

Kai_Debug_String_Writer error_writer;

int main(int argc, char** argv) {
    Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
    writer->write_string(writer->user, kai_get_version_string());
    writer->write_string(writer->user, KAI_STRING("\n\n"));
    kai_debug_open_file_writer(&error_writer, "errors.txt");

    int test_count = 0;
    int pass_count = 0;

    test parser();
    test bytecode();
    test hash_table();
    test compile_simple_add();

    kai_debug_close_file_writer(&error_writer);

    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "\n%d/%d test passed\n", pass_count, test_count);
        writer->write_c_string(writer->user, buffer);
    }

    return 0;
}

void begin_test(const char* name) {
    printf("%40s...", name);
    fflush(stdout);
}
