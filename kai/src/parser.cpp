#include <cstddef>
#include "parser.hpp"
#define DEFAULT_PREC -6942069

//#define DEBUG_LEXER
//#define DEBUG_PARSER

#if defined(DEBUG_PARSER)
struct scoped_debug_logger {
    char const* function_name;
    static int scope;

    scoped_debug_logger(char const* f): function_name(f) {
        for_n (scope) printf("   ");
        printf("==> \"%s\"\n", function_name);
        ++scope;
    }
    ~scoped_debug_logger() {
        --scope;
        for_n (scope) printf("   ");
        printf("<== \"%s\"\n", function_name);
    }
};
inline int scoped_debug_logger::scope = 0;
#define log_function() auto __function_logger = scoped_debug_logger(__FUNCTION__)
#else
#define log_function() (void)0
#endif

#if 0
// file.kai --> Syntax Error: unexpected ';'
// 4 |    x := 5 + ;
//   |             ^ wanted an expression here
// file.kai --> Note: error occured while parsing a Declaration
// 4 |    x := 5 + ;
//   |    ^
// file.kai --> Note: error occured while parsing a Procedure body
// 2 | main :: () {
//   | ^~~~
static void* error_info() {
    auto& err = context.error;
    if (err.next != nullptr) return nullptr;
    
    kai_Error info = err;
    info.message = {};
    info.context = {};
    info.location = {};
    return nullptr;
}
#endif

void adjust_source_location(kai_str& src, kai_u32 type) {
    if (type == token_string) {
        src.data -= 1; // we know there will at least be two " in strings
        src.count += 2;
    }
    if (type == token_directive) {
        src.data -= 1;
        src.count += 1;
    }
}

//! @NOTE: Do we need to be using `kai_str` here? Only static strings are passed in...
static void* error_unexpected(Token token, kai_str const& where, kai_str const& wanted) {
    auto& err = context.error;
    if (err.result != kai_Result_Success) return nullptr;

    err.result          = kai_Result_Error_Syntax;
    err.location.string = token.string;
    err.location.line   = token.line_number;
    err.context         = wanted;
    err.message         = { 0, (kai_u8*)context.memory.temperary }; // uses temperary memory

    adjust_source_location(err.location.string, token.type);

    str_insert_string(err.message, "unexpected ");
    insert_token_type_string(err.message, token.type);
    err.message.data[err.message.count++] = ' ';
    str_insert_str(err.message, where);
    return nullptr;
}

#define error_unexpected_s(WHERE, CONTEXT) \
(kai_Expr)error_unexpected(lexer.current_token, KAI_STR(WHERE), KAI_STR(CONTEXT))

#define error_unexpected_t(TOKEN, WHERE, CONTEXT) \
(kai_Expr)error_unexpected(TOKEN, KAI_STR(WHERE), KAI_STR(CONTEXT))

#define expect(EXPRESSION, WHERE, CONTEXT) \
if (!(EXPRESSION)) return error_unexpected_s(WHERE, CONTEXT)

// requires current token to be '('
bool Parser_Context::
is_procedure_next() {
    auto& peek = lexer.peek_token();
    if (peek.type == ')') return true;

    auto lexer_state = lexer;

    Token& cur = lexer.next_token();
    bool found = false;

    // go until we hit ')' or END, searching for ':'
    while (cur.type != token_end && cur.type != ')') {
        if (cur.type == ':') {
            found = true;
            break;
        }
        lexer.next_token();
    }

    lexer = lexer_state;
    return found;
}

kai_Expr Parser_Context::
parse_type() {
    log_function();
    auto& token = lexer.current_token;

    if (token.type == '[') {
        lexer.next_token(); // get token after
        
        expect(token.type == ']', "[todo]", "array");

        lexer.next_token();
        auto type = parse_type();
        expect(type, "[todo]", "type");

        auto unary = alloc_expr<kai_Expr_Unary>();
        unary->expr = type;
        unary->op   = '[';
        return (kai_Expr) unary;
    }

    if (token.type != '(') return parse_expression();

    lexer.next_token(); // get next after parenthesis

    // use temperary memory to store the types we parse.
    auto type_array = temperary_array<kai_Expr>();

    // Parse Procedure input types
    if (token.type != ')') for (;;) {
        auto type = parse_type();
        if (!type) return nullptr;

        if (context.has_stack_overflow(type_array)) return nullptr;
        array_add(type_array, type);

        lexer.next_token(); // get ',' token or ')'

        if (token.type == ')') break;
        if (token.type == ',') lexer.next_token();
        else return error_unexpected_s("in procedure type", "',' or ')' expected here");
    }
    
    auto param_count = type_array.count;
    auto& peek = lexer.peek_token(); // see if we have any returns

    ///////////////////////////////////////////////////////////////////////
    // "This code is broken as hell."
    if (peek.type == token_2("->")) {
        lexer.next_token(); // eat '->'
        lexer.next_token(); // get token after

        bool enclosed_return = false;
        if (token.type == '(') {
            enclosed_return = true;
            lexer.next_token();
        }

        for (;;) {
            auto type = parse_type();

            if (!type) {
                if((type_array.count-param_count) == 0)
                    return nullptr;
                break;
            }

            if (context.has_stack_overflow(type_array)) return nullptr;
            array_add(type_array, type);

            lexer.peek_token(); // get ',' token or something else

            if (peek.type == ',') {
                lexer.next_token(); // eat ','
                lexer.next_token(); // get token after
            }
            else break;
        }

        if (enclosed_return) {
            if (peek.type != ')') {
                return error_unexpected_t(peek, "in procedure type", "')' after return types");
            }
            else lexer.next_token();
        }
    }
    ///////////////////////////////////////////////////////////////////////

    auto proc = alloc_expr<kai_Expr_Procedure_Type>();
    proc->param_count = param_count;
    proc->ret_count = type_array.count - param_count;

    // allocate the space we need for how many expressions we have:
    proc->input_output = ctx_alloc_array(kai_Expr, type_array.count);

    // copy expressions from temperary storage:
    type_array.copy_to(proc->input_output);
    free_temperary_array(type_array);

    return(kai_Expr) proc;
}

enum class op_type {
    binary,
    index,
    procedure_call,
};

struct operator_info {
    kai_u32 op;
    int     prec;
    op_type type;
};

#define CAST_PREC     0x0900

operator_info get_operator(kai_u32 t) {
    switch (t)
    {
    case token_2("&&"): return {t, 0x0010,    op_type::binary};
    case token_2("||"): return {t, 0x0010,    op_type::binary};
    case token_2("=="): return {t, 0x0040,    op_type::binary};
    case token_2("<="): return {t, 0x0040,    op_type::binary};
    case '+':           return {t, 0x0100,    op_type::binary};
    case '-':           return {t, 0x0100,    op_type::binary};
    case '*':           return {t, 0x0200,    op_type::binary};
    case '/':           return {t, 0x0200,    op_type::binary};
    case token_2("->"): return {t, CAST_PREC, op_type::binary};

    case '[':  return {t, 0xBEEF,    op_type::index};
    case '(':  return {t, 0xBEEF,    op_type::procedure_call};
    case '.':  return {t, 0xFFFF,    op_type::binary}; // member access
    default:   return {t, 0};
    }
}

kai_Expr Parser_Context::
parse_expression(int prec) {
    log_function();
	auto& token = lexer.current_token;
	kai_Expr left = nullptr;

    switch (token.type) {
    ////////////////////////////////////////////////////////////
    // Handle Parenthesis
    break; case '(': {
		lexer.next_token(); // see whats after paren

		left = parse_expression(); // parse whatever the hell we get
		expect(left, "in expression", "an expression with paranthesis");

		// advance to closing parenthesis hopefully
		lexer.next_token();

		// when opening a paranthesis, you kinda need to close it
		expect(token.type == ')', "in expression", "an operator or ')' in expression");
    }
    ////////////////////////////////////////////////////////////
    // Handle Unary Operators
    break;
    case '-':
    case '+':
    case '*':
    case '/': {
		auto op = token.type;

        lexer.next_token();
        left = parse_expression(0x1000);
        expect(left, "in unary expression", "should be an expression here");

        auto unary = alloc_expr<kai_Expr_Unary>();
        unary->expr = left;
        unary->op   = op;
        left = (kai_Expr) unary;
    }
    ////////////////////////////////////////////////////////////
    // Handle Explicit Casting "cast(int) x"
    break; case token_kw_cast: {
        lexer.next_token();

        expect(token.type == '(', "in expression", "'(' after cast keyword");

        lexer.next_token();
        left = parse_type();
        expect(left, "in expression", "type");

        lexer.next_token();
        expect(token.type == ')', "in expression", "')' after Type in cast expression");

        lexer.next_token();
        auto right = parse_expression(CAST_PREC);
        expect(right, "in expression", "expression after cast");

        auto binary = alloc_expr<kai_Expr_Binary>();
        binary->op = token_2("->");
        binary->left  = left;
        binary->right = right;
        binary->line_number;
        binary->source_code;
        left = (kai_Expr) binary;
    }
    ////////////////////////////////////////////////////////////
    // Handle Directives
    break; case token_directive: {
        if (kai_string_equals(KAI_STR("type"), token.string)) {
            lexer.next_token(); // see what is next
            left = parse_type();
            if (!left) return error_unexpected_s("in expression", "type");
        }
        else if (kai_string_equals(KAI_STR("string"), token.string)) {
            lexer.next_token();
            expect(token.type == token_identifier, "[todo]", "[todo]");

        //  auto s = lexer.raw_string(token.string);

            return error_unexpected_s("", "raw strings not implemented");
        }
        else return nullptr;
    }
    ////////////////////////////////////////////////////////////
    // Handle Single-Token Expressions
    break; case token_identifier: {
        auto ident = alloc_expr<kai_Expr_Identifier>();
        ident->line_number = token.line_number;
        ident->source_code = token.string;
        left = (kai_Expr) ident;
    }
    break; case token_number: {
        auto num = alloc_expr<kai_Expr_Number>();
        num->info        = token.number;
        num->line_number = token.line_number;
        num->source_code = token.string;
        left = (kai_Expr) num;
    }
    break; case token_string: {
        auto str = alloc_expr<kai_Expr_String>();
        str->line_number = token.line_number;
        str->source_code = token.string;
        left = (kai_Expr) str;
    }
    break; default: return nullptr; // what did we get?
    }

LoopBack:
    auto& peek = lexer.peek_token();
    auto op_info = get_operator(peek.type);

    // Handle Precedence by returning early
    if (!op_info.prec || op_info.prec <= prec)
        return left;

    lexer.next_token(); // eat operator token
    lexer.next_token(); // get next token

    switch (op_info.type)
    {
    case op_type::binary: {
        auto right = parse_expression(op_info.prec);
        expect(right, "in binary expression", "should be an expression after binary operator");
        
        auto binary = alloc_expr<kai_Expr_Binary>();
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
        auto right = parse_expression();
        expect(right, "in index operation", "should be an expression here");

        lexer.next_token(); // hopefully get a closing bracket
        expect(token.type == ']', "in index operation", "expected ']' here");

        auto binary = alloc_expr<kai_Expr_Binary>();
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
        auto expression_array = temperary_array<kai_Expr>();

        if (token.type != ')') for(;;) {
            auto expr = parse_expression();
            expect(expr, "[todo]", "an expression in procedure call");

            if (context.has_stack_overflow(expression_array)) return nullptr;
            array_add(expression_array, expr);

            lexer.next_token(); // get ',' token or ')'

            if (token.type == ')') break;
            if (token.type == ',') lexer.next_token();
            else return error_unexpected_s("in procedure call", "',' or ')' expected here");
        }

        auto call = alloc_expr<kai_Expr_Procedure_Call>();
        call->proc = left;

        // allocate the space we need for how many expressions we have:
        call->arguments = ctx_alloc_array(kai_Expr, expression_array.count);
        call->arg_count = expression_array.count;

        // copy expressions from temperary storage:
        expression_array.copy_to(call->arguments);
        free_temperary_array(expression_array);

        left = (kai_Expr) call;
        break;
    }

    default: unreachable();
    }

    goto LoopBack;
}

kai_Expr Parser_Context::
parse_statement(bool is_top_level) {
    log_function();
    auto& token = lexer.current_token;

    switch (token.type)
    {
    case token_end: if (is_top_level) return nullptr;
    default: {
    parseExpression:
        if (is_top_level) {
            return error_unexpected_s("in top level statement", "a top level statement");
        }

        auto expression = parse_expression();
        expect(expression, "in statement", "should be an expression or statement");
    
        lexer.next_token();

        if (token.type == '=') {
            lexer.next_token();

            auto right = parse_expression();
            expect(right, "in assignment statement", "should be an expression");

            auto assignment = alloc_expr<kai_Stmt_Assignment>();
            assignment->left = expression;
            assignment->expr = right;
            expression = (kai_Expr) assignment;

            lexer.next_token();
        }

        expect(token.type == ';', "in expression statement", "should be ';' after expression");
        return expression;
    }
//  case ';': return nullptr;

    case '{': {
        if (is_top_level) {
            return error_unexpected_s("in top level statement", "a top level statement");
        }

        lexer.next_token();

        // use temperary memory to store the statements we parse.
        auto statement_array = temperary_array<kai_Expr>();

        // parse statements until we get a '}'
        while (token.type != '}') {
            auto statement = parse_statement();
            if (!statement) return nullptr;

            if (context.has_stack_overflow(statement_array)) return nullptr;
            array_add(statement_array, statement);

            lexer.next_token(); // eat ';' (get token after)
        }

        auto compound = alloc_expr<kai_Stmt_Compound>();
        compound->statements = ctx_alloc_array(kai_Stmt, statement_array.count);
        compound->count = statement_array.count;

        statement_array.copy_to(compound->statements);
        free_temperary_array(statement_array);

        return(kai_Stmt) compound;
    }

    case token_kw_ret: {
        if (is_top_level) {
            return error_unexpected_s("in top level statement", "should be within a function");
        }

        lexer.next_token(); // whats after ret?

        //! @TODO: fix whatever this is
        if (token.type == ';') {
            auto ret = alloc_expr<kai_Stmt_Return>();
            ret->expr = nullptr;
            return(kai_Stmt)ret;
        }

        auto expr = parse_expression();
        if (!expr) return error_unexpected_s("[todo]", "expression");

        lexer.next_token(); // whats after expression?
        expect(token.type == ';', "after statement", "there should be a ';' before this");

        auto ret = alloc_expr<kai_Stmt_Return>();
        ret->expr = expr;
        ret->line_number;
        ret->source_code;
        return(kai_Stmt) ret;
    }

    case token_kw_if: {
        if (is_top_level) {
            return error_unexpected_s("in top level statement", "should be within a function");
        }

        lexer.next_token(); // get token after 'if'
        auto expr = parse_expression();
        expect(expr, "in if statement", "should be an expression here");

        lexer.next_token(); // get token after expression
        auto body = parse_statement();
        if (body == nullptr) return nullptr;

        // Parse 'else' body
    	auto& peek = lexer.peek_token();
		kai_Stmt else_body = nullptr;
		if (peek.type == token_kw_else) {
			lexer.next_token();
			lexer.next_token();
        	else_body = parse_statement();
			if (else_body == nullptr) return nullptr;
		}

        auto if_statement = alloc_expr<kai_Stmt_If>();
        if_statement->expr      = expr;
        if_statement->body      = body;
		if_statement->else_body = else_body;
        return(kai_Stmt) if_statement;
    }

    case token_kw_for: {
        if (is_top_level) {
            return error_unexpected_s("in top level statement", "should be within a function");
        }

        lexer.next_token();
        expect(token.type == token_identifier, "in for statement", "should be the name of the iterator");
        auto iterator_name = token.string;

        lexer.next_token();
        expect(token.type == ':', "in for statement", "should be ':' here");

        lexer.next_token();
        auto from = parse_expression();
        expect(from, "in for statement", "should be an expression here");

        kai_Expr to = nullptr;
        lexer.next_token();
        if (token.type == token_2("..")) {
            lexer.next_token(); // eat '..'
            to = parse_expression();
            expect(to, "in for statement", "should be an expression here");
            lexer.next_token();
        }

        auto body = parse_statement();
        if (body == nullptr) return nullptr;

        auto for_statement = alloc_expr<kai_Stmt_For>();
        for_statement->body = body;
        for_statement->from = from;
        for_statement->to   = to;
        for_statement->iterator_name = iterator_name;
        for_statement->flags = 0;
        return(kai_Stmt) for_statement;
    }

    case token_identifier: {
        auto& peek = lexer.peek_token();

        // just an expression?
        if (peek.type != ':') goto parseExpression;

        auto name = token.string;
        auto line_number = token.line_number;
        lexer.next_token(); // 
        lexer.next_token(); // w

        kai_u32 flags = 0;

        switch (token.type)
        {
        case ':': flags |= kai_Decl_Flag_Const; break;
        case '=': break;
        default: return error_unexpected_s("in declaration", "should be '=' or ':'");
        }

        kai_Expr expr;
        // Procedures will always end in a statement, so there is no need to check for semicolon.
        bool require_semicolon = false;

        lexer.next_token();
        if (token.type == '(' && is_procedure_next()) {
            expr = parse_procedure();
            if (!expr) return nullptr;
        }
        else {
            expr = parse_expression();
            expect(expr, "in declaration", "should be an expression here");
            require_semicolon = true;
        }

        if (require_semicolon) {
            lexer.next_token(); // what is after?
            expect(token.type == ';', "after statement", "there should be a ';' before this");
        }

        auto decl = alloc_expr<kai_Stmt_Declaration>();
        decl->expr = expr;
        decl->name = name;
        decl->flags = flags;
        decl->line_number = line_number;
        return(kai_Stmt) decl;
    }
    }
}


kai_Expr Parser_Context::
parse_procedure() {
    log_function();
    auto& token = lexer.current_token;

    // sanity check
    expect(token.type == '(', "", "this is likely a compiler bug, sorry :c");
    lexer.next_token();

    // use temperary memory to store the parameters we parse.
    auto parameter_array = temperary_array<kai_Expr_Procedure_Parameter>();
    kai_u32 ret_count = 0;

    if (token.type != ')')
    {
    parseParam:
        if (token.type != token_identifier) return error_unexpected_s("in procedure input", "should be an identifier");
        auto name = token.string;

        lexer.next_token();
        if (token.type != ':') return error_unexpected_s("[todo]", "':'");

        lexer.next_token();
        auto ty = parse_type();
        if (!ty) return error_unexpected_s("[todo]", "Type");

        kai_Expr_Procedure_Parameter p;
        p.name = name;
        p.type = ty;

        if (context.has_stack_overflow(parameter_array)) return nullptr;
        array_add(parameter_array, p);

        lexer.next_token(); // get token after type
        switch (token.type)
        {
        case ')': break;
        case ',':
            lexer.next_token(); // eat ',' (get token after)
            goto parseParam;
        default: return error_unexpected_s("[todo]", "wanted ')' or ',' here");
        }
    }

    lexer.next_token(); // eat ')'

    // return value
    if (token.type == token_2("->")) {
        lexer.next_token(); // eat ')'

        auto ty = parse_type();
        if (!ty) return error_unexpected_s("[todo]", "Type");

        kai_Expr_Procedure_Parameter p;
        p.name = {0,nullptr};
        p.type = ty;

        if (context.has_stack_overflow(parameter_array)) return nullptr;
        array_add(parameter_array, p);
        
        ++ret_count;

        lexer.next_token();
    }

    auto proc = alloc_expr<kai_Expr_Procedure>();
    // allocate the space we need for how many expressions we have:
    proc->input_output = ctx_alloc_array(kai_Expr_Procedure_Parameter, parameter_array.count);
    proc->param_count = parameter_array.count - ret_count;
    proc->ret_count   = ret_count;

    // copy expressions from temperary storage:
    parameter_array.copy_to(proc->input_output);
    free_temperary_array(parameter_array);

    kai_Stmt body = nullptr;
    if (token.type == token_directive
    && kai_string_equals(KAI_STR("native"), token.string)) {
        lexer.next_token();
        expect(token.type == ';', "[todo]", "[todo]");
    }
    else {
        body = parse_statement();
        if (!body) return nullptr;
    }

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

kai_result kai_create_syntax_tree(kai_Syntax_Tree_Create_Info* info, kai_AST* out_AST)
{
    Lexer_Context lexer{info->source_code};

    out_AST->toplevel_count = 0;

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
kai_result kai_create_syntax_tree(kai_Syntax_Tree_Create_Info* info, kai_AST* out_ast)
{
	context.reset(&info->memory);

    Parser_Context parser{
        Lexer_Context{info->source_code},
        info->memory.temperary
    };

    // setup first token
    auto& token = parser.lexer.next_token();

    // use temperary memory to store the statements we parse.
    auto statement_array = parser.temperary_array<kai_Stmt>();

    while (token.type != token_end) {
        auto statement = parser.parse_statement(true);
        
        if (!statement || context.has_stack_overflow(statement_array))
            goto error;
        
        parser.array_add(statement_array, statement);
        parser.lexer.next_token(); // get first token of next statement
    }

    // allocate the space we need for how many statements we have:
    out_ast->toplevel_stmts = ctx_alloc_array(kai_Stmt, statement_array.count);
    out_ast->toplevel_count = statement_array.count;

    // copy statements to AST:
    statement_array.copy_to(out_ast->toplevel_stmts);

    if (context.error.result) {
        error:
        if (info->error) {
            *info->error = context.error;
            info->error->location.file_name = out_ast->source_filename;
            info->error->location.source = info->source_code.data;
        }
        return context.error.result;
    }

    return kai_Result_Success;
}
#endif
