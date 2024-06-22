#pragma once
#include <vector>
#include <unordered_map>
#include <optional>
#include <kai/parser.h>
#include "config.hpp"
#include "bytecode_interpreter.hpp"
#include "builtin_types.hpp"

struct Node_Reference {
    enum {
        VALUE = 0,
        TYPE  = 1,
    };

    u32 type;
    u32 node_index;

    constexpr auto operator<=>(Node_Reference const&) const = default;
};

enum Node_Flags: u32 {
	Evaluated              = KAI_BIT(0),
	Has_No_Self_Dependency = KAI_BIT(1),
	Is_Local_Variable      = KAI_BIT(2),
};

using Dependency_List = std::vector<Node_Reference>;


struct Type_Node {
    Dependency_List dependencies;
    u32 flags;
	kai_Type type;  // evaluated type
};
struct Value_Node {
    Dependency_List dependencies;
    u32 flags;
	Register value; // evaluated value
};
struct Info_Node {
	std::string_view name;
	kai_Expr expr; // AST node to be evaluated
	u32 line_number;
	u32 scope;
};

#define GLOBAL_SCOPE 0
#define LOCAL_VARIABLE_INDEX -1
struct Scope {
    // maps Identifiers to Dependency Nodes
	std::unordered_map<std::string_view, u32> map;
//	std::vector<Procedure_Overload> overloaded_procedures;
	u32 parent = -1;
	bool is_proc_scope = false;
};

struct Dependency_Graph {
	std::vector<Scope>            scopes;
	std::vector<Value_Node>  value_nodes;
	std::vector<Type_Node>    type_nodes;
	std::vector<Info_Node>    info_nodes;

    void insert_builtin_types();
    void insert_builtin_procedures();

	no_discard bool create(kai_AST* tree);

	void insert_procedure_input(int arg_index, kai_Type type, std::string_view name, u32 scope_index);
	void insert_local_variable(u32 reg_index, kai_Type type, std::string_view name, u32 scope_index);
	
	no_discard
	std::optional<u32>
	resolve_dependency_node(std::string_view name, u32 scope_index);

	void debug_print();
private:
	no_discard
    bool insert_node_for_statement(
		kai_Stmt statement,
		bool in_proc,
		u32 scope_index = GLOBAL_SCOPE
	);

	no_discard
    bool insert_node_for_declaration(
		kai_Stmt_Declaration* decl,
		bool in_proc,
		u32 scope_index = GLOBAL_SCOPE
	);

	no_discard
	bool insert_value_dependencies(
		Dependency_List& deps,
		u32 scope_index,
		kai_Expr expr,
		bool in_procedure
	);

	no_discard
	bool insert_type_dependencies(
		Dependency_List& deps,
		u32 scope_index,
		kai_Expr expr
	);

	no_discard
	std::optional<u32>
	resolve_dependency_node_procedure(std::string_view name, Scope* scope);
};
