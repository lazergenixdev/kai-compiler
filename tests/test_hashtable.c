#include "test.h"
#include <stdlib.h>
#include <string.h>

void* kai__create_hash_table_stride(void* Table, Kai_u32 stride)
{
    KAI__HASH_TABLE(int)* base = Table;
    base->count = 0;
    base->capacity = 100;
    base->elements = calloc(100, stride);
    return NULL;
}
void* kai__destroy_hash_table(void* Table)
{
    KAI__HASH_TABLE(int)* base = Table;
    free(base->elements);
    return NULL;
}

// http://www.cse.yorku.ca/~oz/hash.html
// " this algorithm (k=33) was first reported by dan bernstein many years ago in comp.lang.c.
// " another version of this algorithm (now favored by bernstein) uses xor:
// " hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
// " (why it works better than many other constants, prime or not) has never been adequately explained. 
extern inline Kai_u64 kai__hash_djb2(Kai_str in) {
    Kai_u64 hash = 5381;
    int c;
    for (Kai_u32 i = 0; i < in.count; ++i)
        hash = ((hash << 5) + hash) + in.data[i]; /* hash * 33 + c */
    return hash;
}

static Kai_u64 kai__hash(Kai_str in) {
    Kai_u64 out = kai__hash_djb2(in);
    //printf("hash %016llx <- \"%*s\"\n", out, in.count, in.data);
    return out;
}

#define KAI__HASH_TABLE_OCCUPIED_BIT 0x8000000000000000

#define kai__hash_table_insert(TABLE, KEY, VALUE) \
    TABLE.elements[kai__hash_table_raw_insert_key(&(TABLE), sizeof(TABLE.elements[0]), KEY)].value = VALUE

Kai_u32 kai__hash_table_raw_insert_key(void* raw_table, Kai_u64 stride, Kai_str key) {
    KAI__HASH_TABLE(int)* table = raw_table;
    Kai_u64 hash = KAI__HASH_TABLE_OCCUPIED_BIT | kai__hash(key);
    Kai_u64 start_index = hash % table->capacity;
    for (Kai_u64 i = start_index;; i = (i + 1) % table->capacity) {
        KAI__HASH_TABLE_SLOT(char)* element_header = (void*)((Kai_u8*)table->elements + stride * i);

        //printf("hash[%i] -> %016llx\n", (int)i, element_header->hash); \

        if ((element_header->hash >> 63) == 0) {
            element_header->hash = hash;
            element_header->key = key;
            return i;
        }
    }
}

void* kai__hash_table_raw_find(void* raw_table, Kai_u64 stride, Kai_str key) {
    KAI__HASH_TABLE(int)* table = raw_table;
    Kai_u64 hash = KAI__HASH_TABLE_OCCUPIED_BIT | kai__hash(key);
    Kai_u64 start_index = hash % table->capacity;
    for (Kai_u64 i = start_index;; i = (i + 1) % table->capacity) {
        KAI__HASH_TABLE_SLOT(char)* element_header = (void*)((Kai_u8*)table->elements + stride * i);

        //printf("hash[%i] -> %016llx\n", (int)i, element_header->hash);

        if ((element_header->hash >> 63) == 0) {
            return NULL;
        }
        if (element_header->hash == hash) {
            if (kai_string_equals(element_header->key, key)) {
                return &element_header->value;
            }
        }
    }
}

Kai_bool kai__hash_table_raw_get(void* raw_table, Kai_u64 stride, Kai_str key, void* out_value, Kai_u32 value_size) {
    void* ptr = kai__hash_table_raw_find(raw_table, stride, key);
    if (ptr) {
        memcpy(out_value, ptr, value_size);
    }
    return ptr != NULL;
}

#define kai__hash_table_get_str(TABLE, KEY, OUT_VALUE) \
    kai__hash_table_raw_get(&(TABLE), sizeof(TABLE.elements[0]), KAI_STRING(KEY), OUT_VALUE, sizeof(*OUT_VALUE))

int hash_table() {
    TEST();

    KAI__HASH_TABLE(Kai_u64) table;
    kai__create_hash_table(&table);

    Kai_str keys [7] = {
        KAI_STRING("apple"),
        KAI_STRING("grape"),
        KAI_STRING("orange"),
        KAI_STRING("banana"),
        KAI_STRING("strawberry"),
        KAI_STRING("blueberry"),
        KAI_STRING("mango"),
    };
    Kai_u64 values [7] = { 9, 8, 7, 6, 5, 4, 3 };

    //putchar('\n');
    for (int i = 0; i < 7; ++i) {
        kai__hash_table_insert(table, keys[i], values[i]);
    }

    int value;
    Kai_bool found;

    found = kai__hash_table_get_str(table, "banana", &value);     if (!found || value != 6) return FAIL;
    found = kai__hash_table_get_str(table, "grape", &value);      if (!found || value != 8) return FAIL;
    found = kai__hash_table_get_str(table, "blueberry", &value);  if (!found || value != 4) return FAIL;
    found = kai__hash_table_get_str(table, "orange", &value);     if (!found || value != 7) return FAIL;
    found = kai__hash_table_get_str(table, "mango", &value);      if (!found || value != 3) return FAIL;
    found = kai__hash_table_get_str(table, "strawberry", &value); if (!found || value != 5) return FAIL;
    found = kai__hash_table_get_str(table, "apple", &value);      if (!found || value != 9) return FAIL;

    kai__destroy_hash_table(&table);

    return PASS;
}