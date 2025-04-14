#define KAI_USE_DEBUG_API
#include "compiler.h"
#include "builtin_types.h"
#include "bytecode.h"
#include <stdlib.h>
#include "program.h"

#define DEBUG_DEPENDENCY_GRAPH

Kai_Result
kai_create_program(Kai_Program_Create_Info* info, Kai_Program* program) {
	*info->error = (Kai_Error) {
		.message = KAI_STRING("kai_create_program not implemented"),
		.result = KAI_ERROR_INTERNAL,
	};
	return KAI_ERROR_INTERNAL;
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

    //kai_debug_write_syntax_tree(kai_debug_stdout_writer(), &syntax_tree);
	
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
		};
		result = kai__generate_bytecode(&info, &bytecode);
	}
	kai__check(result);

	{
		Kai__Program_Create_Info info = {
			.bytecode = &bytecode,
			.error = out_Error,
		};
		result = kai__create_program(&info, out_Program);
	}
	kai__check(result);

cleanup:
	kai_destroy_syntax_tree(&syntax_tree);
	kai__destroy_dependency_graph(&dependency_graph);
	kai__destroy_bytecode(&bytecode);
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

Kai_u8 delete_me [128];

Kai_Result kai__error_redefinition(
	Kai_Error* out_Error,
	Kai_str string,
	Kai_u32 line_number,
	Kai_str original_name,
	Kai_u32 original_line_number)
{
	*out_Error = (Kai_Error) {
		.result = KAI_ERROR_SEMANTIC,
		.location = {
			.line = line_number,
			.string = string,
		},
		.message = {
			.data = delete_me,
		},
	};

	// TODO: if identifier is too long, then truncate 
    str_insert_string(out_Error->message, "indentifier \"");
    str_insert_str   (out_Error->message, string);
    str_insert_string(out_Error->message, "\" is already declared");

    u8* end = delete_me + out_Error->message.count;
    Kai_Error* info = (Kai_Error*)end;
    end += sizeof(Kai_Error);
    out_Error->next = info;

	*info = (Kai_Error) {
		.result = KAI_ERROR_INFO,
		.location = {
			.line = original_line_number,
			.string = original_name,
		},
		.message = {
			.data = end,
		}
	};

    str_insert_string(info->message, "see original definition");

	return KAI_ERROR_SEMANTIC;
}

Kai_Result kai__error_not_declared(
	Kai_Error* out_Error,
	Kai_str string,
	Kai_u32 line_number)
{
    *out_Error = (Kai_Error) {
		.result = KAI_ERROR_SEMANTIC,
		.location = {
			.string = string,
			.line = line_number,
		},
		.message = {
			.data = delete_me,
		},
	};
    str_insert_string(out_Error->message, "indentifier \"");
    str_insert_str   (out_Error->message, string);
    str_insert_string(out_Error->message, "\" not declared");
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

		u32 new_scope_index = Graph->scopes.count;
		kai__array_append(&Graph->scopes, (Kai__DG_Scope) {
			.parent = Scope_Index,
			.is_proc_scope = KAI_TRUE,
		});
		kai__hash_table_create(&Graph->scopes.elements[new_scope_index].identifiers);

		node->_scope = new_scope_index;
		return kai__dg_create_nodes_from_statement(Graph, node->body, new_scope_index, KAI_TRUE, KAI_TRUE);
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
			kai__hash_table_insert(scope->identifiers, node->name, index);
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
		u32 new_scope_index = Scope_Index;

		if (!From_Procedure) {
			new_scope_index = Graph->scopes.count;
			kai__array_append(&Graph->scopes, (Kai__DG_Scope) {
				.parent = Scope_Index,
				.is_proc_scope = KAI_TRUE,
			});
			kai__hash_table_create(&Graph->scopes.elements[new_scope_index].identifiers);
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

Kai__DG_Node_Index* kai__dg_resolve_dependency_node(
	Kai__Dependency_Graph* Graph,
	Kai_str                Name,
	u32                    Scope_Index,
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

void kai__remove_local_variables(Kai__DG_Scope* Scope) {
	for (Kai_u32 i = 0; i < Scope->identifiers.capacity; ++i) {
		Kai_u64 hash = Scope->identifiers.elements[i].hash;
		if (!(hash & KAI__HASH_TABLE_OCCUPIED_BIT))
			continue;
		Kai__DG_Node_Index node_index = Scope->identifiers.elements[i].value;
		if (!(node_index.flags & KAI__DG_NODE_LOCAL_VARIABLE))
			continue;
		Scope->identifiers.elements[i].hash = 0; // remove item
	}
}

Kai_Result kai__dg_insert_value_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    u32                       Scope_Index,
    Kai_Expr                  Expr,
    Kai_bool                  In_Procedure)
{
	Kai_Allocator* allocator = &Graph->allocator;
	Kai_Result result = KAI_SUCCESS;
	void* base = Expr;
    if (Expr == NULL) { panic_with_message("null expression\n"); return 0; }

    switch (Expr->id)
    {
    default: {
		panic_with_message("undefined expr [insert_value_dependencies] (id = %d)\n", Expr->id);
	} break;

    case KAI_EXPR_IDENTIFIER: {
		Kai__DG_Node_Index* node_index = kai__dg_resolve_dependency_node(
			Graph,
			Expr->source_code,
			Scope_Index,
			In_Procedure
		);

		if (node_index == NULL) {
			return kai__error_not_declared(Graph->error, Expr->source_code, Expr->line_number);
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
		panic_with_message("procedure type\n");
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
					current->name,
					node->line_number,
					original->name,
					original->line_number
				);
			}

			kai__hash_table_insert(local_scope->identifiers, current->name, (Kai__DG_Node_Index) {
				.flags = KAI__DG_NODE_LOCAL_VARIABLE,
			});
        }

		result = kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			node->_scope,
			node->body,
			KAI_TRUE
		);

		kai__remove_local_variables(local_scope);
    } break;

    //break; case KAI_STMT_DECLARATION: {
    //    if (In_Procedure) {
    //        Kai_Stmt_Declaration* node = base;
    //        // Already has a dependency node
    //        if (node->flags & KAI_DECL_FLAG_CONST)
	//			return KAI_SUCCESS;
    //        Scope* scope = scopes.data + scope_index;
    //        // Insert into local scope (if not already defined)
    //        u32 it = scope_find(scope, decl->name);
    //        if (it != NONE && scope->identifiers.data[it].index != LOCAL_VARIABLE_INDEX)
    //            return c_error_redefinition(decl->name, decl->line_number, nodes.data + it);
	//		Named_Node_Ref local = {.name = decl->name, .index = LOCAL_VARIABLE_INDEX};
	//	    array_reserve(scope->identifiers, scope->identifiers.count + 1);
	//		array_push_back(scope->identifiers, local);
    //        // Look into it's definition to find possible dependencies
    //        return c_insert_value_dependencies(deps, scope_index, decl->expr, KAI_TRUE);
    //    }
    //    else panic_with_message("invalid declaration\n");
    //}

    //break; case KAI_STMT_ASSIGNMENT: {
    //    if (In_Procedure) {
    //        Kai_Stmt_Assignment* assignment = base;
    //        return c_insert_value_dependencies(deps, scope_index, assignment->expr, KAI_TRUE);
    //    }
    //    else panic_with_message("invalid assignment\n");
    //}

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

    //break; case KAI_STMT_FOR: {
    //    Kai_Stmt_For* node = base;
    //    if (c_insert_value_dependencies(deps, scope_index, node->body, in_procedure))
    //        return KAI_TRUE;
    //}

    break;
    }

	return KAI_SUCCESS;
}

Kai_Result kai__dg_insert_type_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    u32                       Scope_Index,
    Kai_Expr                  Expr)
{
	Kai_Allocator* allocator = &Graph->allocator;
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
            return kai__error_not_declared(Graph->error, Expr->source_code, Expr->line_number);

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
        panic_with_message("procedure type\n");
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
	kai__hash_table_create(&global->identifiers);

	// Insert builtin types
	for (int i = 0; i < count_of(kai__builtin_types); ++i) {
		kai__array_append(&out_Graph->nodes, (Kai__DG_Node) {
			.type        = &kai__type_info_type,
			.value       = { .type = kai__builtin_types[i].type },
			.value_flags = KAI__DG_NODE_EVALUATED,
			.type_flags  = KAI__DG_NODE_EVALUATED,
			.name        = kai__builtin_types[i].name,
			.scope_index = KAI__SCOPE_GLOBAL_INDEX,
		});
		kai__hash_table_insert(global->identifiers,
			kai__builtin_types[i].name,
			(Kai__DG_Node_Index) {
				.value = i
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
	for (int i = 0; i < out_Graph->nodes.count; ++i) {
		Kai__DG_Node* node = out_Graph->nodes.elements+i;

		printf("Node %2d \"", i);
		kai_debug_stdout_writer()->write_string(NULL, node->name);
		putchar('"');
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
			for (Kai_u32 i = 0; i < node->value_dependencies.count; ++i) {
				Kai__DG_Node_Index node_index = node->value_dependencies.elements[i];
				if (node_index.flags & KAI__DG_NODE_TYPE) {
					printf("T(%d) ", node_index.value);
				}
				else {
					printf("V(%d) ", node_index.value);
				}
			}
			putchar('\n');
		}

		if (node->type_dependencies.count) {
			printf("    T deps: ");
			for (Kai_u32 i = 0; i < node->type_dependencies.count; ++i) {
				Kai__DG_Node_Index node_index = node->type_dependencies.elements[i];
				if (node_index.flags & KAI__DG_NODE_TYPE) {
					printf("T(%d) ", node_index.value);
				}
				else {
					printf("V(%d) ", node_index.value);
				}
			}
			putchar('\n');
		}
	}
#endif // DEBUG_DEPENDENCY_GRAPH

	return result;
}

Kai_Result kai__determine_compilation_order(Kai__Dependency_Graph* Graph, Kai_Error* out_Error)
{
	*out_Error = (Kai_Error) {
		.message = KAI_STRING("kai__determine_compilation_order not implemented"),
		.result = KAI_ERROR_INTERNAL,
	};
	return KAI_ERROR_INTERNAL;
}

Kai_Result kai__generate_bytecode(Kai__Bytecode_Create_Info* Info, Kai__Bytecode* out_Bytecode)
{
	*Info->error = (Kai_Error) {
		.message = KAI_STRING("kai__generate_bytecode not implemented"),
		.result = KAI_ERROR_INTERNAL,
	};
	return KAI_ERROR_INTERNAL;
}

Kai_Result kai__create_program(Kai__Program_Create_Info* Info, Kai_Program* out_Program)
{
	*Info->error = (Kai_Error) {
		.message = KAI_STRING("kai__create_program not implemented"),
		.result = KAI_ERROR_INTERNAL,
	};
	return KAI_ERROR_INTERNAL;
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
		kai__hash_table_destroy(&scope->identifiers);
	}
	kai__array_destroy(&Graph->scopes);
}

void kai__destroy_bytecode(Kai__Bytecode* Bytecode)
{

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

#define c_error_redefinition(NAME, LINE, NODE) error_redefinition(context, NAME, LINE, NODE)

Kai_bool error_redefinition(
	Compiler_Context* context,
	Kai_str string,
	Kai_u32 line_number,
	Node* original
) {
    return KAI_TRUE;
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
	return KAI_FALSE;
}

u32 scope_find(Scope* scope, Kai_str name) {
	return NONE;
}

Kai_bool insert_node_for_declaration(
	Compiler_Context* context,
	Kai_Stmt_Declaration* decl,
	Kai_bool in_proc,
	u32 scope_index
) {
	return KAI_FALSE;
}

#define LOCAL_VARIABLE_INDEX ((u32)-1)

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
