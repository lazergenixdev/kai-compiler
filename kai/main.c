#include <stdio.h>
#include <stdlib.h>
#define KAI_USE_DEBUG_API
#define KAI_USE_MEMORY_API
#include "kai/kai.h"
#include <time.h>

#define ESC "\x1b"

void set_underline(int enable) {
    printf(ESC "[%im", enable ? 4 : 24);
}

int load_file(const char* file_path, Kai_str* out) {
    FILE* file = fopen(file_path, "rb");
    if (!file) return 1;
    fseek(file, 0, SEEK_END);
    off_t size = ftello(file);
    void* data = malloc(size);
    fseek(file, 0, SEEK_SET);
    fread(data, 1, size, file);
    fclose(file);
    out->count = size;
    out->data = data;
    return 0;
}

#define try(X) if (X) return printf("%s \x1b[91mfailed\x1b[0m (%s:%i)\n", #X, __FILE__, __LINE__), 1

extern uintptr_t bci__call_native_procedure(void* address, uintptr_t* args, int arg_count);

Kai_s32 foo(Kai_s32 a, Kai_s32 b) {
    puts(__PRETTY_FUNCTION__);
    return a + 0x45 - b;
}

int what() {
    {
        uintptr_t args [] = { 74, 420 };
        bci__call_native_procedure(foo, args, 2);
    }
    return 0;
}

int simple_example(int argc, char* argv[]);

// Simple Machine Code Generation
////////////////////////////////////////////////////////////////////
#define kai__arm64_opcode(x) ((x) << 24)
uint32_t kai__arm64_add_u32(uint32_t Rd, uint32_t Rn, uint32_t Rm) {
    return kai__arm64_opcode(0b00001011) | (Rd << 0) | (Rn << 5) | (Rm << 16);
}
uint32_t kai__arm64_add_u64(uint32_t Rd, uint32_t Rn, uint32_t Rm) {
    return kai__arm64_opcode(0b10001011) | (Rd << 0) | (Rn << 5) | (Rm << 16);
}
#define kai__arm64_ret() 0xd65f03c0
////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    what();
    //return simple_example(argc, argv);

    Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
    set_underline(1);
    writer->write_string(writer->user, kai_get_version_string());
    set_underline(0);
    putchar('\n');

    for (int i = 0; i < argc; ++i) {
        printf("%2i: \"%s\"\n", i, argv[i]);
    }

    if (argc <= 1) return 0;
    Kai_str source_code = {0};

    try( load_file(argv[1], &source_code) );

    Kai_Memory_Allocator memory;
    kai_create_memory(&memory);

    {
        Kai_int size = memory.page_size;
        Kai_ptr exe_mem = memory.allocate(memory.user, size, KAI_MEMORY_ACCESS_WRITE);
        
        typedef Kai_u32 (* Func)(Kai_u32 a, Kai_u32 b);

        Kai_u32* instr = exe_mem;
        Kai_u32 count = 0;

        instr[count++] = kai__arm64_add_u32(0, 0, 1);
        instr[count++] = kai__arm64_ret();

        memory.set_access(memory.user, exe_mem, size, KAI_MEMORY_ACCESS_EXECUTE);

        Func f = (Func)exe_mem;
        Kai_u32 result = f(3, 5);

        printf("(add) result = %u\n", result);

        memory.free(memory.user, exe_mem, size);
    }

    Kai_Error error = {0};
    Kai_Syntax_Tree tree = {0};
    Kai_Syntax_Tree_Create_Info info = {
        .error = &error,
        .memory = memory,
        .source_code = source_code,
    };

    clock_t time_start = clock();
    Kai_Result result = kai_create_syntax_tree(&info, &tree);
    clock_t time_end = clock();
    
    double elapsed_time = (double)(time_end - time_start) / CLOCKS_PER_SEC;
    //printf("parsing took: \x1b[36m%f ms\x1b[0m\n", 1e3 * elapsed_time);
    printf("parsing took: \x1b[36m%.2f us\x1b[0m\n", 1e6 * elapsed_time);

    if (KAI_FAILED(result)) {
        error.location.file_name = kai_str_from_c(argv[1]);
        kai_debug_write_error(writer, &error);
    }
    else {
        kai_debug_write_syntax_tree(writer, &tree);
    }

    printf("memory usage: %u bytes\n", kai_memory_usage(&memory));

    kai_destroy_syntax_tree(&tree);
    kai_destroy_memory(&memory);

    return 0;
}

#if 0
typedef Kai_int Main_Proc(Kai_array);

// ./kai "script.kai" arg1 arg2 ...
int simple_example(int argc, char* argv[]) {
    if (argc < 2) return 1;

    Kai_str source_code = {0};
    try( load_file(argv[1], &source_code) );

    Kai_Error error = {0};
    Kai_Program program = {0};
    Kai_Result result = kai_create_program_from_source(source_code, &error, &program);

    if (result != KAI_SUCCESS) {
        kai_debug_write_error(kai_debug_stdout_writer(), &error);
        return 1;
    }

    void* main_raw = kai_find_procedure(program, "main", NULL);
    Main_Proc* main_proc = (Main_Proc*)main_raw;
    
    if (main_proc == NULL) {
        return 1;
    }

    Kai_array args = {0};
    if (argc > 2) {
        args.count = argc - 2;
        args.data = malloc(sizeof(Kai_str) * args.count);
        Kai_str* command_line = (Kai_str*)args.data;
        for (int i = 0; i < args.count; ++i) {
            command_line[i] = kai_str_from_c(argv[i + 2]);
        }
    }

    return main_proc(args);
}
#endif
