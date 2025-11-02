#include "test.h"

void check_procedure(Kai_Program* program, Kai_string name)
{
    void* proc = kai_find_procedure(program, name, (Kai_string){});
    assert_true(proc != NULL);
}

int main()
{
    Kai_Program program = {0};
    Kai_Source sources[] = { load_source_file("scripts/simple-procedures.kai") };
    Kai_Program_Create_Info info = {
        .allocator = default_allocator(),
        .error = default_error(),
        .sources = MAKE_SLICE(sources),
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
    };
    kai_create_program(&info, &program);
    assert_no_error();

    check_procedure(&program, KAI_STRING("ceil_div"));
    check_procedure(&program, KAI_STRING("ceil_div_fast"));
}