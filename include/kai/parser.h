#ifndef KAI_PARSER_H
#define KAI_PARSER_H
#include "core.h"
__KAI_BEGIN_API__

typedef struct Kai_Expr_Base* Kai_Expr; // Expression Nodes
typedef struct Kai_Expr_Base* Kai_Stmt; // Statement Nodes

// Used to describe number literals
// Value = ( Whole + Frac / (10 ^ Frac_Denom) ) * 10 ^ Exp
typedef struct {
	Kai_u64 Whole_Part;
	Kai_u64 Frac_Part;
	Kai_s32 Exp_Part;
	Kai_u16 Frac_Denom;
} Kai_Number_Info;

typedef struct {
	Kai_Stmt* top_level_statements;
	Kai_int   top_level_count;
	Kai_str   source_filename;
} Kai_AST;

typedef struct {
	Kai_str         source_code;
	Kai_Memory      memory;
	Kai_Error*      error;
} Kai_Syntax_Tree_Create_Info;


/* -------------------------------------------------------------------
	info: [in]
	AST:  [in] [out]
		- source_filename: expected to contain the file name of the
		                    source code

	On success, out_AST will be filled with the syntax tree of the
	source code.
   ------------------------------------------------------------------- */
KAI_API(Kai_Result)
	kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* Info, Kai_AST* AST);



// ==========================> Syntax Tree Nodes <=============================

typedef enum {
	KAI_EXPR_IDENTIFIER     = 0,
	KAI_EXPR_STRING         = 1,
	KAI_EXPR_NUMBER         = 2,
	KAI_EXPR_BINARY         = 3,
	KAI_EXPR_UNARY          = 4,
	KAI_EXPR_PROCEDURE_TYPE = 5,
	KAI_EXPR_PROCEDURE_CALL = 6,
	KAI_EXPR_PROCEDURE      = 7, // defines a procedure, e.g. "(a: int, b: int) -> int { ret a + b; }"

	KAI_STMT_RETURN         = 8,
	KAI_STMT_DECLARATION    = 9,
	KAI_STMT_ASSIGNMENT     = 10,
	KAI_STMT_COMPOUND       = 11,
	KAI_STMT_IF             = 12,
	KAI_STMT_FOR            = 13,
	KAI_STMT_DEFER          = 20,
} Kai_Node_ID;

// `line_number` is the line number for the first token in an Expression/Statement
// am I a good programmer?
#define KAI_BASE_MEMBERS \
	Kai_u8  id;          \
	Kai_u32 line_number; \
	Kai_str source_code
//	Kai_Type_Info* type

typedef struct Kai_Expr_Base {
	KAI_BASE_MEMBERS;
} Kai_Expr_Base;

typedef struct {
	KAI_BASE_MEMBERS;
} Kai_Expr_Identifier;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Number_Info info;
} Kai_Expr_Number;

typedef struct {
	KAI_BASE_MEMBERS;
} Kai_Expr_String;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr expr;
	Kai_u32  op;
} Kai_Expr_Unary;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr left;
	Kai_Expr right;
	Kai_u32  op;
} Kai_Expr_Binary;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr  proc;
	Kai_Expr* arguments;
	Kai_u8    arg_count;
} Kai_Expr_Procedure_Call;
 
typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr* input_output;
	Kai_u8    param_count;
	Kai_u8    ret_count; // If this is 0, then the return type defaults to "void"
} Kai_Expr_Procedure_Type;

typedef struct {
	Kai_str  name;
	Kai_Expr type;
	Kai_u8   flags; // note: for keyword using
} Kai_Expr_Procedure_Parameter;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr_Procedure_Parameter* input_output;
	Kai_Stmt body;
	Kai_u8   param_count;
	Kai_u8   ret_count; // If this is 0, then the return defaults to "void"

	Kai_u32 _scope;
} Kai_Expr_Procedure;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr expr; // optional
} Kai_Stmt_Return;

enum {
	KAI_DECL_FLAG_CONST = 1 << 0, // compile-time constant, not like C const
	KAI_DECL_FLAG_USING = 1 << 1,
};

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr expr;
    Kai_Expr type; // optional
	Kai_str  name;
	Kai_u8   flags;
} Kai_Stmt_Declaration;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr left;
	Kai_Expr expr;
} Kai_Stmt_Assignment;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Stmt* statements;
	Kai_u32   count;

	Kai_u32 _scope;
} Kai_Stmt_Compound;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Expr expr;
	Kai_Stmt body;
	Kai_Stmt else_body;
} Kai_Stmt_If;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Stmt body;
	Kai_Expr from;
	Kai_Expr to; // optional (interates through `from` if this is null)
	Kai_str  iterator_name;
	Kai_u8   flags;
} Kai_Stmt_For;

__KAI_END_API__
#endif//KAI_PARSER_H
