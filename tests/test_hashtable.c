#include "test.h"
#include <stdlib.h>
#include <string.h>

int hash_table(void)
{
    TEST();

    Kai_Allocator _allocator;
    kai_memory_create(&_allocator);
    Kai_Allocator* allocator = &_allocator;

    KAI__HASH_TABLE(int) table = {0};

    // Test generic usage
    {
        Kai_string keys [7] = {
            KAI_STRING("apple"),
            KAI_STRING("grape"),
            KAI_STRING("orange"),
            KAI_STRING("banana"),
            KAI_STRING("strawberry"),
            KAI_STRING("blueberry"),
            KAI_STRING("mango"),
        };
        int values [7] = { 9, 8, 7, 6, 5, 4, 3 };

        //putchar('\n');
        for (int i = 0; i < 7; ++i) {
            kai__hash_table_emplace(&table, keys[i], values[i]);
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
    }
    kai__hash_table_destroy(table);

    // Test collision insertion/removal
    {
        Kai_string s0 = KAI_STRING("\x00\x42");
        Kai_string s1 = KAI_STRING("\x01\x21");
        Kai_string s2 = KAI_STRING("\x02\x00"); // gauranteed to collide

        kai__hash_table_emplace(&table, s0, 0x45);
        kai__hash_table_emplace(&table, s1, 0x46);
        kai__hash_table_emplace(&table, s2, 0x47);

        int* find_before = kai__hash_table_find(&table, s2);
        if (find_before == NULL) return FAIL;

        int* value = kai__hash_table_find(&table, s1);
        Kai_u32 index = (int)(value - table.values) / sizeof(int);
        kai__hash_table_remove_index(table, index);
        
        int* find_after = kai__hash_table_find(&table, s2);
        int* find_s0 = kai__hash_table_find(&table, s0);
        int* find_s1 = kai__hash_table_find(&table, s1);

        if (find_after == NULL) return FAIL;
        if (*find_after != 0x47) return FAIL;
        if (find_s0 == NULL) return FAIL;
        if (*find_s0 != 0x45) return FAIL;
        if (find_s1 != NULL) return FAIL;
    }
    kai__hash_table_destroy(table);

    if (kai_memory_destroy(&_allocator)) return FAIL;

    return PASS;
}
