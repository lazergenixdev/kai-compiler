#include <kai/parser.h>
#include <iostream>
#include <vector>

#define NODE  "\x1b[0m"
#define INFO  "\x1b[1;91m"
#define TREE  "\x1b[1;97m"
#define RESET "\x1b[0m"

static constexpr char const* binary_operator_name(kai_u32 op) {
    switch (op)
    {
    case '+':return "add";
    case '-':return "subtract";
    case '*':return "multiply";
    case '/':return "divide";

    case '?':return "cast";

    default:return "undefined";
    }
}

struct tree_context {
    std::vector<kai_u32> stack;
};

enum : kai_u32 {
    element,
    last_element,
};

void print_tree(tree_context& ctx, kai_Expr expr) {
    std::cout << TREE;

    int i = 0;
    auto const last = ctx.stack.size() - 1;
    for (auto& t : ctx.stack) {
        switch (t)
        {
        case element:
            if (i == last) std::cout << "├───";
            else           std::cout << "│   ";
            break;

        case last_element:
            if (i == last) std::cout << "└───";
            else           std::cout << "    ";
            break;

        default:break;
        }
        ++i;
    }

    std::cout << NODE;
    if (expr == nullptr) {
        std::cout << "null";
        std::cout << '\n';
        return;
    }

    switch (expr->id)
    {
    case kai_Expr_ID_Identifier: {
        std::cout << "identifier \"";
        std::cout << INFO;
        std::cout.write((char*)expr->source_code.data, expr->source_code.count);
        std::cout << NODE;
        std::cout << '"';
        std::cout << '\n';
        break;
    }

    case kai_Expr_ID_Number: {
        std::cout << "number \"";
        std::cout << INFO;
        std::cout.write((char*)expr->source_code.data, expr->source_code.count);
        std::cout << NODE;
        std::cout << '"';
        std::cout << '\n';
        break;
    }

    case kai_Expr_ID_String: {
        std::cout << "string \"";
        std::cout << INFO;
        std::cout.write((char*)expr->source_code.data, expr->source_code.count);
        std::cout << NODE;
        std::cout << '"';
        std::cout << '\n';
        break;
    }

    case kai_Expr_ID_Unary: {
        auto unary = (kai_Expr_Unary*)expr;

        std::cout << "unary (op = ";
        std::cout << INFO;
        std::cout << (char)unary->op;
        std::cout << NODE;
        std::cout << ")\n";

        ctx.stack.push_back(last_element);
        print_tree(ctx, unary->expr);
        ctx.stack.pop_back();

        break;
    }

    case kai_Expr_ID_Binary: {
        auto binary = (kai_Expr_Binary*)expr;

        std::cout << "binary (op = ";
        std::cout << INFO;
        std::cout << binary_operator_name(binary->op);
        std::cout << NODE;
        std::cout << ")\n";

        ctx.stack.push_back(element);
        print_tree(ctx, binary->left);
        ctx.stack.back() = last_element;
        print_tree(ctx, binary->right);
        ctx.stack.pop_back();

        break;
    }

    case kai_Expr_ID_Procedure_Call: {
        auto call = (kai_Expr_Procedure_Call*)expr;

        std::cout << "procedure call\n";

        ctx.stack.push_back(element);

        if (call->arg_count == 0)
            ctx.stack.back() = last_element;

        print_tree(ctx, call->proc);

        for (kai_u32 i = 0; i < call->arg_count; ++i) {
            if (i == call->arg_count - 1) {
                ctx.stack.back() = last_element;
            }
            print_tree(ctx, call->arguments[i]);
        }

        ctx.stack.pop_back();
        break;
    }

    case kai_Expr_ID_Procedure_Type: {
        auto proc = (kai_Expr_Procedure_Type*)expr;

        std::cout << "procedure type (" << proc->parameter_count << " input";
        if (proc->parameter_count > 1) std::cout << 's';
        std::cout << ", " << proc->ret_count << " output";
        if (proc->ret_count > 1) std::cout << 's';
        std::cout << ")\n";

        ctx.stack.push_back(element);

        kai_int total = proc->parameter_count + proc->ret_count;

        for (int i = 0; i < total; ++i) {
            if (i == total - 1) {
                ctx.stack.back() = last_element;
            }
            print_tree(ctx, proc->types[i]);
        }

        ctx.stack.pop_back();
        break;
    }

    case kai_Expr_ID_Procedure: {
        auto proc = (kai_Expr_Procedure*)expr;

        std::cout << "procedure (" << proc->param_count << " input";
        if (proc->param_count > 1) std::cout << 's';
        std::cout << ", " << proc->ret_count << " output";
        if (proc->ret_count > 1) std::cout << 's';
        std::cout << ")\n";

        ctx.stack.push_back(element);
        auto const n = proc->param_count + proc->ret_count;
        for (int i = 0; i < n; ++i) {
            print_tree(ctx, proc->input_output[i].type);
        }
        ctx.stack.pop_back();

        ctx.stack.push_back(last_element);
        print_tree(ctx, proc->body);
        ctx.stack.pop_back();
        break;
    }

    case kai_Stmt_ID_Declaration: {
        auto decl = (kai_Stmt_Declaration*)expr;

        std::cout << "declaration (name = \"";
        std::cout << INFO;
        std::cout.write((char*)decl->name.data, decl->name.count);
        std::cout << NODE;
        std::cout << "\")\n";

        ctx.stack.push_back(last_element);
        print_tree(ctx, decl->expr);
        ctx.stack.pop_back();
        break;
    }

    case kai_Stmt_ID_Return: {
        auto ret = (kai_Stmt_Return*)expr;

        std::cout << "return\n";

        ctx.stack.push_back(last_element);
        print_tree(ctx, ret->expr);
        ctx.stack.pop_back();
        break;
    }

    case kai_Stmt_ID_Compound: {
        auto comp = (kai_Stmt_Compound*)expr;

        std::cout << "compound statement\n";

        ctx.stack.push_back(element);

        auto const n = comp->count;
        for (int i = 0; i < n; ++i) {
            if (i == n - 1) {
                ctx.stack.back() = last_element;
            }
            print_tree(ctx, comp->statements[i]);
        }

        ctx.stack.pop_back();
        break;
    }

    default: {
        std::cout << "undefined";
        std::cout << '\n';
        break;
    }
    }

}

void kai_Lib_print_syntax_tree(kai_Module* mod) {
    tree_context ctx;
    print_tree(ctx, (kai_Expr)mod->AST_Root);
    std::cout << RESET;
}
