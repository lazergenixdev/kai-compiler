#define KAI_USE_DEBUG_API
#include "../config.h"

static char const* const kai__result_string_map[KAI_RESULT_COUNT] = {
    [KAI_SUCCESS]         = "Success",
    [KAI_ERROR_SYNTAX]    = "Syntax Error",
    [KAI_ERROR_SEMANTIC]  = "Semantic Error",
    [KAI_ERROR_INFO]      = "Info",
    [KAI_ERROR_FATAL]     = "Fatal Error",
    [KAI_ERROR_INTERNAL]  = "Internal Error",
};

int kai__base10_digit_count(Kai_int x) {
    if (x <         10) return 1;
    if (x <        100) return 2;
    if (x <       1000) return 3;
    if (x <      10000) return 4;
    if (x <     100000) return 5;
    if (x <    1000000) return 6;
    if (x <   10000000) return 7;
    if (x <  100000000) return 8;
    if (x < 1000000000) return 9;
    return 0;
}

Kai_u8 const* kai__advance_to_line(Kai_u8 const* source, Kai_int line) {
    --line;
    while (line > 0) {
        if (*source++ == '\n') --line;
    }
    return source;
}

void kai__write_source_code(Kai_Debug_String_Writer* writer, Kai_u8 const* src) {
    while (*src != 0 && *src != '\n') {
        if (*src == '\t')
            kai__write_char(' ');
        else
            kai__write_char(*src);
        ++src;
    }
}

void kai__write_source_code_count(Kai_Debug_String_Writer* writer, Kai_u8 const* src, Kai_int count) {
    while (*src != 0 && *src != '\n' && count != 0) {
        kai__write_char(' ');
        ++src;
        --count;
    }
}

void kai__write_base10(Kai_Debug_String_Writer* writer, Kai_u32 value) {
    if (value == 0) {
        kai__write_char('0');
        return;
    }
    Kai_u32 exp = 1000000000;
    Kai_u32 d = 0;
    for (;;) {
        d = value / exp;
        if (d == 0)
            exp /= 10;
        else
            break;
    }
    while (exp != 0) {
        Kai_u8 digit = (Kai_u8)(value / exp);
        kai__write_char('0' + digit);
        value -= digit * exp;
        exp /= 10;
    }
}


void kai_debug_write_error(Kai_Debug_String_Writer* writer, Kai_Error* error) {
write_error_message:
    if (error->result >= KAI_RESULT_COUNT) {
        kai__write("[Invalid result value]\n");
        return;
    }
    else if (error->result == KAI_SUCCESS) {
        kai__write("[Success]\n");
        return;
    }

    // ------------------------- Write Error Message --------------------------

    kai__set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
    kai__write_string(error->location.file_name);
    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
#if KAI_SHOW_LINE_NUMBER_WITH_FILE
    kai__write_char(':');
    kai__set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
    kai__write_format("%u", error->location.line);
    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
#endif
    kai__write(" --> ");
    if (error->result != KAI_ERROR_INFO) kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
    kai__write(kai__result_string_map[error->result]);
    if (error->result != KAI_ERROR_INFO) kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
    kai__write(": ");
    kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
    kai__write_string(error->message);
    kai__write_char('\n');

    // -------------------------- Write Source Code ---------------------------

    if (error->result == KAI_ERROR_FATAL || error->result == KAI_ERROR_INTERNAL)
        goto repeat;

    kai__set_color(KAI_DEBUG_COLOR_DECORATION);
    Kai_int digits = kai__base10_digit_count(error->location.line);
    for_n(digits) kai__write_char(' ');
    kai__write("  |\n");

    //kai__write_format(" %" PRIu32, error->location.line);
    kai__write_char(' ');
    kai__write_base10(writer, error->location.line);
    kai__write(" | ");

    Kai_u8 const* begin = kai__advance_to_line(error->location.source, error->location.line);

    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
    kai__write_source_code(writer, begin);
    kai__write_char('\n');

    kai__set_color(KAI_DEBUG_COLOR_DECORATION);
    for_n(digits) kai__write_char(' ');
    kai__write("  | ");

    kai__write_source_code_count(writer, begin,
        (Kai_int)(error->location.string.data - begin)
    );

    kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
    kai__write_char('^');
    Kai_int n = (Kai_int)error->location.string.count - 1;
    for_n(n) kai__write_char('~');

    kai__write_char(' ');
    kai__write_string(error->context);

    kai__write_char('\n');
    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);

    // -------------------------------- Repeat --------------------------------

repeat: 
    if (error->next) {
        error = error->next;
        goto write_error_message;
    }
}

// ****************************************************************************

void kai_debug_write_type(Kai_Debug_String_Writer* writer, Kai_Type Type)
{
    void* void_Type = Type;
    char temp[64];
    if (Type == NULL) {
        kai__write("[null]");
        return;
    }
    switch (Type->type) {
        default:                 { kai__write("[Unknown]"); } break;
        case KAI_TYPE_TYPE:      { kai__write("Type");      } break;
        case KAI_TYPE_INTEGER: {
            Kai_Type_Info_Integer* info = void_Type;
            kai__write_format("%s%i", info->is_signed? "s":"u", info->bits);
        } break;
        case KAI_TYPE_FLOAT: {
            Kai_Type_Info_Float* info = void_Type;
            kai__write_format("f%i", info->bits);
        } break;
        case KAI_TYPE_POINTER:   { kai__write("Pointer");   } break;   
        case KAI_TYPE_PROCEDURE: {
            Kai_Type_Info_Procedure* info = void_Type;
            kai__write("(");
            for (int i = 0;;) {
                kai_debug_write_type(writer, info->sub_types[i]);
                if (++i < info->in_count)
                {
                    kai__write(", ");
                }
                else break;
            }
            kai__write(") -> (");
            for (int i = 0;;) {
                kai_debug_write_type(writer, info->sub_types[info->in_count+i]);
                if (++i < info->out_count)
                {
                    kai__write(", ");
                }
                else break;
            }
            kai__write(")");
        } break;
        case KAI_TYPE_SLICE:     { kai__write("Slice");     } break; 
        case KAI_TYPE_STRING:    { kai__write("String");    } break;  
        case KAI_TYPE_STRUCT:    { kai__write("Struct");    } break;  
    }
}

// ****************************************************************************

char const* const branches[4] = {
    KAI_UTF8("\u2503   "),
    KAI_UTF8("\u2523\u2501\u2501 "),
    KAI_UTF8("    "),
    KAI_UTF8("\u2517\u2501\u2501 "),
};

typedef struct { 
    Kai_Debug_String_Writer* writer;
    Kai_u64 stack[256]; // 64 * 256 max depth
    Kai_u32 stack_count;
    char const* prefix;
    char temp[32];
} Tree_Traversal_Context;

#define binary_operator_name(OP) _binary_operator_name(context, OP)
char const* _binary_operator_name(Tree_Traversal_Context* ctx, Kai_u32 op) {
    switch (op) {
    case '->': return "cast";
    case '&&': return "and";
    case '||': return "or";
    case '==': return "equals";
    case '<=': return "less or equal";
    case '>=': return "greater or equal";
    case '+':  return "add";
    case '-':  return "subtract";
    case '*':  return "multiply";
    case '/':  return "divide";
    case '.':  return "member access";
    case '[':  return "index";
    default:
        //snprintf(ctx->temp, sizeof(ctx->temp), "undefined(%u)", op);
        return ctx->temp;
    }
}

#define unary_operator_name(OP) _unary_operator_name(context, OP)
char const* _unary_operator_name(Tree_Traversal_Context* ctx, Kai_u32 op) {
    switch (op) {
    case '-':  return "negate";
    case '*':  return "pointer to";
    case '[':  return "array";
    default:
        //snprintf(ctx->temp, sizeof(ctx->temp), "undefined(%u)", op);
        return ctx->temp;
    }
}

#define push_stack(B) \
    do { Kai_u32 i = context->stack_count++; \
         if (i >= sizeof(context->stack)*8) panic_with_message("stack overflow?"); \
         Kai_u64 mask = 1llu << (i%64); \
         if (B) context->stack[i/64] |= mask; \
         else   context->stack[i/64] &=~ mask; \
    } while (0)
#define pop_stack() \
    if (context->stack_count == 0) panic_with_message("cannot pop empty stack"); \
    else context->stack_count -= 1
#define get_stack(I) \
    ( (context->stack[I/64] >> I%64) & 1 )

#define prefix context->prefix
#define temp context->temp
#define explore(EXPR, IS_LAST) _explore(context, EXPR, IS_LAST, KAI_TRUE)
void _explore(Tree_Traversal_Context* context, Kai_Expr p, Kai_u8 is_last, Kai_bool push) {
    Kai_Debug_String_Writer* const writer = context->writer;

    if (push) push_stack(is_last);

    kai__set_color(KAI_DEBUG_COLOR_DECORATION);
    Kai_int const last = context->stack_count - 1;
    for_n (context->stack_count)
        kai__write(branches[(get_stack(i) << 1) | (i == last)]);

    if (prefix) {
        kai__set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
        kai__write(prefix);
        kai__write_char(' ');
        prefix = NULL;
    }
    if (p == NULL) {
        kai__set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
        kai__write("null\n");
        kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
        pop_stack();
        return;
    }

#define set_node(TYPE) TYPE* node = (TYPE*)p
    switch (p->id) {
        default: {
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("unknown");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" (id = ");
            kai__write_format("%i", p->id);
            kai__write_char(')');
            kai__write_char('\n');
        }
        break; case KAI_EXPR_IDENTIFIER: {
            set_node(Kai_Expr_Identifier);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("identifier");
            if (node->name.count != 0)
            {
                kai__write(" (");
                kai__write_string(node->name);
                kai__write_char(')');
            }
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" \"");
            kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
            kai__write_string(node->source_code);
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write("\"\n");
        }
        break; case KAI_EXPR_NUMBER: {
            set_node(Kai_Expr_Number);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("number");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" \"");
            kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
            kai__write_string(node->source_code);
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write("\"");
            kai__write_format(" whole: %llu, frac: %llu, den: %hu",
				node->value.Whole_Part, node->value.Frac_Part, node->value.Frac_Denom);
            kai__write("\n");
        }
        break; case KAI_EXPR_STRING: {
            set_node(Kai_Expr_String);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("string");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" \"");
            kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
            kai__write_string(node->source_code);
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write("\"\n");
        }
        break; case KAI_EXPR_UNARY: {
            set_node(Kai_Expr_Unary);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("unary");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" (op = ");
            kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
            kai__write(unary_operator_name(node->op));
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(")\n");

            explore(node->expr, 1);
        }
        break; case KAI_EXPR_BINARY: {
            set_node(Kai_Expr_Binary);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("binary");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" (op = ");
            kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
            kai__write(binary_operator_name(node->op));
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(")\n");

            explore(node->left,  0);
            explore(node->right, 1);
        }
        break; case KAI_EXPR_PROCEDURE_TYPE: {
            set_node(Kai_Expr_Procedure_Type);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("procedure type");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" (");
            kai__write_format("%u", node->in_count);
            kai__write(" in, ");
            kai__write_format("%u", node->out_count);
            kai__write(" out)\n");

            Kai_u32 end = node->in_count + node->out_count - 1;
            Kai_Expr current = node->in_out_expr;

            for_n (node->in_count) {
                prefix = "in ";
                explore(current, i == end);
                current = current->next;
            }

            for_n (node->out_count) {
                prefix = "out";
                Kai_int idx = i + node->in_count;
                explore(current, idx == end);
                current = current->next;
            }
        }
        break; case KAI_EXPR_PROCEDURE_CALL: {
            set_node(Kai_Expr_Procedure_Call);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("procedure call\n");

            prefix = "proc";
            explore(node->proc, node->arg_count == 0);

            Kai_int n = node->arg_count;
            Kai_Expr current = node->arg_head;
            for_n(n) {
                explore(current, i == n - 1);
                current = current->next;
            }
        }
        break; case KAI_EXPR_PROCEDURE: {
            set_node(Kai_Expr_Procedure);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("procedure");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" (");
            kai__write_format("%u", node->in_count);
            kai__write(" in, ");
            kai__write_format("%u", node->out_count);
            kai__write(" out)\n");

            Kai_Expr current = node->in_out_expr;

            for_n(node->in_count) {
                prefix = "in ";
                explore(current, 0);
                current = current->next;
            }

            for_n(node->out_count) {
                prefix = "out";
                explore(current, 0);
                current = current->next;
            }

            explore(node->body, 1);
        }
        break; case KAI_STMT_RETURN: {
            set_node(Kai_Stmt_Return);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("return\n");
            explore(node->expr, 1);
        }
        break; case KAI_STMT_DECLARATION: {
            set_node(Kai_Stmt_Declaration);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("declaration");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(" (name = \"");
            kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
            kai__write_string(node->name);
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write("\") ");

            if (node->flags & KAI_DECL_FLAG_CONST) kai__write("CONST ");

            kai__write_char('\n');

            if (node->type) {
                prefix = "type";
                explore(node->type, 0);
            }

            explore(node->expr, 1);
        }
        break; case KAI_STMT_ASSIGNMENT: {
            set_node(Kai_Stmt_Assignment);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("assignment\n");

            prefix = "left ";
            explore(node->left, 0);
            prefix = "right";
            explore(node->expr, 1);
        }
        break; case KAI_STMT_COMPOUND: {
            set_node(Kai_Stmt_Compound);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("compound statement\n");

            Kai_Stmt current = node->head;
            while (current) {
                explore(current, current->next == NULL);
                current = current->next;
            }
        }
        break; case KAI_STMT_IF: {
            set_node(Kai_Stmt_If);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("if statement\n");

            prefix = "expr";
            explore(node->expr, 0);
            explore(node->body, node->else_body == NULL);
            if (node->else_body != NULL) {
                explore(node->else_body, 1);
            }
        }
        break; case KAI_STMT_FOR: {
            set_node(Kai_Stmt_For);
            kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
            kai__write("for statement");
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write("(iterator name = ");
            kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
            kai__write_string(node->iterator_name);
            kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
            kai__write(")\n");

            prefix = "from";
            explore(node->from, 0);

            prefix = "to";
            explore(node->to, 0);

            explore(node->body, 1);
        }
    }

    if (push) { pop_stack(); }
}
#undef prefix
#undef temp

void
kai_debug_write_syntax_tree(Kai_Debug_String_Writer* writer, Kai_Syntax_Tree* tree) {
    Tree_Traversal_Context context = {
        .prefix = NULL,
        .stack_count = 0,
        .writer = writer,
    };
    kai__write("Top Level\n");
    _explore(&context, (Kai_Stmt)&tree->root, 1, KAI_TRUE);
}

void
kai_debug_write_expression(Kai_Debug_String_Writer* writer, Kai_Expr expr) {
    Tree_Traversal_Context context = {
        .prefix = NULL,
        .stack_count = 0,
        .writer = writer,
    };
    
    _explore(&context, expr, KAI_TRUE, KAI_FALSE);
}

// ****************************************************************************

#if !defined(KAI__PLATFORM_WASM)
#include <stdio.h>
#include <locale.h>

char const* kai__term_debug_colors [KAI_DEBUG_COLOR_COUNT] = {
    [KAI_DEBUG_COLOR_PRIMARY]     = "\x1b[0;37m",
    [KAI_DEBUG_COLOR_SECONDARY]   = "\x1b[1;97m",
    [KAI_DEBUG_COLOR_IMPORTANT]   = "\x1b[1;91m",
    [KAI_DEBUG_COLOR_IMPORTANT_2] = "\x1b[1;94m",
    [KAI_DEBUG_COLOR_DECORATION]  = "\x1b[0;90m",
};

void kai__stdout_writer_write_string(Kai_ptr user, Kai_str string) {
    kai__unused(user);
    fwrite(string.data, 1, string.count, stdout);
}
void kai__stdout_writer_write_c_string(Kai_ptr user, char const* string) {
    kai__unused(user);
    printf("%s", string);
}
void kai__stdout_writer_write_char(Kai_ptr user, Kai_u8 c) {
    kai__unused(user);
    putchar(c);
}
void kai__stdout_writer_set_color(Kai_ptr user, Kai_Debug_Color color) {
    kai__unused(user);
    printf("%s", kai__term_debug_colors[color]);
}

#if defined(KAI__PLATFORM_WINDOWS)
int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);
#endif

Kai_Debug_String_Writer* kai_debug_stdout_writer(void)
{
#if defined(KAI__PLATFORM_WINDOWS)
	SetConsoleOutputCP(65001);
#endif
    setlocale(LC_CTYPE, ".UTF8");
    static Kai_Debug_String_Writer writer = {
        .write_string   = kai__stdout_writer_write_string,
        .write_c_string = kai__stdout_writer_write_c_string,
        .write_char     = kai__stdout_writer_write_char,
        .set_color      = kai__stdout_writer_set_color,
        .user           = NULL
    };
    return &writer;
}

void kai__file_writer_write_string(Kai_ptr user, Kai_str string) {
    if (user == NULL) return;
    fwrite(string.data, 1, string.count, user);
}
void kai__file_writer_write_c_string(Kai_ptr user, char const* string) {
    if (user == NULL) return;
    fprintf(user, "%s", string);
}
void kai__file_writer_write_char(Kai_ptr user, Kai_u8 c) {
    if (user == NULL) return;
    fputc(c, user);
}
void kai__file_writer_set_color(Kai_ptr user, Kai_Debug_Color color) {
    kai__unused(user);
    kai__unused(color);
}

#if defined(KAI__COMPILER_MSVC)
static inline FILE* stdc_file_open(char const* path, char const* mode) {
    FILE* handle = NULL;
    fopen_s(&handle, path, mode); // this is somehow more safe? :/
    return (void*)handle;
}
#else
#    define stdc_file_open fopen
#endif

void kai_debug_open_file_writer(Kai_Debug_String_Writer* writer, const char* path)
{
    *writer = (Kai_Debug_String_Writer) {
        .write_string   = kai__file_writer_write_string,
        .write_c_string = kai__file_writer_write_c_string,
        .write_char     = kai__file_writer_write_char,
        .set_color      = kai__file_writer_set_color,
        .user           = stdc_file_open(path, "wb"),
    };
}

void kai_debug_close_file_writer(Kai_Debug_String_Writer* writer)
{
    if (writer->user != NULL)
        fclose(writer->user);
}
#endif
