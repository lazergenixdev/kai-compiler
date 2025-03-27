#define KAI_USE_DEBUG_API
#define KAI_USE_MEMORY_API
#include "kai/kai.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>

void set_underline(int enable) {
    printf("\x1b[%im", enable ? 4 : 24);
}

int load_file(const char* file_path, Kai_str* out) {
    FILE* file = fopen(file_path, "rb");
    if (!file) return 1;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    void* data = malloc(size);
    fseek(file, 0, SEEK_SET);
    fread(data, 1, size, file);
    fclose(file);
    out->count = size;
    out->data = data;
    return 0;
}

#define ERROR "\x1b[91merror\x1b[0m: "

typedef Kai_s32 Main_Proc(Kai_slice);

int main(int argc, char** argv) {
    Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
    
	set_underline(1);
    writer->write_string(writer->user, kai_get_version_string());
    set_underline(0);
    writer->write_char(writer->user, '\n');

    if (argc < 2) {
		printf("usage: kai <SOURCE> [args...]");
		return 1;
	}

    Kai_str source_code = {0};
    if (load_file(argv[1], &source_code)) {
		printf(ERROR "unable to load file \"%s\"", argv[1]);
		return 1;
	}

	Kai_Memory_Allocator allocator;
	kai_create_memory(&allocator);

    Kai_Error error = {0};
    Kai_Program program = {0};
    Kai_Result result = kai_create_program_from_source(source_code, &allocator, &error, &program);

    if (result != KAI_SUCCESS) {
		error.location.file_name = kai_str_from_cstring(argv[1]);
        kai_debug_write_error(kai_debug_stdout_writer(), &error);
        return 1;
    }

    void* main_raw = kai_find_procedure(program, "main", NULL);
    Main_Proc* main_proc = (Main_Proc*)main_raw;
    
    if (main_proc == NULL) {
		printf(ERROR "unable to find procedure \"main\"");
        return 1;
    }

    Kai_slice args = {0};
    if (argc > 2) {
        args.count = argc - 2;
        args.data = malloc(sizeof(Kai_str) * args.count);
        Kai_str* command_line = (Kai_str*)args.data;
        for (Kai_u32 i = 0; i < args.count; ++i) {
            command_line[i] = kai_str_from_cstring(argv[i + 2]);
        }
    }

    int exit_value = main_proc(args);
	
	kai_destroy_memory(&allocator);
	return exit_value;
}
