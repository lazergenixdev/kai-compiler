#include "generation.hpp"
#include <iomanip>
#include <string_view>
#include <kai/debug.h>
#include "program.hpp"
#include "compiler_context.hpp"
#include "builtin_types.hpp"
#define KAI_STRING_HELPER(K) #K
#define KAI_STRING(K) KAI_STRING_HELPER(K)
#define __LOCATION__ __FILE__ ":" KAI_STRING(__LINE__)
// BYTECODE GENERATION AND TYPECHECKING

#if 0
void* operator new(std::size_t sz)
{
    std::printf("1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
    if (void *ptr = std::malloc(sz))
        return ptr;
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
void* operator new[](std::size_t sz)
{
    std::printf("2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
    if (void *ptr = std::malloc(sz))
        return ptr;
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
void operator delete(void* ptr) noexcept
{
    std::puts("3) delete(void*)");
    std::free(ptr);
}
void operator delete(void* ptr, std::size_t size) noexcept
{
    std::printf("4) delete(void*, size_t), size = %zu\n", size);
    std::free(ptr);
}
void operator delete[](void* ptr) noexcept
{
    std::puts("5) delete[](void* ptr)");
    std::free(ptr);
}
void operator delete[](void* ptr, std::size_t size) noexcept
{
    std::printf("6) delete[](void*, size_t), size = %zu\n", size);
    std::free(ptr);
}
#endif

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

// expression_as_type
kai_Type expression_as_type(Dependency_Graph& dg, u32 scope_index, kai_Expr expr) {
	switch (expr->id)
	{
	case kai_Expr_ID_Identifier: {
		auto const& name = expr->source_code;

		auto node_index = dg.resolve_dependency_node(SV(name), scope_index);

		if (!node_index)
			panic_with_message("failed to find identifier value");

		if(dg.type_nodes[*node_index].type->type != kai_Type_Type) {
			std::string s{(char*)name.data, (size_t)name.count};
			panic_with_message("Identifer is not a type! [name = %s]", s.c_str());
		}

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
	return nullptr;
}

u64 Bytecode_Generation_Context::
generate_bytecode(Bytecode_Context& ctx, kai_Stmt body) {
	switch (body->id)
	{
	default: {
		panic_with_message("cannot generate bytecode from AST node [id = %i]", body->id);
	}
	break; case kai_Expr_ID_Procedure_Call: {
		auto call = (kai_Expr_Procedure_Call*)body;

		// TODO: [FIX] assuming there is only one arguement
		auto arg1 = generate_bytecode(ctx, call->arguments[0]);

		auto reg = ctx.next_reg;
		auto call_pos = bytecode_stream.procedure_call(1);
		bytecode_stream.procedure_input(arg1);
		bytecode_stream.procedure_output_count(1);
		bytecode_stream.procedure_output(reg);

		// TODO: this function call needs to be resolved.. somehow....
		bytecode_stream.set_branch_position(call_pos, 0x19);
		
		ctx.next_reg += 1;
		return reg;
	}
	break; case kai_Stmt_ID_If: {
		auto ifs = (kai_Stmt_If*)body;
		// binary operator --> cmp -> branch
		// anything else   --> evaluate -> test -> branch.nz

		// TODO: [FIX] asssuming there should be a comparison here
		auto left = generate_bytecode(ctx, ((kai_Expr_Binary*)(ifs->expr))->left); 
		bytecode_stream.insert_compare_imm(Prim_Type_s64, left, {.s64value = 2});
		
		auto branch_pos = bytecode_stream.insert_conditional_branch(Condition::LESS_OR_EQUAL);

		generate_bytecode(ctx, ifs->body);

		bytecode_stream.set_branch_position(branch_pos, bytecode_stream.position());

		return -1;
	}
	break; case kai_Expr_ID_Number: {
		auto num = (kai_Expr_Number*)body;
		auto reg = ctx.next_reg;
		auto value = Register{.s64value = (signed)num->info.Whole_Part};
		bytecode_stream.insert_load_value(Prim_Type_s64, reg, value);
		ctx.next_reg += 1;
		return reg;
	}
	break; case kai_Stmt_ID_Declaration: {
		auto decl = (kai_Stmt_Declaration*)body;

		std::cout << "decl name: " << SV(decl->name) << "\n";

		if (decl->flags & kai_Decl_Flag_Const) {
		//	dg.resolve_dependency_node(SV(decl->name), ctx.scope_index);
		//	auto reg = generate_bytecode(ctx, decl->expr);
		//	bytecode_stream.insert_return((u32)reg);
			panic_with_message("I don't know what to do here!");
		}
		else {
			auto reg = generate_bytecode(ctx, decl->expr);
			dg.insert_local_variable(reg, nullptr, SV(decl->name), ctx.scope_index);
			return reg;
		}
	}
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

		if (!node_index) {
			auto name = std::string(
				(char*)ident->source_code.data,
				ident->source_code.count
			);
			panic_with_message("could not locate identifier [name = %s]", name.c_str());
		}

		auto  type_node = dg.type_nodes[*node_index];
		auto value_node = dg.value_nodes[*node_index];

		if (type_node.type == (kai_Type)&_type_type_info) {
			panic_with_message("oh no");
		}

		if (value_node.flags& Dependency_Flags::Is_Local_Variable) {
			return value_node.value.u32value;
		}
		else debug_break();
	}
	break; case kai_Expr_ID_Binary: {
		auto bin = (kai_Expr_Binary*)body;

		auto reg1 = (u32)generate_bytecode(ctx, bin->left);

		auto dst_reg = ctx.next_reg++;
		u8 pt = Prim_Type_s64;
		u8 op = binary_to_bytecode_operation(bin->op);
		if (bin->right->id == kai_Expr_ID_Number) {
			Register value;
			value.s64value = ((kai_Expr_Number*)(bin->right))->info.Whole_Part;			
			bytecode_stream.insert_primitive_operation_imm(op, pt, dst_reg, reg1, value);
		}
		else {
			auto reg2 = (u32)generate_bytecode(ctx, bin->right);
			bytecode_stream.insert_primitive_operation(op, pt, dst_reg, reg1, reg2);
		}
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
		panic_with_message("cannot be nullptr");

	switch (info.expr->id)
	{
	default: {
		panic_with_message("dont know how to evaluate expression [id=%i]", info.expr->id);
	}

	break; case kai_Expr_ID_Identifier: {
		node.value.u64value = 0;
	//	panic_with_message("[todo]");
	}

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
		auto ident = (kai_Expr_Identifier*)info.expr;
		auto node_index = dg.resolve_dependency_node(SV(ident->source_code), info.scope);
		if (node_index) {
		//	node.type = (kai_Type)dg.value_nodes[*node_index].value.ptrvalue;
			node.type = dg.type_nodes[*node_index].type;
		}
		else panic_with_message("node does not exist");
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
				expression_as_type(dg, info.scope, proc->input_output->type);
		//	kai_debug_write_type(kai_debug_clib_writer(), type->input_output[i]);
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
	//goto TEST_BYTECODE_GENERATION;
	{
	Bytecode_Generation_Context ctx{};
	context.reset(&info->memory);

	// evaluate nodes that are unevaluated
	std::vector<u32> uneval_values;
	std::vector<u32>  uneval_types;

	Machine_Code output;

	// Create Dependency Graph for the syntax trees
	if(ctx.dg.create(info->trees)) goto error;

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

	std::cout << "------------ generating bytecode ------------\n";
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
	}
TEST_BYTECODE_GENERATION:
	Bytecode_Instruction_Stream stream;
	
	// main :: (x: s32) -> s32 {
	//     x = 0;
	//     if x < 4 then ret x;
	//     ret 1;
	// }
	// %0 = load_value.s64 0
	// compare.s64 %0, 4
	// branch.ge {end_if}
	// ret %0
	// {end_if}
	// %1 = load_value.s64 1
	// ret %1
	stream.insert_load_value(Prim_Type_s64, PROC_ARG(0), {.s64value = -12});
	stream.insert_compare_imm(Prim_Type_s64, PROC_ARG(0), {.s64value = 4});
	auto if_end_branch = stream.insert_conditional_branch(Condition::GREATER_OR_EQUAL);
	stream.insert_return(PROC_ARG(0));
	auto end_pos = stream.position();
	stream.insert_load_value(Prim_Type_s64, 0, {.s64value = 1});
	stream.insert_return(0);

	/////////////
	stream.set_branch_position(if_end_branch, end_pos);

	stream.debug_print();

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
