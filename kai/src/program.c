#include <kai/kai.h>
#include "compiler.h"
#include "builtin_types.h"
#include <stdlib.h>

#include <kai/debug.h>

#define iterate(CONST_ARRAY) \
	for (int i = 0; i < sizeof(CONST_ARRAY)/sizeof(CONST_ARRAY[0]); ++i)

void insert_builtin_types(Dependency_Graph* graph);
void reserve(void** base, u64* capacity, u64 new_capacity, u32 stride);
#define array_reserve(A, NEW_CAPACITY) reserve(&A.data, &A.capacity, NEW_CAPACITY, sizeof(A.data[0]))
#define array_push_back(A, VALUE) A.data[A.count++] = VALUE

#define c_insert_node_for_statement(STATEMENT,IN_PROC,SCOPE_INDEX) \
insert_node_for_statement(context, STATEMENT, IN_PROC, SCOPE_INDEX)
Kai_bool insert_node_for_statement(
	Compiler_Context*  context,
	Kai_Stmt           statement,
	Kai_bool           in_procedure,
	u32                scope_index
);
#define c_insert_node_for_declaration(DECL,IN_PROC,SCOPE_INDEX) \
insert_node_for_declaration(context, DECL, IN_PROC, SCOPE_INDEX)
Kai_bool insert_node_for_declaration(
	Compiler_Context* context,
	Kai_Stmt_Declaration* decl,
	Kai_bool in_proc,
	u32 scope_index
);

#define c_insert_value_dependencies(DEPS,SCOPE_INDEX,EXPR,IN_PROC) \
insert_value_dependencies(context, DEPS, SCOPE_INDEX, EXPR, IN_PROC)
Kai_bool insert_value_dependencies(
    Compiler_Context* context,
	Kai_range*        deps,
    u32               scope_index,
    Kai_Expr          expr,
    Kai_bool          in_procedure
);

#define c_error_redefinition(NAME, LINE, NODE) \
	error_redefinition(context, NAME, LINE, NODE)
Kai_bool error_redefinition(
	Compiler_Context* context,
	Kai_str string,
	Kai_u32 line_number,
	Node* original
) {
#define err context->error
    err.result = KAI_ERROR_SEMANTIC;
    err.location.string = string;
    err.location.line = line_number;
    err.message.count = 0;
	err.message.data = (u8*)context->memory.temperary; // uses temperary memory

    str_insert_string(err.message, "indentifier \"");
    str_insert_str   (err.message, string);
    str_insert_string(err.message, "\" is already declared");

    u8* end = (u8*)context->memory.temperary + err.message.count;
    Kai_Error* info = (Kai_Error*)end;
    end += sizeof(Kai_Error);
    err.next = info;

    info->result = KAI_ERROR_INFO;
    info->location.string = original->name;
    info->location.line = original->line_number;
    info->message = (Kai_str){.count = 0,.data = end}; // uses temperary memory

    str_insert_string(info->message, "see original definition");

    return KAI_TRUE;
#undef err
}

#define c_error_not_declared(NAME, LINE) \
	error_not_declared(context, NAME, LINE)
Kai_bool error_not_declared(Compiler_Context* context, Kai_str string, Kai_int line_number) {
#define err context->error
    err.result = KAI_ERROR_SEMANTIC;
    err.location.string = string;
    err.location.line = line_number;
    err.message.count = 0;
	err.message.data = (u8*)context->memory.temperary; // uses temperary memory

    str_insert_string(err.message, "indentifier \"");
    str_insert_str   (err.message, string);
    str_insert_string(err.message, "\" not declared");

    return KAI_TRUE;
#undef err
}

Kai_Result
kai_create_program(Kai_Program_Create_Info* info, Kai_Program* program) {
	Compiler_Context context = {
		.memory = info->memory,
	};

	array_reserve(context.dependency_graph.scopes,         64);
	array_reserve(context.dependency_graph.nodes,         256);
	array_reserve(context.dependency_graph.dependencies, 1024);

	context.dependency_graph.scopes.count += 1;
    Scope* global_scope = context.dependency_graph.scopes.data + GLOBAL_SCOPE;
    *global_scope = (Scope){.parent = NONE};
    array_reserve(global_scope->identifiers, 64);

	insert_builtin_types(&context.dependency_graph);
	
	Kai_AST* ast = info->trees;

	// Make nodes for every statement
	for_n (ast->top_level_count) {
		Kai_Stmt tl = ast->top_level_statements[i];
		if (insert_node_for_statement(&context, tl, KAI_FALSE, GLOBAL_SCOPE))
            goto error;
	}

	// Insert dependencies for each node
	for_n (context.dependency_graph.nodes.count) {
		Node* node = context.dependency_graph.nodes.data + i;
		if (
            !(node->value_flags&NODE_EVALUATED) &&
            insert_value_dependencies(&context, &node->value_depenencies, node->scope, node->expr, KAI_FALSE)
        ) return KAI_TRUE;
	//	if (
    //        !(tnode.flags&Evaluated) &&
    //        insert_type_dependencies(tnode.dependencies, info.scope, info.expr)
    //    ) return KAI_TRUE;
	}

	char temp[64];
	Kai_Debug_String_Writer* writer = kai_debug_clib_writer();
	for_n (context.dependency_graph.nodes.count) {
		_write_format("%2u ", (int)i);
		_write_string(context.dependency_graph.nodes.data[i].name);
		_write(": ");
		Dependency_Graph* g = &context.dependency_graph;
		Node* node = context.dependency_graph.nodes.data + i;
		for_n (node->value_depenencies.count) {
			_write_format(" %u", g->dependencies.data[node->value_depenencies.offset + i]&(~TYPE_BIT));
		}
		_write_char('\n');
	}

	if (0) {
	error:
		*info->error = context.error;
		return KAI_ERROR_INTERNAL;
	}

	info->error->message = KAI_STR("compiler is in development :)");
	info->error->result = KAI_ERROR_FATAL;
    return KAI_ERROR_FATAL;
}

void insert_builtin_types(Dependency_Graph* graph) {
    Scope* global_scope = graph->scopes.data + GLOBAL_SCOPE;

    iterate (builtin_types) {
		Node node = {
			.type        = &_type_type_info,
			.value       = {._Type = builtin_types[i].info},
			.value_flags = NODE_EVALUATED,
			.type_flags  = NODE_EVALUATED,
			.name        = builtin_types[i].name,
			.scope       = GLOBAL_SCOPE,
		};
		Named_Node_Ref named_ref = {
			.name  = builtin_types[i].name,
			.index = graph->nodes.count,
		};
		array_push_back(global_scope->identifiers, named_ref);
		array_push_back(graph->nodes, node);
    }  
}

#define scopes       context->dependency_graph.scopes
#define dependencies context->dependency_graph.dependencies
#define nodes        context->dependency_graph.nodes
Kai_bool insert_node_for_statement(
	Compiler_Context*  context,
	Kai_Stmt           statement,
	Kai_bool           in_procedure,
	u32                scope_index
) {
    switch (statement->id)
	{
	default:
    break; case KAI_STMT_IF: {
        Kai_Stmt_If* ifs = (Kai_Stmt_If*)statement;
		if (c_insert_node_for_statement(ifs->body, in_procedure, scope_index))
			return KAI_TRUE;
		if (ifs->else_body &&
			c_insert_node_for_statement(ifs->else_body, in_procedure, scope_index))
			return KAI_TRUE;
    }
    break; case KAI_STMT_FOR: {
        Kai_Stmt_For* fors = (Kai_Stmt_For*)statement;
		if (c_insert_node_for_statement(fors->body, in_procedure, scope_index))
			return KAI_TRUE;
        // TODO: do we need to check the expression of the for statement? 
    }
    break; case KAI_STMT_DECLARATION: {
		Kai_Stmt_Declaration* decl = (Kai_Stmt_Declaration*)statement;
		if (c_insert_node_for_declaration(decl, in_procedure, scope_index))
			return KAI_TRUE;
	}
	break; case KAI_STMT_COMPOUND: {
		Kai_Stmt_Compound* comp = (Kai_Stmt_Compound*)statement;

		u32 new_scope_index = scopes.count;
		Scope new_scope = {
			.parent = scope_index,
			.is_proc_scope = KAI_TRUE,
		};
		array_reserve(scopes, scopes.count + 1);
		array_push_back(scopes, new_scope);

		comp->_scope = new_scope_index;
		for_n(comp->count) {
			if (c_insert_node_for_statement(comp->statements[i], in_procedure, new_scope_index))
				return KAI_TRUE;
		}
	}
	break;
	}
	return KAI_FALSE;
}

u32 scope_find(Scope* scope, Kai_str name) {
	for_n (scope->identifiers.count) {
		if (kai_string_equals(scope->identifiers.data[i].name, name))
			return i;
	}
	return NONE;
}

Kai_bool insert_node_for_declaration(
	Compiler_Context* context,
	Kai_Stmt_Declaration* decl,
	Kai_bool in_proc,
	u32 scope_index
) {
    if (in_proc && !(decl->flags & KAI_DECL_FLAG_CONST))
		return KAI_FALSE;

	Scope* scope = scopes.data + scope_index;

	// Does this declaration already exist for this Scope?
	u32 it = scope_find(scope, decl->name);
	if (it != NONE) {
		return c_error_redefinition(decl->name, decl->line_number, nodes.data + it);
	}

	// Add node to dependency graph
	{
		u32 index = nodes.count;

		u32 first_dependency = dependencies.count;
		array_reserve(dependencies, dependencies.count + 1);
		array_push_back(dependencies, index | TYPE_BIT);

		Node node = {
			.value_depenencies = {.offset = first_dependency, .count = 1},
			.name = decl->name,
			.expr = decl->expr,
			.line_number = decl->line_number,
			.scope = scope_index,			
		};

		Named_Node_Ref node_ref = {
			.name  = decl->name,
			.index = index,
		};

		array_reserve(scope->identifiers, scope->identifiers.count + 1);
		array_push_back(scope->identifiers, node_ref);

		array_reserve(nodes, nodes.count + 1);
		array_push_back(nodes, node);
	}

	switch (decl->expr->id)
	{
	default:
   break; case KAI_EXPR_PROCEDURE: {
		Kai_Expr_Procedure* proc = (Kai_Expr_Procedure*)decl->expr;

		u32 new_scope_index = scopes.count;
		Scope new_scope = {
			.parent = scope_index,
			.is_proc_scope = KAI_TRUE,
		};
		array_reserve(scopes, scopes.count + 1);
		array_push_back(scopes, new_scope);

		proc->_scope = new_scope_index;
		return c_insert_node_for_statement(proc->body, KAI_TRUE, new_scope_index);
	}
	}

	return KAI_FALSE;
}

#define LOCAL_VARIABLE_INDEX ((u32)-1)

void remove_all_locals(Scope* scope) {
	Named_Node_Ref* data = scope->identifiers.data;
	u32 count = scope->identifiers.count;
	u32 end = 0;
	for_n (count) {
		if (data[i].index == LOCAL_VARIABLE_INDEX) continue;
		if (i != end) data[end] = data[i];
		++end;
	}
	scope->identifiers.count = end;
}

#define c_resolve_dependency_node(NAME, OUT_INDEX, SCOPE_INDEX, IN_PROC) \
	resolve_dependency_node(context, NAME, OUT_INDEX, SCOPE_INDEX, IN_PROC)
Kai_bool resolve_dependency_node(
	Compiler_Context* context,
	Kai_str name, u32* out_index, u32 scope_index, Kai_bool const in_proc
) {
    Kai_bool allow_locals = KAI_TRUE;
    Scope* scope = NULL;
    while (1) {
        scope = scopes.data + scope_index;

		u32 it = scope_find(scope, name);

        if (it != NONE) {
            Kai_bool is_local = scope->identifiers.data[it].index == LOCAL_VARIABLE_INDEX;
            if (in_proc && is_local) {
                if (allow_locals) {
					*out_index = it;
					return KAI_TRUE;
                }
            }
            else {
				*out_index = it;
				return KAI_TRUE;
            }
		}

        if (scope_index == GLOBAL_SCOPE)
            return KAI_FALSE;
        
        // Do not allow Local variables from higher procedure scopes
        if (scope->is_proc_scope)
            allow_locals = KAI_FALSE;

        scope_index = scope->parent;
    }
}

#define c_add_dependency(RANGE, REF) \
	add_dependency(context, RANGE, REF)
void add_dependency(Compiler_Context* context, Kai_range* range, Node_Ref ref) {
	for (Kai_int i = 0; i < range->count; ++i) {
		if (dependencies.data[i + range->offset] == ref)
			return;
	}
	u32 pos = range->offset + range->count;
	array_reserve(dependencies, dependencies.count + 1);
	for (Kai_int i = dependencies.count; i > pos; --i) {
		dependencies.data[i] = dependencies.data[i-1];
	}
	// Ranges that are higher will be invalidated, fix them here:
	for_n (nodes.count) {
		Kai_range* r = &nodes.data[i].value_depenencies;
		if (r->offset >= pos && r != range) {
			r->offset += 1;
		}
	}
	dependencies.data[pos] = ref;
	dependencies.count += 1;
	range->count += 1;
}

Kai_bool insert_value_dependencies(
    Compiler_Context* context,
	Kai_range*        deps,
    u32               scope_index,
    Kai_Expr          expr,
    Kai_bool          in_procedure
) {
    if (expr == NULL) { panic_with_message("null expression\n"); return 0; }

    switch (expr->id)
    {
    default: {
		panic_with_message("undefined expr [insert_value_dependencies] (id = %d)\n", expr->id);
	}

    break; case KAI_EXPR_IDENTIFIER: {
		u32 it;
		Kai_bool result = c_resolve_dependency_node(expr->source_code, &it, scope_index, in_procedure);

		if (result == KAI_FALSE)
			return c_error_not_declared(expr->source_code, expr->line_number);

        // DO NOT depend on local variables
        if (in_procedure && it == LOCAL_VARIABLE_INDEX)
            break;
		
		c_add_dependency(deps, it);
    }

    break; case KAI_EXPR_NUMBER:
    break; case KAI_EXPR_STRING:

    break; case KAI_EXPR_UNARY: {
        Kai_Expr_Unary* unary = (Kai_Expr_Unary*)expr;
		return c_insert_value_dependencies(deps, scope_index, unary->expr, in_procedure);
    }

    break; case KAI_EXPR_BINARY: {
        Kai_Expr_Binary* binary = (Kai_Expr_Binary*)expr;
		if (c_insert_value_dependencies(deps, scope_index, binary->left, in_procedure))
            return KAI_TRUE;
        return c_insert_value_dependencies(deps, scope_index, binary->right, in_procedure);
    }

    break; case KAI_EXPR_PROCEDURE_CALL: {
    //    Kai_Expr_Procedure_Call* call = (Kai_Expr_Procedure_Call*)expr;
//
    //    if (in_procedure) {
    //        if (c_insert_type_dependencies(deps, scope_index, call->proc))
    //            return KAI_TRUE;
    //    }
    //    else {
    //        if (c_insert_value_dependencies(deps, scope_index, call->proc, in_procedure))
    //            return KAI_TRUE;
    //    }
//
    //    for (u32 i = 0; i < call->arg_count; ++i) {
	//		if(c_insert_value_dependencies(deps, scope_index, call->arguments[i], in_procedure))
    //            return KAI_TRUE;
    //    }
    }

    break; case KAI_EXPR_PROCEDURE_TYPE: {
        Kai_Expr_Procedure_Type* proc = (Kai_Expr_Procedure_Type*)expr;
		panic_with_message("procedure type\n");
    }

    break; case KAI_EXPR_PROCEDURE: {
        Kai_Expr_Procedure* proc = (Kai_Expr_Procedure*)expr;
		Scope* local_scope = scopes.data + proc->_scope;

		// Insert procedure input names to local scope
		int n = proc->param_count;
        for (int i = 0; i < n; ++i) {
			Kai_Expr_Procedure_Parameter* parameter = proc->input_output + i;

			u32 it = scope_find(local_scope, parameter->name);
			if (it != NONE)
				return c_error_redefinition(parameter->name, proc->line_number, nodes.data + it);

			Named_Node_Ref local = {.name = parameter->name, .index = LOCAL_VARIABLE_INDEX};
			array_reserve(local_scope->identifiers, local_scope->identifiers.count + 1);
			array_push_back(local_scope->identifiers, local);
        }

		if (c_insert_value_dependencies(deps, proc->_scope, proc->body, KAI_TRUE))
			return KAI_TRUE;

		remove_all_locals(local_scope);
    }

    break; case KAI_STMT_DECLARATION: {
        if (in_procedure) {
            Kai_Stmt_Declaration* decl = (Kai_Stmt_Declaration*)expr;

            // Already has a dependency node
            if (decl->flags & KAI_DECL_FLAG_CONST) return KAI_FALSE;

            Scope* scope = scopes.data + scope_index;

            // Insert into local scope (if not already defined)
            u32 it = scope_find(scope, decl->name);
            if (it != NONE && scope->identifiers.data[it].index != LOCAL_VARIABLE_INDEX)
                return c_error_redefinition(decl->name, decl->line_number, nodes.data + it);

			Named_Node_Ref local = {.name = decl->name, .index = LOCAL_VARIABLE_INDEX};
			array_push_back(scope->identifiers, local);

            // Look into it's definition to find possible dependencies
            return c_insert_value_dependencies(deps, scope_index, decl->expr, KAI_TRUE);
        }
        else panic_with_message("invalid declaration\n");
    }

    break; case KAI_STMT_ASSIGNMENT: {
        if (in_procedure) {
            Kai_Stmt_Assignment* assignment = (Kai_Stmt_Assignment*)expr;
            return c_insert_value_dependencies(deps, scope_index, assignment->expr, KAI_TRUE);
        }
        else panic_with_message("invalid assignment\n");
    }

    break; case KAI_STMT_RETURN: {
        if (in_procedure) {
            Kai_Stmt_Return* ret = (Kai_Stmt_Return*)expr;
            return c_insert_value_dependencies(deps, scope_index, ret->expr, KAI_TRUE);
        }
		else panic_with_message("invalid return\n");
    }

    break; case KAI_STMT_IF: {
		Kai_Stmt_If* ifs = (Kai_Stmt_If*)expr;
		if(c_insert_value_dependencies(deps, scope_index, ifs->body, in_procedure))
			return KAI_TRUE;
		if(ifs->else_body && c_insert_value_dependencies(deps, scope_index, ifs->else_body, in_procedure))
			return KAI_TRUE;
    }

    break; case KAI_STMT_COMPOUND: {
        if (in_procedure) {
            Kai_Stmt_Compound* comp = (Kai_Stmt_Compound*)expr;

            u32 const n = comp->count;
            for_n(n) {
                if(c_insert_value_dependencies(deps, comp->_scope, comp->statements[i], KAI_TRUE))
                    return KAI_TRUE;
            }

            Scope* scope = scopes.data + comp->_scope;
            remove_all_locals(scope);
        }
        else panic_with_message("invalid compound statement\n");
    }

    break; case KAI_STMT_FOR: {
        Kai_Stmt_For* for_ = (Kai_Stmt_For*)expr;
        if (c_insert_value_dependencies(deps, scope_index, for_->body, in_procedure))
            return KAI_TRUE;
    }

    break;
    }

	return KAI_FALSE;
}

void reserve(void** base, u64* capacity, u64 new_capacity, u32 stride) {
	if (*capacity >= new_capacity) return;
	new_capacity = (new_capacity * 3) / 2;
	void* n = realloc(*base, new_capacity*(u64)stride);
	*capacity = new_capacity;
    *base = n;
}
