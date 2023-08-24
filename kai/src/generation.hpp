#pragma once
#include <kai/generation.h>
#include "bytecode_interpreter.hpp"
#include <unordered_map>
#include <optional>

// @TODO: don't repeat dependencies pls thx

#define catch_and_throw(a) if(a) return true

#define GLOBAL_SCOPE 0
#define LOCAL_VARIABLE_INDEX -1

struct Scope {
	std::unordered_map<std::string_view, kai_u32> map; // maps Identifiers to Dependency Nodes
	kai_u32 parent = -1;
	kai_bool is_proc_scope = false;
};

struct Dependency {
	union {
		struct {
			u32 what; // value or type
			u32 node_index;
		};
		u64 _data;
	};

	enum {
		VALUE,
		TYPE,
	};

	constexpr Dependency() {
		this->_data = 0;
	}
	constexpr Dependency(u32 type, u32 index) {
		this->what       = type;
		this->node_index = index;
	}
	constexpr bool operator==(Dependency const& right) const {
		return this->_data == right._data;
	}
};

enum Dependency_Flag: kai_u32 {
	Evaluated = KAI_BIT(0),
	Incomplete_Dependency_List = KAI_BIT(1), // for depending on an overloaded function
};

using Dependency_List = std::vector<Dependency>;

struct Dependency_Node {
	kai_u32 flags;
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
	kai_u32 scope;
	kai_Expr expr; // AST node to be evaluated
	kai_int line_number;
};

// High level dependency graph
// @TODO: (optimization) use own dynamic array type, std::vector is dumb
struct HL_Dependency_Graph {
	std::vector<Scope> scopes;
	std::vector<Value_Dependency_Node> value_nodes;
	std::vector<Type_Dependency_Node>   type_nodes;
	std::vector<Dependency_Node_Info> node_infos;
	std::vector<Bytecode_Instruction_Stream> bytecode_streams;
};

struct Bytecode_Generation_Context {
	HL_Dependency_Graph dg;
	kai_Module*         mod;
	kai_Memory          memory;
	kai_Error      error_info;

	bool error_redefinition(kai_str const& string, kai_int line_number) {
		if (error_info.result != kai_Result_Success) return true; // already have error value

		error_info.result = kai_Result_Error_Semantic;
		error_info.location.string = string;
		error_info.location.line = line_number;
		error_info.message = {0, (kai_u8*)memory.temperary}; // uses temperary memory
		error_info.context = {0, nullptr}; // static memory

		auto& w = error_info.message;
		memcpy(w.data, "Indentifier \"", 13);
		w.count += 13;

		memcpy(w.data + w.count, string.data, string.count);
		w.count += string.count;

		memcpy(w.data + w.count, "\" is already declared", 21);
		w.count += 21;

		return true;
	}
	bool error_not_declared(kai_str const& string, kai_int line_number) {
		if (error_info.result != kai_Result_Success) return true; // already have error value

		error_info.result = kai_Result_Error_Semantic;
		error_info.location.string = string;
		error_info.location.line = line_number;
		error_info.message = {0, (kai_u8*)memory.temperary}; // uses temperary memory
		error_info.context = {0, nullptr}; // static memory

		auto& w = error_info.message;
		memcpy(w.data, "indentifier \"", 13);
		w.count += 13;

		memcpy(w.data + w.count, string.data, string.count);
		w.count += string.count;

		memcpy(w.data + w.count, "\" not declared", 14);
		w.count += 14;

		return true;
	}
	bool error_circular_dependency(std::string_view const& string, kai_int line_number) {
		if (error_info.result != kai_Result_Success) return true; // already have error value

		error_info.result = kai_Result_Error_Semantic;
		error_info.location.string.data = (u8*)string.data();
		error_info.location.string.count = (u64)string.size();
		error_info.location.line = line_number;
		error_info.message = {0, (kai_u8*)memory.temperary}; // uses temperary memory
		error_info.context = {0, nullptr}; // static memory

		auto& w = error_info.message;
		memcpy(w.data, "indentifier \"", 13);
		w.count += 13;

		memcpy(w.data + w.count, string.data(), string.size());
		w.count += string.size();

		memcpy(w.data + w.count, "\" cannot depend on itself", 25);
		w.count += 25;

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
	// TODO: Flatten out this recursive algorithm to be iterative
	bool recursize_circular_dependency_check(Dependency const& dep, Dependency_Node& node) {
		for (auto const& d : node.dependencies) {
			if (dep == d) {
				auto const& info = dg.node_infos[dep.node_index];
				return error_circular_dependency(info.name, info.line_number);
			}
			if(d.what == Dependency::VALUE)
				catch_and_throw(recursize_circular_dependency_check(dep, dg.value_nodes[d.node_index]));
			else
				catch_and_throw(recursize_circular_dependency_check(dep, dg.type_nodes[d.node_index]));
		}
		return false;
	}
	bool detect_circular_dependencies() {
		for range(dg.node_infos.size()) {
			{
				Dependency value_dep{Dependency::VALUE, (u32)i};
				auto& node = dg.value_nodes[i];
				catch_and_throw(recursize_circular_dependency_check(value_dep, node));
			}
			{
				Dependency type_dep{Dependency::TYPE, (u32)i};
				auto& node = dg.type_nodes[i];
				catch_and_throw(recursize_circular_dependency_check(type_dep, node));
			}
		}
		return false;
	}

	bool all_evaluated(Dependency_List const& deps) {
		Dependency_Node const* node = nullptr;
		for (auto&& d : deps) {
			switch (d.what)
			{
			case Dependency::VALUE:
				node = &dg.value_nodes[d.node_index];
				break;
			case Dependency::TYPE:
				node = &dg.type_nodes[d.node_index];
				break;
			default: panic_with_message("something got corrupted.."); break;
			}
			if (0 == (node->flags & Evaluated))
				return false;
		}
		return true;
	}

	void evaluate_value(u32 node_index);
	void evaluate_type(u32 node_index);
};

void remove_all_locals(Scope& s) {
	for (auto it = s.map.begin(); it != s.map.end();) {
		if (it->second == LOCAL_VARIABLE_INDEX)
			it = s.map.erase(it);
		else ++it;
	}
}
