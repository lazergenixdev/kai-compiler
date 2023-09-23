#pragma once
#include <kai/generation.h>
#include "config.hpp"
#include "dependency_graph.hpp"

struct Procedure_Reference {
	u64 position;
	u32 node_index;
};

struct Bytecode_Context {
	u32 next_reg = 0;
	u32 scope_index;
};

struct Procedure {
	u64 position;
};

struct Procedure_Signature {
	kai_str  name;
	kai_Type type;
};

template <>
struct std::hash<Procedure_Signature> {
	no_discard size_t operator()(const Procedure_Signature& _Keyval) const noexcept {
		return 0;
	}
};

struct Bytecode_Generation_Context {
	Dependency_Graph dg;
	Bytecode_Instruction_Stream bytecode_stream;
	std::vector<Procedure_Reference> procedure_references;
	std::vector<Procedure> procedures;
	std::unordered_map<Procedure_Signature, u32> procedure_map;

	u64 generate_bytecode(Bytecode_Context& ctx, kai_Stmt body);
	void evaluate_value(u32 node_index);
	void evaluate_type(u32 node_index);
};

