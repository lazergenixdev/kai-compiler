#include "test.h"

int parser() {
    TEST();

    Kai_Allocator allocator = {0};
    kai_create_memory(&allocator);

    Kai_Error error = {0};
    Kai_Syntax_Tree tree = {0};
    Kai_Syntax_Tree_Create_Info info = {
        .error = &error,
        .allocator = allocator,
        .source_code = KAI_STRING(
            "E0 :: identifier;\n"
            "E1 :: **unary;\n"
            "E2 :: a + (b + c) * (d + e) -> f;\n"
            "E3 :: 3.14e23 + 0xFF_AB__23_00 + 0b1100_1000 + \"string\";\n"
            "E4 :: base.member.member[index + index];\n"
            "E5 :: function(call(1, 2, 3), 1, 2, 3);\n"
            "E6 :: #type (A, B, C) -> (A, B);\n"
            "S0 :: () {\n"
            "\tconstant :: 1 + 2 + ok();\n"
            "\tvariable := ok();\n"
            "\tvariable = assignment + ok();\n"
            "\tif 1 + 2 { print(\"Hi\"); }\n"
            "\tfor i: 0..10 { print(\"Bye\"); }\n"
            "\t{ something(); { nested(); } }\n"
            "\tret EXPR;\n"
            "}\n"
        ),
    };

    Kai_Result result = kai_create_syntax_tree(&info, &tree);

    if (result != KAI_SUCCESS) {
        error.location.file_name = KAI_STRING(__FILE__);
        kai_debug_write_error(&error_writer, &error);
    }

    kai_destroy_syntax_tree(&tree);
    kai_destroy_memory(&allocator);

    return (result != KAI_SUCCESS) ? FAIL : PASS;
}
