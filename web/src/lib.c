#define KAI_USE_DEBUG_API
#define KAI_USE_MEMORY_API
#include "../../include/kai/kai.h"
extern void __wasm_console_log(const char* message, int value);
extern void __wasm_write_string(Kai_ptr User, Kai_str String);
extern void __wasm_write_c_string(Kai_ptr User, char const* C_String);
extern void __wasm_write_char(Kai_ptr User, Kai_u8 Char);
extern void __wasm_set_color(Kai_ptr User, Kai_Debug_Color Color_Index);

int test(const char* source) {
    Kai_Allocator allocator = {0};
    Kai_Error error = {0};

    kai_create_memory(&allocator);

    Kai_Syntax_Tree tree = {0};
    Kai_Syntax_Tree_Create_Info info = {
        .allocator = allocator,
        .error = &error,
        .source_code = kai_str_from_cstring(source),
    };
    Kai_Result result = kai_create_syntax_tree(&info, &tree);
    //visit_ast_node((Kai_Expr)&tree.root, 0);

    Kai_Debug_String_Writer div_writer = {
        .write_string   = &__wasm_write_string,
        .write_c_string = &__wasm_write_c_string,
        .write_char     = &__wasm_write_char,
        .set_color      = &__wasm_set_color,
    };
    if (result != KAI_SUCCESS)
        kai_debug_write_error(&div_writer, &error);
    else
        kai_debug_write_syntax_tree(&div_writer, &tree);

    kai_destroy_error(&error, &allocator);
    kai_destroy_syntax_tree(&tree);
    return kai_destroy_memory(&allocator);
}
