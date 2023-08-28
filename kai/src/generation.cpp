#include "generation.hpp"
#include "program.hpp"
#include <iomanip>
#include <cassert>
// BYTECODE GENERATION AND TYPECHECKING

kai_Type type_of_expression(Dependency_Graph& dg, u32 scope_index, kai_Expr expr) {
	switch (expr->id)
	{
	case kai_Expr_ID_Identifier: {
		auto const& name = expr->source_code;

		u32 node_index = -1;
		while (scope_index != -1) {
			auto& scope = dg.scopes[scope_index];
			auto it = scope.map.find(view(name));
			if (it != scope.map.end()) {
				node_index = it->second;
				break;
			}
			scope_index = scope.parent;
		}

		if (node_index == -1)
			panic_with_message("failed to find identifier value");

	//	if(dg.type_nodes[node_index].type->type != kai_Type_Type)
	//		panic_with_message("Identifer is not a type!");

		return (kai_Type)dg.value_nodes[node_index].value.ptrvalue;
	}
	case kai_Expr_ID_Number: {
		debug_log("number");
		return nullptr;
	}

	// this is not going to be trivial to implement..
	// need to interpret some bytecode for this
	case kai_Expr_ID_Procedure_Call: {
		debug_log("procedure call");
		return nullptr;
	}
	case kai_Expr_ID_Procedure: {
		debug_log("procedure");
		return nullptr;
	}
	default:break;
	}
}

void generate_bytecode_for_procedure(kai_Stmt body) {

}

void evaluate_value(Dependency_Graph& dg, u32 node_index)
{
	auto& node = dg.value_nodes[node_index];
	auto& info = dg.node_infos[node_index];

	node.flags |= Evaluated;

	if (info.expr == nullptr)
		panic_with_message("Node was not given an expression, this is bad.");

	switch (info.expr->id)
	{
	default:
	//	evaluate_value_from_expression(node.value, info.expr); break;
	break; case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)info.expr;
		generate_bytecode_for_procedure(proc->body);
	}
	break;
	}

	std::cout << "evaluated value: " << info.name << '\n';
}

void evaluate_type(Dependency_Graph& dg, u32 node_index)
{
	auto& node = dg.type_nodes[node_index];
	auto& info = dg.node_infos[node_index];
	
	node.flags |= Evaluated;

	switch (info.expr->id)
	{
	default:
	break;case kai_Expr_ID_Number: {
		std::cout << "number found\n";
	}
	break;case kai_Expr_ID_Identifier: {
		std::cout << "identifier found\n";
	}
	break;case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)info.expr;

		auto type = ctx_alloc_T(kai_Type_Info_Procedure);
		type->type = kai_Type_Procedure;
		type->param_count = proc->param_count;
		type->ret_count = proc->ret_count;

		auto total_count = type->param_count + type->ret_count;

		type->input_output = ctx_alloc_array(kai_Type, total_count);

		for range(total_count) {
			type->input_output[i] =
				type_of_expression(dg, info.scope, proc->input_output->type);
		}

		break;
	}
	break;case kai_Expr_ID_Procedure_Call: {
		std::cout << "procedure call found\n";
	}
	}

	std::cout << "evaluated type: " << info.name << '\n';
}
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


kai_result kai_create_program(kai_Program_Create_Info* info, kai_Program* program)
{
	Bytecode_Generation_Context ctx{};
	context.reset(&info->memory);

	// evaluate nodes that are unevaluated
	std::vector<u32> uneval_values;
	std::vector<u32>  uneval_types;

	Dependency_Graph dg;
	if(dg.create(info->module))
		goto error;

	{
		int i;
		i = 0;
		for (auto&& n : dg.value_nodes) {
			if (!(n.flags & Evaluated))
				uneval_values.emplace_back(i);
			++i;
		}
		i = 0;
		for (auto&& n : dg.type_nodes) {
			if (!(n.flags & Evaluated))
				uneval_types.emplace_back(i);
			++i;
		}
	}

//	return kai_Result_Success;
	while(uneval_values.size() != 0 || uneval_types.size() != 0) {
		// evaluate types
		for iterate(uneval_types) {
			if (dg.all_evaluated(dg.type_nodes[*it].dependencies)) {
				evaluate_type(dg, *it);
				uneval_types.erase(it);
				break;
			}
		}
		// evaluate value
		for iterate(uneval_values) {
			if (dg.all_evaluated(dg.value_nodes[*it].dependencies)) {
				evaluate_value(dg, *it);
				uneval_values.erase(it);
				break;
			}
		}
	}

	if (context.error.result) {
	error:
		// This is terrible
		context.error.location.file = info->module->source_filename;
		if (context.error.next) context.error.next->location.file = info->module->source_filename;

		*info->error_info = context.error;
		return context.error.result;
	}
	else *info->error_info = {};

	return kai_Result_Error_Fatal;

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
	std::cout << '\n';
	stream.debug_print();
	return kai_Result_Error_Fatal;

	std::cout << '\n';
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
