#pragma once
#include <kai/generation.h>
#include "dependency_graph.hpp"

struct Procedure_Reference {
	u64 position;
	u32 node_index;
};

struct Bytecode_Generation_Context {
	Bytecode_Instruction_Stream bytecode_stream;
	std::vector<Procedure_Reference> procedure_references;
};

