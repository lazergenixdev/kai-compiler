#include "test.h"
#include <stdlib.h>
#include <string.h>

int hash_table() {
    TEST();

    Kai_Allocator _allocator;
    kai_memory_create(&_allocator);
    Kai_Allocator* allocator = &_allocator;

    KAI__HASH_TABLE(Kai_u64) table = {0};
    //kai__hash_table_create(&table);

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
        kai__hash_table_emplace(table, keys[i], values[i]);
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

    kai__hash_table_destroy(table);
    kai_memory_destroy(&_allocator);

    return PASS;
}