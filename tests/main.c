#include <stdio.h>
#define KAI_IMPLEMENTATION
#include "test.h"

#define test ++test_count, pass_count +=

extern int bytecode           (void);
extern int parser             (void);
extern int hash_table         (void);
extern int compile_simple_add (void);

int main(int argc, char** argv)
{
	kai__unused(argc);
	kai__unused(argv);
    Kai_String_Writer* writer = kai_writer_stdout();
    kai__write_string(kai_version_string());
    kai__write_string(KAI_STRING("\n\n"));
    kai_writer_file_open(error_writer(), "errors.txt");

    int test_count = 0;
    int pass_count = 0;

    test parser();
    test bytecode();
    test hash_table();
    test compile_simple_add();

    kai_writer_file_close(error_writer());

    {
        char buffer[128];
        int size = snprintf(buffer, sizeof(buffer), "\n%d/%d test passed\n", pass_count, test_count);
        writer->write_string(writer->user, (Kai_string){.count = size, .data = (Kai_u8*)buffer});
    }

    return 0;
}

void begin_test(const char* name) {
    printf("%40s...", name);
    fflush(stdout);
    char buffer[64];
    int size = snprintf(buffer, sizeof(buffer), "******** %40s ********\n", name);
    Kai_String_Writer* writer = error_writer();
    kai__write_string((Kai_string) { .count = size, .data = (Kai_u8*)buffer });
}

extern void* error_writer(void) {
    static Kai_String_Writer writer;
    return &writer;
}
