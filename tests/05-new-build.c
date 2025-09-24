#include "test.h"

void check_procedure(Kai_Program* program, Kai_string name)
{
    void* proc = kai_find_procedure(program, name, (Kai_string){});
    assert_true(proc != NULL);
    
    Kai_Expr* expr = *(Kai_Expr**)proc;
    assert_true(expr != NULL);
    assert_true(expr->id == KAI_EXPR_PROCEDURE);
}

int main()
{
    Kai_Program program = {0};
    Kai_Source sources[] = { load_source_file("scripts/new.kai") };
    Kai_Import imports[] = {
        {.name = KAI_CONST_STRING("BUILD_DATE"),     .type = KAI_CONST_STRING("u64"), .value = {.u64 = 0x45}},
        {.name = KAI_CONST_STRING("VERSION_STRING"), .type = KAI_CONST_STRING("u64"), .value = {.u64 = 0x45}},
        {.name = KAI_CONST_STRING("VERSION_MAJOR"),  .type = KAI_CONST_STRING("u32"), .value = {.u32 = 0}},
        {.name = KAI_CONST_STRING("VERSION_MINOR"),  .type = KAI_CONST_STRING("u32"), .value = {.u32 = 1}},
        {.name = KAI_CONST_STRING("VERSION_PATCH"),  .type = KAI_CONST_STRING("u32"), .value = {.u32 = 0}},
    };
    Kai_Program_Create_Info info = {
        .allocator = default_allocator(),
        .error = default_error(),
        .sources = MAKE_SLICE(sources),
        .imports = MAKE_SLICE(imports),
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
    };
    kai_create_program(&info, &program);
    assert_no_error();

    for (Kai_u32 i = 0; i < program.code.trees.count; ++i)
    {
        write_expression((Kai_Expr*)&program.code.trees.data[i].root);
    }

    Kai_Type type = {0};
    kai_find_variable(&program, KAI_STRING("a"), &type);

    default_writer()->set_color(0, KAI_WRITE_COLOR_TYPE);
    kai_write_type(default_writer(), type);
    printf("\n");
    default_writer()->set_color(0, KAI_WRITE_COLOR_PRIMARY);
}