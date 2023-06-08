#ifndef KAI_PARSER_H
#define KAI_PARSER_H
#include <kai/core.h>
__KAI_BEGIN_API__

// Notes:
// "Expr" => Expression
// "Stmt" => Statement

typedef struct kai_Expr_Base* kai_Expr;
typedef struct kai_Expr_Base* kai_Stmt; // Same as Expr_Base

// Used to describe number literals
// Value = ( Whole + Frac / (10 ^ Frac_Denom) ) * 10 ^ Exp
typedef struct {
	kai_u64 Whole_Part;
	kai_u64 Frac_Part;
	kai_s32 Exp_Part;
	kai_u16 Frac_Denom;
} kai_Number_Info;

typedef struct {
	kai_Stmt* toplevel_stmts;
	kai_int   toplevel_count;
	kai_Memory memory;
} kai_Module;

typedef struct {
	kai_str         source;
	kai_str         filename;
	kai_Module*     module;
	kai_Error_Info* error_info;
} kai_Syntax_Tree_Create_Info;

KAI_API kai_result kai_create_syntax_tree(kai_Syntax_Tree_Create_Info* info);

KAI_API void kai_Lib_print_syntax_tree(kai_Module* mod);

KAI_API void kai_create_error_message(kai_Error_Info);

typedef enum: kai_u32 {
	kai_Expr_ID_Identifier,
	kai_Expr_ID_String,
	kai_Expr_ID_Number,
	kai_Expr_ID_Binary,
	kai_Expr_ID_Unary,
	kai_Expr_ID_Procedure_Type,
	kai_Expr_ID_Procedure_Call,
	kai_Expr_ID_Procedure, // defines a procedure, e.g. "(a: int, b: int) -> int { ret a + b; }"

	kai_Stmt_ID_Expression, // use?
	kai_Stmt_ID_Return,
	kai_Stmt_ID_Declaration,
	kai_Stmt_ID_Compound,

} kai_Node_ID;

typedef struct {
	kai_u32 line_number;
	kai_str source_code; // source code for this Expression/Statement/Type/...
	                     //   example: "x == 0" for Binary Expression
} kai_Source_Location;

#define KAI_BASE_MEMBERS \
	kai_u32 id;          \
	kai_u32 line_number; \
	kai_str source_code; \
	kai_Type_Info* type

//////////////////////////////////////////////////
// Expressions

typedef struct kai_Expr_Base {
	KAI_BASE_MEMBERS;
} kai_Expr_Base;

typedef struct {
	KAI_BASE_MEMBERS;
} kai_Expr_Identifier;

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Number_Info info;
} kai_Expr_Number;

typedef struct {
	KAI_BASE_MEMBERS;
} kai_Expr_String;

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Expr      expr;
	kai_u32       op;
} kai_Expr_Unary;

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Expr left;
	kai_Expr right;
	kai_u32  op;
} kai_Expr_Binary;

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Expr  proc;
	kai_Expr* arguments;
	kai_u32   arg_count;
} kai_Expr_Procedure_Call;
 
typedef struct {
	KAI_BASE_MEMBERS;
	kai_Expr* types;
	kai_u32 parameter_count;
	kai_u32 ret_count; // If this is 0, then the return type defaults to "void"
} kai_Expr_Procedure_Type;

typedef struct {
	kai_str  name;
	kai_Expr type;
//	kai_u32  flags; // for keyword using
} kai_Expr_Procedure_Parameter;

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Expr_Procedure_Parameter* input_output;
	kai_u32 param_count;
	kai_u32 ret_count; // If this is 0, then the return defaults to "void"
	kai_Stmt body;

	kai_u32 _scope;
} kai_Expr_Procedure;

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Expr expr;
} kai_Stmt_Return;

enum: kai_u32 {
	kai_Decl_Flag_Const = KAI_BIT(0), // compile-time constant, not like C const
	kai_Decl_Flag_Using = KAI_BIT(1),
};

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Expr expr;
	kai_str name;
	kai_u32 flags;
} kai_Stmt_Declaration;

typedef struct {
	KAI_BASE_MEMBERS;
	kai_Stmt* statements;
	kai_int   count;

	kai_u32 _scope;
} kai_Stmt_Compound;

//////////////////////////////////////////////////


__KAI_END_API__
#ifdef  KAI_CPP_API
namespace kai {

}
#endif//KAI_CPP_API
#endif//KAI_PARSER_H