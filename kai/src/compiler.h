#include "kai/parser.h"
#include "config.h"

#define TYPE_BIT (1u << 31)
typedef u32 Node_Ref;

#define array(T) struct { Kai_u32 count; Kai_u32 capacity; T* data; }

typedef union {
#define X(NAME, UNUSED) NAME _##NAME;
	XPRIMITIVE_TYPES
#undef X
	Kai_Type _Type;
} Value;

enum {
	NODE_EVALUATED      = 1 << 0,
	NODE_LOCAL_VARIABLE = 1 << 1,
};
typedef struct {
	Kai_Type   type;  // evaluated type
	Value      value; // evaluated value
	Kai_range  value_dependencies;
    Kai_range   type_dependencies;
    u32        value_flags;
    u32         type_flags;
	Kai_str    name;
	Kai_Expr   expr; // AST node to be evaluated
	u32        line_number;
	u32        scope;
} Node;

typedef struct {
	Kai_str name;
	u32     index;
} Named_Node_Ref;

#define GLOBAL_SCOPE 0
#define NONE ((u32)-1)
typedef struct {
	array(Named_Node_Ref) identifiers;
	u32 parent;
	Kai_bool is_proc_scope;
} Scope;

typedef struct {
	array(Scope)     scopes;
	array(Node)      nodes;
	array(Node_Ref)  dependencies;
} Dependency_Graph;

typedef struct {
	Kai_Memory       memory;
	Kai_Error        error;
	Dependency_Graph dependency_graph;
} Compiler_Context;

typedef struct {
	Compiler_Context* compiler;
    u32*              post;
    u32*              prev;
    Kai_bool*         visited;
    u32               next;
} DFS_Context;

typedef struct {
	int asdf;
} Type_Checker;


#define c_insert_node_for_statement(STATEMENT,SCOPE_INDEX,IN_PROC,FROM_PROC) \
insert_node_for_statement(context, STATEMENT, SCOPE_INDEX, IN_PROC, FROM_PROC)

Kai_bool insert_node_for_statement(
	Compiler_Context*  context,
	Kai_Stmt           statement,
	u32                scope_index,
	Kai_bool           in_procedure,
	Kai_bool           from_procedure
);
#define c_insert_node_for_declaration(DECL,IN_PROC,SCOPE_INDEX) \
insert_node_for_declaration(context, DECL, IN_PROC, SCOPE_INDEX)

Kai_bool insert_node_for_declaration(
	Compiler_Context* context,
	Kai_Stmt_Declaration* decl,
	Kai_bool in_proc,
	u32 scope_index
);

#define c_insert_value_dependencies(DEPS,SCOPE_INDEX,EXPR,IN_PROC) \
insert_value_dependencies(context, DEPS, SCOPE_INDEX, EXPR, IN_PROC)

Kai_bool insert_value_dependencies(
    Compiler_Context* context,
	Kai_range*        deps,
    u32               scope_index,
    Kai_Expr          expr,
    Kai_bool          in_procedure
);

#define c_insert_type_dependencies(DEPS,SCOPE_INDEX,EXPR) \
insert_type_dependencies(context, DEPS, SCOPE_INDEX, EXPR)

Kai_bool insert_type_dependencies(
    Compiler_Context* context,
	Kai_range*        deps,
    u32               scope_index,
    Kai_Expr          expr
);