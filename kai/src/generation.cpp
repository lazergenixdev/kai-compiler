#include "generation.hpp"
#include "program.hpp"
#include <iomanip>
#include <cassert>

//VirtualReg Generate_Instruction(Type_Checker_Context& ctx, kai_Expr root) {
//	switch (root->id)
//	{
//	case kai_Expr_ID_Identifier: {
//		auto ident = (kai_Expr_Identifier*)root;
//		auto it = ctx.ident_index_table.find(view(ident->source_code));
//		if (it == ctx.ident_index_table.end()) {
//			return {-1};
//		}
//		return {it->second, 0};
//	}
//	case kai_Expr_ID_Number: {
//		auto num = (kai_Expr_Number*)root;
//		return {VirtualReg_Immediate, num->info.Whole_Part};
//	}
//	case kai_Expr_ID_Binary: {
//		auto binary = (kai_Expr_Binary*)root;
//
//		ByteCodeInstruction instr;
//		instr.intruction = translate_instruction(binary->op);
//		instr.left       = Generate_Instruction(ctx, binary->left);
//		instr.right      = Generate_Instruction(ctx, binary->right);
//		auto index = (int)ctx.intruction_stream.size();
//		ctx.intruction_stream.emplace_back(instr);
//
//		return {index};
//	}
//
//	default:return {-1};
//	}
//}
//
//void Generate_Statement_Instruction(Type_Checker_Context& ctx, kai_Stmt root) {
//	switch (root->id)
//	{
//	case kai_Stmt_ID_Return: {
//		auto ret = (kai_Stmt_Return*)root;
//
//		ByteCodeInstruction instr;
//		instr.intruction = Instr_Return;
//		instr.left = Generate_Instruction(ctx, ret->expr);
//		ctx.intruction_stream.emplace_back(instr);
//		break;
//	}
//	case kai_Stmt_ID_Declaration: {
//		auto decl = (kai_Stmt_Declaration*)root;
//
//		auto instr = Generate_Instruction(ctx, decl->expr);
//		ctx.ident_index_table.emplace(view(decl->name), instr.index);
//		break;
//	}
//	case kai_Stmt_ID_Compound: {
//		auto comp = (kai_Stmt_Compound*)root;
//		for range(comp->count) {
//			Generate_Statement_Instruction(ctx, comp->statements[i]);
//		}
//		break;
//	}
//
//	default:break;
//	}
//}

kai_result kai_create_program(kai_Program_Create_Info* info, kai_Program* program)
{
	Bytecode_Instruction_Stream stream;
	
	// main :: (x: s32) -> s32 {
	//     a := x + 1;
	//     b := x * 2;
	//     h := fn(a, b);
	//     ret h + fn(h, b);
	// }
	// %0 = add.s32 arg(0), 1
	// %1 = mul.s32 arg(0), 2
	// %2 = call "fn" (%0, %1)
	// %3 = call "fn" (%2, %1)
	// %4 = add.s32 %2, %3
	// ret %4
	stream.insert_primitive_operation_imm(Operation_Add, Prim_Type_s32, 0, PROC_ARG(0), {.s32value = 1});
	stream.insert_primitive_operation_imm(Operation_Mul, Prim_Type_s32, 1, PROC_ARG(0), {.s32value = 2});

	auto call_pos1 = stream.procedure_call(2);
	stream.procedure_input(0);
	stream.procedure_input(1);
	stream.procedure_output_count(1); // we only want the first output
	stream.procedure_output(2);       // put output into register # 2

	auto call_pos2 = stream.procedure_call(2);
	stream.procedure_input(2);
	stream.procedure_input(1);
	stream.procedure_output_count(1); // we only want the first output
	stream.procedure_output(3);       // put output into register # 3
	
	stream.insert_primitive_operation(Operation_Add, Prim_Type_s32, 4, 2, 3);
	stream.insert_return(4);
// ----------------------------------------

	auto fn_pos = stream.position();
	// fn :: (a: s32, b : s32) -> s32, s32 {
	//     t := a + 2;
	//     ret t * b, t;
	// }
	// %0 = add.s32 arg(0), 2
	// %1 = mul.s32 %0, arg(1)
	// return %1, %0
	stream.insert_primitive_operation_imm(Operation_Add, Prim_Type_s32, 0, PROC_ARG(0), {.s32value = 2});
	stream.insert_primitive_operation(Operation_Mul, Prim_Type_s32, 1, 0, PROC_ARG(1));
	stream.insert_return({1, 0});

	// here we link our procedure call in main, to "fn"s position
	// (a procedure call is just a more sophisticated branch instruction)
	stream.set_branch_position(call_pos1, fn_pos);
	stream.set_branch_position(call_pos2, fn_pos);

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// DEBUG
	std::cout << std::hex << std::setfill('0');
	for range(stream.stream.size()) {
		std::cout << std::setw(2) << (int)stream.stream[i];
		if ((i&0b1111) == 0b1111) std::cout << '\n';
		else std::cout << ' ';
	}
	std::cout << '\n';
	std::cout << std::dec << std::setfill(' ');
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	Bytecode_Interpreter interp{
		(u8*)stream.stream.data(),
		(u64)stream.stream.size()
	};

	Register x;
	x.s32value = 4;
	
	interp.begin_procedure_input();
	interp.add_procedure_input(x);
	interp.call_procedure(0);
	
	// Run interpreter
	std::cout << '\n';
	interp.debug_output_registers();
	while (interp.step()) {
		interp.debug_output_registers();
	}
	
	std::cout << "result = ";
	std::cout << interp.registers[0].s32value;
	std::cout << '\n';

	return kai_Result_Error_Fatal;
}

void* kai_find_procedure(kai_Program program, char const* name, char const* signature) {
	return program->executable_memory;
}
