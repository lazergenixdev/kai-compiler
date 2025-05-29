#ifndef __WASM__
#define KAI_USE_DEBUG_API
#define KAI_USE_MEMORY_API
#include "builtin_types.h"
#include <stdlib.h>
#include "program.h"

//#define DEBUG_DEPENDENCY_GRAPH
//#define DEBUG_COMPILATION_ORDER
#define DEBUG_CODE_GENERATION

Kai_Result kai_create_program(Kai_Program_Create_Info* info, Kai_Program* program)
{
	(void)(program);
	return kai__error_internal(info->error, KAI_STRING("kai_create_program not implemented"));
}

#define kai__check(RESULT) if (RESULT != KAI_SUCCESS) goto cleanup;
Kai_Result kai_create_program_from_source(
	Kai_str        Source,
	Kai_Allocator* Allocator,
	Kai_Error*     out_Error,
	Kai_Program*   out_Program)
{
	Kai_Result            result           = KAI_SUCCESS;
	Kai_Syntax_Tree       syntax_tree      = {0};
	Kai__Dependency_Graph dependency_graph = {0};
	Kai__Bytecode         bytecode         = {0};

	{
		Kai_Syntax_Tree_Create_Info info = {
			.source_code = Source,
			.error = out_Error,
			.allocator = *Allocator,
		};
		result = kai_create_syntax_tree(&info, &syntax_tree);
	}
	kai__check(result);
	
	{
		Kai__Dependency_Graph_Create_Info info = {
			.trees = &syntax_tree,
			.tree_count = 1,
			.allocator = *Allocator,
			.error = out_Error,
		};
		result = kai__create_dependency_graph(&info, &dependency_graph);
	}
	kai__check(result);

	{
		result = kai__determine_compilation_order(&dependency_graph, out_Error);
	}
	kai__check(result);

	{
		Kai__Bytecode_Create_Info info = {
			.error = out_Error,
			.dependency_graph = &dependency_graph,
		};
		result = kai__generate_bytecode(&info, &bytecode);
	}
	kai__check(result);

	{
		Kai__Program_Create_Info info = {
			.bytecode = &bytecode,
			.error = out_Error,
			.allocator = *Allocator,
		};
		result = kai__create_program(&info, out_Program);
	}
	kai__check(result);

cleanup:
	kai__destroy_bytecode(&bytecode);
	kai__destroy_dependency_graph(&dependency_graph);
	kai_destroy_syntax_tree(&syntax_tree);
	return result;
}
#undef kai__check

Kai__DG_Node_Index* kai__recursive_scope_find(
	Kai__Dependency_Graph* Graph,
	Kai_u32                Scope_Index,
	Kai_str                Identifier)
{
	do {
		Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;
		Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, Identifier);
		if (node_index != NULL) {
			return node_index;
		}
		Scope_Index = scope->parent;
	} while (Scope_Index != KAI__SCOPE_NO_PARENT);
	return NULL;
}

Kai_Result kai__error_redefinition(
	Kai_Error*     out_Error,
	Kai_Allocator* allocator,
	Kai_str        string,
	Kai_u32        line_number,
	Kai_str        original_name,
	Kai_u32        original_line_number)
{
	*out_Error = (Kai_Error) {
		.result = KAI_ERROR_SEMANTIC,
		.location = {
			.line = line_number,
			.string = string,
		},
	};

	Kai__Dynamic_Buffer buffer = {0};

	// First error message	
	{
		kai__dynamic_buffer_append_string(&buffer, KAI_STRING("indentifier \""));
		kai__dynamic_buffer_append_string_max(&buffer, string, 24);
		kai__dynamic_buffer_append_string(&buffer, KAI_STRING("\" is already declared"));
		Kai_range range = kai__dynamic_buffer_next(&buffer);
		out_Error->memory = kai__dynamic_buffer_release(&buffer);
		out_Error->message = kai__range_to_string(range, out_Error->memory);
	}

	// Extra info
	{
		// Append to memory
    	Kai_range info_range = kai__dynamic_buffer_push(&buffer, sizeof(Kai_Error));
		kai__dynamic_buffer_append_string(&buffer, KAI_STRING("see original definition of \""));
		kai__dynamic_buffer_append_string_max(&buffer, string, 24);
		kai__dynamic_buffer_append_string(&buffer, KAI_STRING("\""));
		Kai_range message_range = kai__dynamic_buffer_next(&buffer);
		Kai_Memory memory = kai__dynamic_buffer_release(&buffer);

		// Put everything together
		Kai_Error* info = kai__range_to_data(info_range, memory);
		*info = (Kai_Error) {
			.result = KAI_ERROR_INFO,
			.location = {
				.line = original_line_number,
				.string = original_name,
			},
			.message = kai__range_to_string(message_range, memory),
			.memory = memory,
		};
		out_Error->next = info;
	}

	return KAI_ERROR_SEMANTIC;
}

Kai_Result kai__error_not_declared(
	Kai_Error*      out_Error,
	Kai_Allocator*  allocator,
	Kai_str         string,
	Kai_u32         line_number)
{
    *out_Error = (Kai_Error) {
		.result = KAI_ERROR_SEMANTIC,
		.location = {
			.string = string,
			.line = line_number,
		},
	};
	Kai__Dynamic_Buffer buffer = {0};
	kai__dynamic_buffer_append_string(&buffer, KAI_STRING("indentifier \""));
	kai__dynamic_buffer_append_string_max(&buffer, string, 24);
	kai__dynamic_buffer_append_string(&buffer, KAI_STRING("\" not declared"));
	Kai_range range = kai__dynamic_buffer_next(&buffer);
	out_Error->memory = kai__dynamic_buffer_release(&buffer);
	out_Error->message = kai__range_to_string(range, out_Error->memory);
    return KAI_ERROR_SEMANTIC;
}

void add_dependency(
	Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
	Kai__DG_Node_Index        Reference)
{
	Kai_Allocator* allocator = &Graph->allocator;
	for (Kai_int i = 0; i < out_Dependency_Array->count; ++i) {
		Kai__DG_Node_Index node_index = out_Dependency_Array->elements[i];
		if (node_index.flags == Reference.flags
		&&  node_index.value == Reference.value)
			return;
	}
	kai__array_append(out_Dependency_Array, Reference);
}

Kai_Result kai__dg_create_nodes_from_statement(
	Kai__Dependency_Graph* Graph,
	Kai_Stmt               Stmt,
	Kai_u32                Scope_Index,
	Kai_bool               In_Procedure,
	Kai_bool               From_Procedure)
{
	Kai_Result result = KAI_SUCCESS;
	Kai_Allocator* allocator = &Graph->allocator;
	void* base = Stmt;

    switch (Stmt->id)
	{
	case KAI_EXPR_PROCEDURE: {
		Kai_Expr_Procedure* node = base;

		Kai_u32 new_scope_index = Graph->scopes.count;
		kai__array_append(&Graph->scopes, (Kai__DG_Scope) {
			.parent = Scope_Index,
			.is_proc_scope = KAI_TRUE,
		});
		//kai__hash_table_create(&Graph->scopes.elements[new_scope_index].identifiers);

		node->_scope = new_scope_index;
		if (node->body) {
			return kai__dg_create_nodes_from_statement(Graph, node->body, new_scope_index, KAI_TRUE, KAI_TRUE);
		}
		else {
			printf("\x1b[93mWarning\x1b[0m: Native functions not implemented yet!\n");
		}
	} break;

    case KAI_STMT_IF: {
        Kai_Stmt_If* node = base;

		result = kai__dg_create_nodes_from_statement(Graph, node->body, Scope_Index, In_Procedure, KAI_FALSE);
		if (result != KAI_SUCCESS)
			return result;

		if (node->else_body) {
			result = kai__dg_create_nodes_from_statement(Graph, node->else_body, Scope_Index, In_Procedure, KAI_FALSE);
			if (result != KAI_SUCCESS)
				return result;
		}
    } break;

    case KAI_STMT_FOR: {
        Kai_Stmt_For* node = base;

		return kai__dg_create_nodes_from_statement(Graph, node->body, Scope_Index, In_Procedure, KAI_FALSE);
    } break;

    case KAI_STMT_DECLARATION: {
		Kai_Stmt_Declaration* node = base;

		if (In_Procedure && !(node->flags & KAI_DECL_FLAG_CONST))
			return result;

		Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;

		// Does this declaration already exist for this Scope?
		Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, node->name);
		if (node_index != NULL) {
			Kai__DG_Node* existing = &Graph->nodes.elements[node_index->value];
			return kai__error_redefinition(
				Graph->error,
				allocator,
				node->name,
				node->line_number,
				existing->name,
				existing->line_number
			);
		}

		{
			Kai__DG_Node_Index index = {
				.value = Graph->nodes.count
			};

			Kai__DG_Node dg_node = {
				.name = node->name,
				.expr = node->expr,
				.line_number = node->line_number,
				.scope_index = Scope_Index,			
			};

			kai__array_append(&dg_node.value_dependencies, (Kai__DG_Node_Index) {
				.flags = KAI__DG_NODE_TYPE,
				.value = index.value,
			});

			kai__array_append(&Graph->nodes, dg_node);
			kai__hash_table_emplace(scope->identifiers, node->name, index);
		}

		if (node->expr) {
			return kai__dg_create_nodes_from_statement(
				Graph,
				node->expr,
				Scope_Index,
				In_Procedure,
				KAI_FALSE
			);
		}
	} break;

	case KAI_STMT_COMPOUND: {
		Kai_Stmt_Compound* node = base;
		Kai_u32 new_scope_index = Scope_Index;

		if (!From_Procedure) {
			new_scope_index = Graph->scopes.count;
			kai__array_append(&Graph->scopes, (Kai__DG_Scope) {
				.parent = Scope_Index,
				.is_proc_scope = KAI_TRUE,
			});
			//kai__hash_table_create(&Graph->scopes.elements[new_scope_index].identifiers);
		}

		node->_scope = new_scope_index;

		Kai_Stmt current = node->head;
		while (current) {
			result = kai__dg_create_nodes_from_statement(Graph, current, new_scope_index, In_Procedure, KAI_FALSE);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
		}
	} break;
	}
	return result;
}

void kai__print_scope(Kai__DG_Scope* Scope)
{
	kai__hash_table_iterate(Scope->identifiers, i)
	{
		Kai__DG_Node_Index node_index = Scope->identifiers.values[i];
		if (node_index.flags & KAI__DG_NODE_LOCAL_VARIABLE)
			putchar('L');
		Kai_str name = Scope->identifiers.keys[i];
		printf("\"%.*s\"+%i ", name.count, name.data, node_index.value);
	}
}

Kai__DG_Node_Index* kai__dg_resolve_dependency_node(
	Kai__Dependency_Graph* Graph,
	Kai_str                Name,
	Kai_u32                Scope_Index,
	Kai_bool               In_Procedure)
{
    Kai_bool allow_locals = KAI_TRUE;
    while (1) {
        Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;
		Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, Name);

        if (node_index != NULL) {
            Kai_bool is_local = KAI_BOOL(node_index->flags & KAI__DG_NODE_LOCAL_VARIABLE);
            if (In_Procedure && is_local) {
                if (allow_locals) {
					return node_index;
                }
            }
            else {
				return node_index;
            }
		}

        if (Scope_Index == KAI__SCOPE_GLOBAL_INDEX)
            return NULL;
        
        // Do not allow Local variables from higher procedure scopes
        if (scope->is_proc_scope)
            allow_locals = KAI_FALSE;

        Scope_Index = scope->parent;
    }
}

void kai__remove_local_variables(Kai__DG_Scope* Scope)
{
	kai__hash_table_iterate(Scope->identifiers, i)
	{
		Kai__DG_Node_Index node_index = Scope->identifiers.values[i];
		if (!(node_index.flags & KAI__DG_NODE_LOCAL_VARIABLE))
			continue;
		kai__hash_table_remove_index(Scope->identifiers, i);
	}
}

Kai_Result kai__dg_insert_value_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr,
    Kai_bool                  In_Procedure)
{
	Kai_Result result = KAI_SUCCESS;
	Kai_Allocator* allocator = &Graph->allocator;
	void* base = Expr;
    if (Expr == NULL) { panic_with_message("null expression\n"); return 0; }

    switch (Expr->id)
    {
    default: {
		panic_with_message("undefined expr (id = %d)\n", Expr->id);
	} break;

    case KAI_EXPR_IDENTIFIER: {
		Kai__DG_Node_Index* node_index = kai__dg_resolve_dependency_node(
			Graph,
			Expr->source_code,
			Scope_Index,
			In_Procedure
		);

		if (node_index == NULL) {
			return kai__error_not_declared(Graph->error, allocator, Expr->source_code, Expr->line_number);
		}

        // DO NOT depend on local variables
        if (In_Procedure && (node_index->flags & KAI__DG_NODE_LOCAL_VARIABLE))
            break;
		
		add_dependency(Graph, out_Dependency_Array, *node_index);
    } break;

    case KAI_EXPR_NUMBER: {} break;
    case KAI_EXPR_STRING: {} break;

    case KAI_EXPR_UNARY: {
        Kai_Expr_Unary* node = base;
		return kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->expr,
			In_Procedure
		);
    } break;

    case KAI_EXPR_BINARY: {
        Kai_Expr_Binary* node = base;
		result = kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->left,
			In_Procedure
		);
		if (result != KAI_SUCCESS)
            return result;

		return kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->right,
			In_Procedure
		);
    } break;

    case KAI_EXPR_PROCEDURE_CALL: {
        Kai_Expr_Procedure_Call* node = base;

		// Procedure calls in procedures are fine, we only need to know the type to generate the bytecode
		// TODO: possible bug for nested procedures
        if (In_Procedure) {
			result = kai__dg_insert_type_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->proc
			);
        }
        else {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->proc,
				In_Procedure
			);
        }
		if (result != KAI_SUCCESS)
			return result;

		Kai_Expr current = node->arg_head;
        while (current != NULL) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				current,
				In_Procedure
			);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
        }
    } break;

    case KAI_EXPR_PROCEDURE_TYPE: {
        //Kai_Expr_Procedure_Type* proc = base;
		//panic_with_message("procedure type\n");
    } break;

    case KAI_EXPR_PROCEDURE: {
        Kai_Expr_Procedure* node = base;
		Kai__DG_Scope* local_scope = Graph->scopes.elements + node->_scope;

		// Insert procedure input names to local scope
		Kai_Expr current = node->in_out_expr;
        for (int i = 0; i < (int)node->in_count; ++i) {
			Kai__DG_Node_Index* node_index = kai__hash_table_find(local_scope->identifiers, current->name);
			
			if (node_index != NULL) {
				Kai__DG_Node* original = &Graph->nodes.elements[node_index->value];
				return kai__error_redefinition(
					Graph->error,
					allocator,
					current->name,
					current->line_number,
					original->name,
					original->line_number
				);
			}

			kai__hash_table_emplace(local_scope->identifiers, current->name, (Kai__DG_Node_Index) {
				.flags = KAI__DG_NODE_LOCAL_VARIABLE,
			});

            current = current->next;
        }

		if (node->body) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				node->_scope,
				node->body,
				KAI_TRUE
			);
		}

		kai__remove_local_variables(local_scope);
    } break;

    break; case KAI_STMT_DECLARATION: {
        if (In_Procedure) {
            Kai_Stmt_Declaration* node = base;
            // Already has a dependency node
            if (node->flags & KAI_DECL_FLAG_CONST)
				return KAI_SUCCESS;
            Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;
            // Insert into local scope (if not already defined)
			Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, node->name);
            if (node_index != NULL && node_index->flags != KAI__DG_NODE_LOCAL_VARIABLE) {
				Kai__DG_Node* original = &Graph->nodes.elements[node_index->value];
				print_location();
                return kai__error_redefinition(
					Graph->error,
					allocator,
					node->name,
					node->line_number,
					original->name,
					original->line_number
				);
			}

			kai__hash_table_emplace(scope->identifiers,
				node->name,
				(Kai__DG_Node_Index) {
					.flags = KAI__DG_NODE_LOCAL_VARIABLE,
				}
			);

            // Look into it's definition to find possible dependencies
            return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->expr,
				KAI_TRUE
			);
        }
        else panic_with_message("invalid declaration\n");
    }

    break; case KAI_STMT_ASSIGNMENT: {
        if (In_Procedure) {
            Kai_Stmt_Assignment* node = base;
			return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->expr,
				KAI_TRUE
			);
        }
        else panic_with_message("invalid assignment\n");
    }

    case KAI_STMT_RETURN: {
		Kai_Stmt_Return* node = base;
        if (In_Procedure) {
			return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->expr,
				KAI_TRUE
			);
        }
		else panic_with_message("invalid return\n");
    } break;

    break; case KAI_STMT_IF: {
		Kai_Stmt_If* node = base;
		result = kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->body,
			In_Procedure
		);
		if (result != KAI_SUCCESS)
			return result;
		if (node->else_body != NULL) {
			return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->else_body,
				In_Procedure
			);
		}
    }

    break; case KAI_STMT_COMPOUND: {
        if (In_Procedure) {
            Kai_Stmt_Compound* node = base;
			Kai_Expr current = node->head;
            while (current != NULL) {
				result = kai__dg_insert_value_dependencies(
					Graph,
					out_Dependency_Array,
					node->_scope,
					current,
					KAI_TRUE
				);
				if (result != KAI_SUCCESS)
					return result;
				current = current->next;
            }
            Kai__DG_Scope* scope = Graph->scopes.elements + node->_scope;
            kai__remove_local_variables(scope);
        }
        else panic_with_message("invalid compound statement\n");
    }

    break; case KAI_STMT_FOR: {
        Kai_Stmt_For* node = base;
		return kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->body,
			In_Procedure
		);
    }

    break;
    }

	return KAI_SUCCESS;
}

Kai_Result kai__dg_insert_type_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr)
{
	Kai_Result result = KAI_SUCCESS;
	void* base = Expr;
    if (Expr == NULL) { panic_with_message("null expression\n"); return KAI_FALSE; }

    switch (Expr->id)
    {
    default:
    break; case KAI_EXPR_IDENTIFIER: {
		// TODO: why true?
		Kai__DG_Node_Index* node_index = kai__dg_resolve_dependency_node(
			Graph,
			Expr->source_code,
			Scope_Index,
			KAI_TRUE
		);

        if (node_index == NULL)
            return kai__error_not_declared(Graph->error, &Graph->allocator, Expr->source_code, Expr->line_number);

        // DO NOT depend on local variables
        if (node_index->flags & KAI__DG_NODE_LOCAL_VARIABLE)
            return result;

		add_dependency(Graph, out_Dependency_Array, (Kai__DG_Node_Index) {
			.flags = KAI__DG_NODE_TYPE,
			.value = node_index->value,
		});
    }

    break; case KAI_EXPR_UNARY: {
        Kai_Expr_Unary* node = base;
        return kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->expr
		);
    }

    break; case KAI_EXPR_BINARY: {
        Kai_Expr_Binary* node = base;
        result = kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->left
		);
		if (result != KAI_SUCCESS)
			return result;
        return kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->right
		);
    }

    break; case KAI_EXPR_PROCEDURE_CALL: {
        Kai_Expr_Procedure_Call* node = base;

        result = kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->proc
		);
		if (result != KAI_SUCCESS)
			return result;

		Kai_Expr current = node->arg_head;
		while (current != NULL) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				current,
				KAI_FALSE // TODO: possible bug here ??
			);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
        }
    }

    break; case KAI_EXPR_PROCEDURE_TYPE: {
        //panic_with_message("procedure type\n");
    }

    break; case KAI_EXPR_PROCEDURE: {
        Kai_Expr_Procedure* node = base;
		Kai_Expr current = node->in_out_expr;
        while (current != NULL) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				current,
				KAI_FALSE // TODO: possible bug here ??
			);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
        }
    }

    break; case KAI_STMT_DECLARATION: {
		// What do we do here exactly?
        panic_with_message("declaration\n");
    }

    break; case KAI_STMT_RETURN: {
        Kai_Stmt_Return* node = base;
        return kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->expr
		);
    }

    break; case KAI_STMT_COMPOUND: {
        panic_with_message("compound\n");
    }

    break;
    }
	
    return result;
}

Kai_Result kai__create_dependency_graph(
	Kai__Dependency_Graph_Create_Info* Info,
	Kai__Dependency_Graph*             out_Graph)
{
	Kai_Result result = KAI_SUCCESS;
	Kai_Allocator* allocator = &Info->allocator;
	out_Graph->compilation_order = NULL;
	out_Graph->allocator = Info->allocator;
	out_Graph->error = Info->error;

	// Initialize Global Scope
	kai__array_append(&out_Graph->scopes, (Kai__DG_Scope) {
		.is_proc_scope = KAI_FALSE,
		.parent = KAI__SCOPE_NO_PARENT,
	});
	Kai__DG_Scope* global = &out_Graph->scopes.elements[KAI__SCOPE_GLOBAL_INDEX];
	//kai__hash_table_create(&global->identifiers);

	// Insert builtin types
	for (int i = 0, next = 0; i < count_of(kai__builtin_types); ++i)
	{
		kai__array_append(&out_Graph->nodes, (Kai__DG_Node) {
			.type        = &kai__type_info_type,
			.value       = { .type = kai__builtin_types[i].type },
			.value_flags = KAI__DG_NODE_EVALUATED,
			.type_flags  = KAI__DG_NODE_EVALUATED,
			.name        = kai__builtin_types[i].name,
			.scope_index = KAI__SCOPE_GLOBAL_INDEX,
		});
		kai__hash_table_emplace(global->identifiers,
			kai__builtin_types[i].name,
			(Kai__DG_Node_Index) {
				.value = next++
			}
		);
	}

	// Insert nodes from syntax tree
	for (Kai_u32 i = 0; i < Info->tree_count; ++i) {
		Kai_Syntax_Tree* tree = Info->trees + i;
		result = kai__dg_create_nodes_from_statement(
			out_Graph,
			(Kai_Stmt)&tree->root,
			KAI__SCOPE_GLOBAL_INDEX,
			KAI_FALSE,
			KAI_FALSE
		);
		if (result != KAI_SUCCESS)
			return result;
	}

	// Insert dependencies for each node
	for (Kai_u32 i = 0; i < out_Graph->nodes.count; ++i) {
		Kai__DG_Node* node = out_Graph->nodes.elements + i;

		if (!(node->value_flags&KAI__DG_NODE_EVALUATED)) {
			result = kai__dg_insert_value_dependencies(
				out_Graph,
				&node->value_dependencies,
				node->scope_index,
				node->expr,
				KAI_FALSE
			);
            if (result != KAI_SUCCESS)
				return result;
		}

		if (!(node->type_flags&KAI__DG_NODE_EVALUATED)) {
			result = kai__dg_insert_type_dependencies(
				out_Graph,
				&node->type_dependencies,
				node->scope_index,
				node->expr
			);
            if (result != KAI_SUCCESS)
				return result;
		}
	}

#if defined(DEBUG_DEPENDENCY_GRAPH)
	for (Kai_u32 i = 0; i < out_Graph->nodes.count; ++i) {
		Kai__DG_Node* node = out_Graph->nodes.elements+i;

		printf("Node \"\x1b[94m%.*s\x1b[0m\"", node->name.count, node->name.data);
		if (node->type_flags & KAI__DG_NODE_EVALUATED) {
			printf(" type: ");
			kai_debug_write_type(kai_debug_stdout_writer(), node->type);
		}
		if (node->value_flags & KAI__DG_NODE_EVALUATED) {
			printf(" value: ");
			if (node->type->type == KAI_TYPE_TYPE) {
				kai_debug_write_type(kai_debug_stdout_writer(), node->value.type);
			}
		}
		putchar('\n');

		if (node->value_dependencies.count) {
			printf("    V deps: ");
			for (Kai_u32 d = 0; d < node->value_dependencies.count; ++d) {
				Kai__DG_Node_Index node_index = node->value_dependencies.elements[d];
				Kai_str name = out_Graph->nodes.elements[node_index.value].name;
				if (node_index.flags & KAI__DG_NODE_TYPE) {
					printf("T(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
				else {
					printf("V(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
			}
			putchar('\n');
		}

		if (node->type_dependencies.count) {
			printf("    T deps: ");
			for (Kai_u32 d = 0; d < node->type_dependencies.count; ++d) {
				Kai__DG_Node_Index node_index = node->type_dependencies.elements[d];
				Kai_str name = out_Graph->nodes.elements[node_index.value].name;
				if (node_index.flags & KAI__DG_NODE_TYPE) {
					printf("T(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
				else {
					printf("V(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
			}
			putchar('\n');
		}
	}
#endif // DEBUG_DEPENDENCY_GRAPH

	return result;
}

// TODO: bad name, rename
Kai_u32 kai__node_index_to_index(Kai__DG_Node_Index node_index)
{
	Kai_bool is_type = KAI_BOOL(node_index.flags & KAI__DG_NODE_TYPE);
	return node_index.value << 1 | is_type;
}

typedef struct {
	Kai__Dependency_Graph* graph;
    Kai_u32*       post;
    Kai_u32*       prev;
    Kai_bool*      visited;
    Kai_u32        next;
} Kai__DFS_Context;

void dfs_explore(Kai__DFS_Context* dfs, Kai__DG_Node_Index node_index)
{
	Kai_u32 index = kai__node_index_to_index(node_index);
	dfs->visited[index] = KAI_TRUE;

	Kai__Dependency_Graph* dg = dfs->graph;
	Kai__DG_Node* node = &dg->nodes.elements[node_index.value];

	Kai__DG_Dependency_Array* deps;
	if (node_index.flags & KAI__DG_NODE_TYPE) {
		deps = &node->type_dependencies;
	} else {
		deps = &node->value_dependencies;
	}

	for (Kai_u32 d = 0; d < deps->count; ++d) {
		Kai__DG_Node_Index dep = deps->elements[d];
		Kai_u32 d_index = kai__node_index_to_index(dep);

		if (!dfs->visited[d_index]) {
			dfs->prev[d_index] = index;
			dfs_explore(dfs, dep);
		}
	}

	dfs->post[index] = dfs->next++;
}

Kai_Result kai__determine_compilation_order(Kai__Dependency_Graph* Graph, Kai_Error* out_Error)
{
	(void)out_Error; // TODO: remove
	// TODO: only one allocation necessary
	Kai__DFS_Context dfs = { .graph = Graph, .next = 0 };
	dfs.post    = calloc(Graph->nodes.count * 2, sizeof(Kai_u32));
	dfs.prev    = malloc(Graph->nodes.count * 2* sizeof(Kai_u32));
	dfs.visited = calloc(Graph->nodes.count * 2, sizeof(Kai_bool));

	kai__memory_fill(dfs.prev, 0xFF, Graph->nodes.count * 2* sizeof(Kai_u32));

	// Do not waste time on builtins
	//iterate (builtin_types) {
	//	dfs.visited[i] = dfs.visited[i + context.dependency_graph.nodes.count] = KAI_TRUE;
	//}

	// Perform DFS traversal
	for_n (Graph->nodes.count) {
		Kai__DG_Node_Index node_index = {.value = (Kai_u32)i};
		Kai_u32 v = kai__node_index_to_index(node_index);
		if (!dfs.visited[v]) dfs_explore(&dfs, node_index);

		node_index.flags = KAI__DG_NODE_TYPE;
		Kai_u32 t = kai__node_index_to_index(node_index);
		if (!dfs.visited[t]) dfs_explore(&dfs, node_index);
	}

	// Get compilation order  TODO: fix this O(N^2) algorithm
	Graph->compilation_order = calloc(Graph->nodes.count * 2, sizeof(Kai_u32));
	{
		Kai_u32 next = 0;
		Kai_u32 count = 0;
		for_n (Graph->nodes.count * 2) {
			cont:
			if (dfs.post[i] == next) {
				Graph->compilation_order[count++] = (int)i;
				if (count >= Graph->nodes.count * 2)
					break;
				++next;
				i = 0;
				goto cont;
			}
		}

#if defined(DEBUG_COMPILATION_ORDER)
		char temp[256];
		Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
		kai__write("Compilation Order:\n");
		for_ (i, Graph->nodes.count * 2) {
			Kai_u32 v = Graph->compilation_order[i];
			Kai_bool is_type = v & 1;
			Kai_u32 index = v >> 1;
			if (is_type) {
				kai__write_char('T');
			} else {
				kai__write_char('V');
			}
			Kai__DG_Node* node = &Graph->nodes.elements[index];
			kai__write_format("(%.*s,%i) ", node->name.count, node->name.data, dfs.post[v]);
		}
		kai__write_char('\n');
#endif
	}

	// Check for any circular dependencies
    //for_n (context->dependency_graph.nodes.count * 2) {
	//	_write_format("prev(%i) = %i\n", (int)i, dfs.prev[i]);
	//}

	// Check for any circular dependencies
    /*for_ (u, Graph->nodes.count * 2)
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
    }*/

	free(dfs.post);
	free(dfs.prev);
	free(dfs.visited);
	return KAI_SUCCESS;
}

Kai__DG_Node* kai__dg_find_node(Kai__Dependency_Graph* graph, Kai_str name)
{
	for_n (graph->nodes.count)
	{
		if (kai_str_equals(graph->nodes.elements[i].name, name))
		{
			return &graph->nodes.elements[i];
		}
	}
	return NULL;
}

Kai__DG_Value kai__value_from_expression(Kai__Bytecode_Generation_Context* Context, Kai_Expr Expr, Kai_Type* Type);

Kai_Type kai__type_from_expression(Kai__Bytecode_Generation_Context* Context, Kai_Expr Expr) {
	void* void_Expr = Expr;
	switch (Expr->id) {
		case KAI_EXPR_IDENTIFIER: {
			Kai__DG_Node* d = kai__dg_find_node(Context->dependency_graph, Expr->source_code);
			kai__assert(d != NULL);
			return d->type;
		} break;

		case KAI_EXPR_PROCEDURE: {
			Kai_Expr_Procedure* node = void_Expr;
			Kai_Type_Info_Procedure* type_info = kai__arena_allocate(&Context->arena, sizeof(Kai_Type_Info_Procedure));
			type_info->type = KAI_TYPE_PROCEDURE;
			type_info->in_count = node->in_count;
			type_info->out_count = node->out_count;
			type_info->sub_types = kai__arena_allocate(&Context->arena, sizeof(Kai_Type) * (node->in_count + node->out_count));
			Kai_Expr current = node->in_out_expr;
			int i = 0;
			while (current != NULL)
			{
				Kai_Type type = NULL;
				Kai__DG_Value value = kai__value_from_expression(Context, current, &type);
				if (type != NULL && type->type == KAI_TYPE_TYPE)
					type_info->sub_types[i++] = value.type;
				else
					panic_with_message("Type was not a type!");
				current = current->next;
			}
			return (Kai_Type)type_info;
		}
		case KAI_EXPR_PROCEDURE_CALL: {
			Kai_Expr_Procedure_Call* node = void_Expr;
			// Find type of the procedure we are calling
			if (node->proc->id != KAI_EXPR_IDENTIFIER)
				panic_with_message("procedure calls only implemented for identifiers");

			Kai_Expr_Identifier* proc = (Kai_Expr_Identifier*)node->proc;
			Kai__DG_Node* dg_node = kai__dg_find_node(Context->dependency_graph, proc->source_code);

			if (dg_node == NULL)
				panic_with_message("could not resolve node");

			Kai_Type type = dg_node->type;
			if (type->type != KAI_TYPE_PROCEDURE)
				panic_with_message("calling something that does not have procedure type!");

			Kai_Type_Info_Procedure* proc_type = (Kai_Type_Info_Procedure*)type;

			if (proc_type->out_count == 0)
				panic_with_message("procedure does not have a return");

			return proc_type->sub_types[proc_type->in_count];
		} break;

		default: {
			panic_with_message("not handled %i", Expr->id);
		}
	}

	return NULL;
}

Kai_Result kai__bytecode_generate_type(Kai__Bytecode_Generation_Context* Context, Kai__DG_Node* dg_node)
{
	if (dg_node->type_flags & KAI__DG_NODE_EVALUATED)
		return KAI_SUCCESS;

	dg_node->type = kai__type_from_expression(Context, dg_node->expr);

	return KAI_SUCCESS;
}

/*
	fibonacci :: (n: int) -> int {
		if n <= 2 ret 1;
		ret fibonacci(n-2) + fibonacci(n-1);
	}

	fibonacci_location = stream.get_location();
	insert_if(expr, body, else_body)
		r = insert_expr(expr)
			r = allocate register
			stream.insert_compare(r, 'n', comp_gt, '2')
			return r
		stream.insert_branch(&endif_branch, r)
		insert_statement(body)
			stream.insert_return('1')
		else_location = stream.get_location();
		if (...) insert_statement(else_body);
		endif_location = stream.get_location();
		stream.set_branch(endif_branch, endif_location)
	insert_statement()
		r = insert_expr()
		stream.insert_return(r)
*/

Kai_bool kai__bytecode_find_register(Kai__Bytecode_Generation_Context* Context, Kai_str Name, Kai_Reg* out)
{
	for (int i = Context->registers.count - 1; i >= 0; i--)
	{
		Kai__Bytecode_Register br = Context->registers.elements[i];
		if (kai_str_equals(br.name, Name))
		{
			*out = br.reg;
			return KAI_TRUE;
		}
	}
	return KAI_FALSE;
}

Kai_Reg kai__bytecode_allocate_register(Kai__Bytecode_Generation_Context* Context)
{
	Kai_Allocator* allocator = &Context->dependency_graph->allocator;
	Kai_Reg reg = Context->registers.count;
	kai__array_append(&Context->registers, (Kai__Bytecode_Register){
		.reg = reg,
	});
	return reg;
}

Kai_Result kai__bytecode_emit_expression(
	Kai__Bytecode_Generation_Context* Context,
	Kai_Expr Expr,
	Kai_u32* output_register)
{
	void* void_Expr = Expr;
	switch (Expr->id) {
		case KAI_EXPR_NUMBER: {
			// TODO: Only Parse number HERE, generate error if number can not be represented by the type
			Kai_Expr_Number* node = void_Expr;
			Kai_Reg dst = kai__bytecode_allocate_register(Context);
			Kai_Value value;
			value.s32 = node->value.Whole_Part;
			kai_bc_insert_load_constant(&Context->bytecode->stream, KAI_S32, dst, value);
			*output_register = dst;
		} break;

		case KAI_EXPR_IDENTIFIER: {
			Kai_Expr_Identifier* node = void_Expr;
			kai__bytecode_find_register(Context, node->source_code, output_register);
		} break;

		case KAI_EXPR_BINARY: {
			Kai_Expr_Binary* node = void_Expr;
			Kai_Reg left, right;
			kai__bytecode_emit_expression(Context, node->left, &left);
			kai__bytecode_emit_expression(Context, node->right, &right);
			Kai_Reg dst = kai__bytecode_allocate_register(Context);
			Kai_u8 op = 0;
			switch (node->op)
			{
				case '+': op = KAI_BOP_ADD; break;
				case '-': op = KAI_BOP_SUB; break;
				case '*': op = KAI_BOP_MUL; break;
				case '/': op = KAI_BOP_DIV; break;
			}
			kai_bc_insert_math(&Context->bytecode->stream, KAI_S32, op, dst, left, right);
			*output_register = dst;
		} break;

		default: {
			char temp[64];
			Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
			kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
			kai__write_format("[emit_expression] skipping Expr(%i)\n", Expr->id);
			kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
		} break;
	}

	return KAI_SUCCESS;
}

Kai_Result kai__bytecode_emit_statement(
	Kai__Bytecode_Generation_Context* Context,
	Kai_Expr Expr)
{
	void* void_Expr = Expr;
	switch (Expr->id) {
		case KAI_STMT_COMPOUND: {
			Kai_Stmt_Compound* node = void_Expr;
			Kai_Expr current = node->head;
			while (current != NULL)
			{
				kai__bytecode_emit_statement(Context, current);
				current = current->next;
			}
		} break;

		case KAI_STMT_RETURN: {
			Kai_Stmt_Return* node = void_Expr;
			Kai_Reg result;
			kai__bytecode_emit_expression(Context, node->expr, &result);
			kai_bc_insert_return(&Context->bytecode->stream, 1, &result);
		} break;

		default: {
			char temp[64];
			Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
			kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
			kai__write_format("[emit_statement] skipping Stmt(%i)\n", Expr->id);
			kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
		} break;
	}

	return KAI_SUCCESS;
}

Kai_Result kai__bytecode_emit_procedure(
	Kai__Bytecode_Generation_Context* Context,
	Kai_Expr_Procedure* Procedure,
	Kai_u32* out_Location)
{
	Kai_Expr current = Procedure->body;
	while (current != NULL)
	{
		kai__bytecode_emit_statement(Context, current);
		current = current->next;
	}

	return KAI_SUCCESS;
}

void kai__stdout_write(void* User, Kai_str String)
{
	kai_debug_stdout_writer()->write_string(NULL, String);
}

#if 0
// TODO: Now we can give our compiler options like:
typedef struct {
	Kai_u32 Interpreter_Max_Step_Count;
	Kai_u32 Interpreter_Max_Call_Depth;
} Kai_Compile_Options;

#define KAI_DEFAULT_COMPILE_OPTIONS            \
	(Kai_Compile_Options) {                    \
		.Interpreter_Max_Call_Depth = 1024,    \
		.Interpreter_Max_Step_Count = 1000000, \
	}

void __example() {
	Kai_Compile_Options options = KAI_DEFAULT_COMPILE_OPTIONS;
	Kai_Program_Create_Info info = {0};
	Kai_Program program;
	kai_create_program(&info, &program);
}
#endif

Kai__DG_Value kai__value_from_expression(Kai__Bytecode_Generation_Context* Context, Kai_Expr Expr, Kai_Type* type)
{
	Kai_Allocator* allocator = &Context->dependency_graph->allocator;
	void* void_Expr = Expr;
	switch (Expr->id)
	{
		case KAI_EXPR_NUMBER: {
			// TODO: Only Parse number HERE, generate error if number can not be represented by the type
			Kai_Expr_Number* node = void_Expr;
			return (Kai__DG_Value) {.value = {.u64 = node->value.Whole_Part}};
		} break;

		case KAI_EXPR_PROCEDURE: {
			Kai_Expr_Procedure* procedure = void_Expr;

			// Add all input registers
			Kai_Expr current = procedure->in_out_expr;
			for (int i = 0; i < procedure->in_count; i++)
			{
				kai__array_append(&Context->registers, (Kai__Bytecode_Register) {
					.reg = i,
					.name = current->name,
				});
				current = current->next;
			}

			Kai_u32 location = 0;
			Kai_Result result = kai__bytecode_emit_procedure(Context, procedure, &location);

#ifdef DEBUG_CODE_GENERATION
			Kai_Bytecode bytecode = {
				.data = Context->bytecode->stream.data,
				.count = Context->bytecode->stream.count,
				.ret_count = 1,
				.arg_count = 2,
				.natives = NULL,
				.native_count = 0,
				.branch_hints = NULL,
				.branch_count = 0
			};
			Kai_Writer writer = {
				.write = kai__stdout_write,
				.user = NULL,
			};
			kai_bytecode_to_c(&bytecode, &writer);
			kai__stdout_write(NULL, KAI_STRING("\n"));
#endif

			if (result != KAI_SUCCESS)
				panic_with_message("something else went wrong...");

			return (Kai__DG_Value) {
				.procedure_location = location,
			};
		} break;

		case KAI_EXPR_IDENTIFIER: {
			Kai_Expr_Identifier* expr = void_Expr;
			Kai__DG_Node* other_node = kai__dg_find_node(Context->dependency_graph, expr->source_code);

			if (other_node == NULL)
			{
				panic_with_message("something went wrong");
				//return kai__error_internal(Context->error, KAI_STRING("I did not think this through..."));
			}

			if (type != NULL)
				*type = other_node->type;
			return other_node->value;
		} break;

		case KAI_EXPR_PROCEDURE_CALL: {
			// TODO: check that procedure type matches call input
			Kai_Expr_Procedure_Call* call = void_Expr;

			if (call->proc->id != KAI_EXPR_IDENTIFIER)
				panic_with_message("procedure must be identifier");

			Kai_Expr_Identifier* proc = (Kai_Expr_Identifier*)call->proc;
			Kai__DG_Node* proc_node = kai__dg_find_node(Context->dependency_graph, proc->source_code);

			// Generate procedure input
			// Note: have to put this here since interpreter must be free
			//       for recursive calls to use
			Kai_Value values[8] = {0};
			{
				Kai_Expr current = call->arg_head;
				for (Kai_u32 i = 0; i < call->arg_count; i++)
				{
					values[i] = kai__value_from_expression(Context, current, NULL).value;
					current = current->next;
				}
			}

			Kai_Interpreter* interp = &Context->bytecode->interp;
			kai_interp_load_from_stream(interp, &Context->bytecode->stream);
			kai_interp_reset(interp, proc_node->value.procedure_location);

			for (Kai_u32 i = 0; i < call->arg_count; i++)
			{
				kai_interp_set_input(interp, i, values[i]);
			}

			// TODO: don't assume only one return value
			kai_interp_push_output(interp, 0);

			int max_step_count = 65536;
			kai_interp_run(interp, max_step_count);

			if (interp->flags != KAI_INTERP_FLAGS_DONE) {
				panic_with_message("failed to interpret bytecode...");
			}

			return (Kai__DG_Value) { .value = interp->registers[0] };
		} break;

		default: {
			panic_with_message("kai__bytecode_generate_value ? %i", Expr->id);
		}
	}

	return (Kai__DG_Value) {.value = {.u64 = 0xDEADBEEFDEADBEEF}};
}

Kai_Result kai__bytecode_generate_value(Kai__Bytecode_Generation_Context* Context, Kai__DG_Node* node)
{
	if (node->value_flags & KAI__DG_NODE_EVALUATED)
		return KAI_SUCCESS;

	node->value = kai__value_from_expression(Context, node->expr, NULL);
	return KAI_SUCCESS;
}

Kai_Result kai__generate_bytecode(Kai__Bytecode_Create_Info* Info, Kai__Bytecode* out_Bytecode)
{
	Kai_Result result = KAI_SUCCESS;
	Kai__Dependency_Graph* graph = Info->dependency_graph;
	Kai_Allocator* allocator = &graph->allocator;
	int* order = graph->compilation_order;

	Kai__Bytecode_Generation_Context context = {
		.error = Info->error,
		.dependency_graph = Info->dependency_graph,
		.bytecode = out_Bytecode,
	};

	Kai_u32 _size = 0;
	// Initialization
	{
		Kai_Interpreter_Setup interp_info = {
			.max_register_count     = 4096,
			.max_call_stack_count   = 1024,
			.max_return_value_count = 1024,
		};
		_size = kai_interp_required_memory_size(&interp_info);
		interp_info.memory = kai__allocate(NULL, _size, 0);
		kai_interp_create(&context.bytecode->interp, &interp_info);
		context.bytecode->stream.allocator = allocator;
	}

	kai__dynamic_arena_allocator_create(&context.arena, &Info->dependency_graph->allocator);

	for (Kai_u32 i = 0; i < graph->nodes.count * 2; ++i)
	{
		int index = order[i];
		Kai__DG_Node_Index node_index = {
			.flags = (index & 1 ? KAI__DG_NODE_TYPE : 0),
			.value = index >> 1,
		};

		Kai__DG_Node* node = &graph->nodes.elements[node_index.value];
		
        #if 1
		if (node_index.flags & KAI__DG_NODE_TYPE)
        {
			if (node->type_flags & KAI__DG_NODE_EVALUATED)
				continue;
        }
        else
        {
			if (node->value_flags & KAI__DG_NODE_EVALUATED)
				continue;
        }
        #endif

#ifdef DEBUG_CODE_GENERATION
		kai_debug_stdout_writer()->set_color(0, KAI_DEBUG_COLOR_IMPORTANT_2);
		printf("Generating %i -> %.*s\n", i, node->name.count, node->name.data);
		kai_debug_stdout_writer()->set_color(0, KAI_DEBUG_COLOR_PRIMARY);
#endif

		if (node_index.flags & KAI__DG_NODE_TYPE)
		{
			result = kai__bytecode_generate_type(&context, node);

#ifdef DEBUG_CODE_GENERATION
			kai_debug_stdout_writer()->write_c_string(0, "--> Type ");
			kai_debug_write_type(kai_debug_stdout_writer(), node->type);
			kai_debug_stdout_writer()->write_char(0, '\n');
#endif
		}
		else {
			result = kai__bytecode_generate_value(&context, node);

#ifdef DEBUG_CODE_GENERATION
			// TODO: write a function that does EXACTLY THIS!
			switch (node->type->type)
			{
				case KAI_TYPE_TYPE: {
					kai_debug_stdout_writer()->write_c_string(0, "--> Value ");
					kai_debug_write_type(kai_debug_stdout_writer(), node->value.type);
					kai_debug_stdout_writer()->write_char(0, '\n');
				} break;
				case KAI_TYPE_INTEGER: {
					printf("--> Value %lli\n", node->value.value.s64);
				} break;
				case KAI_TYPE_FLOAT: {
					if (((Kai_Type_Info_Float*)node->type)->bits == 32)
						printf("--> Value %f\n", node->value.value.f32);
					else
						printf("--> Value %f\n", node->value.value.f64);
				} break;
				case KAI_TYPE_PROCEDURE: {
					printf("--> Procedure %u\n", node->value.procedure_location);
				} break;
				default: {
					panic_with_message("Excuse me, what type IS THIS?! %i", node->type->type);
				}
			}
#endif
		}

		if (result != KAI_SUCCESS)
			return result;
	}

	kai__array_destroy(&context.registers);
	kai__free(context.bytecode->interp.registers, _size);
	kai__dynamic_arena_allocator_destroy(&context.arena);
	return result;
}

extern inline uint32_t kai__arm64_add(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b0001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_sub(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b1001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_subs(uint32_t imm12, uint32_t Rn, uint8_t sf) { return (sf << 31) | (0b11100010 << 23) | (imm12 << 10) | (Rn << 5) | 0b11111; }
extern inline uint32_t kai__arm64_movz(uint32_t Rd, uint32_t imm16, uint8_t sf) { return (sf << 31) | (0b10100101 << 23) | (imm16 << 5) | Rd; }
extern inline uint32_t kai__arm64_bl(int32_t imm26) { return (0b100101 << 26) | (imm26 & 0x3FFFFFF); }
extern inline uint32_t kai__arm64_ret() { return 0xd65f03c0; }

Kai_Result kai__create_program(Kai__Program_Create_Info* Info, Kai_Program* out_Program)
{
	Kai_Allocator* allocator = &Info->allocator;

	//uint32_t code [] = {
	//	kai__arm64_sub(0, 1, 2, 1),
	//	kai__arm64_add(2, 1, 0, 1),
	//	kai__arm64_subs(69, 5, 0),
	//	kai__arm64_movz(4, 42069, 1),
	//	kai__arm64_bl(-16),
	//	kai__arm64_ret(),
	//};
//
	//for (int i = 0; i < sizeof(code)/sizeof(uint32_t); ++i)
	//{
	//	union {
	//		uint8_t byte[4];
	//		uint32_t u32;
	//	} temp = { .u32 = code[i] }, out;
	//	out.byte[0] = temp.byte[3];
	//	out.byte[1] = temp.byte[2];
	//	out.byte[2] = temp.byte[1];
	//	out.byte[3] = temp.byte[0];
	//	printf("%x\n", out.u32);
	//}

#define kai__bytecode_decode_add(Position, Dst_Var, Src1_Var, Src2_Var) \
    uint32_t Dst_Var = *(uint32_t*)(stream->data + Position);           \
    Position += sizeof(uint32_t);                                       \
    uint32_t Src1_Var = *(uint32_t*)(stream->data + Position);          \
    Position += sizeof(uint32_t);                                       \
    uint32_t Src2_Var = *(uint32_t*)(stream->data + Position);          \
    Position += sizeof(uint32_t);

	//KAI__ARRAY(uint32_t) arm64_machine_code = {0};

	// Generate machine code
	//Bc_Stream* stream = &Info->bytecode->stream;
	//
	//for (Kai_u32 i = 0; i < stream->count; ++i) \
	//	switch (stream->data[i])
	//	{
	//		case BC_OP_ADD: {
	//			kai__bytecode_decode_add(i, dst, src1, src2);
	//			uint32_t instr = kai__arm64_add(dst, src1, src2, 0);
	//			kai__array_append(&arm64_machine_code, instr);
	//		} break;
	//	}

	// Allocate memory to store machine code
	Kai_Program program = {0};
	program.allocator = *allocator;
	program.platform_machine_code = allocator->allocate(allocator->user, allocator->page_size, KAI_MEMORY_ACCESS_WRITE);
	program.code_size = allocator->page_size;

	//kai__memory_copy(
	//	program.platform_machine_code,
	//	arm64_machine_code.elements,
	//	arm64_machine_code.count * sizeof(uint32_t)
	//);

	uint32_t arm_instructions[] = {
		kai__arm64_movz(0, 0xA, 0),
		kai__arm64_ret(),
	};

	//kai__memory_copy(program.platform_machine_code,
    //                     "\xB8\x45\x00\x00\x00\xC3", 7);
	kai__memory_copy(program.platform_machine_code,
                         arm_instructions, sizeof(arm_instructions));

	// Set memory as executable
	allocator->set_access(allocator->user, program.platform_machine_code, program.code_size, KAI_MEMORY_ACCESS_EXECUTE);

	kai__hash_table_emplace(program.procedure_table, KAI_STRING("main"), program.platform_machine_code);

	*out_Program = program;

	Info->error->result = KAI_ERROR_FATAL;
	Info->error->message = KAI_STRING("todo");
	return KAI_ERROR_FATAL;
}

void kai__destroy_dependency_graph(Kai__Dependency_Graph* Graph)
{
	Kai_Allocator* allocator = &Graph->allocator;
	for (Kai_u32 i = 0; i < Graph->nodes.count; ++i) {
		Kai__DG_Node* node = &Graph->nodes.elements[i];
		kai__array_destroy(&node->value_dependencies);
		kai__array_destroy(&node->type_dependencies);
	}
	kai__array_destroy(&Graph->nodes);
	for (Kai_u32 i = 0; i < Graph->scopes.count; ++i) {
		Kai__DG_Scope* scope = &Graph->scopes.elements[i];
		kai__hash_table_destroy(scope->identifiers);
	}
	kai__array_destroy(&Graph->scopes);
}

void kai__destroy_bytecode(Kai__Bytecode* Bytecode)
{
	Kai_Allocator* allocator = Bytecode->stream.allocator;
	kai__free(Bytecode->stream.data, Bytecode->stream.capacity);
}

void kai_destroy_program(Kai_Program Program)
{
	Kai_Allocator* allocator = &Program.allocator;
	kai__hash_table_destroy(Program.procedure_table);
	if (Program.code_size != 0)
		Program.allocator.free(
			Program.allocator.user,
			Program.platform_machine_code,
			Program.code_size
		);
}

void* kai_find_procedure(Kai_Program Program, Kai_str Name, Kai_Type Type)
{
	(void)Type;
	return *(void**)kai__hash_table_find(Program.procedure_table, Name);
}

#if 0

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

#define scopes       context->dependency_graph.scopes
#define dependencies context->dependency_graph.dependencies
#define nodes        context->dependency_graph.nodes

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

#endif
#endif
