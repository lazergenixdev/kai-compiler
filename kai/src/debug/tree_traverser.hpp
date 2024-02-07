#include <vector>
#include <kai/parser.h>
#include "../config.hpp"

// This is terribly inefficient :(
// where is my zero cost abstraction??
struct Syntax_Tree_Traverser {

	void visit(kai_Expr node, u8 is_last = 0) {
        if (visit_begin(node, is_last))
        switch (node->id)
        {
        case kai_Expr_ID_Identifier:
            visit_identifier((kai_Expr_Identifier*)node);
        break;

        case kai_Expr_ID_String:
            visit_string((kai_Expr_String*)node);
        break;

        case kai_Expr_ID_Number:
            visit_number((kai_Expr_Number*)node);
        break;

        case kai_Expr_ID_Binary:
            visit_binary((kai_Expr_Binary*)node);
        break;
        
        case kai_Expr_ID_Unary:
            visit_unary((kai_Expr_Unary*)node);
        break;

        case kai_Expr_ID_Procedure_Type:
            visit_procedure_type((kai_Expr_Procedure_Type*)node);
        break;

        case kai_Expr_ID_Procedure_Call:
            visit_procedure_call((kai_Expr_Procedure_Call*)node);
        break;

        case kai_Expr_ID_Procedure:
            visit_procedure((kai_Expr_Procedure*)node);
        break;

        case kai_Stmt_ID_Return:
            visit_return((kai_Stmt_Return*)node);
        break;

        case kai_Stmt_ID_Declaration:
            visit_declaration((kai_Stmt_Declaration*)node);
        break;

        case kai_Stmt_ID_Assignment:
            visit_assignment((kai_Stmt_Assignment*)node);
        break;

        case kai_Stmt_ID_Compound:
	        visit_compound((kai_Stmt_Compound*)node);
        break;

        case kai_Stmt_ID_If:
	        visit_if((kai_Stmt_If*)node);
        break;

        case kai_Stmt_ID_For:
	        visit_for((kai_Stmt_For*)node);
        break;

        default:
            visit_unknown(node);
        break;
        }

        visit_end();
	}

    virtual bool visit_begin(kai_Expr node, u8 is_last) { return node != nullptr; }
    virtual void visit_end() {}

	virtual void visit_unknown        (kai_Expr                  node) = 0;
	virtual void visit_identifier     (kai_Expr_Identifier     * node) = 0;
    virtual void visit_string         (kai_Expr_String         * node) = 0;
	virtual void visit_number         (kai_Expr_Number         * node) = 0;
	virtual void visit_binary         (kai_Expr_Binary         * node) = 0;
	virtual void visit_unary          (kai_Expr_Unary          * node) = 0;
    virtual void visit_procedure_type (kai_Expr_Procedure_Type * node) = 0;
    virtual void visit_procedure_call (kai_Expr_Procedure_Call * node) = 0;
	virtual void visit_procedure      (kai_Expr_Procedure      * node) = 0;
    virtual void visit_return         (kai_Stmt_Return         * node) = 0;
	virtual void visit_declaration    (kai_Stmt_Declaration    * node) = 0;
	virtual void visit_assignment     (kai_Stmt_Assignment     * node) = 0;
	virtual void visit_compound       (kai_Stmt_Compound       * node) = 0;
	virtual void visit_if             (kai_Stmt_If             * node) = 0;
	virtual void visit_for            (kai_Stmt_For            * node) = 0;
};

struct Type_Tree_Traverser {

	void visit(kai_Type node, u8 is_last = 0) {
        if (visit_begin(node, is_last))
        switch (node->type)
        {
        case kai_Type_Type:
            visit_type(node);
        break;

        case kai_Type_Integer:
            visit_integer((kai_Type_Info_Integer*) node);
        break;
        
        case kai_Type_Float:
            visit_float((kai_Type_Info_Float*) node);
        break;
        
        case kai_Type_Pointer:
            visit_pointer((kai_Type_Info_Pointer*) node);
        break;
        
        case kai_Type_Procedure:
            visit_procedure((kai_Type_Info_Procedure*) node);
        break;

        default:
            visit_unknown(node);
        break;
        }

        visit_end();
	}

    virtual bool visit_begin(kai_Type node, u8 is_last) { return node != nullptr; }
    virtual void visit_end() {}

	virtual void visit_unknown   (kai_Type                  node) = 0;
	virtual void visit_type      (kai_Type                  node) = 0;
	virtual void visit_integer   (kai_Type_Info_Integer   * node) = 0;
	virtual void visit_float     (kai_Type_Info_Float     * node) = 0;
	virtual void visit_pointer   (kai_Type_Info_Pointer   * node) = 0;
	virtual void visit_procedure (kai_Type_Info_Procedure * node) = 0;
};
