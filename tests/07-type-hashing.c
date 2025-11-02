#include "test.h"

typedef struct {
    Kai_u64  value;
    Kai_u32  line_number;
    Kai_Type type;
} Hash;

typedef KAI_SLICE(Hash) Kai_Hash_Slice;

Kai_bool binary_search(Kai_Hash_Slice array, Kai_int* out_index, Kai_u64 value)
{
    Kai_int low = 0, high = (Kai_int)(array.count) - 1;
    while (low <= high) {
        Kai_int mid = (low + high) / 2;
        Kai_u64 mv = array.data[mid].value;
        if (value > mv) {
            low = mid + 1;
        }
        else if (value < mv) {
            high = mid - 1;
        }
        else {
            *out_index = mid;
            return KAI_TRUE;
        }
    }
    *out_index = low;
    return KAI_FALSE;
}

void print_slice(Kai_Hash_Slice array)
{
    printf("[");
    for (Kai_int i = 0; i < (Kai_int)(array.count); ++i)
        printf(i+1==array.count?"%llx":"%llx, ", array.data[i].value);
    printf("]\n");
}

void slice_insert(Kai_Hash_Slice* array, Kai_int index, Hash hash)
{
    array->count += 1;
    for (Kai_int i = (Kai_int)(array->count) - 1; i > index; --i)
        array->data[i] = array->data[i-1];
    array->data[index] = hash;
}

#define write_type(TYPE) \
    default_writer()->write(0, KAI_WRITE_COLOR_TYPE, (Kai_Value){0}, (Kai_Write_Format){0}); \
    kai_write_type(default_writer(), TYPE); \
    default_writer()->write(0, KAI_WRITE_COLOR_DEFAULT, (Kai_Value){0}, (Kai_Write_Format){0});

int main()
{
    Kai_Program program = {0};
    Kai_Source sources[] = { load_source_file("scripts/types.kai") };
    Kai_Program_Create_Info info = {
        .allocator = default_allocator(),
        .error = default_error(),
        .sources = MAKE_SLICE(sources),
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
        //.debug_writer = default_writer(),
    };
    kai_create_program(&info, &program);
    assert_no_error();

    Kai_Hash_Slice hashes = {0};
    hashes.data = calloc(2048, sizeof(Hash));
    hashes.count = 0;

    Kai_Syntax_Tree* tree = &program.trees.data[0];
    Kai_Stmt* stmt = tree->root.head;
    while (stmt) {
        //write_expression(stmt);

        Kai_Type type = NULL;
        Kai_Type* value = (Kai_Type*)kai_find_variable(&program, stmt->name, &type);
        assert_true(type != NULL && type->id == KAI_TYPE_ID_TYPE);
        type = *value;

        Kai_u64 hash = kai_hash_type(type);
        Kai_int index = 0;
        Kai_bool found = binary_search(hashes, &index, hash);
        if (found) {
            printf("Found collision\n");

            printf("Type ");
            write_type(type);
            printf(" at (types.kai:%u)\n", stmt->line_number);

            printf("Type ");
            write_type(hashes.data[index].type);
            printf(" at (types.kai:%u)\n", hashes.data[index].line_number);

            FAIL("Found a duplicate hash: %016llX", hash);
        }
        slice_insert(&hashes, index, (Hash){hash, stmt->line_number, type});

        stmt = stmt->next;
    }
}