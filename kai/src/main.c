#define KAI_USE_DEBUG_API
#define KAI_USE_MEMORY_API
#include "kai/kai.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define error(...) printf("\x1b[91mError\x1b[0m: "), printf(__VA_ARGS__), putchar('\n')

typedef Kai_s32 Main_Proc(Kai_slice);

void parse(char const* file, Kai_str source_code, Kai_Memory_Allocator* allocator);

int main(int argc, char** argv) {
    int exit_value = 1;
    struct {
        Kai_bool parse_only;
    } options = {
        .parse_only = KAI_FALSE,
    };

    Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
    
	set_underline(1);
    writer->write_string(writer->user, kai_get_version_string());
    set_underline(0);
    writer->write_char(writer->user, '\n');

    if (argc < 2) {
		printf("usage: kai [--parse-only] <SOURCE> [args...]\n");
		return 1;
	}

    char const* file = argv[1];

    if (strcmp(argv[1], "--parse-only") == 0) {
        if (argc < 3) {
		    error("no input file provided");
            return 1;
        }
        options.parse_only = KAI_TRUE;
        file = argv[2];
    }

    Kai_str source_code = {0};
    if (load_file(file, &source_code)) {
		error("unable to load file \"%s\"", argv[1]);
		return 1;
	}

	Kai_Memory_Allocator allocator;
	kai_create_memory(&allocator);

    if (options.parse_only) {
        parse(file, source_code, &allocator);
        goto cleanup;
    }

    Kai_Error error = {0};
    Kai_Program program = {0};
    Kai_Result result;
    Timer timer = {0};
    Timer_Init();

    Timer_Start(&timer);
    result = kai_create_program_from_source(source_code, &allocator, &error, &program);
    double elapsed_ms = Timer_ElapsedMilliseconds(&timer);

    printf("Compilation Took: %.4f ms\n", elapsed_ms);

    if (result != KAI_SUCCESS) {
		error.location.file_name = kai_str_from_cstring(file);
        kai_debug_write_error(kai_debug_stdout_writer(), &error);
        goto cleanup;
    }

    void* main_raw = kai_find_procedure(program, "main", NULL);
    Main_Proc* main_proc = (Main_Proc*)main_raw;
    
    if (main_proc == NULL) {
		error("unable to find procedure \"main\"");
        goto cleanup;
    }

    {
        Kai_slice args = {0};
        if (argc > 2) {
            args.count = argc - 2;
            args.data = malloc(sizeof(Kai_str) * args.count);
            Kai_str* command_line = (Kai_str*)args.data;
            for (Kai_u32 i = 0; i < args.count; ++i) {
                command_line[i] = kai_str_from_cstring(argv[i + 2]);
            }
        }
        exit_value = main_proc(args);
    }
	
cleanup:
    {
        Kai_Result result = kai_destroy_memory(&allocator);
        if (result) {
		    error("Some allocations were not freed! (amount=%u B)", kai_memory_usage(&allocator));
        }
    }
	return exit_value;
}

void parse(char const* file, Kai_str source_code, Kai_Memory_Allocator* allocator) {
    Kai_Error error = {0};
    Kai_Syntax_Tree_Create_Info info = {
        .allocator = *allocator,
        .error = &error,
        .source_code = source_code,
    };

    Kai_Syntax_Tree tree = {0};
    Kai_Result result = kai_create_syntax_tree(&info, &tree);

    if (result != KAI_SUCCESS) {
		error.location.file_name = kai_str_from_cstring(file);
        kai_debug_write_error(kai_debug_stdout_writer(), &error);
    }
    else {
        kai_debug_write_syntax_tree(kai_debug_stdout_writer(), &tree);
    }

    kai_destroy_syntax_tree(&tree);
}
