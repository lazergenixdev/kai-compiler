#include <cstring>
#include "bytecode_stream.hpp"

// PRIMITIVE_OP(u8) PRIMITIVE_TYPE(u8) DESTINATION(reg) LEFT(reg) IMMEDIATE(FALSE) RIGHT(reg)
void Bytecode_Instruction_Stream::insert_primitive_operation(u8 op, u8 primtype, u32 reg_dst, u32 reg_op1, u32 reg_op2) {
	stream.emplace_back(op);
	stream.emplace_back(primtype);
	_insert_register(reg_dst);
	_insert_register(reg_op1);
	stream.emplace_back(false);
	_insert_register(reg_op2);
}

// PRIMITIVE_OP(u8) PRIMITIVE_TYPE(u8) DESTINATION(reg) LEFT(reg) IMMEDIATE(TRUE) RIGHT(imm)
void Bytecode_Instruction_Stream::insert_primitive_operation_imm(u8 op, u8 primtype, u32 reg_dst, u32 reg_op1, Register value) {
	stream.emplace_back(op);
	stream.emplace_back(primtype);
	_insert_register(reg_dst);
	_insert_register(reg_op1);
	stream.emplace_back(true);
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
	for_n(count) {
		_insert_register(reg[i]);
	}
}

// CALL(u8) FN_ADDRESS(u64) ARG_COUNT(u8) ARG1(reg) ARG2(reg) ARG3(reg) ... OUT_COUNT(u8) OUT1(reg) OUT2(reg) ...
u64 Bytecode_Instruction_Stream::procedure_call(u8 arg_count) // -> branch_position: u64
{
	stream.emplace_back(Operation_Procedure_Call);
	u64 pos = position();
	for_n(8) stream.emplace_back(0xBA);
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
u64 Bytecode_Instruction_Stream::insert_jump() { return 0; }
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


template <typename T>
T load(void* data, u64& cursor) {
	T value;
	void* src = (u8*)data + cursor;
	void* dst = &value;
	memcpy(dst, src, sizeof(T));
	cursor += sizeof(T);
	return value;
}

Register load_prim(u8 type, void* data, u64& cursor) {
	Register value;
	void* src = (u8*)data + cursor;
	void* dst = &value;
	memcpy(dst, src, sizeof_Type(type));
	cursor += sizeof_Type(type);
	return value;
}


char const* opstr(u8 op) {
	Operation o = (Operation)op;
	switch (o)
	{
	case Operation_Add:            return "add";
	case Operation_Sub:            return "sub";
	case Operation_Mul:            return "mul";
	case Operation_Div:            return "div";
	case Operation_Procedure_Call: return "call";
	case Operation_Return:         return "ret";
	case Operation_Continue_If:
	case Operation_Jump_If:
	case Operation_Jump:
	case Operation_Load_Value:
	case Operation_Load:
	case Operation_Store:
	case Operation_Stack_Alloc:
	case Operation_Stack_Load:
	case Operation_Stack_Store:
	case Operation_Convert_From_u8:
	case Operation_Convert_From_u16:
	case Operation_Convert_From_u32:
	case Operation_Convert_From_u64:
	case Operation_Convert_From_s8:
	case Operation_Convert_From_s16:
	case Operation_Convert_From_s32:
	case Operation_Convert_From_s64:
	case Operation_Convert_From_f32:
	case Operation_Convert_From_f64:
	case Operation_Native_Procedure_Call:
	default:return "undefined";
	}
}

char const* typestr(u8 type) {
	Primitive_Type t = (Primitive_Type)type;
	switch (t)
	{
#define X(NAME,TYPE) case Prim_Type_ ## NAME: return #NAME;
		XPRIMITIVE_TYPES
#undef  X
	case Prim_Type_Pointer:
	default: return "undefined";
	}
}

void _print_primitive_value(u8 type, Register value) {
	Primitive_Type t = (Primitive_Type)type;
	switch (t)
	{
#define X(NAME,TYPE) case Prim_Type_ ## NAME: std::cout << value. NAME ## value; break;
		XPRIMITIVE_TYPES
#undef  X
	case Prim_Type_Pointer:
	default: std::cout << "undefined"; break;
	}
}

void _print_register(u32 reg) {
	if (reg > MAX_PROC_ARG_REGISTER) {
		std::cout << "arg(" << PROC_ARG(reg) << ')';
	} else {
		std::cout << '%' << reg;
	}
}

void _print_primitive_operation(u8 op, u8 type, u32 dst, u32 op1) {
	_print_register(dst);
	std::cout << " = ";
	std::cout << opstr(op) << '.' << typestr(type) << ' ';
	_print_register(op1);
	std::cout << ", ";
}

#define LOAD(TYPE) load<TYPE>(stream.data(), cursor)
void Bytecode_Instruction_Stream::debug_print()
{
	u64 cursor = 0;

	while (cursor < stream.size()) {
		u8 op = stream[cursor++];

		switch (op)
		{

		case Operation_Add:
		case Operation_Sub:
		case Operation_Mul:
		case Operation_Div:
		{
			auto type = stream[cursor++];
			auto dst = LOAD(u32);
			auto op1 = LOAD(u32);
			
			_print_primitive_operation(op, type, dst, op1);

			auto is_imm = stream[cursor++];
			if (is_imm) {
				auto value = load_prim(type, stream.data(), cursor);
				_print_primitive_value(type, value);
			}
			else {
				auto op2 = load<u32>(stream.data(), cursor);
				_print_register(op2);
			}
			std::cout << '\n';
			break;
		}
		// FN_ADDRESS(u64) ARG_COUNT(u8) ARG1(reg) ARG2(reg) ARG3(reg) ... OUT_COUNT(u8) OUT1(reg) OUT2(reg) ...
		case Operation_Procedure_Call: {
			auto addr = LOAD(u64);
			auto arg_count = stream[cursor++];

			auto args_start = cursor;
			cursor += (u64)arg_count * sizeof(u32);

			auto out_count = stream[cursor++];
			for_n(out_count) {
				auto reg = LOAD(u32);
				_print_register(reg);
				if (i != out_count - 1) std::cout << ", ";
			}

			std::cout << " = call 0x";
			std::cout << std::hex << addr << std::dec << " (";

			cursor = args_start;
			for_n(arg_count) {
				auto reg = LOAD(u32);
				_print_register(reg);
				if (i != arg_count - 1) std::cout << ", ";
			}
			std::cout << ")\n";
			cursor += (u64)out_count * sizeof(u32) + 1;
			break;
		}
		case Operation_Return: {
			auto ret_count = stream[cursor++];
			std::cout << "ret ";
			for_n(ret_count) {
				auto reg = LOAD(u32);
				_print_register(reg);
				if (i != ret_count - 1) std::cout << ", ";
			}
			std::cout << '\n';
			break;
		}
		default: { std::cout << "undefined bytecode instruction\n";  goto end; }
		}
	}
end:(void)0;
}
