#include "test.h"

void check_procedure(Kai_Program* program, Kai_string name)
{
    void* proc = kai_find_procedure(program, name, (Kai_string){});
    assert_true(proc != NULL);
    
    Kai_Expr* expr = *(Kai_Expr**)proc;
    assert_true(expr != NULL);
    assert_true(expr->id == KAI_EXPR_PROCEDURE);
}

#define hash_table_iterate(Table, Iter_Var)                               \
  for (Kai_u32 Iter_Var = 0; Iter_Var < (Table).capacity; ++Iter_Var)     \
    if ((Table).occupied[Iter_Var / 64] & ((Kai_u64)1 << (Iter_Var % 64)))

int main()
{
    Kai_Writer* writer = default_writer();

    Kai_Program program = {0};
    Kai_Source sources[] = { load_source_file("scripts/new.kai") };
    Kai_Import imports[] = {
        {.name = KAI_CONST_STRING("BUILD_DATE"),     .type = KAI_CONST_STRING("u64"), .value = {.u64 = 0x45}},
        {.name = KAI_CONST_STRING("VERSION_STRING"), .type = KAI_CONST_STRING("string"), .value = {.string = KAI_STRING("v1.0.0")}},
        {.name = KAI_CONST_STRING("VERSION_MAJOR"),  .type = KAI_CONST_STRING("u32"), .value = {.u32 = 0}},
        {.name = KAI_CONST_STRING("VERSION_MINOR"),  .type = KAI_CONST_STRING("u32"), .value = {.u32 = 1}},
        {.name = KAI_CONST_STRING("VERSION_PATCH"),  .type = KAI_CONST_STRING("u32"), .value = {.u32 = 0}},
    };
    //printf("(%p)", imports[1].value.string.data);
    Kai_Program_Create_Info info = {
        .allocator = default_allocator(),
        .error = default_error(),
        .sources = MAKE_SLICE(sources),
        .imports = MAKE_SLICE(imports),
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
    };
    kai_create_program(&info, &program);
    assert_no_error();

/*
    for (Kai_u32 i = 0; i < program.code.trees.count; ++i)
    {
        write_expression((Kai_Expr*)&program.code.trees.data[i].root);
    }
*/

    hash_table_iterate(program.variable_table, i)
    {
        Kai_string name = program.variable_table.keys[i];
        Kai_Variable var = program.variable_table.values[i];
        printf("%.*s = ", (int)(name.count), name.data);
        kai_write_value(writer, program.data.data + var.location, var.type);
        kai__write(" [");
        kai__set_color(KAI_WRITE_COLOR_TYPE);
        kai_write_type(writer, var.type);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write("]\n");
    }
}