#include "generation.hpp"
#include "program.hpp"
#include <iomanip>
#include <cassert>
#define view(S) std::string_view( (char*)S.data, S.count )

bool Bytecode_Generation_Context::insert_value_dependencies(Dependency_List& deps, kai_u32 scope, kai_Expr expr) {
	if (expr == nullptr) { panic_with_message("null expression\n"); return true; }

    switch (expr->id)
    {
    case kai_Expr_ID_Identifier: {
        auto name = view(expr->source_code);
		auto result = resolve_dependency_node(name, scope);
        
		if (!result)
			return error_not_declared(expr->source_code, expr->line_number);

		deps.emplace_back(Dependency::VALUE, *result);
        break;
    }

    case kai_Expr_ID_Number: break;
    case kai_Expr_ID_String: break;

    case kai_Expr_ID_Unary: {
        auto unary = (kai_Expr_Unary*)expr;
		return insert_value_dependencies(deps, scope, unary->expr);
    }

    case kai_Expr_ID_Binary: {
        auto binary = (kai_Expr_Binary*)expr;
		if (insert_value_dependencies(deps, scope, binary->left)) return true;
        return insert_value_dependencies(deps, scope, binary->right);
    }

    case kai_Expr_ID_Procedure_Call: {
        auto call = (kai_Expr_Procedure_Call*)expr;

		return insert_type_dependencies(deps, scope, call->proc);

        for (kai_u32 i = 0; i < call->arg_count; ++i) {
			insert_value_dependencies(deps, scope, call->arguments[i]);
        }
        break;
    }

    case kai_Expr_ID_Procedure_Type: {
        auto proc = (kai_Expr_Procedure_Type*)expr;

		panic_with_message("procedure type\n");
        //kai_int total = proc->parameter_count + proc->ret_count;
        //for (int i = 0; i < total; ++i) {
        //    print_tree(ctx, proc->types[i]);
        //}
        break;
    }

    case kai_Expr_ID_Procedure: {
        auto proc = (kai_Expr_Procedure*)expr;

		Scope& local_scope = dg.scopes[proc->_scope];

		// Insert procedure input names to local scope
		auto const n = proc->param_count;
        for (int i = 0; i < n; ++i) {
			auto const& parameter = proc->input_output[i];
			auto name = view(parameter.name);

			auto it = local_scope.map.find(name);
			if (it != local_scope.map.end())
				return error_redefinition(parameter.name, proc->line_number);

			local_scope.map.emplace(name, LOCAL_VARIABLE_INDEX);
        }

		if (insert_value_dependencies_proc(deps, proc->_scope, proc->body))
			return true;

		remove_all_locals(local_scope);
		break;
    }

    case kai_Stmt_ID_Declaration: {

        break;
    }

    case kai_Stmt_ID_Return: {
		panic_with_message("return\n");
        break;
    }

    case kai_Stmt_ID_Compound: {
        auto comp = (kai_Stmt_Compound*)expr;

		panic_with_message("compound statement\n");
        //auto const n = comp->count;
        //for (int i = 0; i < n; ++i) {
        //    print_tree(ctx, comp->statements[i]);
        //}
        break;
    }

    default: panic_with_message("undefined expression\n");
    }

	return false;
}

bool Bytecode_Generation_Context::insert_type_dependencies(Dependency_List& deps, kai_u32 scope, kai_Expr expr)
{
	if (expr == nullptr) { panic_with_message("null expression\n"); return false; }

	switch (expr->id)
	{
	case kai_Expr_ID_Identifier: {
		auto name = view(expr->source_code);

		auto result = resolve_dependency_node_procedure(name, &dg.scopes[scope]);

		if (!result)
			return error_not_declared(expr->source_code, expr->line_number);

		if (*result != LOCAL_VARIABLE_INDEX) // DO NOT depend on local variables
			deps.emplace_back(Dependency::TYPE, *result);
		break;
	}

	case kai_Expr_ID_Unary: {
		auto unary = (kai_Expr_Unary*)expr;
		return insert_type_dependencies(deps, scope, unary->expr);
	}

	case kai_Expr_ID_Binary: {
		auto binary = (kai_Expr_Binary*)expr;
		catch_and_throw(insert_type_dependencies(deps, scope, binary->left));
		return insert_type_dependencies(deps, scope, binary->right);
	}

	case kai_Expr_ID_Procedure_Call: {
		auto call = (kai_Expr_Procedure_Call*)expr;

		return insert_type_dependencies(deps, scope, call->proc);

		for (kai_u32 i = 0; i < call->arg_count; ++i) {
			insert_value_dependencies(deps, scope, call->arguments[i]);
		}
		break;
	}

	case kai_Expr_ID_Procedure_Type: {
		auto proc = (kai_Expr_Procedure_Type*)expr;
		panic_with_message("procedure type\n");
		//kai_int total = proc->parameter_count + proc->ret_count;
		//for (int i = 0; i < total; ++i) {
		//    print_tree(ctx, proc->types[i]);
		//}
		break;
	}

	case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)expr;
		auto const n = proc->param_count + proc->ret_count;
		for range(n) {
			auto param = proc->input_output[i];
			catch_and_throw(insert_value_dependencies(deps, scope, param.type));
		}
		break;
	}

	case kai_Stmt_ID_Declaration: {
		auto decl = (kai_Stmt_Declaration*)expr;

		//// Already has a dependency node
		//if (decl->flags & kai_Decl_Flag_Const) return false;

		//auto name = view(decl->name);
		//auto& s = dg.scopes[scope];

		//// Insert into local scope (if not already defined)
		//auto it = s.map.find(name);
		//if (it != s.map.end())
		//	return error_redefinition(decl->name, decl->line_number);
		//s.map.emplace(name, LOCAL_VARIABLE_INDEX);

		//// Look into it's definition to find possible dependencies
		//return insert_value_dependencies_proc(deps, scope, decl->expr);
		break;
	}

	case kai_Stmt_ID_Return: {
		auto ret = (kai_Stmt_Return*)expr;
		return insert_type_dependencies(deps, scope, ret->expr);
	}

	case kai_Stmt_ID_Compound: {
		//auto comp = (kai_Stmt_Compound*)expr;

		//auto const n = comp->count;
		//for range(n) {
		//	catch_and_throw(insert_type_dependencies(deps, comp->_scope, comp->statements[i]));
		//}

		//Scope& s = dg.scopes[comp->_scope];
		//remove_all_locals(s);
		break;
	}

	default: break;
	}

	return false;
}

bool Bytecode_Generation_Context::insert_value_dependencies_proc(Dependency_List& deps, kai_u32 scope, kai_Expr expr)
{
	if (expr == nullptr) { panic_with_message("null expression\n"); return true; }

	switch (expr->id)
	{
	case kai_Expr_ID_Identifier: {
		auto name = view(expr->source_code);

		auto result = resolve_dependency_node_procedure(name, &dg.scopes[scope]);

		if (!result)
			return error_not_declared(expr->source_code, expr->line_number);

		if(*result != LOCAL_VARIABLE_INDEX) // DO NOT depend on local variables
			deps.emplace_back(Dependency::VALUE, *result);
		break;
	}

	case kai_Expr_ID_Unary: {
		auto unary = (kai_Expr_Unary*)expr;
		return insert_value_dependencies_proc(deps, scope, unary->expr);
	}

	case kai_Expr_ID_Binary: {
		auto binary = (kai_Expr_Binary*)expr;
		catch_and_throw(insert_value_dependencies_proc(deps, scope, binary->left));
		return insert_value_dependencies_proc(deps, scope, binary->right);
	}

	case kai_Expr_ID_Procedure_Call: {
		auto call = (kai_Expr_Procedure_Call*)expr;

		insert_type_dependencies(deps, scope, call->proc);
		for (kai_u32 i = 0; i < call->arg_count; ++i) {
			catch_and_throw(insert_value_dependencies_proc(deps, scope, call->arguments[i]));
		}
		break;
	}

	case kai_Expr_ID_Procedure_Type: {
		auto proc = (kai_Expr_Procedure_Type*)expr;
		panic_with_message("procedure type\n");
		//kai_int total = proc->parameter_count + proc->ret_count;
		//for (int i = 0; i < total; ++i) {
		//    print_tree(ctx, proc->types[i]);
		//}
		break;
	}

	case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)expr;

		Scope& local_scope = dg.scopes[proc->_scope];

		// Insert procedure input names to local scope
		auto const n = proc->param_count;
		for (int i = 0; i < n; ++i) {
			auto const& parameter = proc->input_output[i];
			auto name = view(parameter.name);

			auto it = local_scope.map.find(name);
			if (it != local_scope.map.end())
				return error_redefinition(parameter.name, proc->line_number);

			local_scope.map.emplace(name, LOCAL_VARIABLE_INDEX);
		}

		if (insert_value_dependencies_proc(deps, proc->_scope, proc->body))
			return true;

		remove_all_locals(local_scope);
		break;
	}

	case kai_Stmt_ID_Declaration: {
		auto decl = (kai_Stmt_Declaration*)expr;

		// Already has a dependency node
		if (decl->flags & kai_Decl_Flag_Const) return false;

		auto name = view(decl->name);
		auto& s = dg.scopes[scope];

		// Insert into local scope (if not already defined)
		auto it = s.map.find(name);
		if (it != s.map.end())
			return error_redefinition(decl->name, decl->line_number);
		s.map.emplace(name, LOCAL_VARIABLE_INDEX);

		// Look into it's definition to find possible dependencies
		return insert_value_dependencies_proc(deps, scope, decl->expr);
	}

	case kai_Stmt_ID_Return: {
		auto ret = (kai_Stmt_Return*)expr;
		return insert_value_dependencies_proc(deps, scope, ret->expr);
	}

	case kai_Stmt_ID_Compound: {
		auto comp = (kai_Stmt_Compound*)expr;

		auto const n = comp->count;
		for range(n) {
			catch_and_throw(insert_value_dependencies_proc(deps, comp->_scope, comp->statements[i]));
		}

		Scope& s = dg.scopes[comp->_scope];
		remove_all_locals(s);
		break;
	}

	default: break;
	}

	return false;
}

// BYTECODE GENERATION AND TYPECHECKING
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#define msg(S) std::cout << '[' << __FUNCTION__ << ']' << " " S "\n";

kai_Type type_of_expression(Bytecode_Generation_Context& ctx, u32 scope_index, kai_Expr expr) {
	switch (expr->id)
	{
	case kai_Expr_ID_Identifier: {
		auto const& name = expr->source_code;

		u32 node_index = -1;
		while (scope_index != -1) {
			auto& scope = ctx.dg.scopes[scope_index];
			auto it = scope.map.find(view(name));
			if (it != scope.map.end()) {
				node_index = it->second;
				break;
			}
			scope_index = scope.parent;
		}

		if (node_index == -1)
			panic_with_message("failed to find identifier value");

		if(ctx.dg.type_nodes[node_index].type->type != kai_Type_Type)
			panic_with_message("Identifer is not a type!");

		return (kai_Type)ctx.dg.value_nodes[node_index].value.ptrvalue;
	}
	case kai_Expr_ID_Number: {
		msg("number");
		return nullptr;
	}
	case kai_Expr_ID_Procedure_Call: {
		msg("procedure call");
		return nullptr;
	}
	case kai_Expr_ID_Procedure: {
		msg("procedure");
		return nullptr;
	}
	default:break;
	}
}

void Bytecode_Generation_Context::evaluate_value(u32 node_index)
{
	auto& node = dg.value_nodes[node_index];
	
	if (node.flags & Evaluated) return;

	auto& info = dg.node_infos[node_index];

	node.flags |= Evaluated;

	if (info.expr == nullptr)
		panic_with_message("Node was not given an expression, this is bad.");

	//switch (info.expr->id)
	//{
	//case kai_Expr_ID_Procedure: {
	//	auto proc = (kai_Expr_Procedure*)info.expr;
	//	generate_bytecode_for_procedure(proc->body);
	//}
	//default:evaluate_value_from_expression(node.value, info.expr); break;
	//}

	std::cout << "evaluated value: " << info.name << '\n';
}

void Bytecode_Generation_Context::evaluate_type(u32 node_index)
{
	auto& node = dg.type_nodes[node_index];
	if (node.flags & Evaluated) return;

	auto& info = dg.node_infos[node_index];
	
	node.flags |= Evaluated;

	switch (info.expr->id)
	{
	case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)info.expr;

		auto type = (kai_Type_Info_Procedure*)memory.alloc(memory.user, sizeof(kai_Type_Info_Procedure));
		type->type = kai_Type_Procedure;

		type->param_count = proc->param_count;
		type->ret_count = proc->ret_count;

		auto total_count = type->param_count + type->ret_count;

		type->param_ret_types = (kai_Type*)memory.alloc(memory.user, sizeof(kai_Type) * total_count);

		for range(total_count) {
			type->param_ret_types[i] =
				type_of_expression(*this, info.scope, proc->input_output->type);
		}

		break;
	}
	case kai_Expr_ID_Number: {
		std::cout << "number found\n";
		break;
	}
	case kai_Expr_ID_Identifier: {
		std::cout << "identifier found\n";
		break;
	}
	case kai_Expr_ID_Procedure_Call: {
		std::cout << "procedure call found\n";
		break;
	}
	default:break;
	}

	std::cout << "evaluated type: " << info.name << '\n';
}
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

kai_Type_Info_Integer _s64_info = {
	.type = kai_Type_Integer,
	.bits = 64,
	.is_signed = true,
};

kai_Type_Info _Type_info = {kai_Type_Type};

bool Bytecode_Generation_Context::generate_dependency_graph()//-> failed: bool
{
	dg.value_nodes.reserve(128);
	dg.type_nodes.reserve(128);
	dg.node_infos.reserve(128);
	dg.scopes.reserve(64);
	dg.scopes.emplace_back();

	auto& global_scope = dg.scopes[0];
	{
		auto index = (kai_u32)dg.value_nodes.size();

		Value_Dependency_Node vnode;
		vnode.flags = Evaluated;
		vnode.value.ptrvalue = &_s64_info;

		Type_Dependency_Node tnode;
		tnode.flags = Evaluated;
		tnode.type = &_Type_info;

		Dependency_Node_Info info;
		info.name = "s64";
		info.scope = GLOBAL_SCOPE;
		info.expr = nullptr;

		global_scope.map.emplace(info.name, index);

		dg.value_nodes.emplace_back(vnode);
		dg.type_nodes.emplace_back(tnode);
		dg.node_infos.emplace_back(info);
	}

	// Make nodes for every statement
	for range(mod->toplevel_count) {
		auto tl = mod->toplevel_stmts[i];
		if (insert_node_for_statement(tl, false)) return true;
	}

	// Insert dependencies for each node
	for range(dg.node_infos.size()) {
		auto& vnode = dg.value_nodes[i];
		auto& tnode = dg.type_nodes[i];
		auto& info = dg.node_infos[i];

		if (!(vnode.flags&Evaluated) && insert_value_dependencies(vnode.dependencies, info.scope, info.expr))
			return true;

		if (!(tnode.flags&Evaluated) && insert_type_dependencies(tnode.dependencies, info.scope, info.expr))
			return true;
	}

	return false;
}

bool Bytecode_Generation_Context::insert_node_for_statement(kai_Stmt stmt, bool in_proc, kai_u32 scope) {
	switch (stmt->id)
	{
	case kai_Stmt_ID_Declaration: {
		auto decl = (kai_Stmt_Declaration*)stmt;
		if (insert_node_for_declaration(decl, in_proc, scope))
			return true;
		break;
	}
	case kai_Stmt_ID_Compound: {
		auto comp = (kai_Stmt_Compound*)stmt;
		auto new_scope_index = (kai_u32)dg.scopes.size();
		Scope new_scope;
		new_scope.parent = scope;
		dg.scopes.emplace_back(new_scope);
		comp->_scope = new_scope_index;
		for range(comp->count) {
			if (insert_node_for_statement(comp->statements[i], in_proc, new_scope_index))
				return true;
		}
		break;
	}
	default:break;
	}
	return false;
}

bool Bytecode_Generation_Context::insert_node_for_declaration(kai_Stmt_Declaration* decl, bool in_proc, kai_u32 scope)
{
	if (in_proc && !(decl->flags & kai_Decl_Flag_Const))
		return false;

	auto name = view(decl->name);

	Scope* s = &dg.scopes[scope];

	// Does this declaration already exist for this Scope?
	auto it = s->map.find(name);
	if (it != s->map.end()) {
		return error_redefinition(decl->name, decl->line_number);
	}

	// Add node to dependency graph
	{
		auto index = (kai_u32)dg.value_nodes.size();

		Value_Dependency_Node vnode = {};
		{ // @Note: is this dependency always correct?
			Dependency dep;
			dep.node_index = index;
			dep.what = Dependency::TYPE;
			vnode.dependencies.emplace_back(dep);
		}

		Type_Dependency_Node tnode = {};

		Dependency_Node_Info info;
		info.name = name;
		info.scope = scope;
		info.expr = decl->expr;
		info.line_number = decl->line_number;

		s->map.emplace(info.name, index);

		dg.value_nodes.emplace_back(vnode);
		dg.type_nodes.emplace_back(tnode);
		dg.node_infos.emplace_back(info);
	}

	switch (decl->expr->id)
	{
	// rabbit hole goes deeper...
	case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)decl->expr;

		auto new_scope_index = (kai_u32)dg.scopes.size();
		Scope new_scope;
		new_scope.parent = scope;
		new_scope.is_proc_scope = true;
		dg.scopes.emplace_back(new_scope);

		proc->_scope = new_scope_index;

		return insert_node_for_statement(proc->body, true, new_scope_index);
	}

	default:break;
	}

	return false;
}

#define RESET "\x1b[0m"
static char const* _color_table[] = {
	"\x1b[31m", "\x1b[91m",
	/*"\x1b[32m",*/ "\x1b[92m",
	"\x1b[33m", "\x1b[93m",
	"\x1b[34m", "\x1b[94m",
	"\x1b[35m", "\x1b[95m",
	"\x1b[36m", "\x1b[96m",
};
#define GET_COLOR(N) _color_table[N % std::size(_color_table)]
void print_dependecies(HL_Dependency_Graph& dg) {
	for range(dg.value_nodes.size()) {
		auto const& info = dg.node_infos[i];
		auto const& node = dg.value_nodes[i];
		auto const& name = info.name;

		std::cout << GET_COLOR(i);
		std::cout << name << ':' << info.scope << ' ';
		std::cout << RESET;

		if (node.flags & Dependency_Flag::Evaluated) {
			std::cout << "is already evaluated\n";
			continue;
		}

		std::cout << "depends on {";
		int k = 0;
		for (auto const& dep : node.dependencies) {
			auto const& info = dg.node_infos[dep.node_index];
			if (dep.what == Dependency::TYPE)
				std::cout << "type_of(";
			std::cout << GET_COLOR(dep.node_index);
			std::cout << info.name << ':' << info.scope;
			std::cout << RESET;
			if (dep.what == Dependency::TYPE)
				std::cout << ")";
			if (++k < node.dependencies.size()) std::cout << ", ";
		}
		std::cout << "}\n";
	}
	for range(100) std::cout << '-';
	std::cout << '\n';
	for range(dg.type_nodes.size()) {
		auto const& info = dg.node_infos[i];
		auto const& node = dg.type_nodes[i];
		auto const& name = info.name;

		std::cout << "type_of(";
		std::cout << GET_COLOR(i);
		std::cout << name << ':' << info.scope;
		std::cout << RESET;
		std::cout << ") ";

		if (node.flags & Dependency_Flag::Evaluated) {
			std::cout << "is already evaluated\n";
			continue;
		}

		std::cout << "depends on {";
		int k = 0;
		for (auto const& dep : node.dependencies) {
			auto const& info = dg.node_infos[dep.node_index];
			if (dep.what == Dependency::TYPE)
				std::cout << "type_of(";
			std::cout << GET_COLOR(dep.node_index);
			std::cout << info.name << ':' << info.scope;
			std::cout << RESET;
			if (dep.what == Dependency::TYPE)
				std::cout << ")";
			if (++k < node.dependencies.size()) std::cout << ", ";
		}
		std::cout << "}\n";
	}
}
#undef NODE 
#undef DEP 
#undef RESET

kai_result kai_create_program(kai_Program_Create_Info* info, kai_Program* program)
{
	Bytecode_Generation_Context ctx{};
	ctx.mod    = info->module;
	ctx.memory = info->memory;

	// evaluate nodes that are unevaluated
	std::vector<u32> uneval_values;
	std::vector<u32>  uneval_types;

	std::cout << '\n';
	if (ctx.generate_dependency_graph()) {
		goto error;
	}

	print_dependecies(ctx.dg);
	std::cout << '\n';

	if (ctx.detect_circular_dependencies()) {
		goto error;
	}

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
		for iterate(uneval_types) {
			if (ctx.all_evaluated(ctx.dg.type_nodes[*it].dependencies)) {
				ctx.evaluate_type(*it);
				uneval_types.erase(it);
				break;
			}
		}
		// evaluate value
		for iterate(uneval_values) {
			if (ctx.all_evaluated(ctx.dg.value_nodes[*it].dependencies)) {
				ctx.evaluate_value(*it);
				uneval_values.erase(it);
				break;
			}
		}
	}

	if (ctx.error_info.result) {
	error:
		ctx.error_info.location.file = KAI_STR("placeholder.kai"); // RIP
		*info->error_info = ctx.error_info;
		return ctx.error_info.result;
	}

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
