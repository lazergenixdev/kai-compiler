#include "bytecode_interpreter.hpp"

void Bytecode_Interpreter::debug_output_registers() {
	for (auto const& r : registers) {
		std::cout << std::setw(4) << r.s32value << ' ';
	}
	std::cout << '\b' << '\n';
}

Register& Bytecode_Interpreter::register_at(u32 index, base_index b) {
	assert(index != IMMEDIATE, "you have fucked up, sorry :(");

	// procedure input
	if (index > MAX_PROC_ARG_REGISTER)
		return registers[PROC_ARG(index) + b.input];

	u64 reg_index = index + b.reg;

	if (reg_index >= (u64)registers.size())
		panic_with_message("register not allocated");
	
	return registers[reg_index];
}

int Bytecode_Interpreter::step() {
	if (cursor >= bytecount) panic_with_message("how?");

	u8 op = bytecode[cursor++];

	if (returned_from_procedure) {
		// Copy output into registers
		auto last = base.back();
		base.pop_back();
		u32 max_reg_index = 0;
		for_n(op) {
			u32 index = load<u32>();
			u32 k = index + base.back().reg;
			if (k > max_reg_index) max_reg_index = k;
			register_at(index, base.back()) = registers[last.input + i];
		}
		registers.resize(std::max(last.input, max_reg_index + 1));
		returned_from_procedure = false;
		return 1;
	}

	switch (op)
	{
	case Operation_Add:
	case Operation_Sub:
	case Operation_Mul:
	case Operation_Div:
	{
		primitive_operation(op);
		return 1;
	}
	case Operation_Return: {
		u8 ret_count = bytecode[cursor++];

		auto start_return_index = base.back().input;

		if (registers.size() < start_return_index + ret_count)
			registers.resize(start_return_index + ret_count);

		u32* ptr = reinterpret_cast<u32*>(&bytecode[cursor]);

		for_n(ret_count) {
			temp[i] = register_at(ptr[i], base.back());
		}
		memcpy(registers.data() + start_return_index, temp, ret_count * sizeof(Register));

		cursor = call_stack.back();
		call_stack.pop_back();

		bool still_executing = !call_stack.empty();
		if (still_executing) returned_from_procedure = true;
		return still_executing;
	}
	case Operation_Procedure_Call: {
		u64 proc_addr = load<u64>();
		u8 arg_count = bytecode[cursor++];

		auto bi = base.back();
		begin_procedure_input();
		for_n(arg_count) {
			// read register index
			u32 index = load<u32>();
			// push value at register
			add_procedure_input(register_at(index, bi));
		}
		call_procedure(proc_addr);
		return 1;
	}
	default: panic_with_message("undefined instruction: %i", (int)op); return 0;
	}
}

void Bytecode_Interpreter::begin_procedure_input() {
	base_index bi;
	bi.input = (u32)registers.size();
	base.emplace_back(bi);
}
void Bytecode_Interpreter::add_procedure_input(Register value) {
	registers.emplace_back(value);
}
void Bytecode_Interpreter::call_procedure(u64 address) {
	assert(call_stack.size() < 512, "Too many procedure calls, call stack limit is 512");

	call_stack.emplace_back(cursor);         // save where we are for when we return
	cursor = address;                        // jump to proc address
	base.back().reg = (u32)registers.size(); // setup base register
}

Register Bytecode_Interpreter::load_immediate(u8 primtype, void* addr) {
	Register value;
	// figure out size
	u32 size = sizeof_Type(primtype);
	// figure out pointers
	void* src = addr;
	void* dst = &value;
	// copy raw data
	memcpy(dst, src, size);
	cursor += size;
	return value;
}

void primitive_add(u8 type, Register& dst, Register const& op1, Register const& op2) {
	switch (type) {
#define X(NAME,TYPE) case Prim_Type_ ## NAME: dst. NAME ## value = op1. NAME ## value + op2. NAME ## value; break;
		XPRIMITIVE_TYPES
#undef  X
	default: panic();
	}
}
void primitive_sub(u8 type, Register& dst, Register const& op1, Register const& op2) {
	switch (type) {
#define X(NAME,TYPE) case Prim_Type_ ## NAME: dst. NAME ## value = op1. NAME ## value - op2. NAME ## value; break;
		XPRIMITIVE_TYPES
#undef  X
	default: panic();
	}
}
void primitive_mul(u8 type, Register& dst, Register const& op1, Register const& op2) {
	switch (type) {
#define X(NAME,TYPE) case Prim_Type_ ## NAME: dst. NAME ## value = op1. NAME ## value * op2. NAME ## value; break;
		XPRIMITIVE_TYPES
#undef  X
	default: panic();
	}
}
void primitive_div(u8 type, Register& dst, Register const& op1, Register const& op2) {
	switch (type) {
#define X(NAME,TYPE) case Prim_Type_ ## NAME: dst. NAME ## value = op1. NAME ## value / op2. NAME ## value; break;
		XPRIMITIVE_TYPES
#undef  X
	default: panic();
	}
}

void Bytecode_Interpreter::primitive_operation(u8 op) {
	auto type = bytecode[cursor++];
	auto dst = load<u32>();
	auto op1 = load<u32>();
	auto is_imm = bytecode[cursor++];

	make_registers(dst);

	Register r2;
	if (is_imm)
		r2 = load_immediate(type, bytecode+cursor);
	else
		r2 = register_at(load<u32>(), base.back());

	auto& r1 = register_at(op1, base.back());
	auto& r0 = register_at(dst, base.back());

	switch (op) {
	case Operation_Add: primitive_add(type, r0, r1, r2); break;
	case Operation_Sub: primitive_sub(type, r0, r1, r2); break;
	case Operation_Mul: primitive_mul(type, r0, r1, r2); break;
	case Operation_Div: primitive_div(type, r0, r1, r2); break;
	default: panic();
	}
}

void Bytecode_Interpreter::make_registers(u32 max)
{
	u64 reg_index = max + base.back().reg;
	u64 insert_count = (u64)reg_index - (u64)registers.size() + 1;

	if (insert_count > 0x100) {
		panic_with_message("Insert count too large!");
	}

	Register r{};
	for (u64 i = 0; i < insert_count; ++i)
		registers.emplace_back(r);
}
