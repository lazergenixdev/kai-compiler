#include "test.h"
#include <setjmp.h>

Kai_Result
kai_create_program_from_source(Kai_str Source, Kai_Memory_Allocator* Allocator, Kai_Error* out_Error, Kai_Program* out_Program) {
    return KAI_ERROR_FATAL;        
}

void kai_create_memory_allocator(Kai_Memory_Allocator* a) { (void)a; }
void kai_destroy_memory_allocator(Kai_Memory_Allocator* a) { (void)a; }

typedef Kai_bool Kai_P_Is_Valid_Address(Kai_ptr Address, Kai_u32 Size);

// %3 <- add.u32 %0, %2
// check_address.u32 (%3)
// store.u32 [%3] <- %1

jmp_buf buf;
Kai_P_Is_Valid_Address* is_valid_address;

void kai__check_address(void* address, Kai_u32 size) {
    if (is_valid_address(address, size)) return;
    longjmp(buf, 0);
}

int check_script() {
    return setjmp(buf);
}

int compile_simple_add() {
    TEST();

    Kai_Memory_Allocator allocator = {0};
    kai_create_memory_allocator(&allocator);

    Kai_Program program = {0};
    Kai_Error error = {0};
    Kai_str source_code = KAI_STRING(
        "add :: (a: s32, b: s32) {\n"
        "\tret a + b;\n"
        "}\n"
    );

    Kai_Result result = kai_create_program_from_source(source_code, &allocator, &error, &program);

    if (result != KAI_SUCCESS) {
        error.location.file_name = KAI_STRING(__FUNCTION__);
        kai_debug_write_error(&error_writer, &error);
        return FAIL;
    }

    typedef Kai_s32 Proc(Kai_s32, Kai_s32);
    Proc* proc = kai_find_procedure(program, "add", NULL);

    Kai_bool failed = KAI_FALSE;
    if (5 != proc(2, 3)) failed = KAI_TRUE;
    if (0 != proc(0, 0)) failed = KAI_TRUE;
    if (0x45 != proc(0x22, 0x23)) failed = KAI_TRUE;

    kai_destroy_memory_allocator(&allocator);

    return (result != KAI_SUCCESS) ? FAIL : PASS;
}
