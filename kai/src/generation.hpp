#pragma once
#include <kai/generation.h>
#include "bytecode_interpreter.hpp"
#include <unordered_map>
#include <optional>

// @TODO: don't repeat dependencies pls thx
// @TODO: add type dependencies on procedure calls

#define GLOBAL_SCOPE 0

// local variables, if we see this, DO NOT depend on the identifier
#define LOCAL_VARIABLE_INDEX -1

struct Scope {
	std::unordered_map<std::string_view, kai_u32> map; // maps Identifiers to Dependency Nodes
	kai_u32 parent = -1;
	kai_bool is_proc_scope = false;
};

struct Dependency {
	u32 type; // value or type
	u32 node_index;

	enum {
		TYPE,
		VALUE,
	};
};

enum Dependency_Flag: kai_u32 {
	Evaluated = KAI_BIT(0),
	Temperary = KAI_BIT(1), // temporarily in the dependency system (for procedure input/variables)
};

using Dependency_List = std::vector<Dependency>;

struct Dependency_Node {
	kai_u32 flags;
	Dependency_List dependencies;
};

struct Dependency_Node_Info {
	std::string_view name;
	kai_u32 scope;
	kai_Expr expr; // AST node
};
struct Type_Dependency_Node : public Dependency_Node {
	kai_Type type;  // evaluated type
};
struct Value_Dependency_Node : public Dependency_Node {
	Register value; // evaluated value
};

// @TODO: (optimization) use own dynamic array type, std::vector is dumb
// High level dependency graph
struct HL_Dependency_Graph {
	std::vector<Scope> scopes;
	std::vector<Value_Dependency_Node> value_nodes;
	std::vector<Type_Dependency_Node>   type_nodes;
	std::vector<Dependency_Node_Info> node_infos;
};

struct Bytecode_Generation_Context {
	HL_Dependency_Graph dg;
	kai_Module*         mod;
	kai_Memory          memory;
	kai_Error_Info      error_info;

	bool error_redefinition(kai_str const& string, kai_int line_number) {
		if (error_info.value != kai_Result_Success) return true; // already have error value

		error_info.value = kai_Result_Error_Semantic;
		error_info.loc.string = string;
		error_info.loc.line = line_number;
		error_info.what = {0, (kai_u8*)memory.temperary}; // uses temperary memory
		error_info.context = {0, nullptr}; // static memory

		auto& w = error_info.what;
		memcpy(w.data, "Indentifier \"", 13);
		w.count += 13;

		memcpy(w.data + w.count, string.data, string.count);
		w.count += string.count;

		memcpy(w.data + w.count, "\" is already declared", 21);
		w.count += 21;

		return true;
	}
	bool error_not_declared(kai_str const& string, kai_int line_number) {
		if (error_info.value != kai_Result_Success) return true; // already have error value

		error_info.value = kai_Result_Error_Semantic;
		error_info.loc.string = string;
		error_info.loc.line = line_number;
		error_info.what = {0, (kai_u8*)memory.temperary}; // uses temperary memory
		error_info.context = {0, nullptr}; // static memory

		auto& w = error_info.what;
		memcpy(w.data, "indentifier \"", 13);
		w.count += 13;

		memcpy(w.data + w.count, string.data, string.count);
		w.count += string.count;

		memcpy(w.data + w.count, "\" not declared", 14);
		w.count += 14;

		return true;
	}

	bool generate_dependency_graph();

	bool insert_node_for_declaration(kai_Stmt_Declaration* decl, bool in_proc, kai_u32 scope = GLOBAL_SCOPE);
	bool insert_node_for_statement(kai_Stmt stmt, bool in_proc, kai_u32 scope = GLOBAL_SCOPE);

	bool insert_value_dependencies(Dependency_List& deps, kai_u32 scope, kai_Expr expr);
	bool insert_type_dependencies(Dependency_List& deps, kai_u32 scope, kai_Expr expr);

	bool insert_value_dependencies_proc(Dependency_List& deps, kai_u32 scope, kai_Expr expr);

	std::optional<kai_u32> resolve_dependency_node(std::string_view name, kai_u32 scope) {
		Scope* s = nullptr;
		loop {
			s = &dg.scopes[scope];

			auto it = s->map.find(name);
			if (it != s->map.end())
				return std::make_optional(it->second);

			if (scope == GLOBAL_SCOPE)
				return {};
			
			scope = s->parent;
		}

		return {};
	}
	std::optional<kai_u32> resolve_dependency_node_procedure(std::string_view name, Scope* scope) {
		bool allow_locals = true;
		loop {
			auto it = scope->map.find(name);

			if (it != scope->map.end()) {
				bool const is_local = it->second == LOCAL_VARIABLE_INDEX;
				if (is_local) {
					if (allow_locals) {
						return std::make_optional(it->second);
					}
				}
				else {
					return std::make_optional(it->second);
				}
			}

			if (scope->parent == -1)
				return {};

			if (scope->is_proc_scope)
				allow_locals = false;
			
			scope = &dg.scopes[scope->parent];
		}
	}
};

void remove_all_locals(Scope& s) {
	for (auto it = s.map.begin(); it != s.map.end();)
	{
		if (it->second == LOCAL_VARIABLE_INDEX)
			it = s.map.erase(it);
		else ++it;
	}
}
