#define KAI_API(RETURN_TYPE) __attribute__((__visibility__("default"))) extern RETURN_TYPE
#define KAI_DONT_USE_WRITER_API
#define KAI_DONT_USE_MEMORY_API
#define KAI_HAVE_FATAL_ERROR
#define KAI_IMPLEMENTATION
#include "kai.h"

extern void __wasm_console_log(const char* message, int value);
extern void __wasm_write_string(Kai_ptr User, Kai_string String);
extern void __wasm_write_value(Kai_ptr User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format format);
extern void __wasm_set_color(Kai_ptr User, Kai_Write_Color Color_Index);

void *memset(void *dest, int c, size_t n)
{
	unsigned char *s = dest;
	for (; n; n--, s++) *s = c;
	return dest;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;
	for (; n; n--) *d++ = *s++;
	return dest;
}

void kai__fatal_error(char const* Desc, char const* Message, int Line)
{
	__wasm_console_log(Desc, 0);
	__wasm_console_log(Message, Line);
}

static Kai_Writer debug_writer = {
	.write_string   = &__wasm_write_string,
	.write_value    = &__wasm_write_value,
	.set_color      = &__wasm_set_color,
};

#define kai__debug_writer  (&debug_writer)
#define kai__debug(...)    (void)0

static Kai_Writer div_writer = {
	.write_string   = &__wasm_write_string,
	.write_value    = &__wasm_write_value,
	.set_color      = &__wasm_set_color,
};

__attribute__((__visibility__("default")))
int compile(const char* source)
{
    Kai_Allocator allocator = {0};
    //kai_memory_create(&allocator);
	
	Kai_Error error = {0};
	Kai_Program program = {0};
    Kai_Program_Create_Info info = {
        .allocator = &allocator,
        .error = &error,
        .source = { kai_string_from_c(source) },
        .options = {},
    };
	Kai_Result result = kai_create_program(&info, &program);

    if (result != KAI_SUCCESS)
        kai_write_error(&div_writer, &error);

    kai_destroy_error(&error, &allocator);
    //kai_memory_destroy(&allocator);

    return (result != KAI_SUCCESS);
}

__attribute__((__visibility__("default")))
int create_syntax_tree(const char* source)
{
    Kai_Allocator allocator = {0};
    //kai_memory_create(&allocator);
	
    Kai_Error error = {0};
	Kai_Syntax_Tree tree = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = allocator,
		.error = &error,
		.source_code = kai_string_from_c(source),
	};
	Kai_Result result = kai_create_syntax_tree(&info, &tree);

    if (result != KAI_SUCCESS)
        kai_write_error(&div_writer, &error);
    else
        kai_write_syntax_tree(&div_writer, &tree);

    kai_destroy_error(&error, &allocator);
    kai_destroy_syntax_tree(&tree);
    //kai_memory_destroy(&allocator);

    return (result != KAI_SUCCESS);
}
