#include "test.h"

typedef KAI_HASH_TABLE(int) Table;

static Kai_u8 scratch[12*4096];
static Kai_uint scratch_offset = 0;
static Kai_string seed = KAI_STRING("aaaaaaaaaaa");
Kai_string copy(Kai_string s)
{
    if (scratch_offset + s.count > sizeof(scratch))
        FAIL("ran out of memory! %.*s", (int)s.count, s.data);
    Kai_string out = {
        .data = scratch + scratch_offset,
        .count = s.count,
    };
    kai__memory_copy(out.data, s.data, s.count);
    scratch_offset += s.count;
    return out;
}
Kai_string next(Kai_string s)
{
    int carry = 0;
    Kai_string out = copy(s);

    out.data[s.count - 1] = out.data[out.count - 1] + 1;
    for (Kai_uint i = 0; i < out.count; ++i)
    {
        Kai_uint ri = out.count - i - 1;
        if (carry) {
            out.data[ri] += 1;
            carry = 0;
        }
        if (out.data[ri] > 'z')
        {
            out.data[ri] = 'a';
            carry = 1;
        }
    }
    return out;
}

void write_table(Kai_Writer* writer, Table* table)
{
    kai__write("-> ");
    kai__write("(");
    kai__write_u32(table->count);
    kai__write(")");
    for (Kai_u32 i = 0; i < table->capacity; ++i)     \
    if (table->occupied[i/64] & ((Kai_u64)1 << (i%64)))
    {
        kai__write_string(table->keys[i]);
        kai__write("|0x");

        Kai_Write_Format fmt = {
            .fill_character = '0',
            .flags = KAI_WRITE_FLAGS_BASE_16,
            .min_count = sizeof(Kai_u64) * 2,
        };
        writer->write(writer->user, KAI_WRITE_U64, (Kai_Value){.u64 = table->hashes[i]}, fmt);
        //kai__write(": ");
        //kai__write_s32(table->values[i]);
        kai__write(", ");
    }
    else kai__write("["), kai__write_string(table->keys[i]), kai__write("], ");
    kai__write("\n");
}

int main()
{
    Kai_Allocator _allocator = default_allocator();
    Kai_Allocator* allocator = &_allocator;
    Kai_Writer* writer = default_writer();
    Kai_string test_string = copy(seed);
    Table table = {0};
    UNUSED(writer);

    for (int i = 0; i < 1024; test_string = next(test_string), ++i)
    {
        kai_table_set(&table, test_string, i);
        //write_table(writer, &table);

        for (int j = 0; j < i+1; ++j)
        {
            Kai_string key = {.count = seed.count, .data = scratch + seed.count * j};
            Kai_int k = kai_table_find(&table, key);
            if (k == -1) FAIL("on %i\n", i);
        }
    }
    return 0;
}