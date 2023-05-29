#include "generation.hpp"
#include "program.hpp"
#include <iomanip>
#include <cassert>

#define DO_DEBUG_PRINT

#ifdef DO_DEBUG_PRINT
#include <iostream>
void print_type(kai_Type_Info* info) {
	if (info == nullptr) {
		std::cout << "null\n";
		return;
	}

	auto e = (kai_Type_Enum)info->type;

	switch (info->type)
	{
	case kai_Type_Enum::kai_Type_Integer: {
		auto i = (kai_Type_Info_Integer*)info;
		if( !i->is_signed ) std::cout << 'u';
		std::cout << "int" << (int)i->bits << '\n';
		break;
	}
	case kai_Type_Enum::kai_Type_Float: {
		auto i = (kai_Type_Info_Float*)info;
		std::cout << "float" << (int)i->bits << '\n';
		break;
	}

	default: { std::cout << "undefined\n"; break; }
	}
}
#endif

// Numbers are implicitly converted to any integer/float type.
// ONLY if they can be represented, e.g. 1000 cannot be represented as a u8, therefore it will not convert implicitly
#define NUMBER_TYPE -1


struct int_type_data {
	char const* name;
	kai_u8 bits;
	kai_u8 is_signed;
};

inline kai_Type_Info_Integer static_int_type_data[] = {
	{kai_Type_Integer,  8, kai_bool_true },
	{kai_Type_Integer, 16, kai_bool_true },
	{kai_Type_Integer, 32, kai_bool_true },
	{kai_Type_Integer, 64, kai_bool_true },
	{kai_Type_Integer,  8, kai_bool_false},
	{kai_Type_Integer, 16, kai_bool_false},
	{kai_Type_Integer, 32, kai_bool_false},
	{kai_Type_Integer, 64, kai_bool_false},
};
inline kai_Type_Info_Float static_float_type_data[] = {
	{kai_Type_Float, 32},
	{kai_Type_Float, 64},
};

void insert_primitive_types(Type_Checker_Context& ctx, Type_Table& table) {

	// strings are automatically in static memory, so no need to make this array static
	std::string_view static_int_string_data[] = {
		"s8", "s16", "s32", "s64",
		"u8", "u16", "u32", "u64",
	};
	static_assert( std::size(static_int_type_data) == std::size(static_int_string_data), "There must be a type for each name" );

	for( int i = 0; i < std::size(static_int_type_data); ++i ) {
		table.emplace(static_int_string_data[i], reinterpret_cast<kai_Type_Info*>(static_int_type_data + i));
	}

	table.emplace( "int", table["s64"] );
	table.emplace( "f32", reinterpret_cast<kai_Type_Info*>(static_float_type_data + 0) );
	table.emplace( "f64", reinterpret_cast<kai_Type_Info*>(static_float_type_data + 1) );
}

#define range(N) (int i = 0; i < (N); ++i)
#define view(S) std::string_view( (char*)(S.data), (S.count) )

kai_result Type_Check(Type_Checker_Context& ctx, kai_Stmt root) {

	switch (root->id)
	{
		
	case kai_Expr_ID_Identifier: {
		auto ident = (kai_Expr_Identifier*)root;
#ifdef DO_DEBUG_PRINT
		std::cout << view(ident->source_code) << '\n';
#endif
		return kai_Result_Success;
	}
	case kai_Expr_ID_String: return kai_Result_Success;
	case kai_Expr_ID_Number: return kai_Result_Success;
	case kai_Expr_ID_Binary: {
		auto binary = (kai_Expr_Binary*)root;
		Type_Check(ctx, binary->left);
		Type_Check(ctx, binary->right);
		return kai_Result_Success;
	}
	case kai_Expr_ID_Unary: {
		auto unary = (kai_Expr_Unary*)root;
		Type_Check(ctx, unary->expr);
		return kai_Result_Success;
	}
//	case kai_Expr_ID_Procedure_Type: return kai_Result_Success;
	case kai_Expr_ID_Procedure_Call: {
		auto call = (kai_Expr_Procedure_Call*)root;
		Type_Check(ctx, call->proc);
		for range(call->arg_count)
			Type_Check(ctx, call->arguments[i]);
		return kai_Result_Success;
	}
	case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)root;
		auto count = proc->param_count;
		for range(count) {
			auto param = proc->input_output[i];
#ifdef DO_DEBUG_PRINT
			std::cout << '{' << view(param.name) << '}' << '\n';
#endif
			Type_Check(ctx, param.type);
		}
		Type_Check(ctx, proc->body);
		return kai_Result_Success;
	}
	case kai_Stmt_ID_Return: {
		auto ret = (kai_Stmt_Return*)root;
		Type_Check(ctx, ret->expr);
		return kai_Result_Success;
	}
	case kai_Stmt_ID_Declaration: {
		auto decl = (kai_Stmt_Declaration*)root;
#ifdef DO_DEBUG_PRINT
		std::cout << '[' << view(decl->name) << ']' << '\n';
#endif
		Type_Check(ctx, decl->expr);
		return kai_Result_Success;
	}
	case kai_Stmt_ID_Compound: {
		auto comp = (kai_Stmt_Compound*)root;
		for range(comp->count)
			Type_Check(ctx, comp->statements[i]);
		return kai_Result_Success;
	}
	default:break;
	}

	return kai_Result_Error_Fatal;
}

kai_u32 translate_instruction(kai_u32 op) {
	switch (op)
	{
	case '+': return Byte_Code_Instrution_ID::Instr_Add;
	case '-': return Byte_Code_Instrution_ID::Instr_Subtract;
	case '*': return Byte_Code_Instrution_ID::Instr_Multiply;
	case '/': return Byte_Code_Instrution_ID::Instr_Divide;

	default:return -1;
	}
}

VirtualReg Generate_Instruction(Type_Checker_Context& ctx, kai_Expr root) {
	switch (root->id)
	{
	case kai_Expr_ID_Identifier: {
		auto ident = (kai_Expr_Identifier*)root;
		auto it = ctx.ident_index_table.find(view(ident->source_code));
		if (it == ctx.ident_index_table.end()) {
			return {-1};
		}
		return {it->second, 0};
	}
	case kai_Expr_ID_Number: {
		auto num = (kai_Expr_Number*)root;
		return {VirtualReg_Immediate, num->info.Whole_Part};
	}
	case kai_Expr_ID_Binary: {
		auto binary = (kai_Expr_Binary*)root;

		ByteCodeInstruction instr;
		instr.intruction = translate_instruction(binary->op);
		instr.left       = Generate_Instruction(ctx, binary->left);
		instr.right      = Generate_Instruction(ctx, binary->right);
		auto index = (int)ctx.intruction_stream.size();
		ctx.intruction_stream.emplace_back(instr);

		return {index};
	}

	default:return {-1};
	}
}

void Generate_Statement_Instruction(Type_Checker_Context& ctx, kai_Stmt root) {
	switch (root->id)
	{
	case kai_Stmt_ID_Return: {
		auto ret = (kai_Stmt_Return*)root;

		ByteCodeInstruction instr;
		instr.intruction = Instr_Return;
		instr.left = Generate_Instruction(ctx, ret->expr);
		ctx.intruction_stream.emplace_back(instr);
		break;
	}
	case kai_Stmt_ID_Declaration: {
		auto decl = (kai_Stmt_Declaration*)root;

		auto instr = Generate_Instruction(ctx, decl->expr);
		ctx.ident_index_table.emplace(view(decl->name), instr.index);
		break;
	}
	case kai_Stmt_ID_Compound: {
		auto comp = (kai_Stmt_Compound*)root;
		for range(comp->count) {
			Generate_Statement_Instruction(ctx, comp->statements[i]);
		}
		break;
	}

	default:break;
	}
}

static constexpr char const* instr_to_string(kai_u32 instr) {
	switch (instr)
	{
	case Instr_Add:      return "add";
	case Instr_Subtract: return "sub";
	case Instr_Multiply: return "mul";
	case Instr_Divide:   return "div";
	case Instr_Return:   return "ret";
	case Instr_Load_Argument:return "load_arg";

	default:return "undefined";
	}
}
static constexpr bool has_right(kai_u32 instr) {
	switch (instr)
	{
	default:return true;
	case Instr_Return:       return false;
	case Instr_Load_Argument:return false;
	}
}
static constexpr bool declares_virtual_reg(kai_u32 instr) {
	switch (instr)
	{
	default:return true;
	case Instr_Return:       return false;
	}
}

#ifdef DO_DEBUG_PRINT
void print_virtual_reg(VirtualReg const& vr) {
	switch (vr.index)
	{
	default: {
		std::cout << '%' << vr.index;
		break;
	}
	case VirtualReg_Immediate: {
		std::cout << vr.value;
		break;
	}
	case -1: {
		std::cout << "error";
		break;
	}
	}
}
#endif

#define REG_RAX 0
#define REG_RBX 1

void load_register_or_immediate(std::vector<kai_u8>& bytestream, int& stack_size, VirtualReg& vr, int reg_out) {
	switch (vr.index)
	{
	default: {
		// get value from stack
		kai_u8 mov_rax_or_rbx[] = {
			0x48, 0x8b, 0x84, 0x24,
			0x48, 0x8b, 0x9c, 0x24,
		};
		auto first = mov_rax_or_rbx + reg_out*4;
		auto last  = first + 4;
		bytestream.insert(bytestream.end(), first, last);

		kai_u32 offset = (stack_size - vr.index - 1)*8;
		bytestream.insert(bytestream.end(), (kai_u8*)&offset, ((kai_u8*)&offset) + sizeof(offset));
		break;
	}
	case VirtualReg_Immediate: {
		// mov value into register
		kai_u8 movabs[] = {
			0x48, 0xb8,
			0x48, 0xbb,
		};
		auto first = movabs + reg_out*2;
		auto last  = first + 2;
		bytestream.insert(bytestream.end(), first, last);

		kai_u64 value = vr.value;
		bytestream.insert(bytestream.end(), (kai_u8*)&value, ((kai_u8*)&value) + sizeof(kai_u64));
		break;
	}
	case -1: {
		__debugbreak();
		break;
	}
	}
}

kai_result kai_create_program(kai_Program_Create_Info* info, kai_Program* program)
{
	Type_Checker_Context ctx{
		info->module->memory
	};

	//Type_Table all_types;
	//insert_primitive_types(ctx, all_types);

	//for (auto&& [name, info]: all_types) {
	//	std::cout << std::setw(6) << name << ": ";
	//	print_type(info);
	//}

	//std::cout << '\n';
	//auto result = Type_Check( ctx, (kai_Stmt)info->module->AST_Root );
	//if( result != kai_Result_Success ) return result;

#ifdef DO_DEBUG_PRINT
	std::cout << '\n';
#endif
	{
		auto root = (kai_Stmt)info->module->AST_Root;

		if (root->id != kai_Stmt_ID_Declaration) goto end;
		auto decl = (kai_Stmt_Declaration*)root;

		if (decl->expr->id != kai_Expr_ID_Procedure) goto end;
		auto proc = (kai_Expr_Procedure*)decl->expr;

		for range(proc->param_count) {
			auto const& param = proc->input_output[i];
			ctx.ident_index_table[view(param.name)] = i;
			ByteCodeInstruction instr;
			instr.intruction = Instr_Load_Argument;
			instr.left = {VirtualReg_Immediate, (unsigned)i};
			ctx.intruction_stream.emplace_back(instr);
		}

		Generate_Statement_Instruction(ctx, proc->body);

#ifdef DO_DEBUG_PRINT
		int i = 0;
		for (auto&& [instr, left, right] : ctx.intruction_stream) {
			if(declares_virtual_reg(instr))
				std::cout << "%" << i << " = ";
			
			std::cout << instr_to_string(instr) << " ";
			print_virtual_reg(left);
			if (has_right(instr)) {
				std::cout << ", ";
				print_virtual_reg(right);
			}
			std::cout << '\n';
			++i;
		}
#endif
	}

	// Generate x64 Assembly
	{
		std::vector<kai_u8> machine_code_stream;
		machine_code_stream.reserve(1024);

		int stack_size = 0;

		for (auto&& [instr, left, right] : ctx.intruction_stream) {
			switch (instr)
			{
			case Instr_Load_Argument: {
				// push {input reg}
				kai_u8 calling_conv_pushes[] = {
					0x51,       // rcx
					0x52,       // rdx
					0x41, 0x50, // r8
					0x41, 0x51, // r9
				};
				kai_u8 instr_index[] = {
					0, 1, 2, 4
				};
				kai_u8 instr_length[] = {
					1, 1, 2, 2
				};
				int i = (int)left.value;
				assert(i >= 0 && i < 4);

				auto first = calling_conv_pushes + instr_index[i];
				auto last = first + instr_length[i];

				machine_code_stream.insert(machine_code_stream.end(), first, last );
				++stack_size;
				break;
			}
			case Instr_Return: {
				// load into rax
				load_register_or_immediate(machine_code_stream, stack_size, left, REG_RAX);

				// restore the stack
				kai_u8 add_rsp[] = {
					0x48, 0x81, 0xc4
				};
				machine_code_stream.insert(machine_code_stream.end(), add_rsp, add_rsp + std::size(add_rsp));
				kai_u32 byte_count = stack_size * 8;
				machine_code_stream.insert(machine_code_stream.end(), (kai_u8*)&byte_count, ((kai_u8*)&byte_count) + sizeof(kai_u32));

				machine_code_stream.push_back(0xc3);
				break;
			}

			case Instr_Add: {
				// load into rax and rbx
				load_register_or_immediate(machine_code_stream, stack_size,  left, REG_RAX);
				load_register_or_immediate(machine_code_stream, stack_size, right, REG_RBX);
				
				// do operation
				kai_u8 add_instr[] = {
					0x48, 0x01, 0xd8,
				};
				machine_code_stream.insert(machine_code_stream.end(), add_instr, add_instr + std::size(add_instr));

				// push rax
				machine_code_stream.push_back(0x50);
				++stack_size;
				break;
			}
			case Instr_Subtract: {
				// load into rax and rbx
				load_register_or_immediate(machine_code_stream, stack_size,  left, REG_RAX);
				load_register_or_immediate(machine_code_stream, stack_size, right, REG_RBX);
				
				// do operation
				kai_u8 sub_instr[] = {
					0x48, 0x29, 0xd8,
				};
				machine_code_stream.insert(machine_code_stream.end(), sub_instr, sub_instr + std::size(sub_instr));

				// push rax
				machine_code_stream.push_back(0x50);
				++stack_size;
				break;
			}
			case Instr_Multiply: {
				// load into rax and rbx
				load_register_or_immediate(machine_code_stream, stack_size,  left, REG_RAX);
				load_register_or_immediate(machine_code_stream, stack_size, right, REG_RBX);
				
				// do operation
				kai_u8 mul_instr[] = {
					0x48, 0x0f, 0xaf, 0xc3,
				};
				machine_code_stream.insert(machine_code_stream.end(), mul_instr, mul_instr + std::size(mul_instr));

				// push rax
				machine_code_stream.push_back(0x50);
				++stack_size;
				break;
			}
			case Instr_Divide: {
				// load into rax and rbx
				load_register_or_immediate(machine_code_stream, stack_size,  left, REG_RAX);
				load_register_or_immediate(machine_code_stream, stack_size, right, REG_RBX);
				
				// mov rcx, rbx ; xor rdx, rdx
				kai_u8 mov[] = {
						0x48, 0x89, 0xd9,
						0x48, 0x31, 0xd2,
				};
				machine_code_stream.insert(machine_code_stream.end(), mov, mov + std::size(mov));

				// do operation
				kai_u8 div_instr[] = {
				//	0x48, 0xf7, 0xf3,
				//	0x48, 0xf7, 0xfd,
					0x48, 0xf7, 0xf1,
				};
				machine_code_stream.insert(machine_code_stream.end(), div_instr, div_instr + std::size(div_instr));

				// push rax
				machine_code_stream.push_back(0x50);
				++stack_size;
				break;
			}

			default: break;
			}
		}

#ifdef DO_DEBUG_PRINT
		std::cout << '\n';
		std::cout << std::hex;
		for (auto&& byte : machine_code_stream) {
			std::cout << (int)byte;
		}
		std::cout << std::dec << '\n';
#endif

		*program = init_program(machine_code_stream.data(), machine_code_stream.size());
	}

	end:
	
	return kai_Result_Success;
}

void* kai_find_procedure(kai_Program program, char const* name, char const* signature) {
	return program->executable_memory;
}
