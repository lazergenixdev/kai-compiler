#include "dependency_graph.hpp"
#include <algorithm>
#include "builtin_types.hpp"
#include "compiler_context.hpp"

void print_node_reference(Node_Reference ref, std::vector<Info_Node>& info_nodes);

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

bool error_redefinition(kai_str const& string, kai_u32 line_number, Info_Node const& original) {
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

void* error_circular_dependency(u32 type, Info_Node const& info) {
    auto& err = context.error;
    err.result = kai_Result_Error_Semantic;
    err.location.string.data = (u8*)info.name.data();
    err.location.string.count = (u64)info.name.size();
    err.location.line = info.line_number;
    err.message = { 0, (kai_u8*)context.memory.temperary }; // uses temperary memory

    switch (type)
    {
    default:
    break; case Node_Reference::VALUE:
        str_insert_string(err.message, "value");
    break; case Node_Reference::TYPE:
        str_insert_string(err.message, "type");
    break;
    }

    str_insert_string(err.message, " of \"");
    str_insert_std   (err.message, info.name);
    str_insert_string(err.message, "\" cannot depend on itself");

    return (u8*)err.message.data + err.message.count;
}

void* error_dependency_info(void* start, u32 type, Info_Node const& info) {
    auto& err = *reinterpret_cast<kai_Error*>(start);

    err.result = kai_Result_Error_Info;
    err.location.string.data  = (u8*)info.name.data();
    err.location.string.count = (u64)info.name.size();
    err.location.line = info.line_number;
    err.message = { 0, (kai_u8*)start + sizeof(kai_Error) }; // uses temperary memory
    err.context = {};
    err.next = nullptr;

    str_insert_string(err.message, "see ");

    switch (type)
    {
    default:
    break; case Node_Reference::VALUE:
        str_insert_string(err.message, "value");
    break; case Node_Reference::TYPE:
        str_insert_string(err.message, "type");
    break;
    }
    
    str_insert_string(err.message, " of \"");
    str_insert_std   (err.message, info.name);
    str_insert_string(err.message, "\"");

    return (u8*)err.message.data + err.message.count;
}

struct DFS_Explore_Context {
    std::vector<Node_Reference> nodes;
    std::vector<int>            post;
    std::vector<u32>            prev;
    std::vector<bool>           visited;
    std::vector<Value_Node> & value_nodes;
    std::vector<Type_Node>  & type_nodes;
    unsigned int n = 0;

    DFS_Explore_Context(Dependency_Graph& dg):
        post(2*dg.info_nodes.size(), 0),
        prev(2*dg.info_nodes.size(), 0),
        visited(2*dg.info_nodes.size(), false),
        value_nodes(dg.value_nodes),
        type_nodes(dg.type_nodes)
    {
        nodes.reserve(2*dg.info_nodes.size());
        for_n (dg.info_nodes.size()) {
            nodes.emplace_back(Node_Reference::VALUE, i);
            nodes.emplace_back(Node_Reference::TYPE,  i);
        }
    }

    void clear() {
        std::fill(visited.begin(), visited.end(), false);
    }
    
    void dfs() {
        for_n (nodes.size()) {
            if (!visited[i])
                explore(i);
        }
    }

    void explore(u32 s) {
        visited[s] = true;
        ++n;
        auto ref = nodes[s];

        if (ref.type == Node_Reference::VALUE) {
            auto deps = value_nodes[ref.node_index].dependencies;

            for (auto&& dep: deps) {
                u32 v = (dep.node_index*2)+dep.type;
                if (!visited[v]) {
                    prev[v] = s;
                    explore(v);
                }
            }
        }
        if (ref.type == Node_Reference::TYPE) {
            auto deps = type_nodes[ref.node_index].dependencies;

            for (auto&& dep: deps) {
                u32 v = (dep.node_index*2)+dep.type;
                if (!visited[v]) {
                    prev[v] = s;
                    explore(v);
                }
            }
        }
        post[s] = ++n;
    }
};


bool Dependency_Graph::create(kai_AST* ast) {
    scopes.reserve(128);
    value_nodes.reserve(256);
    type_nodes.reserve(256);
    info_nodes.reserve(256);

    scopes.emplace_back();
    insert_builtin_types();

	// Make nodes for every statement
	for_n(ast->toplevel_count) {
		auto tl = ast->toplevel_stmts[i];
		if (insert_node_for_statement(tl, false))
            return true;
	}
    
	// Insert dependencies for each node
	for_n(info_nodes.size()) {
		auto& vnode = value_nodes[i];
		auto& tnode =  type_nodes[i];
		auto& info  =  info_nodes[i];

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

    DFS_Explore_Context explorer(*this);
    explorer.dfs();


 //   std::vector<Node_Reference> nodes;
//
 //   for_n (info_nodes.size()) {
 //       nodes.emplace_back(Node_Reference::VALUE, i);
 //       nodes.emplace_back(Node_Reference::TYPE, i);
 //   }
//
 //   std::vector<int> post(nodes.size(), 0);
 //   std::vector<bool> visited(nodes.size(), false);
 //   int k = 0;
//
//	for_n (nodes.size()) {
//		if (!visited[i])
//			explore(k, nodes, post, visited, value_nodes, type_nodes, i);
 //   }

    std::vector<unsigned int> order;
    for_n (explorer.post.size()) {
        order.emplace_back(i);
    }

    std::sort(order.begin(), order.end(), 
        [&](unsigned int l, unsigned int r) {
            return explorer.post[l] < explorer.post[r];
        }
    );

    std::cout << "COMPILATION ORDER:\n";
    for (auto&& i: order) {
        print_node_reference(explorer.nodes[i], info_nodes);
    //    std::cout << " -> " << post;
        std::cout << "\n";
    }

    // DFS on the reverse graph, counting how many nodes are in each SCC,
    // If this number is every >1, then we found a circular dependency

    // OR: we could look for back edges in the graph G, then those back
    //     edges are cycles
    for (auto&& [type,node_index]: explorer.nodes)
    {
        auto u = (node_index*2)+type;
        Dependency_List* deps = nullptr;
        if (type == Node_Reference::VALUE)
            deps = &(value_nodes[node_index].dependencies);
        if (type == Node_Reference::TYPE)
            deps = &(type_nodes[node_index].dependencies);

        for (auto&& dep: *deps) {
            int v = (dep.node_index*2)+dep.type;
            if (explorer.post[u] < explorer.post[v]) {
            //    error_circular_dependency(type, info_nodes[node_index]);
            //    return true;
                
                explorer.clear();
                explorer.explore(u);

                // loop through prev pointers to get to u, and add errors in reverse order

                void* start = error_circular_dependency(type, info_nodes[node_index]);
                kai_Error* last = &context.error;
                for (u32 i = explorer.prev[u];; i = explorer.prev[i]) {
                    last->next = (kai_Error*)start;
                    auto const& n = info_nodes[i >> 1];
                    start = error_dependency_info(start, i & 1, n);
                    last = last->next;

                    if (i == 0 || i == u) break;
                }
                return true;
            }
        }
    }
	return false;
}

void Dependency_Graph::insert_builtin_types() {
    auto& global_scope = scopes[GLOBAL_SCOPE];

    for (auto&&[name, type]: __builtin_types) {
        auto index = (u32)value_nodes.size();

        Value_Node vnode;
        vnode.flags = Node_Flags::Evaluated;
        vnode.value.ptrvalue = type;
        value_nodes.emplace_back(vnode);

        Type_Node tnode;
        tnode.flags = Node_Flags::Evaluated;
        tnode.type = &_type_type_info;
        type_nodes.emplace_back(tnode);
        
        Info_Node info;
        info.name = SV(name);
        info.scope = GLOBAL_SCOPE;
        info.expr = nullptr;
        info_nodes.emplace_back(info);

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
    break; case kai_Stmt_ID_If: {
        auto ifs = (kai_Stmt_If*)statement;
		if (insert_node_for_statement(ifs->body, in_proc, scope_index))
			return true;
		if (ifs->else_body && insert_node_for_statement(ifs->else_body, in_proc, scope_index))
			return true;
    }
    break; case kai_Stmt_ID_For: {
        auto fors = (kai_Stmt_For*)statement;
		if (insert_node_for_statement(fors->body, in_proc, scope_index))
			return true;
        // TODO: do we need to check the expression of the for statement? 
    }
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
		for_n(comp->count) {
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

	auto name = SV(decl->name);

	Scope* s = &scopes[scope_index];

	// Does this declaration already exist for this Scope?
	auto it = s->map.find(name);
	if (it != s->map.end()) {
		return error_redefinition(decl->name, decl->line_number, info_nodes[it->second]);
	}

	// Add node to dependency graph
	{
		auto index = (kai_u32)value_nodes.size();

		Value_Node vnode = {};
		{ // @Note: is this dependency always correct?
			Node_Reference dep;
			dep.node_index = index;
			dep.type = Node_Reference::TYPE;
			vnode.dependencies.emplace_back(dep);
		}

		Type_Node tnode = {};

		Info_Node info;
		info.name = name;
		info.expr = decl->expr;
        info.line_number = decl->line_number;
		info.scope = scope_index;

		s->map.emplace(info.name, index);

		value_nodes.emplace_back(vnode);
		type_nodes.emplace_back(tnode);
		info_nodes.emplace_back(info);
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
    default: panic_with_message("undefined expr [insert_value_dependencies] (id = %d)\n", expr->id);

    break; case kai_Expr_ID_Identifier: {
        auto name = SV(expr->source_code);
		auto result = in_procedure ?
            resolve_dependency_node_procedure(name, &scopes[scope_index]):
            resolve_dependency_node(name, scope_index);
        
		if (!result)
			return error_not_declared(expr->source_code, expr->line_number);

        // DO NOT depend on local variables
        if (in_procedure && *result == LOCAL_VARIABLE_INDEX)
            break;
		
        Node_Reference ref = {Node_Reference::VALUE, *result};
        auto it = std::find(deps.begin(), deps.end(), ref);
        if (it == deps.end()) deps.emplace_back(ref);
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
			auto name = SV(parameter.name);

			auto it = local_scope.map.find(name);
			if (it != local_scope.map.end())
				return error_redefinition(parameter.name, proc->line_number, info_nodes[it->second]);

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

            auto name = SV(decl->name);
            auto& s = scopes[scope_index];

            // Insert into local scope (if not already defined)
            auto it = s.map.find(name);
            if (it != s.map.end() && it->second != LOCAL_VARIABLE_INDEX)
                return error_redefinition(decl->name, decl->line_number, info_nodes[it->second]);
            s.map.emplace(name, LOCAL_VARIABLE_INDEX);

            // Look into it's definition to find possible dependencies
            return insert_value_dependencies(deps, scope_index, decl->expr, true);
        }
        else panic_with_message("invalid declaration\n");
    }

    break; case kai_Stmt_ID_Assignment: {
        if (in_procedure) {
            auto assignment = (kai_Stmt_Assignment*)expr;
            return insert_value_dependencies(deps, scope_index, assignment->expr, true);
        }
        else panic_with_message("invalid assignment\n");
    }

    break; case kai_Stmt_ID_Return: {
        if (in_procedure) {
            auto ret = (kai_Stmt_Return*)expr;
            return insert_value_dependencies(deps, scope_index, ret->expr, true);
        }
		else panic_with_message("invalid return\n");
    }

    break; case kai_Stmt_ID_If: {
		auto ifs = (kai_Stmt_If*)expr;
		if(insert_value_dependencies(deps, scope_index, ifs->body, in_procedure))
			return true;
		if(ifs->else_body && insert_value_dependencies(deps, scope_index, ifs->else_body, in_procedure))
			return true;
    }

    break; case kai_Stmt_ID_Compound: {
        if (in_procedure) {
            auto comp = (kai_Stmt_Compound*)expr;

            auto const n = comp->count;
            for_n(n) {
                if(insert_value_dependencies(deps, comp->_scope, comp->statements[i], true))
                    return true;
            }

            Scope& s = scopes[comp->_scope];
            remove_all_locals(s);
        }
        else panic_with_message("invalid compound statement\n");
    }

    break; case kai_Stmt_ID_For: {
        auto for_ = (kai_Stmt_For*)expr;
        if (insert_value_dependencies(deps, scope_index, for_->body, in_procedure))
            return true;
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
        auto name = SV(expr->source_code);

        auto result = resolve_dependency_node_procedure(name, &scopes[scope_index]);

        if (!result) {
            std::cout << "scope [" << scope_index << "] contains:\n";
            for (auto&& [name,_]: scopes[scope_index].map) {
                std::cout << name << '\n';
            }
            return error_not_declared(expr->source_code, expr->line_number);
        }

        // DO NOT depend on local variables
        if (*result == LOCAL_VARIABLE_INDEX)
            return false;

        Node_Reference ref = {Node_Reference::TYPE, *result};
        auto it = std::find(deps.begin(), deps.end(), ref);
        if (it == deps.end()) deps.emplace_back(ref);
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
        for_n(n) {
            auto param = proc->input_output[i];
            if (insert_value_dependencies(deps, scope_index, param.type, false))
                return true;
        }
    }

    break; case kai_Stmt_ID_Declaration: {
        auto decl = (kai_Stmt_Declaration*)expr;

        //// Already has a dependency node
        //if (decl->flags & kai_Decl_Flag_Const) return false;

        //auto name = SV(decl->name);
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
        //for_n(n) {
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
    Value_Node vnode;
    vnode.flags = Node_Flags::Is_Local_Variable;
    vnode.value.u32value = PROC_ARG(arg_index);
    value_nodes.emplace_back(vnode);

    Type_Node tnode;
    tnode.flags = Node_Flags::Is_Local_Variable;
    tnode.type = type;
    type_nodes.emplace_back(tnode);

    Info_Node info;
    info.name = name;
    info.scope = scope_index;
    info.expr = nullptr;
    info_nodes.emplace_back(info);

    auto new_node_index = u32(info_nodes.size() - 1);

    scopes[scope_index].map.emplace(info.name, new_node_index);
}

// void Dependency_Graph::
// insert_local_variable(u32 reg_index, kai_Type type, std::string_view name, u32 scope_index) {
//     Value_Node vnode;
//     vnode.flags = Node_Flags::Is_Local_Variable;
//     vnode.value.u32value = reg_index;
//     value_nodes.emplace_back(vnode);

//     Type_Node tnode;
//     tnode.flags = Node_Flags::Is_Local_Variable;
//     tnode.type = type;
//     type_nodes.emplace_back(tnode);

//     Info_Node info;
//     info.name = name;
//     info.scope = scope_index;
//     info.expr = nullptr;
//     info_nodes.emplace_back(info);

//     auto new_node_index = u32(info_nodes.size() - 1);

//     scopes[scope_index].map.emplace(info.name, new_node_index);
// }


std::optional<u32> Dependency_Graph::
resolve_dependency_node(std::string_view name, u32 scope_index) {
    Scope* s = nullptr;
    while (1) {
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
    while (1) {
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

#if 1
#define RESET "\x1b[0m"
static char const* _color_table[] = {
    "\x1b[31m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[34m",
    "\x1b[35m",
    "\x1b[36m",
    "\x1b[91m",
    "\x1b[92m",
    "\x1b[93m",
    "\x1b[94m",
    "\x1b[95m",
    "\x1b[96m",
};
#define GET_COLOR(N) _color_table[N % std::size(_color_table)]

void print_node_reference(Node_Reference ref, std::vector<Info_Node>& info_nodes) {
    auto const& info = info_nodes[ref.node_index];
    if (ref.type == Node_Reference::TYPE)
        std::cout << "type_of(";
    std::cout << GET_COLOR(ref.node_index);
    std::cout << info.name << ':' << info.scope;
    std::cout << RESET;
    if (ref.type == Node_Reference::TYPE)
        std::cout << ")";
}

void Dependency_Graph::debug_print() {
    std::cout << '\n';
    for_n(value_nodes.size()) {
        auto const& info = info_nodes[i];
        auto const& node = value_nodes[i];
        auto const& name = info.name;

        std::cout << GET_COLOR(i);
        std::cout << name << ':' << info.scope << ' ';
        std::cout << RESET;

        if (node.flags & Node_Flags::Evaluated) {
            std::cout << "is already evaluated\n";
            continue;
        }

        std::cout << "depends on {";
        int k = 0;
        for (auto const& dep : node.dependencies) {
            print_node_reference(dep, info_nodes);
            if (++k < node.dependencies.size()) std::cout << ", ";
        }
        std::cout << "}\n";
    }
//  for_n(100) std::cout << '-';
    std::cout << '\n';
    for_n(type_nodes.size()) {
        auto const& info = info_nodes[i];
        auto const& node = type_nodes[i];
        auto const& name = info.name;

        std::cout << "type_of(";
        std::cout << GET_COLOR(i);
        std::cout << name << ':' << info.scope;
        std::cout << RESET;
        std::cout << ") ";

        if (node.flags & Node_Flags::Evaluated) {
            std::cout << "is already evaluated\n";
            continue;
        }

        std::cout << "depends on {";
        int k = 0;
        for (auto const& dep : node.dependencies) {
            print_node_reference(dep, info_nodes);
            if (++k < node.dependencies.size()) std::cout << ", ";
        }
        std::cout << "}\n";
    }
    std::cout << '\n';
}
#endif
