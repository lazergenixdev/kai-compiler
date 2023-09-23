#include "generation.hpp"
#include <iomanip>
#include <kai/debug.h>
#include "program.hpp"
#include "compiler_context.hpp"
// BYTECODE GENERATION AND TYPECHECKING

u8 binary_to_bytecode_operation(u32 tree_op) {
	switch (tree_op)
	{
	case '+': return Operation_Add;
	case '*': return Operation_Mul;
	case '-': return Operation_Sub;
	case '/': return Operation_Div;
	default: return 255;
	}
}

kai_Type type_of_expression(Dependency_Graph& dg, u32 scope_index, kai_Expr expr) {
	switch (expr->id)
	{
	case kai_Expr_ID_Identifier: {
		auto const& name = expr->source_code;

		auto node_index = dg.resolve_dependency_node(SV(name), scope_index);

		if (!node_index)
			panic_with_message("failed to find identifier value");

	//	if(dg.type_nodes[node_index].type->type != kai_Type_Type)
	//		panic_with_message("Identifer is not a type!");

		return (kai_Type)dg.value_nodes[*node_index].value.ptrvalue;
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

u64 Bytecode_Generation_Context::
generate_bytecode(Bytecode_Context& ctx, kai_Stmt body) {
	switch (body->id)
	{
	default: debug_break();
//	evaluate_value_from_expression(node.value, info.expr); break;
	break; case kai_Stmt_ID_Compound: {
		auto comp = (kai_Stmt_Compound*)body;
		ctx.scope_index = comp->_scope;
		for_n(comp->count) {
			generate_bytecode(ctx, comp->statements[i]);
		}
		ctx.scope_index = dg.scopes[comp->_scope].parent;
	}
	break; case kai_Stmt_ID_Return: {
		auto ret = (kai_Stmt_Return*)body;
		auto reg = generate_bytecode(ctx, ret->expr);
		bytecode_stream.insert_return((u32)reg);
		return 0;
	}
	break; case kai_Expr_ID_Identifier: {
		auto ident = (kai_Expr_Identifier*)body;
		auto node_index = dg.resolve_dependency_node(
			SV(ident->source_code),
			ctx.scope_index
		);

		if (!node_index) debug_break();

		auto value_node = dg.value_nodes[*node_index];

		if (value_node.flags& Dependency_Flags::Is_Local_Variable) {
			return value_node.value.u32value;
		}
		else debug_break();
	}
	break; case kai_Expr_ID_Binary: {
		auto bin = (kai_Expr_Binary*)body;

		auto reg1 = (u32)generate_bytecode(ctx, bin->left);
		auto reg2 = (u32)generate_bytecode(ctx, bin->right);

		auto dst_reg = ctx.next_reg++;
		u8 pt = Prim_Type_s64;
		u8 op = binary_to_bytecode_operation(bin->op);

		bytecode_stream.insert_primitive_operation(op, pt, dst_reg, reg1, reg2);
		return dst_reg;
	}
	break;
	}
	return 0;
}

void Bytecode_Generation_Context::evaluate_value(u32 node_index)
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

		auto restore_size = dg.node_infos.size();
		auto proc_type = (kai_Type_Info_Procedure*)dg.type_nodes[node_index].type;
		auto const n = proc->param_count + proc->ret_count;
		for_n(n) {
			auto name = proc->input_output[i].name;
			auto type = proc_type->input_output[i];
			dg.insert_procedure_input(i, type, SV(name), proc->_scope);
		}

		Bytecode_Context ctx;
		ctx.scope_index = proc->_scope;
		node.value.u64value = bytecode_stream.position();
		generate_bytecode(ctx, proc->body);

		dg.value_nodes.resize(restore_size);
		dg.type_nodes.resize(restore_size);
		for_n(n) {
			auto name = dg.node_infos[i + restore_size].name;
			dg.scopes[proc->_scope].map.erase(name);
		}
		dg.node_infos.resize(restore_size);
	}
	break;
	}

	std::cout << "evaluated value: " << info.name << '\n';
}

void Bytecode_Generation_Context::evaluate_type(u32 node_index)
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

		for_n(total_count) {
			type->input_output[i] =
				type_of_expression(dg, info.scope, proc->input_output->type);
		}

		node.type = (kai_Type)type;
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

	Machine_Code output;

	if(ctx.dg.create(info->trees))
		goto error;

	{
		int i;
		i = 0;
		for (auto&& n : ctx.dg.value_nodes) {
			if (!(n.flags & Evaluated))
				uneval_values.emplace_back(i);
			++i;
		}
		i = 0;
		for (auto&& n : ctx.dg.type_nodes) {
			if (!(n.flags & Evaluated))
				uneval_types.emplace_back(i);
			++i;
		}
	}

//	return kai_Result_Error_Fatal;
	while(uneval_values.size() != 0 || uneval_types.size() != 0) {
		// evaluate types
		for_iter(uneval_types) {
			if (ctx.dg.all_evaluated(ctx.dg.type_nodes[*it].dependencies)) {
				ctx.evaluate_type(*it);
				uneval_types.erase(it);
				break;
			}
		}
		// evaluate value
		for_iter(uneval_values) {
			if (ctx.dg.all_evaluated(ctx.dg.value_nodes[*it].dependencies)) {
				ctx.evaluate_value(*it);
				uneval_values.erase(it);
				break;
			}
		}
	}

#if ENABLE_DEBUG_PRINT
	std::cout << '\n';
	for_n(ctx.dg.node_infos.size()) {
		auto& t    = ctx.dg.type_nodes[i];
		auto& info = ctx.dg.node_infos[i];
		std::cout << info.name << '\n';
		kai_debug_write_type(kai_debug_clib_writer(), t.type);
		std::cout << '\n';
	}

	ctx.bytecode_stream.debug_print();
#endif

	output = generate_machine_code(
		ctx.bytecode_stream.stream.data(),
		ctx.bytecode_stream.stream.size()
	);

	*program = init_program(output);
	free(output.data);

	if (context.error.result) {
	error:
		*info->error = context.error;
		return context.error.result;
	}
	else *info->error = {};

	return kai_Result_Success;

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
