#include "test.h"

int main()
{
    Kai_Program program = {0};
    Kai_Source sources[] = { load_source_file("scripts/simple-import-float.kai") };
    Kai_Import imports[] = {
        {.name = KAI_CONST_STRING("pi"), .type = KAI_CONST_STRING("f32"), .value = {.f32 = 3.14f}},
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

    Kai_Type type = NULL;
    void* ptr = kai_find_variable(&program, KAI_STRING("result"), &type);
    assert_true(type != NULL);
    assert_true(type->id == KAI_TYPE_ID_FLOAT);

    Kai_Type_Info_Float* type_info = (Kai_Type_Info_Float*)type;
    assert_true(type_info->bits == 32);

    Kai_f32* value = (Kai_f32*)ptr;
    assert_true(value != NULL);
    assert_true(*value == 4.0f);
}