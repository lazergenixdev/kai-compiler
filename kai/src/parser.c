#include <kai/parser.h>
#include "token.h"
#include "config.h"

typedef struct {
    Tokenization_Context tokenizer;
    Kai_u32              stack_top;
    Kai_Memory           memory;
    Kai_Error            error;
} Parser;

// Im somewhat of a scientist myself, what can I say?
#define _ID_Expr_Identifier     KAI_EXPR_IDENTIFIER
#define _ID_Expr_String         KAI_EXPR_STRING         
#define _ID_Expr_Number         KAI_EXPR_NUMBER         
#define _ID_Expr_Binary         KAI_EXPR_BINARY         
#define _ID_Expr_Unary          KAI_EXPR_UNARY          
#define _ID_Expr_Procedure_Type KAI_EXPR_PROCEDURE_TYPE 
#define _ID_Expr_Procedure_Call KAI_EXPR_PROCEDURE_CALL 
#define _ID_Expr_Procedure      KAI_EXPR_PROCEDURE      
#define _ID_Stmt_Return         KAI_STMT_RETURN         
#define _ID_Stmt_Declaration    KAI_STMT_DECLARATION    
#define _ID_Stmt_Assignment     KAI_STMT_ASSIGNMENT     
#define _ID_Stmt_Compound       KAI_STMT_COMPOUND       
#define _ID_Stmt_If             KAI_STMT_IF             
#define _ID_Stmt_For            KAI_STMT_FOR            
#define _ID_Stmt_Defer          KAI_STMT_DEFER 

#define p_alloc_node(TYPE) \
    (Kai_##TYPE*)alloc_type(parser, sizeof(Kai_##TYPE), _ID_##TYPE)

#define p_alloc_array(TYPE, COUNT) \
    (TYPE*)parser->memory.alloc(parser->memory.user, sizeof(TYPE)*COUNT)
    
inline Kai_Expr alloc_type(Parser* p, Kai_u64 size, Kai_u8 id) {
    Kai_Expr expr = p->memory.alloc(p->memory.user, size);
    expr->id = id;
    return expr;
}

typedef struct {
    Kai_u32 offset;
    Kai_u32 count;
    Kai_u32 stride;
} Temperary_Array;

#define p_tarray_create(TYPE) \
    (Temperary_Array){.offset = parser->stack_top,.count = 0,.stride = sizeof(TYPE)}

#define p_tarray_add(ARRAY, VALUE) \
    do { void* dest = (Kai_u8*)parser->memory.temperary + ARRAY.offset + sizeof(VALUE)*ARRAY.count; \
        ++ARRAY.count; \
        memcpy(dest, &VALUE, sizeof(VALUE)); \
        parser->stack_top = ARRAY.offset + ARRAY.count * ARRAY.stride; \
    } while (0)

#define p_tarray_copy(ARRAY, DEST) \
    memcpy(DEST, (Kai_u8*)parser->memory.temperary + ARRAY.offset, ARRAY.count * ARRAY.stride)

#define p_tarray_destroy(ARRAY) \
    parser->stack_top = ARRAY.offset

static Kai_bool error_internal(Parser* parser, Kai_str message) {
    parser->error.result = KAI_ERROR_INTERNAL;
    parser->error.location.string = KAI_EMPTY_STRING;
    parser->error.location.line = 0;
    parser->error.message = message;
    return KAI_TRUE;
}

void adjust_source_location(Kai_str* src, Kai_u32 type) {
    if (type == T_STRING) {
        src->data  -= 1; // strings must begin with "
        src->count += 2;
    }
    else if (type == T_DIRECTIVE) {
        src->data  -= 1;
        src->count += 1;
    }
}

void* error_unexpected(Parser* parser, Token* token, Kai_str where, Kai_str wanted) {
    Kai_Error* e = &parser->error;
    if (e->result != KAI_SUCCESS) return NULL;
    e->result          = KAI_ERROR_SYNTAX;
    e->location.string = token->string;
    e->location.line   = token->line_number;
    e->context         = wanted;
    // uses temperary memory
    e->message = (Kai_str){.count = 0, .data = (Kai_u8*)parser->memory.temperary};
    adjust_source_location(&e->location.string, token->type);
    str_insert_string(e->message, "unexpected ");
    insert_token_type_string(&e->message, token->type);
    e->message.data[e->message.count++] = ' ';
    str_insert_str(e->message, where);
    return NULL;
}

#define p_error_internal_s(MESSAGE) \
    error_internal(parser, KAI_STR(MESSAGE))

#define p_has_stack_overflow(ARRAY) (\
    (ARRAY.offset + (ARRAY.count + 1) * ARRAY.stride) > \
    ((Kai_u64)parser->memory.temperary + parser->memory.temperary_size) ? \
    p_error_internal_s("ran out of temperary memory!") : \
    KAI_FALSE)

#define p_error_unexpected_s(WHERE, CONTEXT) \
    error_unexpected(parser, &parser->tokenizer.current_token, KAI_STR(WHERE), KAI_STR(CONTEXT))

#define p_expect(OK, WHERE, CONTEXT) \
    if (!(OK)) return p_error_unexpected_s(WHERE, CONTEXT)

#define p_next()           next_token(&parser->tokenizer)
#define p_peek()           peek_token(&parser->tokenizer)
#define _parse_expr(PREC)  parse_expression(parser, PREC)
#define _parse_stmt(TOP)   parse_statement(parser, TOP)
#define _parse_proc()      parse_procedure(parser)
#define _parse_type()      parse_type(parser)

Kai_Expr parse_type(Parser* parser);
Kai_Expr parse_expression(Parser* parser, int precedence);
Kai_Expr parse_procedure(Parser* parser);
Kai_Stmt parse_statement(Parser* parser, Kai_bool is_top_level);

enum {
    OP_BINARY,
    OP_INDEX,
    OP_PROCEDURE_CALL,
};

typedef struct {
    Kai_u32 op;
    int prec;
    int type;
} Op_Info;

#define DEFAULT_PREC -6942069
#define CAST_PREC     0x0900
#define UNARY_PREC    0x1000

Op_Info operator_of_token_type(Kai_u32 t) {
    switch (t)
    {
    case '&&':  return (Op_Info){t, 0x0010,    OP_BINARY};
    case '||':  return (Op_Info){t, 0x0010,    OP_BINARY};
    case '==':  return (Op_Info){t, 0x0040,    OP_BINARY};
    case '<=':  return (Op_Info){t, 0x0040,    OP_BINARY};
    case '+':   return (Op_Info){t, 0x0100,    OP_BINARY};
    case '-':   return (Op_Info){t, 0x0100,    OP_BINARY};
    case '*':   return (Op_Info){t, 0x0200,    OP_BINARY};
    case '/':   return (Op_Info){t, 0x0200,    OP_BINARY};
    case '->':  return (Op_Info){t, CAST_PREC, OP_BINARY};
    case '[':   return (Op_Info){t, 0xBEEF,    OP_INDEX};
    case '(':   return (Op_Info){t, 0xBEEF,    OP_PROCEDURE_CALL};
    case '.':   return (Op_Info){t, 0xFFFF,    OP_BINARY}; // member access
    default:    return (Op_Info){t, 0};
    }
}

Kai_bool is_procedurep_next(Parser* parser) {
    Token* p = p_peek();
    if (p->type == ')') return KAI_TRUE;
    Tokenization_Context state = parser->tokenizer;
    Token* cur = p_next();
    Kai_bool found = KAI_FALSE;

    // go until we hit ')' or END, searching for ':'
    while (cur->type != T_END && cur->type != ')') {
        if (cur->type == ':') {
            found = KAI_TRUE;
            break;
        }
        p_next();
    }
    parser->tokenizer = state;
    return found;
}

Kai_Expr parse_type(Parser* parser) {
    Token* t = &parser->tokenizer.current_token;
    if (t->type == '[') {
        p_next(); // get token after
        
        p_expect(t->type == ']', "[todo]", "array");

        p_next();
        Kai_Expr type = _parse_type();
        p_expect(type, "[todo]", "type");

        Kai_Expr_Unary* unary = p_alloc_node(Expr_Unary);
        unary->expr = type;
        unary->op   = '[';
        return (Kai_Expr) unary;
    }

    if (t->type != '(') return _parse_expr(DEFAULT_PREC);

    p_next(); // get next after parenthesis

    // use temperary memory to store the types we parse.
    Temperary_Array type_array = p_tarray_create(Kai_Expr);

    // Parse Procedure input types
    if (t->type != ')') for (;;) {
        Kai_Expr type = _parse_type();
        if (!type) return NULL;

        if (p_has_stack_overflow(type_array)) return NULL;
        p_tarray_add(type_array, type);

        p_next(); // get ',' token or ')'

        if (t->type == ')') break;
        if (t->type == ',') p_next();
        else return p_error_unexpected_s("in procedure type", "',' or ')' expected here");
    }
    
    Kai_u32 param_count = type_array.count;
    Token* peek = p_peek(); // see if we have any returns

    ///////////////////////////////////////////////////////////////////////
    // "This code is broken as hell."
    if (peek->type == '->') {
        p_next(); // eat '->'
        p_next(); // get token after

        Kai_bool enclosed_return = KAI_FALSE;
        if (t->type == '(') {
            enclosed_return = KAI_TRUE;
            p_next();
        }

        for (;;) {
            Kai_Expr type = _parse_type();

            if (!type) {
                if((type_array.count-param_count) == 0)
                    return NULL;
                break;
            }

            if (p_has_stack_overflow(type_array)) return NULL;
            p_tarray_add(type_array, type);

            p_peek(); // get ',' token or something else

            if (peek->type == ',') {
                p_next(); // eat ','
                p_next(); // get token after
            }
            else break;
        }

        if (enclosed_return) {
            if (peek->type != ')') {
				p_next();
                return p_error_unexpected_s("in procedure type", "should be ')' after return types");
            }
            else p_next();
        }
    }
    ///////////////////////////////////////////////////////////////////////

    Kai_Expr_Procedure_Type* proc = p_alloc_node(Expr_Procedure_Type);
    proc->param_count = param_count;
    proc->ret_count = type_array.count - param_count;

    // allocate the space we need for how many expressions we have:
    proc->input_output = p_alloc_array(Kai_Expr, type_array.count);

    // copy expressions from temperary storage:
	p_tarray_copy(type_array, proc->input_output);
    p_tarray_destroy(type_array);

    return (Kai_Expr) proc;
}

Kai_Expr parse_expression(Parser* parser, int prec) {
    D_VERBOSE("prec = %i", prec);
    Kai_Expr left = NULL;
    Token* t = &parser->tokenizer.current_token;

    switch (t->type) {
    ////////////////////////////////////////////////////////////
    // Handle Parenthesis
    break; case '(': {
        p_next();
        left = _parse_expr(DEFAULT_PREC);
        p_expect(left, "in expression", "an expression with parathesis");
        p_next();
        p_expect(t->type == ')', "in expression", "an operator or ')' in expression");
    }
    ////////////////////////////////////////////////////////////
    // Handle Unary Operators
    break;
    case '-':
    case '+':
    case '*':
    case '/': {
        Kai_u32 op = t->type;
        p_next();
        left = _parse_expr(UNARY_PREC);
        p_expect(left, "in unary expression", "should be an expression here");

        Kai_Expr_Unary* unary = p_alloc_node(Expr_Unary);
        unary->expr = left;
        unary->op   = op;
        left = (Kai_Expr) unary;
    }
    ////////////////////////////////////////////////////////////
    // Handle Explicit Casting "cast(int) x"
    break; case T_KW_cast: {
        p_next();
        p_expect(t->type == '(', "in expression", "'(' after cast keyword");
        p_next();
        Kai_Expr type = _parse_type();
        p_expect(type, "in expression", "type");
        p_next();
        p_expect(t->type == ')', "in expression", "')' after Type in cast expression");
        p_next();
        Kai_Expr expr = _parse_expr(CAST_PREC);
        p_expect(expr, "in expression", "expression after cast");
        Kai_Expr_Binary* binary = p_alloc_node(Expr_Binary);
        binary->op    = '->';
        binary->left  = expr;
        binary->right = type;
        left = (Kai_Expr) binary;
    }
    ////////////////////////////////////////////////////////////
    // Handle Directives
    break; case T_DIRECTIVE: {
        if (kai_string_equals(KAI_STR("type"), t->string)) {
            p_next();
            left = _parse_type();
            p_expect(left, "in expression", "type");
        }
        else if (kai_string_equals(KAI_STR("string"), t->string)) {
            // Kai_str s = next_raw_string();
            return p_error_unexpected_s("", "not implemented");
        }
        else return NULL;
    }
    ////////////////////////////////////////////////////////////
    // Handle Single-Token Expressions
    break; case T_IDENTIFIER: {
        Kai_Expr_Identifier* ident = p_alloc_node(Expr_Identifier);
        ident->line_number = t->line_number;
        ident->source_code = t->string;
        left = (Kai_Expr) ident;
    }
    break; case T_NUMBER: {
        Kai_Expr_Number* num = p_alloc_node(Expr_Number);
        num->info        = t->number;
        num->line_number = t->line_number;
        num->source_code = t->string;
        left = (Kai_Expr) num;
    }
    break; case T_STRING: {
        Kai_Expr_String* str = p_alloc_node(Expr_String);
        str->line_number = t->line_number;
        str->source_code = t->string;
        left = (Kai_Expr) str;
    }
    break; default: return NULL;
    }

loop_back:
    Token* p = p_peek();
    Op_Info op_info = operator_of_token_type(p->type);

    D_VERBOSE("op_info = {%i, %i, %i}", op_info.op, op_info.prec, op_info.type);

    // handle precedence by returning early
    if (!op_info.prec || op_info.prec <= prec)
        return left;

    p_next();
    p_next();

    switch (op_info.type) {

    case OP_BINARY: {
        Kai_Expr right = _parse_expr(op_info.prec);
        p_expect(right, "in binary expression", "should be an expression after binary operator");
        Kai_Expr_Binary* binary = p_alloc_node(Expr_Binary);
        binary->op    = op_info.op;
        binary->left  = left;
        binary->right = right;
        left = (Kai_Expr) binary;
        break;
    }

    case OP_INDEX: {
        Kai_Expr right = _parse_expr(DEFAULT_PREC);
        p_expect(right, "in index operation", "should be an expression here");
        p_next();
        p_expect(t->type == ']', "in index operation", "expected ']' here");
        Kai_Expr_Binary* binary = p_alloc_node(Expr_Binary);
        binary->op    = op_info.op;
        binary->left  = left;
        binary->right = right;
        left = (Kai_Expr) binary;
        break;
    }

    case OP_PROCEDURE_CALL: {
        Temperary_Array expr_array = p_tarray_create(Kai_Expr);
        if (t->type != ')') for (;;) {
            Kai_Expr expr = _parse_expr(DEFAULT_PREC);
            p_expect(expr, "[todo]", "an expression");
            if (p_has_stack_overflow(expr_array)) return NULL;
            p_tarray_add(expr_array, expr);
            p_next();
            if (t->type == ')') break;
            if (t->type == ',') p_next();
            else return p_error_unexpected_s("in procedure call", "',' or ')' expected here");
        }
        Kai_Expr_Procedure_Call* call = p_alloc_node(Expr_Procedure_Call);
        call->proc = left;
        call->arguments = p_alloc_array(Kai_Expr, expr_array.count);
        call->arg_count = expr_array.count;
        p_tarray_copy(expr_array, call->arguments);
        p_tarray_destroy(expr_array);
        left = (Kai_Expr) call;
        break;
    }

    default: panic_with_message("u fucked up");

    }
    goto loop_back;
}

Kai_Expr parse_procedure(Parser* parser) {
    Token* t = &parser->tokenizer.current_token;
    // sanity check
    p_expect(t->type == '(', "", "this is likely a compiler bug, sorry :c");
    p_next();
    Temperary_Array parameter_array = p_tarray_create(Kai_Expr_Procedure_Parameter);
    Kai_u32 ret_count = 0;

    if (t->type != ')') {
parse_parameter:
        p_expect(t->type == T_IDENTIFIER, "in procedure input", "should be an identifier");
        Kai_str name = t->string;
        p_next();
        p_expect(t->type == ':', "in procedure input", "wanted a ':' here");
        p_next();
        Kai_Expr type = _parse_type();
        p_expect(type, "in procedure input", "should be type");
        Kai_Expr_Procedure_Parameter parameter;
        parameter.name = name;
        parameter.type = type;

        if (p_has_stack_overflow(parameter_array)) return NULL;
        p_tarray_add(parameter_array, parameter);
        p_next();

        switch (t->type) {
            case ')': break;
            case ',':
                p_next();
                goto parse_parameter;
            default:
                return p_error_unexpected_s("in procedure input", "wanted ')' or ',' here");
        }
    }
    p_next();

    // return value
    if (t->type == '->') {
        p_next();
        Kai_Expr type = _parse_type();
        p_expect(type, "in procedure return type", "should be type");
        Kai_Expr_Procedure_Parameter parameter;
        parameter.name = KAI_EMPTY_STRING;
        parameter.type = type;
        if (p_has_stack_overflow(parameter_array)) return NULL;
        p_tarray_add(parameter_array, parameter);
        ++ret_count;
        p_next();
    }
    Kai_Expr_Procedure* proc = p_alloc_node(Expr_Procedure);
    proc->input_output = p_alloc_array(Kai_Expr_Procedure_Parameter, parameter_array.count);
    proc->param_count  = parameter_array.count - ret_count;
    proc->ret_count    = ret_count;

    p_tarray_copy(parameter_array, proc->input_output);
    p_tarray_destroy(parameter_array);

    Kai_Stmt body = NULL;
    if (t->type == T_DIRECTIVE &&
        kai_string_equals(KAI_STR("native"), t->string)) {
        p_next();
        p_expect(t->type == ';', "???", "???");
    }
    else {
        body = _parse_stmt(KAI_FALSE);
        if (!body) return NULL;
    }
    proc->body = body;
    return (Kai_Expr) proc;
}

Kai_Expr parse_statement(Parser* parser, Kai_bool is_top_level) {
    D_VERBOSE("top_level = %i", (int)is_top_level);
    Token* t = &parser->tokenizer.current_token;
    D_VERBOSE("token type = %u", t->type);
    switch (t->type) {
    case T_END: if (is_top_level) return NULL;
    default: {
    parse_expression_statement:
        if (is_top_level) {
            return p_error_unexpected_s("in top level statement", "a top level statement");
        }
        Kai_Expr expr = _parse_expr(DEFAULT_PREC);
        p_expect(expr, "in statement", "should be an expression or statement");
        p_next();
        if (t->type == '=') {
            p_next();
            Kai_Expr right = _parse_expr(DEFAULT_PREC);
            p_expect(right, "in assignment statement", "should be an expression");
            Kai_Stmt_Assignment* assignment = p_alloc_node(Stmt_Assignment);
            assignment->left = expr;
            assignment->expr = right;
            expr = (Kai_Expr) assignment;
            p_next();
        }
        p_expect(t->type == ';', "in expression statement", "should be ';' after expression");
        return expr;
    }

    case '{': {
        if (is_top_level)
            return p_error_unexpected_s("in top level statement", "a top level statement");

        p_next();

        // use temperary memory to store the statements we parse.
        Temperary_Array statement_array = p_tarray_create(Kai_Stmt);

        // parse statements until we get a '}'
        while (t->type != '}') {
            Kai_Stmt statement = _parse_stmt(KAI_FALSE);
            if (!statement) return NULL;

            if (p_has_stack_overflow(statement_array)) return NULL;
            p_tarray_add(statement_array, statement);

            p_next(); // eat ';' (get token after)
        }

        Kai_Stmt_Compound* compound = p_alloc_node(Stmt_Compound);
        compound->statements = p_alloc_array(Kai_Stmt, statement_array.count);
        compound->count = statement_array.count;

        p_tarray_copy(statement_array, compound->statements);
        p_tarray_destroy(statement_array);

        return (Kai_Stmt) compound;
    }

    case T_KW_ret: {
        if (is_top_level)
            return p_error_unexpected_s("in top level statement", "a top level statement");
        p_next();

        //! @TODO: fix this [ret statement]
        if (t->type == ';') {
            Kai_Stmt_Return* ret = p_alloc_node(Stmt_Return);
            ret->expr = NULL;
            return (Kai_Stmt) ret;
        }

        Kai_Expr expr = _parse_expr(DEFAULT_PREC);
        p_next();
        p_expect(t->type == ';', "after statement", "there should be a ';' before this");
        Kai_Stmt_Return* ret = p_alloc_node(Stmt_Return);
        ret->expr = expr;
        return (Kai_Stmt) ret;
    }

    case T_KW_if: {
        if (is_top_level)
            return p_error_unexpected_s("is top level statement", "should be within a procedure");
        p_next();
        Kai_Expr expr = _parse_expr(DEFAULT_PREC);
        p_expect(expr, "in if statement", "should be an expression here");
        p_next();
        Kai_Stmt body = _parse_stmt(KAI_FALSE);
        if (!body) return NULL;

        // parse "else"
        Token* p = p_peek();
        Kai_Stmt else_body = NULL;
        if (p->type == T_KW_else) {
            p_next();
            p_next();
            else_body = _parse_stmt(KAI_FALSE);
            if (!else_body) return NULL;
        }
        Kai_Stmt_If* if_statement = p_alloc_node(Stmt_If);
        if_statement->expr      = expr;
        if_statement->body      = body;
        if_statement->else_body = else_body;
        return (Kai_Stmt) if_statement;
    }

    case T_KW_for: {
        if (is_top_level)
            return p_error_unexpected_s("in top level statement", "should be within a procedure");
        p_next();
        p_expect(t->type == T_IDENTIFIER, "in for statement", "should be the name of the iterator");
        Kai_str iterator_name = t->string;
        p_next();
        p_expect(t->type == ':', "in for statement", "should be ':' here");
        p_next();
        Kai_Expr from = _parse_expr(DEFAULT_PREC);
        p_expect(from, "in for statement", "should be an expression here");
        Kai_Expr to = NULL;
        p_next();
        if (t->type == '..') {
            p_next();
            to = _parse_expr(DEFAULT_PREC);
            p_expect(to, "in for statement", "should be an expression here");
            p_next();
        }
        Kai_Stmt body = _parse_stmt(KAI_FALSE);
        if (!body) return NULL;

        Kai_Stmt_For* for_statement = p_alloc_node(Stmt_For);
        for_statement->body = body;
        for_statement->from = from;
        for_statement->to   = to;
        for_statement->iterator_name = iterator_name;
        for_statement->flags = 0;
        return (Kai_Stmt) for_statement;
    }

    case T_IDENTIFIER: {
        Token* p = p_peek();
        // just an expression?
        if (p->type != ':') goto parse_expression_statement;
        
        Kai_str name = t->string;
        Kai_int line_number = t->line_number;
        p_next(); // current = peeked
        p_next(); // see what is after current

        Kai_u32 flags = 0;
        Kai_Expr type = _parse_type();
        if (type) p_next();

        switch (t->type) {
            case ':': flags |= KAI_DECL_FLAG_CONST; break;
            case '=': break;
            default: return p_error_unexpected_s("in declaration", "should be '=' or ':'");
        }

        Kai_Expr expr;
        Kai_bool require_semicolon = KAI_FALSE;
        p_next();
        if (t->type == '(' && is_procedurep_next(parser)) {
            expr = _parse_proc();
            if (!expr) return NULL;
        }
        else {
            expr = _parse_expr(DEFAULT_PREC);
            p_expect(expr, "in declaration", "should be an expression here");
            require_semicolon = KAI_TRUE;
        }
        if (require_semicolon) {
            p_next();
            p_expect(t->type == ';', "after statement", "there should be a ';' before this");
        }
        Kai_Stmt_Declaration* decl = p_alloc_node(Stmt_Declaration);
        decl->expr  = expr;
        decl->type  = type;
        decl->name  = name;
        decl->flags = flags;
        decl->line_number = line_number;
        return (Kai_Stmt) decl;
    }
    }
}

Kai_Result kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* info, Kai_AST* ast) {
    D_VERBOSE("source_code = {%p, %lu}", info->source_code.data, info->source_code.count);
    D_VERBOSE("memory = {");
    D_VERBOSE("\talloc     = %p", info->memory.alloc);
    D_VERBOSE("\ttemp      = %p", info->memory.temperary);
    D_VERBOSE("\ttemp_size = %lu", info->memory.temperary_size);
    D_VERBOSE("\tuser      = %p", info->memory.user);
    D_VERBOSE("}");

    Parser p = {
        .tokenizer = {
            .source = info->source_code,
            .cursor = 0,
            .line_number = 1,
        },
        .stack_top = 0,
        .memory = info->memory,
    };
    Parser* const parser = &p; // ok
    
    // setup first token
    Token* token = p_next();

    // use temperary memory to store the statements we parse
    Temperary_Array statement_array = p_tarray_create(Kai_Stmt);

    while (token->type != T_END) {
        Kai_Stmt statement = _parse_stmt(KAI_TRUE);
        if (!statement || p_has_stack_overflow(statement_array))
            goto error;
        p_tarray_add(statement_array, statement);
        p_next();
    }

    // allocate space for the statements we parsed
    ast->top_level_statements = p_alloc_array(Kai_Stmt, statement_array.count);
    ast->top_level_count = statement_array.count;

    // copy statements to AST
    p_tarray_copy(statement_array, ast->top_level_statements);

    if (KAI_FAILED(parser->error.result)) {
	error:
        if (info->error) {
            *info->error = parser->error;
            info->error->location.file_name = ast->source_filename;
            info->error->location.source = info->source_code.data;
        }
        return parser->error.result;
    }
    return KAI_SUCCESS; 
}

