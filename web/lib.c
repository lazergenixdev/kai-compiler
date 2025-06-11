extern void __wasm_console_log(const char* message, int value);
extern void __wasm_write_string(Kai_ptr User, Kai_string String);
extern void __wasm_write_value(Kai_ptr User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format format);
extern void __wasm_set_color(Kai_ptr User, Kai_Write_Color Color_Index);

static Kai_String_Writer debug_writer = {
	.write_string   = &__wasm_write_string,
	.write_value    = &__wasm_write_value,
	.set_color      = &__wasm_set_color,
};

#define kai__debug_writer  (&debug_writer)
#define kai__debug(...)    (void)0

#define KAI_DONT_USE_WRITER_API
#define KAI_IMPLEMENTATION
#include "kai.h"

static Kai_String_Writer div_writer = {
	.write_string   = &__wasm_write_string,
	.write_value    = &__wasm_write_value,
	.set_color      = &__wasm_set_color,
};

__attribute__((__visibility__("default")))
int compile(const char* source)
{
    Kai_Allocator allocator = {0};
    kai_memory_create(&allocator);
	
	Kai_Error error = {0};
	Kai_Program program = {0};
    Kai_Program_Create_Info info = {
        .allocator = &allocator,
        .error = &error,
        .source = { kai_string_from_c(source) },
        .options = KAI_DEFAULT_COMPILE_OPTIONS,
    };
	Kai_Result result = kai_create_program_from_code(&info, &program);

    if (result != KAI_SUCCESS)
        kai_write_error(&div_writer, &error);

    kai_destroy_error(&error, &allocator);
    kai_memory_destroy(&allocator);

    return (result != KAI_SUCCESS);
}

__attribute__((__visibility__("default")))
int create_syntax_tree(const char* source)
{
    Kai_Allocator allocator = {0};
    kai_memory_create(&allocator);
	
    Kai_Error error = {0};
	Kai_Syntax_Tree tree = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = allocator,
		.error = &error,
		.source_code = kai_str_from_cstring(source),
	};
	Kai_Result result = kai_create_syntax_tree(&info, &tree);

    if (result != KAI_SUCCESS)
        kai_write_error(&div_writer, &error);
    else
        kai_write_syntax_tree(&div_writer, &tree);

    kai_destroy_error(&error, &allocator);
    kai_destroy_syntax_tree(&tree);
    kai_memory_destroy(&allocator);

    return (result != KAI_SUCCESS);
}
