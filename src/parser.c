#include "config.h"

// Optimization Idea:
// 2 pass parser,
//   first pass to check for errors and calcuate memory required.
//   then allocate memory required.
//   second pass to store syntax tree.
// (also enables smaller syntax tree size, everything in
//    one memory allocation, so could use 32 bit pointers)
// Would make parser slower, but could
//   save space/be more cache friendly for later stages.
// .. But this would require a lot of duplicate code,
//     so not going to implement.
// NOTE: how would that even fit in with tree rewriting??

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
    
extern inline Kai_Expr alloc_type(Kai__Parser* p, Kai_u32 size, Kai_u8 id) {
    Kai_Expr expr = kai__arena_allocate(&p->arena, size);
    expr->id = id;
    return expr;
}

#define p_error_unexpected_s(WHERE, CONTEXT) \
    kai__error_unexpected(parser, &parser->tokenizer.current_token, KAI_STRING(WHERE), KAI_STRING(CONTEXT)), NULL

#define p_expect(OK, WHERE, CONTEXT) \
    if (!(OK)) return p_error_unexpected_s(WHERE, CONTEXT)

#define p_next()           kai__next_token(&parser->tokenizer)
#define p_peek()           kai__peek_token(&parser->tokenizer)
#define _parse_expr(PREC)  parse_expression(parser, PREC)
#define _parse_stmt(TOP)   parse_statement(parser, TOP)
#define _parse_proc()      parse_procedure(parser)
#define _parse_type()      parse_type(parser)

Kai_Expr parse_type(Kai__Parser* parser);
Kai_Expr parse_expression(Kai__Parser* parser, int precedence);
Kai_Expr parse_procedure(Kai__Parser* parser);
Kai_Stmt parse_statement(Kai__Parser* parser, Kai_bool is_top_level);

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
    case '&&': return (Op_Info){t, 0x0010,    OP_BINARY};
    case '||': return (Op_Info){t, 0x0010,    OP_BINARY};
    case '==': return (Op_Info){t, 0x0040,    OP_BINARY};
    case '<=': return (Op_Info){t, 0x0040,    OP_BINARY};
    case '>=': return (Op_Info){t, 0x0040,    OP_BINARY};
    case '<':  return (Op_Info){t, 0x0040,    OP_BINARY};
    case '>':  return (Op_Info){t, 0x0040,    OP_BINARY};
    case '->': return (Op_Info){t, CAST_PREC, OP_BINARY};
    case '+':  return (Op_Info){t, 0x0100,    OP_BINARY};
    case '-':  return (Op_Info){t, 0x0100,    OP_BINARY};
    case '*':  return (Op_Info){t, 0x0200,    OP_BINARY};
    case '/':  return (Op_Info){t, 0x0200,    OP_BINARY};
    case '[':  return (Op_Info){t, 0xBEEF,    OP_INDEX};
    case '(':  return (Op_Info){t, 0xBEEF,    OP_PROCEDURE_CALL};
    case '.':  return (Op_Info){t, 0xFFFF,    OP_BINARY}; // member access
    default:   return (Op_Info){t, 0};
    }
}

Kai_bool is_procedurep_next(Kai__Parser* parser) {
    Kai__Token* p = p_peek();
    if (p->type == ')') return KAI_TRUE;
    Kai__Tokenizer state = parser->tokenizer;
    Kai__Token* cur = p_next();
    Kai_bool found = KAI_FALSE;

    // go until we hit ')' or END, searching for ':'
    while (cur->type != KAI__TOKEN_END && cur->type != ')') {
        if (cur->type == ':') {
            found = KAI_TRUE;
            break;
        }
        p_next();
    }
    parser->tokenizer = state;
    return found;
}

Kai_Expr parse_type(Kai__Parser* parser) {
    Kai__Token* t = &parser->tokenizer.current_token;
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

    Kai_u8 in_count = 0;
    Kai_u8 out_count = 0;
    Kai_Expr current = NULL;
    Kai_Expr head = NULL;

    // Parse Procedure input types
    if (t->type != ')') for (;;) {
        Kai_Expr type = _parse_type();
        if (!type) return NULL;

        if (!head) head = type;
        else current->next = type;
        current = type;

        p_expect(in_count != 255, "in procedure type", "too many inputs to procedure");
        in_count += 1;

        p_next(); // get ',' token or ')'

        if (t->type == ')') break;
        if (t->type == ',') p_next();
        else return p_error_unexpected_s("in procedure type", "',' or ')' expected here");
    }
    
    Kai__Token* peek = p_peek(); // see if we have any returns

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
            if (!type) return NULL;

            if (!head) head = type;
            else current->next = type;
            current = type;

            p_expect(out_count != 255, "in procedure type", "too many inputs to procedure");
            out_count += 1;

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
    proc->in_out_expr = head;
    proc->in_count = in_count;
    proc->out_count = out_count;
    return (Kai_Expr) proc;
}

Kai_Expr parse_expression(Kai__Parser* parser, int prec) {
    Kai_Expr left = NULL;
    Kai__Token* t = &parser->tokenizer.current_token;

    switch (t->type) {
    ////////////////////////////////////////////////////////////
    // Handle Parenthesis
    case '(': {
        p_next();
        left = _parse_expr(DEFAULT_PREC);
        p_expect(left, "in expression", "should be an expression here");
        p_next();
        p_expect(t->type == ')', "in expression", "an operator or ')' in expression");
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Unary Operators
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
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Explicit Casting "cast(int) x"
    case KAI__TOKEN_cast: {
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
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Directives
    case KAI__TOKEN_DIRECTIVE: {
        if (kai_str_equals(KAI_STRING("type"), t->string)) {
            p_next();
            left = _parse_type();
            p_expect(left, "in expression", "type");
        }
        else if (kai_str_equals(KAI_STRING("string"), t->string)) {
            //p_next();
            //tok_start_raw_string(&parser->tokenizer);
            //p_next();
            //tok_end_raw_string(&parser->tokenizer);
            //Kai_Expr_String* str = p_alloc_node(Expr_String);
            //str->line_number = t->line_number;
            //str->source_code = t->string;
            //left = (Kai_Expr) str;
            return p_error_unexpected_s("", "not implemented");
        }
        else if (kai_str_equals(KAI_STRING("Julie"), t->string)) {
            Kai_Expr_String* str = p_alloc_node(Expr_String);
            str->line_number = t->line_number;
            str->source_code = KAI_STRING("<3");
            left = (Kai_Expr) str;
        }
        else return NULL;
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Single-Token Expressions
    case KAI__TOKEN_IDENTIFIER: {
        Kai_Expr_Identifier* ident = p_alloc_node(Expr_Identifier);
        ident->line_number = t->line_number;
        ident->source_code = t->string;
        left = (Kai_Expr) ident;
    } break;
    case KAI__TOKEN_NUMBER: {
        Kai_Expr_Number* num = p_alloc_node(Expr_Number);
        num->value       = t->number;
        num->line_number = t->line_number;
        num->source_code = t->string;
        left = (Kai_Expr) num;
    } break;
    case KAI__TOKEN_STRING: {
        Kai_Expr_String* str = p_alloc_node(Expr_String);
        str->line_number = t->line_number;
        str->source_code = t->string;
        left = (Kai_Expr) str;
    } break;
    default: return NULL;
    }

loop_back:
    (void)0;
    Kai__Token* p = p_peek();
    Op_Info op_info = operator_of_token_type(p->type);

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
    } break;

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
    } break;

    case OP_PROCEDURE_CALL: {
        Kai_Expr head = NULL;
        Kai_Expr current = NULL;
        Kai_u8 arg_count = 0;
        if (t->type != ')') for (;;) {
            Kai_Expr expr = _parse_expr(DEFAULT_PREC);
            p_expect(expr, "[todo]", "an expression");

            if (!head) head = expr;
            else current->next = expr;
            current = expr;

            p_expect(arg_count != 255, "in procedure call", "too many inputs to procedure");
            arg_count += 1;

            p_next();
            if (t->type == ')') break;
            if (t->type == ',') p_next();
            else return p_error_unexpected_s("in procedure call", "',' or ')' expected here");
        }
        Kai_Expr_Procedure_Call* call = p_alloc_node(Expr_Procedure_Call);
        call->proc = left;
        call->arg_head = head;
        call->arg_count = arg_count;
        left = (Kai_Expr) call;
    } break;

    default: panic_with_message("u fucked up");

    }
    goto loop_back;
}

Kai_Expr parse_procedure(Kai__Parser* parser) {
    Kai__Token* t = &parser->tokenizer.current_token;
    // sanity check
    p_expect(t->type == '(', "", "this is likely a compiler bug, sorry :c");
    p_next();

    Kai_u8 in_count = 0;
    Kai_u8 out_count = 0;
    Kai_Expr head = NULL;
    Kai_Expr current = NULL;

    if (t->type != ')') {
parse_parameter:
        (void)0;
        Kai_u8 flags = 0;
        if (t->type == KAI__TOKEN_using) {
            flags |= 1;//KAI_PARAMETER_FLAG_USING;
            p_next();
        }
        p_expect(t->type == KAI__TOKEN_IDENTIFIER, "in procedure input", "should be an identifier");
        Kai_str name = t->string;
        p_next();
        p_expect(t->type == ':', "in procedure input", "wanted a ':' here");
        p_next();
        Kai_Expr type = _parse_type();
        p_expect(type, "in procedure input", "should be type");

        type->name = name;
        type->flags = flags;

        if (!head) head = type;
        else current->next = type;
        current = type;
        
        p_expect(in_count != 255, "in procedure call", "too many inputs to procedure");
        in_count += 1;

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

        if (!head) head = type;
        else current->next = type;
        current = type;

        p_expect(out_count != 255, "in procedure call", "too many inputs to procedure");
        out_count += 1;

        p_next();
    }
    Kai_Expr_Procedure* proc = p_alloc_node(Expr_Procedure);
    proc->in_out_expr = head;
    proc->in_count = in_count;
    proc->out_count = out_count;

    Kai_Stmt body = NULL;
    if (t->type == KAI__TOKEN_DIRECTIVE &&
        kai_str_equals(KAI_STRING("native"), t->string)) {
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

Kai_Expr parse_statement(Kai__Parser* parser, Kai_bool is_top_level) {
    Kai__Token* t = &parser->tokenizer.current_token;
    switch (t->type) {
    case KAI__TOKEN_END: if (is_top_level) return NULL;
    default: {
    parse_expression_statement:
        if (is_top_level) {
            return p_error_unexpected_s("in top level", "should be a top level statement");
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

        Kai_Stmt head = NULL;
        Kai_Stmt current = NULL;

        // parse statements until we get a '}'
        while (t->type != '}') {
            Kai_Stmt statement = _parse_stmt(KAI_FALSE);
            if (!statement) return NULL;

            if (!head) head = statement;
            else current->next = statement;
            current = statement;

            p_next(); // eat ';' (get token after)
        }

        // TODO: is this OK?
        // No need for compound if there is only one statement
        //if (statement_array.count == 1) {
        //    p_tarray_destroy(statement_array);
        //    return *(Kai_Stmt*)((Kai_u8*)parser->memory.temperary + statement_array.offset);
        //}

        Kai_Stmt_Compound* compound = p_alloc_node(Stmt_Compound);
        compound->head = head;
        return (Kai_Stmt) compound;
    }

    case KAI__TOKEN_ret: {
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
        p_expect(expr, "in return statement", "should be an expression");
        p_next();
        p_expect(t->type == ';', "after statement", "there should be a ';' before this");
        Kai_Stmt_Return* ret = p_alloc_node(Stmt_Return);
        ret->expr = expr;
        return (Kai_Stmt) ret;
    }

    case KAI__TOKEN_if: {
        if (is_top_level)
            return p_error_unexpected_s("is top level statement", "should be within a procedure");
        p_next();
        Kai_Expr expr = _parse_expr(DEFAULT_PREC);
        p_expect(expr, "in if statement", "should be an expression here");
        p_next();
        Kai_Stmt body = _parse_stmt(KAI_FALSE);
        if (!body) return NULL;

        // parse "else"
        Kai__Token* p = p_peek();
        Kai_Stmt else_body = NULL;
        if (p->type == KAI__TOKEN_else) {
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

    case KAI__TOKEN_for: {
        if (is_top_level)
            return p_error_unexpected_s("in top level statement", "should be within a procedure");
        p_next();
        p_expect(t->type == KAI__TOKEN_IDENTIFIER, "in for statement", "should be the name of the iterator");
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

    case KAI__TOKEN_IDENTIFIER: {
        Kai__Token* p = p_peek();
        // just an expression?
        if (p->type != ':') goto parse_expression_statement;
        
        Kai_str name = t->string;
        Kai_u32 line_number = t->line_number;
        p_next(); // current = peeked
        p_next(); // see what is after current

        Kai_u8 flags = 0;
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

Kai_Result kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* info, Kai_Syntax_Tree* tree)
{
    Kai__Parser p = {
        .tokenizer = {
            .source = info->source_code,
            .cursor = 0,
            .line_number = 1,
        },
    };
    Kai__Parser* const parser = &p;
    
    kai__dynamic_arena_allocator_create(&p.arena, &info->allocator);

    // setup first token
    Kai__Token* token = p_next();

    Kai_Stmt current = NULL;
    Kai_Stmt head = NULL;

    while (token->type != KAI__TOKEN_END) {
        Kai_Stmt statement = _parse_stmt(KAI_TRUE);
        if (!statement) break;
        if (!head) head = statement;
        else current->next = statement;
        current = statement;
        p_next();
    }

    tree->allocator = p.arena;
    tree->root.id = KAI_STMT_COMPOUND;
    tree->root.head = head;

    if (p.error.result != KAI_SUCCESS) {
        if (info->error) {
            *info->error = p.error;
            info->error->location.file_name = tree->source_filename;
            info->error->location.source = info->source_code.data;
        }
        return p.error.result;
    }
    return KAI_SUCCESS; 
}

void kai_destroy_syntax_tree(Kai_Syntax_Tree* tree)
{
    kai__dynamic_arena_allocator_destroy(&tree->allocator);
    *tree = (Kai_Syntax_Tree) {0};
}
