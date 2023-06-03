#include "bytecode_stream.hpp"


// OPERATION(u8) PRIMITIVE_TYPE(u8) DESTINATION(reg) LEFT(reg) RIGHT(reg)
void Bytecode_Instruction_Stream::insert_primitive_operation(u8 op, u8 primtype, u32 reg_dst, u32 reg_op1, u32 reg_op2) {
	stream.emplace_back(op);
	stream.emplace_back(primtype);
	_insert_register(reg_dst);
	_insert_register(reg_op1);
	_insert_register(reg_op2);
}

// OPERATION(u8) PRIMITIVE_TYPE(u8) DESTINATION(reg) LEFT(reg) RIGHT_VALUE(immediate)
void Bytecode_Instruction_Stream::insert_primitive_operation_imm(u8 op, u8 primtype, u32 reg_dst, u32 reg_op1, Register value) {
	stream.emplace_back(op);
	stream.emplace_back(primtype);
	_insert_register(reg_dst);
	_insert_register(reg_op1);
	_insert_register(IMMEDIATE);
	_insert_immediate(primtype, value);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Memory operations

// LOAD(u8) PRIMITIVE_TYPE(u8) DESTINATION(reg) ADDRESS(reg) INDEX(reg)
void Bytecode_Instruction_Stream::insert_load(u8 primtype, u32 reg_dst, u32 reg_index) {
	stream.emplace_back(Operation_Load);
	stream.emplace_back(primtype);
	_insert_register(reg_dst);
	_insert_register(reg_index);
}

void Bytecode_Instruction_Stream::insert_load(u8 primtype, u32 reg_dst, u64 index_value) {
	stream.emplace_back(Operation_Load);
	stream.emplace_back(primtype);
	_insert_register(reg_dst);
	_insert_register(IMMEDIATE);
	_insert_immediate(Prim_Type_u64, {.u64value = index_value});
}

//*******************************************************************
// Procedure Calling and Returning

// RETURN(u8) RETURN_COUNT(u8) REG(reg)
void Bytecode_Instruction_Stream::insert_return(u32 reg) {
	stream.emplace_back(Operation_Return);
	stream.emplace_back(1);
	_insert_register(reg);
}

// RETURN(u8) RETURN_COUNT(u8) REG1(reg) REG2(reg) REG3(reg) ...
void Bytecode_Instruction_Stream::insert_return(u32 count, u32 const* reg) {
	if (count > 255) panic_with_message("Too many return values!");

	stream.emplace_back(Operation_Return);
	stream.emplace_back(count);
	for range(count) {
		_insert_register(reg[i]);
	}
}

// CALL(u8) FN_ADDRESS(u64) ARG_COUNT(u8) ARG1(reg) ARG2(reg) ARG3(reg) ... OUT_COUNT(u8) OUT1(reg) OUT2(reg) ...
// %6, %7 = call "main" (%1, %4)
// Procedure knows about the types of it's arguments (dont need type metadata, unlike for operations)
// Caller should never put the incorrect types into the proc call (because it knows the types at compile-time, so that would be a super idiot move)
// OUT_COUNT does NOT need to match the procedure's actual output count, it is for storing the procedure's output into registers
u64 Bytecode_Instruction_Stream::procedure_call(u8 arg_count) // -> branch_position: u64
{
	stream.emplace_back(Operation_Procedure_Call);
	u64 pos = position();
	for range(8) stream.emplace_back(0xFF); // terrible
	stream.emplace_back(arg_count);
	return pos;
}
void Bytecode_Instruction_Stream::procedure_input(u32 reg) {
	_insert_register(reg);
}
void Bytecode_Instruction_Stream::procedure_output_count(u8 count) {
	stream.emplace_back(count);
}
void Bytecode_Instruction_Stream::procedure_output(u32 reg) {
	_insert_register(reg);
}


//*******************************************************************
// WIP
// gives index of where the address is stored in bytecode
u64 Bytecode_Instruction_Stream::insert_branch() { return 0; }
void Bytecode_Instruction_Stream::set_branch_position(u64 branch_position, u64 position) {
	void* dst = (stream.data() + branch_position);
	memcpy(dst, &position, sizeof(u64));
}
//


void Bytecode_Instruction_Stream::_insert_register(u32 reg) {
	stream.emplace_back((u8)(reg & 0xFF));
	stream.emplace_back((u8)((reg >> 8) & 0xFF));
	stream.emplace_back((u8)((reg >> 16) & 0xFF));
	stream.emplace_back((u8)((reg >> 24) & 0xFF));
}
// @TODO: have a function that just copies raw data ?
void Bytecode_Instruction_Stream::_insert_immediate(u8 primtype, Register value) {
	// figure out size
	u32 size = sizeof_Type(primtype);
	// figure out pointers and make room
	stream.resize(stream.size() + size);
	void* src = &value;
	void* dst = stream.data() + (stream.size() - size);
	// copy raw data
	memcpy(dst, src, size);
}
