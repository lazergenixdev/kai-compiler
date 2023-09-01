#include "dependency_graph.hpp"
#include "builtin_types.hpp"

// @TODO: don't repeat dependencies in dependency list pls thx

#define str_insert_string(Dest, String) \
memcpy((Dest).data + (Dest).count, String, sizeof(String)-1), (Dest).count += sizeof(String)-1

#define str_insert_str(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data, (Src).count), (Dest).count += (Src).count

#define str_insert_std(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data(), (Src).size()), (Dest).count += (Src).size()

bool error_not_declared(kai_str const& string, kai_int line_number) {
    auto& err = context.error;
    err.result = kai_Result_Error_Semantic;
    err.location.string = string;
    err.location.line = line_number;
    err.message = { 0, (kai_u8*)context.memory.temperary }; // uses temperary memory

    str_insert_string(err.message, "indentifier \"");
    str_insert_str   (err.message, string);
    str_insert_string(err.message, "\" not declared");

    return true;
}

bool error_redefinition(kai_str const& string, kai_u32 line_number, Dependency_Node_Info const& original) {
    auto& err = context.error;
    err.result = kai_Result_Error_Semantic;
    err.location.string = string;
    err.location.line = line_number;
    err.message = { 0, (kai_u8*)context.memory.temperary }; // uses temperary memory

    str_insert_string(err.message, "indentifier \"");
    str_insert_str   (err.message, string);
    str_insert_string(err.message, "\" is already declared");

    auto end = (u8*)context.memory.temperary + err.message.count;
    auto info = reinterpret_cast<kai_Error*>(end);
    end += sizeof(kai_Error);
    err.next = info;

    info->result = kai_Result_Error_Info;
    info->location.string = { .count = (kai_int)original.name.size(), .data = (kai_u8*)original.name.data() };
    info->location.line = original.line_number;
    info->message = { 0, end }; // uses temperary memory

    str_insert_string(info->message, "see original definition");

    return true;
}

void* error_circular_dependency(u32 dependency_type, Dependency_Node_Info const& info) {
    auto& err = context.error;
    err.result = kai_Result_Error_Semantic;
    err.location.string.data = (u8*)info.name.data();
    err.location.string.count = (u64)info.name.size();
    err.location.line = info.line_number;
    err.message = { 0, (kai_u8*)context.memory.temperary }; // uses temperary memory

    switch (dependency_type)
    {
    default:
    break; case Dependency::VALUE:
        str_insert_string(err.message, "value");
    break; case Dependency::TYPE:
        str_insert_string(err.message, "type");
    break;
    }

    str_insert_string(err.message, " of \"");
    str_insert_std   (err.message, info.name);
    str_insert_string(err.message, "\" cannot depend on itself");

    return (u8*)err.message.data + err.message.count;
}

void* error_dependency_info(void* start, u32 dependency_type, Dependency_Node_Info const& info) {
    auto& err = *reinterpret_cast<kai_Error*>(start);

    err.result = kai_Result_Error_Info;
    err.location.string.data  = (u8*)info.name.data();
    err.location.string.count = (u64)info.name.size();
    err.location.line = info.line_number;
    err.message = { 0, (kai_u8*)start + sizeof(kai_Error) }; // uses temperary memory
    err.context = {};
    err.next = nullptr;

    str_insert_string(err.message, "see ");

    switch (dependency_type)
    {
    default:
    break; case Dependency::VALUE:
        str_insert_string(err.message, "value");
    break; case Dependency::TYPE:
        str_insert_string(err.message, "type");
    break;
    }
    
    str_insert_string(err.message, " of \"");
    str_insert_std   (err.message, info.name);
    str_insert_string(err.message, "\"");

    return (u8*)err.message.data + err.message.count;
}

bool Dependency_Graph::create(kai_AST* ast) {
    scopes.reserve(128);
    value_nodes.reserve(256);
    type_nodes.reserve(256);
    node_infos.reserve(256);

    scopes.emplace_back();
    insert_builtin_types();

	// Make nodes for every statement
	for range(ast->toplevel_count) {
		auto tl = ast->toplevel_stmts[i];
		if (insert_node_for_statement(tl, false))
            return true;
	}
    
	// Insert dependencies for each node
	for range(node_infos.size()) {
		auto& vnode = value_nodes[i];
		auto& tnode = type_nodes[i];
		auto& info  = node_infos[i];

		if (
            !(vnode.flags&Evaluated) &&
            insert_value_dependencies(vnode.dependencies, info.scope, info.expr, false)
        ) return true;

		if (
            !(tnode.flags&Evaluated) &&
            insert_type_dependencies(tnode.dependencies, info.scope, info.expr)
        ) return true;
	}


#if ENABLE_DEBUG_PRINT
    debug_print();
#endif

    if (has_circular_dependency())
        return true; 

	return false;
}

void Dependency_Graph::insert_builtin_types() {
    auto& global_scope = scopes[GLOBAL_SCOPE];

    for (auto&&[name, type]: __builtin_types) {
        auto index = (u32)value_nodes.size();

        Value_Dependency_Node vnode;
        vnode.flags = Dependency_Flags::Evaluated;
        vnode.value.ptrvalue = type;
        value_nodes.emplace_back(vnode);

        Type_Dependency_Node tnode;
        tnode.flags = Dependency_Flags::Evaluated;
        tnode.type = &_type_type_info;
        type_nodes.emplace_back(tnode);
        
        Dependency_Node_Info info;
        info.name = view(name);
        info.scope = GLOBAL_SCOPE;
        info.expr = nullptr;
        node_infos.emplace_back(info);

        global_scope.map.emplace(info.name, index);
    }    
}


bool Dependency_Graph::insert_node_for_statement(
    kai_Stmt statement,
    bool in_proc,
    u32 scope_index
) {
    switch (statement->id)
	{
	default:
    break; case kai_Stmt_ID_Declaration: {
		auto decl = (kai_Stmt_Declaration*)statement;
		if (insert_node_for_declaration(decl, in_proc, scope_index))
			return true;
	}
	break; case kai_Stmt_ID_Compound: {
		auto comp = (kai_Stmt_Compound*)statement;
		auto new_scope_index = (kai_u32)scopes.size();
		Scope new_scope;
		new_scope.parent = scope_index;
		scopes.emplace_back(new_scope);
		comp->_scope = new_scope_index;
		for range(comp->count) {
			if (insert_node_for_statement(comp->statements[i], in_proc, new_scope_index))
				return true;
		}
	}
	break;
	}
	return false;
}

bool Dependency_Graph::insert_node_for_declaration(
    kai_Stmt_Declaration* decl,
    bool in_proc,
    u32 scope_index
) {
    if (in_proc && !(decl->flags & kai_Decl_Flag_Const))
		return false;

	auto name = view(decl->name);

	Scope* s = &scopes[scope_index];

	// Does this declaration already exist for this Scope?
	auto it = s->map.find(name);
	if (it != s->map.end()) {
		return error_redefinition(decl->name, decl->line_number, node_infos[it->second]);
	}

	// Add node to dependency graph
	{
		auto index = (kai_u32)value_nodes.size();

		Value_Dependency_Node vnode = {};
		{ // @Note: is this dependency always correct?
			Dependency dep;
			dep.node_index = index;
			dep.type = Dependency::TYPE;
			vnode.dependencies.emplace_back(dep);
		}

		Type_Dependency_Node tnode = {};

		Dependency_Node_Info info;
		info.name = name;
		info.expr = decl->expr;
        info.line_number = decl->line_number;
		info.scope = scope_index;

		s->map.emplace(info.name, index);

		value_nodes.emplace_back(vnode);
		type_nodes.emplace_back(tnode);
		node_infos.emplace_back(info);
	}

	switch (decl->expr->id)
	{
	default:
    break; case kai_Expr_ID_Procedure: {
		auto proc = (kai_Expr_Procedure*)decl->expr;

		auto new_scope_index = (kai_u32)scopes.size();
		Scope new_scope;
		new_scope.parent = scope_index;
		new_scope.is_proc_scope = true;
		scopes.emplace_back(new_scope);

		proc->_scope = new_scope_index;
		return insert_node_for_statement(proc->body, true, new_scope_index);
	}
	}

	return false;
}

void remove_all_locals(Scope& s) {
    for (auto it = s.map.begin(); it != s.map.end();) {
        if (it->second == LOCAL_VARIABLE_INDEX)
            it = s.map.erase(it);
        else ++it;
    }
}

bool Dependency_Graph::insert_value_dependencies(
    Dependency_List& deps,
    u32 scope_index,
    kai_Expr expr,
    bool in_procedure
) {
    if (expr == nullptr) { panic_with_message("null expression\n"); return true; }

    switch (expr->id)
    {
    default: panic_with_message("undefined expression\n");

    break; case kai_Expr_ID_Identifier: {
        auto name = view(expr->source_code);
		auto result = in_procedure ?
            resolve_dependency_node_procedure(name, &scopes[scope_index]):
            resolve_dependency_node(name, scope_index);
        
		if (!result)
			return error_not_declared(expr->source_code, expr->line_number);

        // DO NOT depend on local variables
        if (in_procedure && *result == LOCAL_VARIABLE_INDEX)
            break;
		
        deps.emplace_back(Dependency::VALUE, *result);
    }

    break; case kai_Expr_ID_Number:
    break; case kai_Expr_ID_String:

    break; case kai_Expr_ID_Unary: {
        auto unary = (kai_Expr_Unary*)expr;
		return insert_value_dependencies(deps, scope_index, unary->expr, in_procedure);
    }

    break; case kai_Expr_ID_Binary: {
        auto binary = (kai_Expr_Binary*)expr;
		if (insert_value_dependencies(deps, scope_index, binary->left, in_procedure))
            return true;
        return insert_value_dependencies(deps, scope_index, binary->right, in_procedure);
    }

    break; case kai_Expr_ID_Procedure_Call: {
        auto call = (kai_Expr_Procedure_Call*)expr;

        if (in_procedure) {
            if (insert_type_dependencies(deps, scope_index, call->proc))
                return true;
        }
        else {
            if (insert_value_dependencies(deps, scope_index, call->proc, in_procedure))
                return true;
        }

        for (kai_u32 i = 0; i < call->arg_count; ++i) {
			if(insert_value_dependencies(deps, scope_index, call->arguments[i], in_procedure))
                return true;
        }
    }

    break; case kai_Expr_ID_Procedure_Type: {
        auto proc = (kai_Expr_Procedure_Type*)expr;

		panic_with_message("procedure type\n");
        //kai_int total = proc->parameter_count + proc->ret_count;
        //for (int i = 0; i < total; ++i) {
        //    print_tree(ctx, proc->types[i]);
        //}
    }

    break; case kai_Expr_ID_Procedure: {
        auto proc = (kai_Expr_Procedure*)expr;
		Scope& local_scope = scopes[proc->_scope];

		// Insert procedure input names to local scope
		auto const n = proc->param_count;
        for (int i = 0; i < n; ++i) {
			auto const& parameter = proc->input_output[i];
			auto name = view(parameter.name);

			auto it = local_scope.map.find(name);
			if (it != local_scope.map.end())
				return error_redefinition(parameter.name, proc->line_number, node_infos[it->second]);

			local_scope.map.emplace(name, LOCAL_VARIABLE_INDEX);
        }

		if (insert_value_dependencies(deps, proc->_scope, proc->body, true))
			return true;

		remove_all_locals(local_scope);
    }

    break; case kai_Stmt_ID_Declaration: {
        if (in_procedure) {
            auto decl = (kai_Stmt_Declaration*)expr;

            // Already has a dependency node
            if (decl->flags & kai_Decl_Flag_Const) return false;

            auto name = view(decl->name);
            auto& s = scopes[scope_index];

            // Insert into local scope (if not already defined)
            auto it = s.map.find(name);
            if (it != s.map.end() && it->second != LOCAL_VARIABLE_INDEX)
                return error_redefinition(decl->name, decl->line_number, node_infos[it->second]);
            s.map.emplace(name, LOCAL_VARIABLE_INDEX);

            // Look into it's definition to find possible dependencies
            return insert_value_dependencies(deps, scope_index, decl->expr, true);
        }
        else panic_with_message("invalid declaration\n");
    }

    break; case kai_Stmt_ID_Return: {
        if (in_procedure) {
            auto ret = (kai_Stmt_Return*)expr;
            return insert_value_dependencies(deps, scope_index, ret->expr, true);
        }
		else panic_with_message("invalid return\n");
    }

    break; case kai_Stmt_ID_Compound: {
        if (in_procedure) {
            auto comp = (kai_Stmt_Compound*)expr;

            auto const n = comp->count;
            for range(n) {
                if(insert_value_dependencies(deps, comp->_scope, comp->statements[i], true))
                    return true;
            }

            Scope& s = scopes[comp->_scope];
            remove_all_locals(s);
        }
        else panic_with_message("invalid compound statement\n");
    }

    break;
    }

	return false;
}

bool Dependency_Graph::insert_type_dependencies(
    Dependency_List& deps,
    u32 scope_index,
    kai_Expr expr
) {
    if (expr == nullptr) { panic_with_message("null expression\n"); return false; }

    switch (expr->id)
    {
    default:
    break; case kai_Expr_ID_Identifier: {
        auto name = view(expr->source_code);

        auto result = resolve_dependency_node_procedure(name, &scopes[scope_index]);

        if (!result)
            return error_not_declared(expr->source_code, expr->line_number);

        if (*result != LOCAL_VARIABLE_INDEX) // DO NOT depend on local variables
            deps.emplace_back(Dependency::TYPE, *result);
    }

    break; case kai_Expr_ID_Unary: {
        auto unary = (kai_Expr_Unary*)expr;
        return insert_type_dependencies(deps, scope_index, unary->expr);
    }

    break; case kai_Expr_ID_Binary: {
        auto binary = (kai_Expr_Binary*)expr;
        if (insert_type_dependencies(deps, scope_index, binary->left))
            return true;
        return insert_type_dependencies(deps, scope_index, binary->right);
    }

    break; case kai_Expr_ID_Procedure_Call: {
        auto call = (kai_Expr_Procedure_Call*)expr;

        if (insert_type_dependencies(deps, scope_index, call->proc))
            return true;

        for (kai_u32 i = 0; i < call->arg_count; ++i) {
            if (insert_value_dependencies(deps, scope_index, call->arguments[i], false))
                return true;
        }
    }

    break; case kai_Expr_ID_Procedure_Type: {
        auto proc = (kai_Expr_Procedure_Type*)expr;
        panic_with_message("procedure type\n");
        //kai_int total = proc->parameter_count + proc->ret_count;
        //for (int i = 0; i < total; ++i) {
        //    print_tree(ctx, proc->types[i]);
        //}
    }

    break; case kai_Expr_ID_Procedure: {
        auto proc = (kai_Expr_Procedure*)expr;
        auto const n = proc->param_count + proc->ret_count;
        for range(n) {
            auto param = proc->input_output[i];
            if (insert_value_dependencies(deps, scope_index, param.type, false))
                return true;
        }
    }

    break; case kai_Stmt_ID_Declaration: {
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
    }

    break; case kai_Stmt_ID_Return: {
        auto ret = (kai_Stmt_Return*)expr;
        return insert_type_dependencies(deps, scope_index, ret->expr);
    }

    break; case kai_Stmt_ID_Compound: {
        //auto comp = (kai_Stmt_Compound*)expr;

        //auto const n = comp->count;
        //for range(n) {
        //	catch_and_throw(insert_type_dependencies(deps, comp->_scope, comp->statements[i]));
        //}

        //Scope& s = dg.scopes[comp->_scope];
        //remove_all_locals(s);
    }

    break;
    }

    return false;
}

void Dependency_Graph::
insert_procedure_input(int arg_index, kai_Type type, std::string_view name, u32 scope_index) {
    Value_Dependency_Node vnode;
    vnode.flags = Dependency_Flags::Is_Local_Variable;
    vnode.value.u32value = PROC_ARG(arg_index);
    value_nodes.emplace_back(vnode);

    Type_Dependency_Node tnode;
    tnode.flags = Dependency_Flags::Is_Local_Variable;
    tnode.type = type;
    type_nodes.emplace_back(tnode);

    Dependency_Node_Info info;
    info.name = name;
    info.scope = scope_index;
    info.expr = nullptr;
    node_infos.emplace_back(info);

    auto new_node_index = u32(node_infos.size() - 1);

    scopes[scope_index].map.emplace(info.name, new_node_index);
}

std::optional<u32> Dependency_Graph::
resolve_dependency_node(std::string_view name, u32 scope_index) {
    Scope* s = nullptr;
    loop {
        s = &scopes[scope_index];

        auto it = s->map.find(name);
        if (it != s->map.end())
            return std::make_optional(it->second);

        if (scope_index == GLOBAL_SCOPE)
            return {};
        
        scope_index = s->parent;
    }
}

std::optional<u32> Dependency_Graph::
resolve_dependency_node_procedure(std::string_view name, Scope* scope) {
    bool allow_locals = true;
    loop {
        auto it = scope->map.find(name);

        if (it != scope->map.end()) {
            bool const is_local = it->second == LOCAL_VARIABLE_INDEX;
            if (is_local) {
                if (allow_locals) {
                    return std::make_optional(it->second);
                }
            }
            else {
                return std::make_optional(it->second);
            }
        }

        if (scope->parent == -1)
            return {};

        // Do not allow Local variables from higher procedure scopes
        if (scope->is_proc_scope)
            allow_locals = false;
        
        scope = &scopes[scope->parent];
    }
}

#define FIND(Container, Value) std::find(Container.begin(), Container.end(), Value)

// TODO: Flatten out this recursive algorithm to be iterative
bool Dependency_Graph::circular_dependency_check(std::vector<Dependency>& dependency_stack, Dependency_Node& node) {
    for (auto const& nd : node.dependencies) {

        // TODO: optimization: should keep a flag that indicates that
        //         we already checked this node's dependencies,
        //         so we never need to check more than once
        auto it = FIND(dependency_stack, nd);
        if (it != dependency_stack.end()) {
            auto self = it->node_index;
            auto const& info = node_infos[self];
            void* start = error_circular_dependency(it->type, info);
            kai_Error* last = &context.error;

            for (auto _it = ++it; _it != dependency_stack.end(); ++_it) {
                last->next = (kai_Error*)start;
                start = error_dependency_info(start, it->type, node_infos[_it->node_index]);
                last = last->next;
            }

            return true;
        }

        // TODO: If this node already KNOWS that it has no self dependencies,
        //        then we dont need to push it onto the stack
        dependency_stack.push_back(nd);

        switch (nd.type)
        {
        default:
        break; case Dependency::VALUE:
            if(circular_dependency_check(dependency_stack, value_nodes[nd.node_index]))
                return true;
        break; case Dependency::TYPE:
            if(circular_dependency_check(dependency_stack, type_nodes[nd.node_index]))
                return true;
        break;
        }
        dependency_stack.pop_back();
    }
    return false;
}

bool Dependency_Graph::has_circular_dependency() {
    std::vector<Dependency> stack;
    stack.emplace_back();
    
    for range(node_infos.size()) {
        stack[0].node_index = (u32)i;

        stack[0].type = Dependency::VALUE;
        if(circular_dependency_check(stack, value_nodes[i]))
            return true;
    
        stack[0].type = Dependency::TYPE;
        if(circular_dependency_check(stack, type_nodes[i]))
            return true;
    }
    return false;
}

#if 1
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
void Dependency_Graph::debug_print() {
    std::cout << '\n';
    for range(value_nodes.size()) {
        auto const& info = node_infos[i];
        auto const& node = value_nodes[i];
        auto const& name = info.name;

        std::cout << GET_COLOR(i);
        std::cout << name << ':' << info.scope << ' ';
        std::cout << RESET;

        if (node.flags & Dependency_Flags::Evaluated) {
            std::cout << "is already evaluated\n";
            continue;
        }

        std::cout << "depends on {";
        int k = 0;
        for (auto const& dep : node.dependencies) {
            auto const& info = node_infos[dep.node_index];
            if (dep.type == Dependency::TYPE)
                std::cout << "type_of(";
            std::cout << GET_COLOR(dep.node_index);
            std::cout << info.name << ':' << info.scope;
            std::cout << RESET;
            if (dep.type == Dependency::TYPE)
                std::cout << ")";
            if (++k < node.dependencies.size()) std::cout << ", ";
        }
        std::cout << "}\n";
    }
    for range(100) std::cout << '-';
    std::cout << '\n';
    for range(type_nodes.size()) {
        auto const& info = node_infos[i];
        auto const& node = type_nodes[i];
        auto const& name = info.name;

        std::cout << "type_of(";
        std::cout << GET_COLOR(i);
        std::cout << name << ':' << info.scope;
        std::cout << RESET;
        std::cout << ") ";

        if (node.flags & Dependency_Flags::Evaluated) {
            std::cout << "is already evaluated\n";
            continue;
        }

        std::cout << "depends on {";
        int k = 0;
        for (auto const& dep : node.dependencies) {
            auto const& info = node_infos[dep.node_index];
            if (dep.type == Dependency::TYPE)
                std::cout << "type_of(";
            std::cout << GET_COLOR(dep.node_index);
            std::cout << info.name << ':' << info.scope;
            std::cout << RESET;
            if (dep.type == Dependency::TYPE)
                std::cout << ")";
            if (++k < node.dependencies.size()) std::cout << ", ";
        }
        std::cout << "}\n";
    }
    std::cout << '\n';
}
#endif
