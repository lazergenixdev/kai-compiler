#include <kai/debug.h>
#include "../config.h"
#include "../token.h"
#include <stdio.h>
#include <inttypes.h>

char const* const branches[4] = {
    u8"\u2503   ",
    u8"\u2523\u2501\u2501 ",
    u8"    ",
    u8"\u2517\u2501\u2501 ",
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
        snprintf(ctx->temp, sizeof(ctx->temp), "undefined(%u)", op);
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
        snprintf(ctx->temp, sizeof(ctx->temp), "undefined(%u)", op);
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
#define explore(EXPR, IS_LAST) _explore(context, EXPR, IS_LAST)
void _explore(Tree_Traversal_Context* context, Kai_Expr p, Kai_u8 is_last) {
    Kai_Debug_String_Writer* const writer = context->writer;
    push_stack(is_last);

    _set_color(KAI_DEBUG_COLOR_DECORATION);
    Kai_int const last = context->stack_count - 1;
    for_n (context->stack_count)
        _write(branches[(get_stack(i) << 1) | (i == last)]);

    if (prefix) {
        _set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
        _write(prefix);
        _write_char(' ');
        prefix = NULL;
    }
    if (p == NULL) {
        _set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
        _write("null\n");
        _set_color(KAI_DEBUG_COLOR_PRIMARY);
        pop_stack();
        return;
    }

#define set_node(TYPE) TYPE* node = (TYPE*)p
    switch (p->id) {
        default: {
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("unknown");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" (id = ");
            _write_format("%i", p->id);
            _write_char(')');
            _write_char('\n');
        }
        break; case KAI_EXPR_IDENTIFIER: {
            set_node(Kai_Expr_Identifier);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("identifier");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" \"");
            _set_color(KAI_DEBUG_COLOR_IMPORTANT);
            _write_string(node->source_code);
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write("\"\n");
        }
        break; case KAI_EXPR_NUMBER: {
            set_node(Kai_Expr_Number);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("number");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" \"");
            _set_color(KAI_DEBUG_COLOR_IMPORTANT);
            _write_string(node->source_code);
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write("\"\n");
        }
        break; case KAI_EXPR_STRING: {
            set_node(Kai_Expr_String);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("string");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" \"");
            _set_color(KAI_DEBUG_COLOR_IMPORTANT);
            _write_string(node->source_code);
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write("\"\n");
        }
        break; case KAI_EXPR_UNARY: {
            set_node(Kai_Expr_Unary);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("unary");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" (op = ");
            _set_color(KAI_DEBUG_COLOR_IMPORTANT);
            _write(unary_operator_name(node->op));
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(")\n");

            explore(node->expr, 1);
        }
        break; case KAI_EXPR_BINARY: {
            set_node(Kai_Expr_Binary);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("binary");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" (op = ");
            _set_color(KAI_DEBUG_COLOR_IMPORTANT);
            _write(binary_operator_name(node->op));
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(")\n");

            explore(node->left, 0);
            explore(node->right, 1);
        }
        break; case KAI_EXPR_PROCEDURE_TYPE: {
            set_node(Kai_Expr_Procedure_Type);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("procedure type");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" (");
            _write_format("%u", node->param_count);
            _write(" in, ");
            _write_format("%u", node->ret_count);
            _write(" out)\n");

            Kai_int end = node->param_count + node->ret_count - 1;

            for_n(node->param_count) {
                prefix = "in ";
                explore(node->input_output[i], i == end);
            }

            for_n(node->ret_count) {
                prefix = "out";
                Kai_int idx = i + node->param_count;
                explore(node->input_output[idx], idx == end);
            }
        }
        break; case KAI_EXPR_PROCEDURE_CALL: {
            set_node(Kai_Expr_Procedure_Call);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("procedure call\n");

            prefix = "proc";
            explore(node->proc, node->arg_count == 0);

            Kai_int n = node->arg_count;
            for_n(n)
                explore(node->arguments[i], i == n - 1);
        }
        break; case KAI_EXPR_PROCEDURE: {
            set_node(Kai_Expr_Procedure);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("procedure");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" (");
            _write_format("%u", node->param_count);
            _write(" in, ");
            _write_format("%u", node->ret_count);
            _write(" out)\n");

            for_n(node->param_count) {
                prefix = "in ";
                explore(node->input_output[i].type, 0);
            }

            for_n(node->ret_count) {
                prefix = "out";
                explore(node->input_output[i+node->param_count].type, 0);
            }

            explore(node->body, 1);
        }
        break; case KAI_STMT_RETURN: {
            set_node(Kai_Stmt_Return);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("return\n");
            explore(node->expr, 1);
        }
        break; case KAI_STMT_DECLARATION: {
            set_node(Kai_Stmt_Declaration);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("declaration");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(" (name = \"");
            _set_color(KAI_DEBUG_COLOR_IMPORTANT);
            _write_string(node->name);
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write("\") ");

            if (node->flags & KAI_DECL_FLAG_CONST) _write("CONST ");

            _write_char('\n');

            if (node->type) {
                prefix = "type";
                explore(node->type, 0);
            }

            explore(node->expr, 1);
        }
        break; case KAI_STMT_ASSIGNMENT: {
            set_node(Kai_Stmt_Assignment);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("assignment\n");

            prefix = "left ";
            explore(node->left, 0);
            prefix = "right";
            explore(node->expr, 1);
        }
        break; case KAI_STMT_COMPOUND: {
            set_node(Kai_Stmt_Compound);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("compound statement\n");

            Kai_int const n = node->count;
            for_n(n) explore(node->statements[i], i == n - 1);
        }
        break; case KAI_STMT_IF: {
            set_node(Kai_Stmt_If);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("if statement\n");

            prefix = "expr";
            explore(node->expr, 0);
            explore(node->body, node->else_body == NULL);
            if (node->else_body != NULL) {
                explore(node->else_body, 1);
            }
        }
        break; case KAI_STMT_FOR: {
            set_node(Kai_Stmt_For);
            _set_color(KAI_DEBUG_COLOR_SECONDARY);
            _write("for statement");
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write("(iterator name = ");
            _set_color(KAI_DEBUG_COLOR_IMPORTANT);
            _write_string(node->iterator_name);
            _set_color(KAI_DEBUG_COLOR_PRIMARY);
            _write(")\n");

            prefix = "from";
            explore(node->from, 0);
            prefix = "to";
            explore(node->to, 0);

            explore(node->body, 1);
        }
//        break; case KAI_STMT_DEFER: {
//        }
    }

    pop_stack();
}
#undef prefix
#undef temp

void kai_debug_write_syntax_tree(Kai_Debug_String_Writer* writer, Kai_AST* ast) {
    Tree_Traversal_Context context;
    context.writer = writer;
    context.stack_count = 0;
    context.prefix = NULL;
    _write("Top Level\n");
    char temp[32];
    for_n (ast->top_level_count) {
        _write_format("%"PRIi64"\n", i);
        _explore(&context, ast->top_level_statements[i], i == (ast->top_level_count - 1));
    }
}

