#include "test.h"

const char* cstr(Kai_string s)
{
    static char buf[4096];
    kai__memory_copy(buf, s.data, s.count);
    buf[s.count] = 0;
    return buf;
}

int main()
{
    Kai_Program program = {0};
    Kai_Source sources[] = { load_source_file("scripts/all-types.kai") };
    Kai_Program_Create_Info info = {
        .allocator = default_allocator(),
        .error = default_error(),
        .sources = MAKE_SLICE(sources),
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
        //.debug_writer = default_writer(),
    };
    kai_create_program(&info, &program);
    assert_no_error();

    Kai_u32 non_match_count = 0;
    Kai_Syntax_Tree* tree = &program.trees.data[0];
    Kai_Stmt* stmt = tree->root.head;
    while (stmt) {
        //write_expression(stmt);

        Kai_Type type = NULL;
        Kai_Type* value = (Kai_Type*)kai_find_variable(&program, stmt->name, &type);
        assert_true(type != NULL && type->id == KAI_TYPE_ID_TYPE);
        type = *value;

        assert_true(stmt->tag != NULL);
        assert_true(stmt->tag->expr != NULL);
        assert_true(stmt->tag->expr->id == KAI_EXPR_NUMBER);

        Kai_u64 expected = kai_number_to_u64(((Kai_Expr_Number*)(stmt->tag->expr))->value);
        Kai_u64 got = kai_hash_type(type);

        if (got != expected)
        {
            nob_log(ERROR, "expected %016llX but got %016llX for \"%s\"", expected, got, cstr(stmt->name));
            non_match_count += 1;
        }

        stmt = stmt->next;
    }

    if (non_match_count)
        FAIL("%i tests did not match", non_match_count);
}