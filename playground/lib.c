// clang.exe -Wall -Wextra --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-dynamic lib.c -o lib.wasm
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
#define KAI_DONT_USE_ALLOCATOR_API
#define KAI_IMPLEMENTATION
#include "../kai.h"

WASM_IMPORT(void, write)(void* user, Kai_Write_Command command, Kai_Value value, Kai_Write_Format format);

#define hash_table_iterate(Table, Iter_Var)                               \
  for (Kai_u32 Iter_Var = 0; Iter_Var < (Table).capacity; ++Iter_Var)     \
    if ((Table).occupied[Iter_Var / 64] & ((Kai_u64)1 << (Iter_Var % 64)))

#define unused(...) (void)sizeof(__VA_ARGS__)

void* _Heap_Allocate(void* user, void* ptr, Kai_u32 new_size, Kai_u32 old_size)
{
	unused(user);

	if (new_size > old_size) {
        // NOTE: allocated memory is set to zero by default
		void* new_ptr = __env_allocate(new_size);
        memset(new_ptr, 0, new_size);
		if (ptr) {
			memcpy(new_ptr, ptr, old_size);
		}
		return new_ptr;
	}
	
	__env_free(ptr);
	return 0;
}

void* _Platform_Allocate(void* user, void* ptr, Kai_u32 size, Kai_Memory_Command op)
{
	unused(user, ptr, size, op);
    kai_fatal_error("Memory Error", "_Platform_Allocate not implemented");
	return NULL;
}

void null_write(void* user, Kai_Write_Command command, Kai_Value value, Kai_Write_Format format)
{
    unused(user, command, value, format);    
}

static Kai_Writer div_writer = {
	.write = &__env_write,
};
static Kai_Error error;
static Kai_Allocator allocator = {
	.heap_allocate = &_Heap_Allocate,
	.platform_allocate = &_Platform_Allocate,
	.page_size = WASM_PAGE_SIZE,
};
static Kai_string file_name;

WASM_EXPORT void set_file_name(Kai_string name)
{
    file_name = name;
}

// NOTE: No destroy procedures ever need to be called,
// because after each of these procedures the WASM
// memory will be reset and cleared.

WASM_EXPORT int create_syntax_tree(Kai_u8* data, Kai_u32 count)
{
	Kai_Error error = {0};
	Kai_Syntax_Tree tree = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = allocator,
		.error = &error,
		.source = {
            .name = file_name,
            .contents = (Kai_string){.count = count, .data = data}
        },
	};

	Kai_Result result = kai_create_syntax_tree(&info, &tree);
    if (result != KAI_SUCCESS) {
        kai_write_error(&div_writer, &error);
	}
    else {
        kai_write_expression(&div_writer, (Kai_Expr*)&tree.root, 0);
	}
    return (result != KAI_SUCCESS);
}

static Kai_Import imports[] = {
    {.name = KAI_STRING("print"), .type = KAI_STRING("s32"), .value = {}},
};

WASM_EXPORT int compile_show_typed_ast(Kai_u8* data, Kai_u32 count)
{
	Kai_Writer* writer = &div_writer;

	error = (Kai_Error){0};
	Kai_Program program = {0};
    Kai_Source source = { .name = file_name, .contents = (Kai_string){.count = count, .data = data} };
    Kai_Program_Create_Info info = {
        .allocator = allocator,
        .error = &error,
        .sources = { .count = 1, .data = &source },
        .imports = { .count = 1, .data = imports },
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
    };
    kai_create_program(&info, &program);

	if (error.result == KAI_SUCCESS)
    {
        for (Kai_u32 i = 0; i < program.trees.count; ++i)
        {
            kai_write_expression(writer, (Kai_Expr*)&program.trees.data[i].root, 0);
        }
    }
    else {
		kai_write_error(&div_writer, &error);
	}
	return error.result != KAI_SUCCESS;
}

WASM_EXPORT int compile_show_exports(Kai_u8* data, Kai_u32 count)
{
	Kai_Writer* writer = &div_writer;
	
	error = (Kai_Error){0};
	Kai_Program program = {0};
    Kai_Source source = { .name = file_name, .contents = (Kai_string){.count = count, .data = data} };
    Kai_Program_Create_Info info = {
        .allocator = allocator,
        .error = &error,
        .sources = { .count = 1, .data = &source },
        .imports = { .count = 1, .data = imports },
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
    };
    kai_create_program(&info, &program);

	if (error.result == KAI_SUCCESS)
    {
        hash_table_iterate(program.variable_table, i)
        {
            Kai_string name = program.variable_table.keys[i];
            Kai_Variable var = program.variable_table.values[i];
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write_string(name);
            kai__set_color(KAI_WRITE_COLOR_DEFAULT);
            kai__write(" = ");
            kai_write_value(writer, program.data.data + var.location, var.type);
            kai__set_color(KAI_WRITE_COLOR_DEFAULT);
            kai__write(" [");
            kai__set_color(KAI_WRITE_COLOR_TYPE);
            kai_write_type(writer, var.type);
            kai__set_color(KAI_WRITE_COLOR_DEFAULT);
            kai__write("]\n");
        }
    }
    else {
		kai_write_error(&div_writer, &error);
	}
	return error.result != KAI_SUCCESS;
}
