#include "test.h"
#include <setjmp.h>

#if 0 // Psuedo-code
typedef Kai_bool Kai_P_Is_Valid_Address(Kai_ptr Address, Kai_u32 Size);

// %3 <- add.u32 %0, %2
// check_address.u32 (%3)
// store.u32 [%3] <- %1

jmp_buf buf;
Kai_P_Is_Valid_Address* is_valid_address;

void kai__check_address(void* address, Kai_u32 size) {
    if (is_valid_address(address, size)) return;
	// signal to host program that script has failed
    longjmp(buf, 0);
}

// Call this once before initializing script
int check_script() {
    return setjmp(buf);
}

typedef void Func(Kai_int);
typedef struct {
	Func* func;
	Kai_bool initialized;
	Kai_bool valid;
} Script;

void example(Script script)
{
	if (!script.initialized)
	{
		script.initialized = KAI_TRUE;
		if (check_script()) {
			script.valid = KAI_FALSE;
		}
	}

	if (script.valid) {
		script.func(7);
	}
}
#endif

#if 0
typedef struct {
	Kai_s32 input;
	Kai_s32 expected_output;
} Test;

Kai_str test_programs[] = {
	KAI_STRING(
		"func :: (a: s32, b: s32) {\n"
		"\tret a + b;\n"
		"}\n"
	),
	KAI_STRING(
		"func :: (a: s32, b: s32) {\n"
		"\tret a + b;\n"
		"}\n"
	),
};
#endif

int compile_simple_add() {
    TEST();

    Kai_Allocator allocator = {0};
    kai_memory_create(&allocator);

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

	#if 0
    typedef Kai_s32 Proc(Kai_s32, Kai_s32);
    Proc* proc = kai_find_procedure(program, KAI_STRING("add"), NULL);

    Kai_bool failed = KAI_FALSE;
    if (5 != proc(2, 3)) failed = KAI_TRUE;
    if (0 != proc(0, 0)) failed = KAI_TRUE;
    if (0x45 != proc(0x22, 0x23)) failed = KAI_TRUE;

    return (result != KAI_SUCCESS) ? FAIL : PASS;
	#endif

    kai_memory_destroy(&allocator);
	return FAIL;
}
