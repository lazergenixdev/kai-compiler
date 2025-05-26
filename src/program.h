#ifndef PROGRAM__H
#define PROGRAM__H
#include "kai/kai.h"

typedef struct {
	Kai_u32 flags; // (LOCAL_VARIABLE, TYPE)
    Kai_u32 value; // stored index value
} Kai__DG_Node_Index;

#define KAI__DG_NODE_EVALUATED      (1 << 0)
#define KAI__DG_NODE_TYPE           (1 << 0)
#define KAI__DG_NODE_LOCAL_VARIABLE (1 << 1)
#define KAI__SCOPE_GLOBAL_INDEX     0
#define KAI__SCOPE_NO_PARENT        0xFFFFFFFF

typedef union {
    Kai_Value value;
    Kai_Type  type;
	Kai_u32   procedure_location;
} Kai__DG_Value;

typedef KAI__ARRAY(Kai__DG_Node_Index)      Kai__DG_Dependency_Array;
typedef KAI__HASH_TABLE(Kai__DG_Node_Index) Kai__DG_Identifier_Map;

typedef struct {
	Kai_Type                 type;  // evaluated type
	Kai__DG_Value            value; // evaluated value
	Kai__DG_Dependency_Array value_dependencies, type_dependencies;
    Kai_u32                  value_flags,        type_flags;  // (NODE_EVALUATED)
	Kai_str                  name;
	Kai_Expr                 expr;
	Kai_u32                  line_number;
	Kai_u32                  scope_index;
} Kai__DG_Node;

typedef struct {
    Kai__DG_Identifier_Map identifiers;
    Kai_u32                parent;
    Kai_bool               is_proc_scope;
} Kai__DG_Scope;

typedef struct {
	Kai_Error*                error;
    Kai_Allocator             allocator;
	KAI__ARRAY(Kai__DG_Scope) scopes;
	KAI__ARRAY(Kai__DG_Node)  nodes;
	int*                      compilation_order;
} Kai__Dependency_Graph;

typedef struct {
	Kai_u32  location;
	Kai_Type type;
} Kai__Bytecode_Procedure;

typedef KAI__HASH_TABLE(Kai__Bytecode_Procedure) Kai__Bytecode_Procedure_Table;

typedef struct {
	Kai_BC_Stream                 stream; // Global bytecode stream
	Kai__Bytecode_Procedure_Table procedure_table;
} Kai__Bytecode;

typedef struct {
	Kai_Error*        error;
	Kai_Syntax_Tree*  trees;
	Kai_u32           tree_count;
	Kai_Allocator     allocator;
} Kai__Dependency_Graph_Create_Info;

typedef struct {
	Kai_Error*              error;
	Kai__Dependency_Graph*  dependency_graph;
} Kai__Bytecode_Create_Info;

typedef struct {
	Kai_str name;
	Kai_Reg reg;
} Kai__Bytecode_Register;

typedef struct {
	Kai_Error*                   error;
	Kai__Dependency_Graph*       dependency_graph;
	Kai__Bytecode*               bytecode;
	Kai__Dynamic_Arena_Allocator arena;
	KAI__ARRAY(Kai__Bytecode_Register) registers;
} Kai__Bytecode_Generation_Context;

typedef struct {
	Kai_Error*     error;
	Kai_Allocator  allocator;
    Kai__Bytecode* bytecode;
} Kai__Program_Create_Info;

Kai_Result kai__create_dependency_graph(
	Kai__Dependency_Graph_Create_Info* Info,
	Kai__Dependency_Graph*             out_Graph);

Kai_Result kai__determine_compilation_order(Kai__Dependency_Graph* Graph, Kai_Error* out_Error);
Kai_Result kai__generate_bytecode(Kai__Bytecode_Create_Info* Info, Kai__Bytecode* out_Bytecode);
Kai_Result kai__create_program(Kai__Program_Create_Info* Info, Kai_Program* out_Program);

void kai__destroy_dependency_graph(Kai__Dependency_Graph* Graph);
void kai__destroy_bytecode(Kai__Bytecode* Bytecode);

Kai_Result kai__dg_insert_value_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr,
    Kai_bool                  In_Procedure);

Kai_Result kai__dg_insert_type_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr);

#endif // PROGRAM__H
