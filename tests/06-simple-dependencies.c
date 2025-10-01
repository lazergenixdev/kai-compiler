#include "test.h"

int main()
{
    Kai_Program program = {0};
    Kai_Source sources[] = { load_source_file("scripts/simple-dependencies.kai") };
    Kai_Program_Create_Info info = {
        .allocator = default_allocator(),
        .error = default_error(),
        .sources = MAKE_SLICE(sources),
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
        .debug_writer = default_writer(),
    };
    kai_create_program(&info, &program);
    assert_no_error();

    Kai_Type type = NULL;
    void* ptr = kai_find_variable(&program, KAI_STRING("A"), &type);
    assert_true(type != NULL);
    assert_true(ptr != NULL);
    assert_true(type->id == KAI_TYPE_ID_PROCEDURE);

    // TODO: do more checking here
}