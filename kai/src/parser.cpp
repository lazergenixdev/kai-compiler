// TODO: Implement Casting "cast(int) num"
// TODO: Long jump on errors ?

#include "parser.hpp"
#include <cassert>
#define DEFAULT_PREC -6942069

//#define DEBUG_LEXER
#define DEBUG_PARSER

// stack for parser
// TODO: test this:
#define check_temp_stack_overflow(CTX) if (CTX.stack >= (kai_u8*)CTX.memory.temperary + CTX.memory.temperary_size) \
                               { return ctx.error("ran out of temperary memory!"); } else

kai_Expr Parse_Expression(Parser_Context& ctx, int prec = DEFAULT_PREC);

kai_Expr Parse_Type(Parser_Context& ctx) {
    auto& tok = ctx.lexer.currentToken;

    if (tok.type == '(') {
        ctx.lexer.next_token(); // get next after parenthesis

        // use temperary memory to store the types we parse.
        auto types = reinterpret_cast<kai_Expr*>(ctx.stack);
        kai_u32 count = 0;

        // Parse Procedure input types
        while (tok.type != ')') {
            ctx.stack = types + count;
            check_temp_stack_overflow(ctx);

            auto type = Parse_Type(ctx);
            if (!type) return nullptr;

            types[count++] = type;

            ctx.lexer.next_token(); // get ',' token or ')'
            switch (tok.type)
            {
            case ',': {
                ctx.lexer.next_token(); // eat ',' (get token after comma)
                break;
            }
            case ')': goto doneParameters;

            default: return ctx.error_expected("',' or ')' in procedure parameter types");
            }
        }
    doneParameters:
        auto param_count = count;
        auto& peek = ctx.lexer.peek_token(); // see if we have any returns

        if (peek.type == '->') {
            ctx.lexer.next_token(); // eat '->'
            ctx.lexer.next_token(); // get token after

            bool enclosed_return = false;
            if (tok.type == '(') {
                enclosed_return = true;
                ctx.lexer.next_token();
            }

            while (1) {
                ctx.stack = types + count;
                check_temp_stack_overflow(ctx);
                
                auto type = Parse_Type(ctx);
                if (!type) {
                    if((count-param_count) == 0)
                        return nullptr;
                    break;
                }

                types[count++] = type;

                ctx.lexer.peek_token(); // get ',' token or something else

                if (peek.type == ',') {
                    ctx.lexer.next_token(); // eat ','
                    ctx.lexer.next_token(); // get token after
                }
                else break;
            }

            if (enclosed_return) {
                if (peek.type != ')') {
                    return ctx.error_expected("')' after return types");
                }
                else ctx.lexer.next_token();
            }
        }
        auto proc = ctx.alloc_expr<kai_Expr_Procedure_Type>();
        proc->parameter_count = param_count;
        proc->ret_count = count - param_count;
        proc->line_number;
        proc->source_code;

        // allocate the space we need for how many expressions we have:
        proc->types = (kai_Expr*)ctx.memory.alloc(ctx.memory.user, count * sizeof(kai_Expr));

        // copy expressions from temperary storage:
        memcpy(proc->types, types, count * sizeof(kai_Expr));
        // reset stack
        ctx.stack = types;

        return(kai_Expr) proc;
    }
    else {
        return Parse_Expression(ctx);
    }
}

enum class op_type {
    binary,
    index,
    procedure_call,
};

struct operator_info {
    kai_u32 op;
    int     prec;
    op_type type = op_type::binary;
};

#define CAST_PREC     0x0900

operator_info get_operator(token_type t) {
    switch (t)
    {
    case '+': return {t, 0x0100};
    case '-': return {t, 0x0100};
    case '*': return {t, 0x0200};
    case '/': return {t, 0x0200};
    case '?': return {t, CAST_PREC};
    case '[': return {t, 0xBEEF, op_type::index};
    case '(': return {t, 0xBEEF, op_type::procedure_call};
    case '.': return {t, 0xFFFF}; // member access
    default:  return {t, 0};
    }
}
int unary_operator_prec(token_type t) {
    switch (t)
    {
    case '-': return 0x1000;
    case '*': return 0x1000;
    default:  return 0;
    }
}

// single token expressions, looks at current token and makes it an expression.
kai_Expr Parse_Simple_Expression(Parser_Context& ctx) {
    auto& tok = ctx.lexer.currentToken;
    switch (tok.type)
    {
    case token_identifier: {
        auto ident = ctx.alloc_expr<kai_Expr_Identifier>();
        ident->line_number;
        ident->source_code = tok.string;
        return(kai_Expr) ident;
    }
    case token_number: {
        auto num = ctx.alloc_expr<kai_Expr_Number>();
        num->info = tok.number;
        num->line_number;
        num->source_code = tok.string;
        return(kai_Expr) num;
    }
    case token_string: {
        auto str = ctx.alloc_expr<kai_Expr_String>();
        str->line_number;
        str->source_code = tok.string;
        return(kai_Expr) str;
    }
    default:return nullptr;
    }
}

kai_Expr Parse_Expression(Parser_Context& ctx, int prec) {
    auto& tok = ctx.lexer.currentToken;
    kai_Expr left = nullptr;

    // @TODO: turn this else if chain into a switch statement

    ////////////////////////////////////////////////////////////
    // Handle Parenthesis around an expression
    if (tok.type == '(') {
        ctx.lexer.next_token(); // see whats after paren
        left = Parse_Expression(ctx); // parse whatever the hell we get

        // advance to closing parenthesis hopefully
        ctx.lexer.next_token();

        // when opening a paranthesis, you kinda need to close it
        if (tok.type != ')') {
            return ctx.error_expected("an operator or ')' in expression");
        }
    }
    ////////////////////////////////////////////////////////////
    // Handle Unary Operators
    else if (auto unary_prec = unary_operator_prec(tok.type)) {
        auto op = tok.type;

        ctx.lexer.next_token();
        left = Parse_Expression(ctx, unary_prec);
        if (!left) return nullptr;

        auto unary = ctx.alloc_expr<kai_Expr_Unary>();
        unary->expr = left;
        unary->op   = op;
        left = (kai_Expr) unary;
    }
    ////////////////////////////////////////////////////////////
    // Handle Explicit Casting "cast(int) x"
    else if (tok.type == token_kw_cast) {
        ctx.lexer.next_token();

        if (tok.type != '(') return ctx.error_expected("'(' after cast keyword");

        ctx.lexer.next_token();
        left = Parse_Type(ctx);
        if (!left) return ctx.error_expected("Type");

        ctx.lexer.next_token();
        if (tok.type != ')') return ctx.error_expected("')' after Type in cast expression");

        ctx.lexer.next_token();
        auto right = Parse_Expression(ctx, CAST_PREC);
        if (!right) return ctx.error_expected("expression after cast");

        auto binary = ctx.alloc_expr<kai_Expr_Binary>();
        binary->op = '?';
        binary->left  = left;
        binary->right = right;
        binary->line_number;
        binary->source_code;
        left = (kai_Expr) binary;
    }
    ////////////////////////////////////////////////////////////
    // Handle Types
    else if (tok.type == token_directive && kai_string_equals(kai_static_string("type"), tok.string)) {
        ctx.lexer.next_token(); // see what is next
        left = Parse_Type(ctx);
        if (!left) return ctx.error_expected("Type");
    }
    ////////////////////////////////////////////////////////////
    // Default: just parse a single-token expression
    else {
        left = Parse_Simple_Expression(ctx);
        if (!left) return nullptr; // no expression :(
    }

loopBack:
    auto& peek = ctx.lexer.peek_token();
    auto op_info = get_operator(peek.type);

    if (!op_info.prec || op_info.prec < prec)
        return left;

    ctx.lexer.next_token(); // eat operator token
    ctx.lexer.next_token(); // get next token

    switch (op_info.type)
    {
    case op_type::binary: {
        auto right = Parse_Expression(ctx, op_info.prec);

        if (!right) return ctx.error_expected("expression after binary operator");
        
        auto binary = ctx.alloc_expr<kai_Expr_Binary>();
        binary->op    = op_info.op;
        binary->left  = left;
        binary->right = right;
        binary->line_number;
        binary->source_code;
        left = (kai_Expr) binary;
        break;
    }

    // special case binary expression
    case op_type::index: {
        auto right = Parse_Expression(ctx);
        if (!right) return ctx.error_expected("expression inside index operator");

        ctx.lexer.next_token(); // hopefully get a closing bracket

        if (tok.type != ']') {
            return ctx.error_expected("']' after expression");
        }

        auto binary = ctx.alloc_expr<kai_Expr_Binary>();
        binary->op    = op_info.op;
        binary->left  = left;
        binary->right = right;
        binary->line_number;
        binary->source_code;
        left = (kai_Expr) binary;
        break;
    }

    case op_type::procedure_call: {
        // use temperary memory to store the expressions we parse.
        auto expressions = reinterpret_cast<kai_Expr*>(ctx.stack);
        kai_u32 count = 0;

        if (tok.type != ')')
        {
        parseArgument:
            ctx.stack = expressions + count;
            check_temp_stack_overflow(ctx);
            auto expr = Parse_Expression(ctx);

            if (!expr) {return ctx.error_expected("an expression in procedure call"); }

            expressions[count++] = expr;

            ctx.lexer.next_token(); // get ',' token or ')'

            switch (tok.type)
            {
            case ',': {
                ctx.lexer.next_token(); // eat ',' (get token after comma)
                goto parseArgument;
            }
            case ')': break;

            default: return ctx.error_unexpected("in procedure call", "',' or ')' expected here");
            }
        }

        auto call = ctx.alloc_expr<kai_Expr_Procedure_Call>();
        call->proc = left;

        // allocate the space we need for how many expressions we have:
        call->arguments = (kai_Expr*)ctx.memory.alloc(ctx.memory.user, count * sizeof(kai_Expr));
        call->arg_count = count;

        // copy expressions from temperary storage:
        memcpy(call->arguments, expressions, count * sizeof(kai_Expr));
        // reset stack
        ctx.stack = expressions;

        left = (kai_Expr) call;
        break;
    }

    default: assert(false);
    }

    goto loopBack;
}

kai_Stmt Parse_Statement(Parser_Context& ctx) {
    auto& tok = ctx.lexer.currentToken;

    switch (tok.type)
    {
    // This is probably just an expression
    default: {
    parseExpression:
        auto expr = Parse_Expression(ctx);
        if (!expr) return ctx.error_expected("an expression");
    
        ctx.lexer.next_token();

        if (tok.type != ';') {
            return ctx.error_expected("';' after expression");
        }

        return expr;
    }

    case token_end: return nullptr;

    case '{': {
        ctx.lexer.next_token();

        // use temperary memory to store the statements we parse.
        auto statements = reinterpret_cast<kai_Expr*>(ctx.stack);
        kai_int count = 0;

        // parse statements until we get a '}'
        if (tok.type != '}')
        {
        parseStatement:
            ctx.stack = statements + count;
            check_temp_stack_overflow(ctx);

            auto stmt = Parse_Statement(ctx);
            if (!stmt) return nullptr;
            
            statements[count++] = stmt;
            ctx.lexer.next_token(); // eat ';' (get token after)

            switch (tok.type)
            {
            case '}': break;
            default: goto parseStatement;
            }
        }

        auto comp = ctx.alloc_expr<kai_Stmt_Compound>();
        // allocate the space we need for how many expressions we have:
        comp->statements = (kai_Stmt*)ctx.memory.alloc(ctx.memory.user, count * sizeof(kai_Stmt));
        comp->count = count;

        // copy expressions from temperary storage:
        memcpy(comp->statements, statements, count * sizeof(kai_Stmt));
        // reset stack
        ctx.stack = statements;

        return(kai_Stmt) comp;
    }

    case token_kw_ret: {
        ctx.lexer.next_token(); // whats after ret?

        auto expr = Parse_Expression(ctx);
        if (!expr) return ctx.error_expected("expression");

        ctx.lexer.next_token(); // whats after expression?

        if (tok.type != ';') {
            return ctx.error_unexpected("after statement", "there should be a ';' before this");
        }

        auto ret = ctx.alloc_expr<kai_Stmt_Return>();
        ret->expr = expr;
        ret->line_number;
        ret->source_code;
        return(kai_Stmt) ret;
    }

    case token_identifier: {
        auto& peek = ctx.lexer.peek_token();

        // just an expression?
        if (peek.type != ':') goto parseExpression;

        auto name = tok.string;
        ctx.lexer.next_token(); // eat ':'
        ctx.lexer.next_token();

        if (tok.type != '=') {
            return ctx.error_expected("'=' in variable declaration");
        }

        ctx.lexer.next_token();

        auto expr = Parse_Expression(ctx);
        if (!expr) return ctx.error_expected("expression");

        ctx.lexer.next_token(); // what is after?

        if (tok.type != ';') {
            return ctx.error_unexpected("after statement", "there should be a ';' before this");
        }

        auto decl = ctx.alloc_expr<kai_Stmt_Declaration>();
        decl->expr = expr;
        decl->name = name;
        decl->line_number;

        return(kai_Stmt) decl;
        break;
    }

    }
}

// @TODO: (Parse_Function) remove this garbage that is hard-coded
void* Parse_Function(Parser_Context& ctx) {
    auto& tok = ctx.lexer.currentToken;

    if (tok.type != token_identifier) return ctx.error_unexpected("", "");

    auto name = tok.string;

    ctx.lexer.next_token();
    if (tok.type != ':') return ctx.error_unexpected("", "");
    ctx.lexer.next_token();
    if (tok.type != ':') return ctx.error_unexpected("", "");

    ctx.lexer.next_token();
    if (tok.type != '(') return ctx.error_unexpected("", "");
    
    ctx.lexer.next_token();

    // use temperary memory to store the statements we parse.
    auto params = reinterpret_cast<kai_Expr_Procedure_Parameter*>(ctx.stack);
    kai_u32 count = 0, ret_count = 0;

    if (tok.type != ')')
    {
    parseParam:
        ctx.stack = params + count;
        check_temp_stack_overflow(ctx);

        if (tok.type != token_identifier) return ctx.error_expected("identifier");
        auto name = tok.string;

        ctx.lexer.next_token();
        if (tok.type != ':') return ctx.error_expected("':'");

        ctx.lexer.next_token();
        auto ty = Parse_Type(ctx);
        if (!ty) return ctx.error_expected("Type");

        kai_Expr_Procedure_Parameter p;
        p.name = name;
        p.type = ty;
        p._flags = 0;

        params[count++] = p;

        ctx.lexer.next_token(); // get token after type
        switch (tok.type)
        {
        case ')': break;
        case ',':
            ctx.lexer.next_token(); // eat ',' (get token after)
            goto parseParam;
        default: return ctx.error_unexpected("", "wanted ')' or ',' here");
        }
    }

    ctx.lexer.next_token(); // eat ')'

    // return value
    if (tok.type == '->') {
        ctx.lexer.next_token(); // eat ')'

        ctx.stack = params + count;
        check_temp_stack_overflow(ctx);

        auto ty = Parse_Type(ctx);
        if (!ty) return ctx.error_expected("Type");

        kai_Expr_Procedure_Parameter p;
        p.name = {0,nullptr};
        p.type = ty;
        params[count++] = p;
        ++ret_count;

        ctx.lexer.next_token();
    }

    auto proc = ctx.alloc_expr<kai_Expr_Procedure>();
    // allocate the space we need for how many expressions we have:
    proc->input_output = (kai_Expr_Procedure_Parameter*)ctx.memory.alloc(ctx.memory.user, count* sizeof(kai_Expr_Procedure_Parameter));
    proc->param_count = count - ret_count;
    proc->ret_count   = ret_count;

    // copy expressions from temperary storage:
    memcpy(proc->input_output, params, count * sizeof(kai_Expr_Procedure_Parameter));
    // reset stack
    ctx.stack = params;

    proc->body = Parse_Statement(ctx);
    if (!proc->body) return ctx.error_expected("function body");

    auto decl = ctx.alloc_expr<kai_Stmt_Declaration>();
    decl->expr = (kai_Expr)proc;
    decl->name = name;
    return decl;
}

#if   defined(DEBUG_LEXER)
#include <set>
#include <iostream>
void print_token(Token const& t) {
    auto& os = std::cout;

    if( is_token_keyword(t.type) ) {
        os << "keyword:" << token_type_string(t.type);
    }
    else switch( t.type )
    {
    case token_identifier: {
        os << "ident[";
        os.write((char*)t.string.data, t.string.count);
        os << "]";
        break;
    }
    case token_string: {
        os << "string[";
        os.write( (char*)t.string.data, t.string.count );
        os << "]";
        break;
    }
    case token_directive: {
        os << "directive[";
        os.write( (char*)t.string.data, t.string.count );
        os << "]";
        break;
    }
    case token_number: {
        os << "number[Whole_Part = " << t.number.Whole_Part;
        os <<       ", Frac_Part = " << t.number.Frac_Part;
        os <<      ", Frac_Denom = " << t.number.Frac_Denom;
        os <<        ", Exp_Part = " << t.number.Exp_Part;
        os << "] \"";
        os.write( (char*)t.string.data, t.string.count );
        os << "\"";
        break;
    }
    case token_end: {
        os << "END";
        break;
    }

    default: {
        os.write((char*)t.string.data, t.string.count);
        break;
    }
    }

    os << '\n';
}

kai_result kai_create_syntax_tree(kai_Syntax_Tree_Create_Info* info)
{
    Lexer_Context lexer{info->source};

    info->module->toplevel = nullptr;
 
    while (1) {
        auto tok = lexer.next_token();

        print_token(tok);

        if (tok.type == token_end) {
            break;
        }
    }

    return kai_Success;
}
#elif defined(DEBUG_PARSER)

kai_result kai_create_syntax_tree(kai_Syntax_Tree_Create_Info* info)
{
    Parser_Context ctx{
        Lexer_Context{info->source},
        info->module->memory
    };
    ctx.stack      = ctx.memory.temperary;
    ctx.stack_size = ctx.memory.temperary_size;

    // setup first token
    ctx.lexer.next_token();

    auto root = Parse_Function(ctx);

    if (ctx.error_info.value) {
        if (info->error_info) {
            *info->error_info = ctx.error_info;
            info->error_info->file = info->filename;
            info->error_info->loc.source = info->source.data;
        }
        return ctx.error_info.value;
    }
    else {
        info->module->AST_Root = root;
    }

    return kai_Result_Success;
}
#else
kai_result kai_create_syntax_tree(kai_Syntax_Tree_Create_Info* info)
{
    Parser_Context ctx{
        Lexer_Context{info->source},
        info->module->memory
    };

    return kai_Success;
}
#endif
