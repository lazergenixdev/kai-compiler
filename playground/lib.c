// clang.exe -Wall -Wextra --target=wasm32 -D__WASM__ -nostdlib -Wl,--no-entry -Wl,--export-dynamic lib.c -o lib.wasm
#include "wasm.h"

WASM_IMPORT(void, console_log)(const char* message, int value);
WASM_IMPORT(void, panic)(const char* desc, const char* message, const char* file, int line);
WASM_IMPORT(void*, allocate)(size_t size);
WASM_IMPORT(void, free)(void* ptr);

const char* join(const char* left, const char* right)
{
    size_t lc = strlen(left);
    size_t rc = strlen(right);
    char* buffer = __env_allocate(lc + rc + 2);
    memcpy(buffer, left, lc);
    buffer[lc] = ' ';
    memcpy(buffer + lc + 1, right, rc);
    buffer[lc + rc + 1] = 0;
    return buffer;
}

#define KAI_API(T) WASM_EXPORT T
#define printf(...) (void)sizeof(__VA_ARGS__)
#define kai_fatal_error(DESC, MESSAGE) __env_panic(DESC, MESSAGE, __FILE__, __LINE__)
#define kai__todo(...) __env_panic("TODO", join(__FUNCTION__, #__VA_ARGS__), __FILE__, __LINE__)
#define KAI_DONT_USE_WRITER_API
#define KAI_DONT_USE_MEMORY_API
#define KAI_IMPLEMENTATION
#include "../kai.h"

WASM_IMPORT(void, write_string)(void* user, Kai_string String);
WASM_IMPORT(void, write_value)(void* user, Kai_u32 type, Kai_Value value, Kai_Write_Format format);
WASM_IMPORT(void, set_color)(void* user, Kai_Write_Color color_index);

#define hash_table_iterate(Table, Iter_Var)                               \
  for (Kai_u32 Iter_Var = 0; Iter_Var < (Table).capacity; ++Iter_Var)     \
    if ((Table).occupied[Iter_Var / 64] & ((Kai_u64)1 << (Iter_Var % 64)))

#define unused(...) (void)sizeof(__VA_ARGS__)


void my_write_string(void* user, Kai_string string)
{
	unused(user, string);
	__env_console_log(__FUNCTION__, 0);
}

static Kai_Writer div_writer = {
	.write_string   = &__env_write_string,
	.write_value    = &__env_write_value,
	.set_color      = &__env_set_color,
	.user           = (void*)0,
};

//static Kai_Writer error_writer = {
//	.write_string   = &__env_write_string,
//	.write_value    = &__env_write_value,
//	.set_color      = &__env_set_color,
//	.user           = (void*)1,
//};

static Kai_Error error = {0};

WASM_EXPORT int last_error_line()
{
	return error.location.line;
}

WASM_EXPORT int last_error_length()
{
	return 0;
}

void* _Heap_Allocate(void* user, void* ptr, Kai_u32 new_size, Kai_u32 old_size)
{
	unused(user);

	if (new_size > old_size) {
		void* new_ptr = __env_allocate(new_size);
		if (ptr) {
			memcpy(new_ptr, ptr, old_size);
		}
		// memset((Kai_u8*)(new_ptr) + old_size, 0, new_size - old_size);
		return new_ptr;
	}
	
	__env_free(ptr);
	return 0;
}
void* _Platform_Allocate(void* user, void* ptr, Kai_u32 size, Kai_Memory_Command op)
{
	unused(user, ptr, size, op);
	__env_console_log(__FUNCTION__, 0);
	return NULL;
}
static Kai_Allocator allocator = {
	.heap_allocate = &_Heap_Allocate,
	.platform_allocate = &_Platform_Allocate,
	.page_size = WASM_PAGE_SIZE,
};

WASM_EXPORT int create_syntax_tree(Kai_u8* data, Kai_u32 count)
{
	error = (Kai_Error){0};
	Kai_Syntax_Tree tree = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = allocator,
		.error = &error,
		.source = { .contents = (Kai_string){.count = count, .data = data} },
	};
	Kai_Result result = kai_create_syntax_tree(&info, &tree);

    if (result != KAI_SUCCESS) {
        kai_write_error(&div_writer, &error);
	}
    else {
        kai_write_expression(&div_writer, (Kai_Expr*)&tree.root, 0);
	}

    kai_destroy_error(&error, &allocator);
    kai_destroy_syntax_tree(&tree);

    return (result != KAI_SUCCESS);
}

void write_value_no_code_gen(Kai_Writer* writer, void* data, Kai_Type type)
{
    switch (type->id)
    {
    case KAI_TYPE_ID_TYPE: {
        Kai_Type type = *(Kai_Type*)data;
        kai__write(" = ");
        kai__set_color(KAI_WRITE_COLOR_TYPE);
        kai_write_type(writer, type);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
		kai__write("\n");
    } break;
    case KAI_TYPE_ID_PROCEDURE: {
        Kai_Expr* expr = *(Kai_Expr**)data;
        kai__write("\n");
        kai_write_expression(writer, expr, 1);
    } break;
    case KAI_TYPE_ID_INTEGER: {
        union { Kai_u64 u; Kai_s64 s; } value;
        Kai_Type_Info_Integer* i = (Kai_Type_Info_Integer*)type;
        if (i->is_signed)
        switch (i->bits)
        {
        case 8:  value.s = *(Kai_s8*)data;
        case 16: value.s = *(Kai_s16*)data;
        case 32: value.s = *(Kai_s32*)data;
        case 64: value.s = *(Kai_s64*)data;
        }
        else switch (i->bits)
        {
        case 8:  value.u = *(Kai_u8*)data;
        case 16: value.u = *(Kai_u16*)data;
        case 32: value.u = *(Kai_u32*)data;
        case 64: value.u = *(Kai_u64*)data;
        }
        kai__write(" = ");
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
        if (i->is_signed) kai__write_s64(value.s);
        else              kai__write_u64(value.u);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write(" [");
        kai__set_color(KAI_WRITE_COLOR_TYPE);
        kai_write_type(writer, type);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write("]\n");
    } break;
    case KAI_TYPE_ID_FLOAT: {
        Kai_Type_Info_Float* f = (Kai_Type_Info_Float*)type;
        kai__write(" = ");
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
        switch (f->bits)
        {
        case 32: kai__write_f64((Kai_f64)(*(Kai_f32*)data)); break;
        case 64: kai__write_f64(*(Kai_f64*)data); break;
        }
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write(" [");
        kai__set_color(KAI_WRITE_COLOR_TYPE);
        kai_write_type(writer, type);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write("]\n");
    } break;
    default: {
        kai__write(" [unknown value type]\n");
    } break;
    }
}

WASM_EXPORT int compile_no_code_gen(Kai_u8* data, Kai_u32 count)
{
	Kai_Writer* writer = &div_writer;
	
	error = (Kai_Error){0};
	Kai_Program program = {0};
    Kai_Source source = { .contents = (Kai_string){.count = count, .data = data} };
    Kai_Program_Create_Info info = {
        .allocator = allocator,
        .error = &error,
        .sources = { .count = 1, .data = &source },
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
    };
    kai_create_program(&info, &program);

	if (error.result == KAI_SUCCESS) {
        hash_table_iterate(program.variable_table, i)
        {
            Kai_string name = program.variable_table.keys[i];
            Kai_Variable var = program.variable_table.values[i];
			kai__write_string(name);
			write_value_no_code_gen(writer, program.data.data + var.location, var.type);
			kai__write("\n");
        }
    } else {
		kai_write_error(&div_writer, &error);
	}
	return error.result != KAI_SUCCESS;
}
