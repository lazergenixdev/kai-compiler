#define KAI_USE_DEBUG_API
#include "compiler.h"
#include "builtin_types.h"
#include "bytecode.h"
#include <stdlib.h>

Kai_Result
kai_create_program(Kai_Program_Create_Info* info, Kai_Program* program) {
	return KAI_ERROR_FATAL;
}

#if 0

// TODO: DEBUG ONLY
char temp[1024];
Kai_Debug_String_Writer* writer;
int debug_index;

void _debug_print_scope(Compiler_Context* context, u32 index) {
	Scope* scope = context->dependency_graph.scopes.data + index;
	_write_format("scope # %i\n", index);
	for_n (scope->identifiers.count) {
		_write(" - id: ");
		_write_string(scope->identifiers.data[i].name);
		_write("\n");
	}
}

#define iterate(CONST_ARRAY) \
	for (int i = 0; i < count_(CONST_ARRAY); ++i)

void insert_builtin_types(Dependency_Graph* graph);
void reserve(void** base, u32* capacity, u32 new_capacity, u32 stride);

#define array_reserve(A, NEW_CAPACITY) reserve((void**)&A.data, &A.capacity, NEW_CAPACITY, sizeof(A.data[0]))
#define array_push_back(A, VALUE) A.data[A.count++] = VALUE

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

#define c_error_circular_dependency(REF) \
	error_circular_dependency(context, REF)

void* error_circular_dependency(Compiler_Context* context, Node_Ref ref) {
#define err context->error
	Node* node = context->dependency_graph.nodes.data + (ref & ~TYPE_BIT);

    err.result = KAI_ERROR_SEMANTIC;
    err.location.string = node->name;
    err.location.line = node->line_number;
    err.message.count = 0;
	err.message.data = (u8*)context->memory.temperary; // uses temperary memory

    str_insert_string(err.message, ref & TYPE_BIT? "type" : "value");
    str_insert_string(err.message, " of \"");
    str_insert_str   (err.message, node->name);
    str_insert_string(err.message, "\" cannot depend on itself");

    return (u8*)err.message.data + err.message.count;
#undef err
}

#define c_error_dependency_info(START, REF) \
	error_dependency_info(context, START, REF)

void* error_dependency_info(Compiler_Context* context, void* start, Node_Ref ref) {
#define err context->error
	_write_format("got node %i\n", ref & ~TYPE_BIT);
	Node* node = context->dependency_graph.nodes.data + (ref & ~TYPE_BIT);

    err.result = KAI_ERROR_INFO;
    err.location.string = node->name;
    err.location.line = node->line_number;
    err.message.count = 0;
	err.message.data = (u8*)context->memory.temperary; // uses temperary memory
    err.context = KAI_EMPTY_STRING;
    err.next = NULL;

    str_insert_string(err.message, "see ");
    str_insert_string(err.message, ref & TYPE_BIT? "type" : "value");
    str_insert_string(err.message, " of \"");
    str_insert_str   (err.message, node->name);
    str_insert_string(err.message, "\"");

    return (u8*)err.message.data + err.message.count;
#undef err
}

Kai_bool compile_value(Compiler_Context* context, u32 index);
Kai_bool compile_type (Compiler_Context* context, u32 index);

// Convert from DFS index to node index in dependency graph
#define node_at(X) \
(X < context->dependency_graph.nodes.count)? X : ((X - context->dependency_graph.nodes.count)|TYPE_BIT)

void dfs_explore(DFS_Context* dfs, u32 s) {
	dfs->visited[s] = KAI_TRUE;

	Dependency_Graph* dg = &dfs->compiler->dependency_graph;
	Node_Ref* dependencies = dg->dependencies.data;
	Node* node;

	Kai_range deps;

	if (s < dg->nodes.count) {
		node = dg->nodes.data + s;
		deps = node->value_dependencies;
	} else {
		node = dg->nodes.data + s - dg->nodes.count;
		deps = node->type_dependencies;
	}

	for (int i = 0; i < deps.count; ++i) {
		u32 v = dependencies[i + deps.offset];

		// From node index to index in DFS tree
		if (v & TYPE_BIT) {
			v &=~ TYPE_BIT;
			v += dg->nodes.count;
		}

		if (!dfs->visited[v]) {
			dfs->prev[v] = s;
			dfs_explore(dfs, v);
		}
	}

	dfs->post[s] = dfs->next++;
}

#define DEBUG_WRITE_COMPILATION_ORDER 1
#define DEBUG_WRITE_NODES             1

u32* get_compilation_order(Compiler_Context* context) {
	// TODO: only one allocation necessary
	DFS_Context dfs = { .compiler = context, .next = 0 };
	dfs.post    = calloc(context->dependency_graph.nodes.count * 2, sizeof(u32));
	dfs.prev    = malloc(context->dependency_graph.nodes.count * 2* sizeof(u32));
	dfs.visited = calloc(context->dependency_graph.nodes.count * 2, sizeof(Kai_bool));

	memset(dfs.prev, 0xFF, context->dependency_graph.nodes.count * 2* sizeof(u32));

	// Do not waste time on builtins
	//iterate (builtin_types) {
	//	dfs.visited[i] = dfs.visited[i + context.dependency_graph.nodes.count] = KAI_TRUE;
	//}

	// Perform DFS traversal
	for_n (context->dependency_graph.nodes.count) {
		if (!dfs.visited[i])
			dfs_explore(&dfs, i);
	}
	for_n (context->dependency_graph.nodes.count) {
		u32 s = i + context->dependency_graph.nodes.count;
		if (!dfs.visited[s])
			dfs_explore(&dfs, s);
	}

	// Get compilation order  TODO: fix this O(N^2) algorithm
	u32* order = calloc(context->dependency_graph.nodes.count * 2, sizeof(u32));
	{
		u32 next = 0;
		u32 count = 0;
		for_n (context->dependency_graph.nodes.count * 2) {
			cont:
			if (dfs.post[i] == next) {
				order[count++] = i;
				if (count >= context->dependency_graph.nodes.count * 2)
					break;
				++next;
				i = 0;
				goto cont;
			}
		}

#if DEBUG_WRITE_COMPILATION_ORDER
		_write("Compilation Order:\n");
		for_n (count) {
			u32 v = order[i];
			if (v >= context->dependency_graph.nodes.count) {
				_write_char('T');
				v -= context->dependency_graph.nodes.count;
			}
			_write_format("%-2i ", v);
		}
		_write_char('\n');
#endif
	}

	// Check for any circular dependencies
    //for_n (context->dependency_graph.nodes.count * 2) {
	//	_write_format("prev(%i) = %i\n", (int)i, dfs.prev[i]);
	//}

	// Check for any circular dependencies
    for_ (u, context->dependency_graph.nodes.count * 2)
    {
		Node_Ref* dependencies = context->dependency_graph.dependencies.data;
		Node* node;
		Kai_range deps;

		if (u < context->dependency_graph.nodes.count) {
			node = context->dependency_graph.nodes.data + u;
			deps = node->value_dependencies;
		} else {
			node = context->dependency_graph.nodes.data + u - context->dependency_graph.nodes.count;
			deps = node->type_dependencies;
		}
		
        for_n (deps.count) {
			Node_Ref v = dependencies[deps.offset + i];
			
			if (v & TYPE_BIT) {
				v &=~ TYPE_BIT;
				v += context->dependency_graph.nodes.count;
			}

            if (dfs.post[u] < dfs.post[v]) {
				memset(dfs.visited, 0, context->dependency_graph.nodes.count * 2 * sizeof(Kai_bool));
                dfs_explore(&dfs, u);

                // loop through prev pointers to get to u, and add errors in reverse order

                void* start = c_error_circular_dependency(node_at(u));

				// TODO: Fix error messages here
                Kai_Error* last = &context->error;
				//_write_format("index = %i\n", (int)u);
                //for (u32 i = dfs.prev[u];; i = dfs.prev[i]) {
				//	_write_format("index = %i\n", (int)i);
                //    last->next = (Kai_Error*)start;
                //    start = c_error_dependency_info(start, node_at(i));
                //    last = last->next;
                //    if (i == (~1) || i == u) break;
                //}
				(void)start;
				(void)last;
				goto error;
            }
        }
    }

//cleanup:
	free(dfs.post);
	free(dfs.prev);
	free(dfs.visited);
	return order;

error:
	free(order);
	free(dfs.post);
	free(dfs.prev);
	free(dfs.visited);
	return NULL;
}

Kai_Result
kai_create_program(Kai_Program_Create_Info* info, Kai_Program* program) {
#if 0
	u8 bytes [1000];
	u32 count = 0;

	bytes[count++] = BYTECODE_OPERATION_LOAD;
	bytes[count++] = BYTECODE_TYPE_S32;
	*((u32*)(bytes+count)) = 0;    count += 4;
	*((s32*)(bytes+count)) = 0x22; count += 4;

	bytes[count++] = BYTECODE_OPERATION_LOAD;
	bytes[count++] = BYTECODE_TYPE_S32;
	*((u32*)(bytes+count)) = 1;    count += 4;
	*((s32*)(bytes+count)) = 0x23; count += 4;

	bytes[count++] = BYTECODE_OPERATION_ADD;
	bytes[count++] = BYTECODE_TYPE_S32;
	*((u32*)(bytes+count)) = 2; count += 4;
	*((u32*)(bytes+count)) = 0; count += 4;
	*((u32*)(bytes+count)) = 1; count += 4;

	#if 0
	Bytecode_Stream stream;

	bytecode_stream_create(&stream);
	/*
	001		%0 <- (s32) 0x22
	002		%1 <- (s32) 0x23
	003		%2 <- add.s32 %0, %1
	004     branch [+1] %2
	005		jump   [-2]
	*/
	bytecode_stream_insert_load(&stream, BYTECODE_TYPE_S32, 0, (Bytecode_Value) {.value_s32 = 0x22});
	bytecode_stream_insert_load(&stream, BYTECODE_TYPE_S32, 0, (Bytecode_Value) {.value_s32 = 0x23});
	bytecode_stream_insert_add(&stream, BYTECODE_TYPE_S32, 2, 0, 1);
	#endif

	//for (int i = 0; i < count; ++i) printf("%2d ", i); putchar('\n');
	//for (int i = 0; i < count; ++i) printf("%02x ", bytes[i]); putchar('\n');

	Bytecode_Interpreter interpreter = {
		.bytecode = bytes,
		.count = count,
	};

	while (interp_step(&interpreter));

	//printf("flags = %x\n", interpreter.flags);
	printf("finished with value = %i\n", interpreter.registers[2].value_s32);
#endif
	return KAI_ERROR_FATAL;
	
	Compiler_Context context = {
		.memory = info->memory,
	};

    writer = kai_debug_stdout_writer();

	array_reserve(context.dependency_graph.scopes,         64);
	array_reserve(context.dependency_graph.nodes,         256);
	array_reserve(context.dependency_graph.dependencies, 1024);

    // Add global scope
	context.dependency_graph.scopes.count += 1;
    Scope* global_scope = context.dependency_graph.scopes.data + GLOBAL_SCOPE;
    *global_scope = (Scope){.parent = NONE};
    array_reserve(global_scope->identifiers, 64);

	insert_builtin_types(&context.dependency_graph);
	
    // TODO: only look at first tree, but want to look at all trees,
    //       all with their own scope somehow..
	Kai_Syntax_Tree* tree = info->trees;

	// Make nodes for every statement
	for_n (tree->top_level_count) {
		Kai_Stmt tl = tree->top_level_statements[i];
		if (insert_node_for_statement(&context, tl, GLOBAL_SCOPE, KAI_FALSE, KAI_FALSE))
            goto error;
	}

	// Insert dependencies for each node
	for_n (context.dependency_graph.nodes.count) {
		Node* node = context.dependency_graph.nodes.data + i;
		debug_index = i;

		if (!(node->value_flags&NODE_EVALUATED)
		&& insert_value_dependencies(&context, &node->value_dependencies, node->scope, node->expr, KAI_FALSE))
            goto error;

		if (!(node->type_flags&NODE_EVALUATED)
        && insert_type_dependencies(&context, &node->type_dependencies, node->scope, node->expr))
        	goto error;
	}

#if DEBUG_WRITE_NODES
	for_n (context.dependency_graph.nodes.count) {
		Node* node = context.dependency_graph.nodes.data + i;

		_write_format("%2u ", (int)i);
		_write_char(node->value_flags & NODE_EVALUATED? 'V' : ' ');
		_write_char(node->type_flags  & NODE_EVALUATED? 'T' : ' ');
		_write_char(' ');
		_write_string(node->name);

		for (int i = 0; i < 12 - (int)node->name.count; ++i)
			_write_char(' ');

		Dependency_Graph* g = &context.dependency_graph;
		for_n (node->value_dependencies.count) {
			u32 ref = g->dependencies.data[node->value_dependencies.offset + i];
			if (ref&TYPE_BIT) _set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
			_write_format("%u ", ref&(~TYPE_BIT));
			if (ref&TYPE_BIT) _set_color(KAI_DEBUG_COLOR_PRIMARY);
		}

		if (node->type_dependencies.count) {
			_write_char('(');
			for_n (node->type_dependencies.count) {
				u32 ref = g->dependencies.data[node->type_dependencies.offset + i];
				if (ref&TYPE_BIT) _set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
				_write_format("%u ", ref&(~TYPE_BIT));
				if (ref&TYPE_BIT) _set_color(KAI_DEBUG_COLOR_PRIMARY);
			}
			_write("\b)");
		}
		_write_char('\n');
	}
#endif

	u32* order = get_compilation_order(&context);
	if (order == NULL) goto error;

	// compile in order ...
	for_n (context.dependency_graph.nodes.count * 2) {
		int u = order[i];
		u = (u < context.dependency_graph.nodes.count)? u : ((u - context.dependency_graph.nodes.count)|TYPE_BIT);

		Node* node = context.dependency_graph.nodes.data + (u & ~TYPE_BIT);

		if (u & TYPE_BIT) {
			if (node->value_flags & NODE_EVALUATED) continue;
		}
		else {
			if (node->type_flags & NODE_EVALUATED) continue;
		}

		if (u & TYPE_BIT) {
			if (compile_type(&context, u & ~TYPE_BIT)) goto cleanup;
		}
		else {
			if (compile_value(&context, u)) goto cleanup;
		}
	}

	cleanup:
	free(order);

	if (KAI_FAILED(context.error.result)) {
	error:
        if (info->error) {
            *info->error = context.error;
            //info->error->location.file_name = ;
            //info->error->location.source = ;
        }
		return context.error.result;
	}

	info->error->message = KAI_STRING("compiler is in development :)");
	info->error->result = KAI_ERROR_INTERNAL;
    return KAI_ERROR_INTERNAL;
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
	u32                scope_index,
	Kai_bool           in_procedure,
	Kai_bool           from_procedure // came from procdure
) {
    switch (statement->id)
	{
	default:
    break; case KAI_STMT_IF: {
        Kai_Stmt_If* ifs = (Kai_Stmt_If*)statement;
		if (c_insert_node_for_statement(ifs->body, scope_index, in_procedure, KAI_FALSE))
			return KAI_TRUE;
		if (ifs->else_body &&
			c_insert_node_for_statement(ifs->else_body, scope_index, in_procedure, KAI_FALSE))
			return KAI_TRUE;
    }
    break; case KAI_STMT_FOR: {
        Kai_Stmt_For* fors = (Kai_Stmt_For*)statement;
		if (c_insert_node_for_statement(fors->body, scope_index, in_procedure, KAI_FALSE))
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

		u32 new_scope_index = scope_index;

		if (!from_procedure) {
			new_scope_index = scopes.count;
			Scope new_scope = {
				.parent = scope_index,
				.is_proc_scope = KAI_TRUE,
			};
			array_reserve(scopes, scopes.count + 1);
			array_push_back(scopes, new_scope);
		}

		comp->_scope = new_scope_index;

		for_n(comp->count) {
			if (c_insert_node_for_statement(comp->statements[i], new_scope_index, in_procedure, KAI_FALSE))
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
			.value_dependencies = {.offset = first_dependency, .count = 1},
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
        (void)proc;

		u32 new_scope_index = scopes.count;
		Scope new_scope = {
			.parent = scope_index,
			.is_proc_scope = KAI_TRUE,
		};
		array_reserve(scopes, scopes.count + 1);
		array_push_back(scopes, new_scope);

		proc->_scope = new_scope_index;
		return c_insert_node_for_statement(proc->body, new_scope_index, KAI_TRUE, KAI_TRUE);
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
	Kai_str name, Named_Node_Ref* out_ref, u32 scope_index, Kai_bool const in_proc
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
					*out_ref = scope->identifiers.data[it];
					return KAI_TRUE;
                }
            }
            else {
				*out_ref = scope->identifiers.data[it];
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
		Kai_range* r = &nodes.data[i].value_dependencies;
		if (r->offset >= pos && r != range) {
			r->offset += 1;
		}
		r = &nodes.data[i].type_dependencies;
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
		Named_Node_Ref dependency;
		Kai_bool result = c_resolve_dependency_node(expr->source_code, &dependency, scope_index, in_procedure);

		if (result == KAI_FALSE)
			return c_error_not_declared(expr->source_code, expr->line_number);

        // DO NOT depend on local variables
        if (in_procedure && dependency.index == LOCAL_VARIABLE_INDEX)
            break;
		
		c_add_dependency(deps, dependency.index);
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
        Kai_Expr_Procedure_Call* call = (Kai_Expr_Procedure_Call*)expr;

		// Procedure calls in procedures are fine, we only need to know the type to generate the bytecode
		// TODO: possible bug for nested procedures
        if (in_procedure) {
            if (c_insert_type_dependencies(deps, scope_index, call->proc))
                return KAI_TRUE;
        }
        else {
            if (c_insert_value_dependencies(deps, scope_index, call->proc, in_procedure))
                return KAI_TRUE;
        }

        for (u32 i = 0; i < call->arg_count; ++i) {
			if(c_insert_value_dependencies(deps, scope_index, call->arguments[i], in_procedure))
                return KAI_TRUE;
        }
    }

    break; case KAI_EXPR_PROCEDURE_TYPE: {
        Kai_Expr_Procedure_Type* proc = (Kai_Expr_Procedure_Type*)expr;
        (void)proc;
		panic_with_message("procedure type\n");
    }

    break; case KAI_EXPR_PROCEDURE: {
        Kai_Expr_Procedure* proc = (Kai_Expr_Procedure*)expr;
		Scope* local_scope = scopes.data + proc->_scope;

		// Insert procedure input names to local scope
        for (int i = 0; i < proc->param_count; ++i) {
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
		    array_reserve(scope->identifiers, scope->identifiers.count + 1);
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

Kai_bool insert_type_dependencies(
	Compiler_Context* context,
    Kai_range* deps,
    u32 scope_index,
    Kai_Expr expr
) {
    if (expr == NULL) { panic_with_message("null expression\n"); return KAI_FALSE; }

    switch (expr->id)
    {
    default:
    break; case KAI_EXPR_IDENTIFIER: {
		Named_Node_Ref dependency;
		// TODO: why true?
		Kai_bool result = c_resolve_dependency_node(expr->source_code, &dependency, scope_index, KAI_TRUE);

        if (result == KAI_FALSE)
            return c_error_not_declared(expr->source_code, expr->line_number);

        // DO NOT depend on local variables
        if (dependency.index == LOCAL_VARIABLE_INDEX)
            return KAI_FALSE;

		c_add_dependency(deps, dependency.index | TYPE_BIT);
    }

    break; case KAI_EXPR_UNARY: {
        Kai_Expr_Unary* unary = (Kai_Expr_Unary*)expr;
        return c_insert_type_dependencies(deps, scope_index, unary->expr);
    }

    break; case KAI_EXPR_BINARY: {
        Kai_Expr_Binary* binary = (Kai_Expr_Binary*)expr;
        if (c_insert_type_dependencies(deps, scope_index, binary->left))
            return KAI_TRUE;
        return c_insert_type_dependencies(deps, scope_index, binary->right);
    }

    break; case KAI_EXPR_PROCEDURE_CALL: {
        Kai_Expr_Procedure_Call* call = (Kai_Expr_Procedure_Call*)expr;

        if (c_insert_type_dependencies(deps, scope_index, call->proc))
            return KAI_TRUE;

        for (Kai_u32 i = 0; i < call->arg_count; ++i) {
            if (c_insert_value_dependencies(deps, scope_index, call->arguments[i], KAI_FALSE))
                return KAI_TRUE;
        }
    }

    break; case KAI_EXPR_PROCEDURE_TYPE: {
        //Kai_Expr_Procedure_Type* proc = (Kai_Expr_Procedure_Type*)expr;
        panic_with_message("procedure type\n");
        //kai_int total = proc->parameter_count + proc->ret_count;
        //for (int i = 0; i < total; ++i) {
        //    print_tree(ctx, proc->types[i]);
        //}
    }

    break; case KAI_EXPR_PROCEDURE: {
        Kai_Expr_Procedure* proc = (Kai_Expr_Procedure*)expr;
        for_n(proc->param_count + proc->ret_count) {
            Kai_Expr_Procedure_Parameter* param = proc->input_output + i;
			//_write_string(param->name);
			//_write(" --- ");
			//kai_debug_write_expression(writer, param->type);
            if (c_insert_value_dependencies(deps, scope_index, param->type, KAI_FALSE))
                return KAI_TRUE;
        }
    }

    break; case KAI_STMT_DECLARATION: {
        //Kai_Stmt_Declaration* decl = (Kai_Stmt_Declaration*)expr;
        panic_with_message("declaration\n");

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

    break; case KAI_STMT_RETURN: {
        Kai_Stmt_Return* ret = (Kai_Stmt_Return*)expr;
        return c_insert_type_dependencies(deps, scope_index, ret->expr);
    }

    break; case KAI_STMT_COMPOUND: {
        panic_with_message("compound\n");
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
	
    return KAI_FALSE;
}

Kai_bool compile_value(Compiler_Context* context, u32 index) {
	Node* node = nodes.data + index;
	Kai_Expr expr = node->expr;

	_write_format("Compiling : %i, expr = %i\n", index, expr->id);

	switch (expr->id)
	{
    default: {
		panic_with_message("undefined expr [compile_value] (id = %d)\n", expr->id);
	}

	break; case KAI_EXPR_IDENTIFIER: {
		_write("Got an identifier!!! --> ");
		_write_string(expr->source_code);
		_write_char('\n');
	}
	
	}
	return KAI_FALSE;
}

u32 recursive_scope_find(Compiler_Context* context, u32 scope_index, Kai_str name) {
	do {
		Scope* scope = scopes.data + scope_index;

		u32 it = scope_find(scope, name);

		if (it != NONE) {
			return it;
		}

		scope_index = scope->parent;
		
	} while (scope_index != GLOBAL_SCOPE);
	return NONE;
}

Kai_Type type_of_expression(Compiler_Context* context, Kai_Expr expr, u32 scope_index) {
	switch (expr->id)
	{
    default: {
		panic_with_message("undefined expr [type_of_expression] (id = %d)\n", expr->id);
	}
	
	break; case KAI_EXPR_IDENTIFIER: {
		// locate this value in the scope
		u32 node_index = recursive_scope_find(context, scope_index, expr->source_code);

		// confirm we found something
		if (node_index == NONE) {
			_write("ERROR: did not find identifier [type_of_expression]");
			return NULL;
		}

		Node* node = nodes.data + node_index;

		// confirm that this value is evaluated
		if (!(node->type_flags & NODE_EVALUATED)) {
			_write("ERROR: node is not evaluated [type_of_expression]");
			return NULL;
		}

		_write("got a type!!\n");

		kai_debug_write_type(writer, node->type);

		// set the value of this node??
		return node->type;
	}

	}

	return NULL;
}

Kai_bool compile_type (Compiler_Context* context, u32 index) {
	Node* node = nodes.data + index;
	Kai_Expr expr = node->expr;

	_write_format("Compiling : %i, expr = %i\n", index, expr->id);

	switch (expr->id)
	{
    default: {
		panic_with_message("undefined expr [compile_type] (id = %d)\n", expr->id);
	}
	
	break; case KAI_EXPR_IDENTIFIER: {
		Kai_Type type = type_of_expression(context, expr, node->scope);

		if (type == NULL) return KAI_FALSE;

		node->type = type;
	}

	}
	return KAI_TRUE;
}

void reserve(void** base, u32* capacity, u32 new_capacity, u32 stride) {
	if (*capacity >= new_capacity) return;
	new_capacity = (new_capacity * 3) / 2;
	void* n = realloc(*base, new_capacity*(u64)stride);
	*capacity = new_capacity;
    *base = n;
}

#endif
