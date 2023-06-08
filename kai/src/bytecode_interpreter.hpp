#include "bytecode_stream.hpp"


struct Bytecode_Interpreter {
	u8* bytecode;
	u64 bytecount;
	u64 cursor = 0; // "Instruction Pointer"

	// procedure calls are split into two parts:
	// 1 - setup call stack and cursor
	// 2 - set output registers from when we return
	// This flag lets me know when we are in step 2
	bool returned_from_procedure = false;

	struct base_index {
		u32 reg;
		u32 input;
	};

	std::vector<Register>   registers;
	std::vector<u64>        call_stack; // where to return to when exiting a procedure
	std::vector<base_index> base;       // what index is "0" for this procedure call

	// This avoids a potential allocation on return instruction
	// @TODO: fix allocations, there does not need to be ANY allocation when executing bytecode
	//           (except registers and when done explicitly in the bytecode itself, like a call to malloc or something)
	Register temp[32];

	void debug_output_registers();

	void call_native_procedure(u64 address);

	// step() -> 0 (DONE)
	// step() -> 1 (STILL EXECUTING)
	int step();

	void begin_procedure_input();
	void add_procedure_input(Register value);
	void call_procedure(u64 address);

	Register load_immediate(u8 primtype, void* addr);
	void primitive_operation(u8 op);

	template <typename T>
	T load() {
		T value;
		void* src = &bytecode[cursor];
		void* dst = &value;
		memcpy(dst, src, sizeof(T));
		cursor += sizeof(T);
		return value;
	}

	Register& register_at(u32 index, base_index b);
	void make_registers(u32 max);
};
