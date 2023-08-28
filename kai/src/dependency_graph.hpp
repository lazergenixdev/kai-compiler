#pragma once
#include "config.hpp"
#include <kai/parser.h>
#include "bytecode_interpreter.hpp"
#include <vector>
#include <unordered_map>
#include <optional>

struct Dependency {
    enum {
        VALUE,
        TYPE,
    };

    u32 type;
    u32 node_index;

    constexpr bool operator==(Dependency const&) const = default;
};

enum Dependency_Flags: u32 {
	Evaluated = KAI_BIT(0),
	Has_No_Self_Dependency = KAI_BIT(1),
};

using Dependency_List = std::vector<Dependency>;

struct Dependency_Node {
    u32 flags;
    Dependency_List dependencies;
};

struct Type_Dependency_Node : public Dependency_Node {
	kai_Type type;  // evaluated type
};
struct Value_Dependency_Node : public Dependency_Node {
	union {
		Register value; // evaluated value
		u32 bytecode_index;
	};
};
struct Dependency_Node_Info {
	std::string_view name;
	kai_Expr expr; // AST node to be evaluated
	u32 line_number;
	u32 scope;
};

struct Procedure_Overload {
	kai_str name;
	u32     node_index;
};

#define GLOBAL_SCOPE 0
#define LOCAL_VARIABLE_INDEX -1
struct Scope {
    // maps Identifiers to Dependency Nodes
	std::unordered_map<std::string_view, u32> map;
	std::vector<Procedure_Overload> overloaded_procedures;
	u32 parent = -1;
	bool is_proc_scope = false;
};

struct Dependency_Graph {
	std::vector<Scope> scopes;
	std::vector<Value_Dependency_Node> value_nodes;
	std::vector<Type_Dependency_Node>   type_nodes;
	std::vector<Dependency_Node_Info> node_infos;

    void insert_builtin_types();
    void insert_builtin_procedures();

    bool create(kai_Module* src);

	bool all_evaluated(Dependency_List const& deps) {
		Dependency_Node const* node = nullptr;
		for (auto&& d : deps) {
			switch (d.type)
			{
			default:
				panic_with_message("something got corrupted.."); break;
			break; case Dependency::VALUE:
				node = &value_nodes[d.node_index];
			break; case Dependency::TYPE:
				node = &type_nodes[d.node_index];
			}
			if (!(node->flags & Evaluated))
				return false;
		}
		return true;
	}

	void debug_print();
private:
	NO_DISCARD
    bool insert_node_for_statement(
		kai_Stmt statement,
		bool in_proc,
		u32 scope_index = GLOBAL_SCOPE
	);

	NO_DISCARD
    bool insert_node_for_declaration(
		kai_Stmt_Declaration* decl,
		bool in_proc,
		u32 scope_index = GLOBAL_SCOPE
	);

	NO_DISCARD
	bool insert_value_dependencies(
		Dependency_List& deps,
		u32 scope_index,
		kai_Expr expr,
		bool in_procedure
	);

	NO_DISCARD
	bool insert_type_dependencies(
		Dependency_List& deps,
		u32 scope_index,
		kai_Expr expr
	);
	
	NO_DISCARD
	std::optional<u32>
	resolve_dependency_node(std::string_view name, u32 scope_index);

	NO_DISCARD
	std::optional<u32>
	resolve_dependency_node_procedure(std::string_view name, Scope* scope);

	NO_DISCARD
	bool has_circular_dependency();
	
	NO_DISCARD
	bool circular_dependency_check(std::vector<Dependency>& dependency_stack, Dependency_Node& node);
};
