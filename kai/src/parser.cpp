#include "parser.hpp"
#include <cassert>
#define DEFAULT_PREC -6942069

//#define DEBUG_LEXER
//#define DEBUG_PARSER

#define check_temp_stack_overflow(END_WRITE, CTX) if ((kai_u8*)(END_WRITE) > (kai_u8*)CTX.memory.temperary + CTX.memory.temperary_size) \
                               { return ctx.error_internal("ran out of temperary memory!"); } else

#define __check_temp_stack_overflow(END_WRITE, CTX) if ((kai_u8*)(END_WRITE) > (kai_u8*)CTX.memory.temperary + CTX.memory.temperary_size) \
                               { ctx.error_internal("ran out of temperary memory!"); goto error; } else

// requires current token to be '('
bool is_procedure_next(Parser_Context& ctx) {
    auto& peek = ctx.lexer.peek_token();
    if (peek.type == ')') return true;

    auto lexer_state = ctx.lexer;

    Token& cur = ctx.lexer.next_token();
    bool found = false;

    // go until we hit ')' or END, searching for ':'
    while (cur.type != token_end && cur.type != ')') {
        if (cur.type == ':') {
            found = true;
            break;
        }
        ctx.lexer.next_token();
    }

    ctx.lexer = lexer_state;
    return found;
}

kai_Expr Parse_Expression(Parser_Context& ctx, int prec = DEFAULT_PREC);
kai_Expr Parse_Procedure(Parser_Context& ctx);

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

            auto type = Parse_Type(ctx);
            if (!type) return nullptr;

            check_temp_stack_overflow(types + count, ctx);
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
                
                auto type = Parse_Type(ctx);
                if (!type) {
                    if((count-param_count) == 0)
                        return nullptr;
                    break;
                }

                check_temp_stack_overflow(types + count, ctx);
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
        proc->param_count = param_count;
        proc->ret_count = count - param_count;
        proc->line_number;
        proc->source_code;

        // allocate the space we need for how many expressions we have:
        proc->input_output = (kai_Expr*)ctx.memory.alloc(ctx.memory.user, count * sizeof(kai_Expr));

        // copy expressions from temperary storage:
        memcpy(proc->input_output, types, count * sizeof(kai_Expr));
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
        ident->line_number = tok.line_number;
        ident->source_code = tok.string;
        return(kai_Expr) ident;
    }
    case token_number: {
        auto num = ctx.alloc_expr<kai_Expr_Number>();
        num->info        = tok.number;
        num->line_number = tok.line_number;
        num->source_code = tok.string;
        return(kai_Expr) num;
    }
    case token_string: {
        auto str = ctx.alloc_expr<kai_Expr_String>();
        str->line_number = tok.line_number;
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
    // Handle Parenthesis
    if (tok.type == '(') {
		ctx.lexer.next_token(); // see whats after paren

		left = Parse_Expression(ctx); // parse whatever the hell we get
		if (!left) return ctx.error_expected("an expression with paranthesis");

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
        if (!left) return ctx.error_expected("type");

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
    else if (tok.type == token_directive && kai_string_equals(KAI_STR("type"), tok.string)) {
        ctx.lexer.next_token(); // see what is next
        left = Parse_Type(ctx);
        if (!left) return ctx.error_expected("type");
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

    if (!op_info.prec || op_info.prec <= prec)
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
            auto expr = Parse_Expression(ctx);

            if (!expr) {return ctx.error_expected("an expression in procedure call"); }

            check_temp_stack_overflow(expressions + count, ctx);
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

kai_Stmt Parse_Statement(Parser_Context& ctx, bool is_top_level = false) {
    auto& tok = ctx.lexer.currentToken;

    switch (tok.type)
    {
    // This is probably just an expression
    default: {
    parseExpression:
        if (is_top_level) return ctx.error_expected("a top level statement");

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
        kai_u32 count = 0;

        // parse statements until we get a '}'
        if (tok.type != '}')
        {
        parseStatement:
            ctx.stack = statements + count;

            auto stmt = Parse_Statement(ctx);
            if (!stmt) return nullptr;
            
            check_temp_stack_overflow(statements + count, ctx);
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
        if (is_top_level) return ctx.error_unexpected("in top level statement", "");

        ctx.lexer.next_token(); // whats after ret?

        // TODO: fix whatever this is
        if (tok.type == ';') {
            auto ret = ctx.alloc_expr<kai_Stmt_Return>();
            ret->expr = nullptr;
            return(kai_Stmt)ret;
        }

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
        auto ln   = tok.line_number;
        ctx.lexer.next_token(); // eat ':'
        ctx.lexer.next_token();

        kai_u32 flags = 0;

        switch (tok.type)
        {
        case ':': flags |= kai_Decl_Flag_Const; break;
        default: return ctx.error_expected("'=' or ':' in declaration");
        case '=': break;
        }

        ctx.lexer.next_token();

        bool require_semicolon = false;

        kai_Expr expr;
        if (tok.type == '(' && is_procedure_next(ctx)) {
            expr = Parse_Procedure(ctx);
            if (!expr) return ctx.error_expected("a procedure");
        }
        else {
            expr = Parse_Expression(ctx);
            if (!expr) return ctx.error_expected("an expression");
            require_semicolon = true;
        }

        if (require_semicolon) {
            ctx.lexer.next_token(); // what is after?
            if (tok.type != ';') {
                return ctx.error_unexpected("after statement", "there should be a ';' before this");
            }
        }

        auto decl = ctx.alloc_expr<kai_Stmt_Declaration>();
        decl->expr = expr;
        decl->name = name;
        decl->flags = flags;
        decl->line_number = ln;

        return(kai_Stmt) decl;
    }

    }
}


kai_Expr Parse_Procedure(Parser_Context& ctx) {
    auto& tok = ctx.lexer.currentToken;

    // sanity check
    if (tok.type != '(') return ctx.error_expected("'(' (compiler bug)");
    ctx.lexer.next_token();

    // use temperary memory to store the parameters we parse.
    auto params = reinterpret_cast<kai_Expr_Procedure_Parameter*>(ctx.stack);
    kai_u32 count = 0, ret_count = 0;

    if (tok.type != ')')
    {
    parseParam:
        ctx.stack = params + count;

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

        check_temp_stack_overflow(params + count, ctx);
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

        auto ty = Parse_Type(ctx);
        if (!ty) return ctx.error_expected("Type");

        kai_Expr_Procedure_Parameter p;
        p.name = {0,nullptr};
        p.type = ty;

        check_temp_stack_overflow(params + count, ctx);
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

    auto body = Parse_Statement(ctx);
    if (!body) return ctx.error_expected("function body");

    proc->body = body;
    return(kai_Expr) proc;
}

#if   defined(DEBUG_LEXER)
#include <set>
#include <iostream>
void print_token(Token const& t) {
    auto& os = std::cout;

    if( is_token_keyword(t.type) ) {
        os << "keyword:" << token_type_string(t.type).data;
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

    info->module->toplevel_count = 0;

    while (1) {
        auto tok = lexer.next_token();

        print_token(tok);

        if (tok.type == token_end) {
            break;
        }
    }

    return kai_Result_Success;
}
#else
kai_result kai_create_syntax_tree(kai_Syntax_Tree_Create_Info* info)
{
    Parser_Context ctx{
        info->memory,
        Lexer_Context{info->source},
    };
    ctx.stack      = ctx.memory.temperary;
    ctx.stack_size = ctx.memory.temperary_size;

    // setup first token
    auto& cur = ctx.lexer.next_token();

#if defined(DEBUG_PARSER)
    auto root = Parse_Function(ctx);
#else
    // use temperary memory to store the statements we parse.
    auto statements = reinterpret_cast<kai_Stmt*>(ctx.stack);
    kai_u32 count = 0;

    while (cur.type != token_end)
    {
        ctx.stack = statements + count;
        auto stmt = Parse_Statement(ctx, true);

        if (!stmt) { ctx.error_expected("a top level statement [from top]"); goto error; }

        __check_temp_stack_overflow(statements + count, ctx);
        statements[count++] = stmt;

        ctx.lexer.next_token(); // get first token of next statement
    }

    // allocate the space we need for how many statements we have:
    info->module->toplevel_stmts = (kai_Stmt*)ctx.memory.alloc(ctx.memory.user, count * sizeof(kai_Stmt));
    info->module->toplevel_count = count;

    // copy expressions from temperary storage:
    memcpy(info->module->toplevel_stmts, statements, count * sizeof(kai_Stmt));
#endif

error:
    if (ctx.error_info.result) {
        if (info->error) {
            *info->error = ctx.error_info;
            info->error->location.file   = info->filename;
            info->error->location.source = info->source.data;
        }
        return ctx.error_info.result;
    }

    return kai_Result_Success;
}
#endif
