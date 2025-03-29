#define KAI_USE_DEBUG_API
#include "../config.h"
#include "../token.h"
#include <stdio.h>
#include <inttypes.h>

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
            kai__write_format(" whole: %llu, frac: %llu, den: %hu", node->info.Whole_Part, node->info.Frac_Part, node->info.Frac_Denom);
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

