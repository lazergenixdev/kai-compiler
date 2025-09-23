#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "3rd-party/nob.h"
#define sb_append nob_sb_append_cstr
#define ASSERT NOB_ASSERT

#define STB_DS_IMPLEMENTATION
#include "3rd-party/stb_ds.h"

#define tab(N) for (int i = 0; i < N; ++i) sb_append(builder, "    ")

#define KAI_IMPLEMENTATION
#ifdef USE_GENERATED
#include "kai.h"
#else
#include "3rd-party/kai.h"
#endif

#if defined(KAI_COMPILER_CLANG) || defined(KAI_COMPILER_GNU)
#pragma GCC diagnostic ignored "-Wmultichar" // ? this is a feature, why warning??
#endif

#define exit_on_fail(E) if (!(E)) exit(1)
#define len(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))
#define forn(LEN) for (int i = 0; i < (LEN); ++i)
#define MACRO_STRING(S) #S
#define MACRO_EXPSTR(S) MACRO_STRING(S)
#define UNIQUE2(N,L) N ## L
#define UNIQUE(N,L) UNIQUE2(N,L)
#define SCOPED_TEMP() for (size_t UNIQUE(__i,__LINE__) = 0, mark = nob_temp_save(); UNIQUE(__i,__LINE__) < 1; ++UNIQUE(__i,__LINE__), nob_temp_rewind(mark))

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0
#define VERSION_EXTRA "alpha"

typedef struct {
	const char* name;
	const char* args;
	const char* value;
} Macro;

#define KAI_MULTI(A,B,C,D) ((Kai_u32)(A) | (Kai_u32)(B << 8) | (Kai_u32)((C+0) << 16) | (Kai_u32)((D+0) << 24))

Macro macros[] = {
	{"VERSION_MAJOR", "", MACRO_EXPSTR(VERSION_MAJOR)},
	{"VERSION_MINOR", "", MACRO_EXPSTR(VERSION_MINOR)},
	{"VERSION_PATCH", "", MACRO_EXPSTR(VERSION_PATCH)},
	{"VERSION_STRING", "", "\"" MACRO_EXPSTR(VERSION_MAJOR) "." MACRO_EXPSTR(VERSION_MINOR) "." MACRO_EXPSTR(VERSION_PATCH) VERSION_EXTRA "\""},
	{"$", "", ""},
	{"BOOL", "(EXPR)", "((Kai_bool)((EXPR) ? KAI_TRUE : KAI_FALSE))"},
	{"STRING", "(LITERAL)", "KAI_STRUCT(Kai_string){.count = (Kai_u32)(sizeof(LITERAL)-1), .data = (Kai_u8*)(LITERAL)}"},
	{"CONST_STRING", "(LITERAL)", "{.count = (Kai_u32)(sizeof(LITERAL)-1), .data = (Kai_u8*)(LITERAL)}"},
	{"SLICE", "(TYPE)", "struct { Kai_u32 count; TYPE* data; }"},
	{"DYNAMIC_ARRAY", "(T)", "struct { Kai_u32 count; Kai_u32 capacity; T* data; }"},
	{"HASH_TABLE", "(T)", "struct { Kai_u32 count; Kai_u32 capacity; Kai_u64* occupied; Kai_u64* hashes; Kai_string* keys; T* values; }"},
	{"LINKED_LIST", "(T)", "struct { T *head, *last; }"},
	{"FILL", "", "0x800"},
};

Macro function_macros[] = {
	{"number_is_integer", "(N)", "((N).n == 0 || ((N).d == 1 && (N).e >= 0))"},
	{"_write_u32", "(Value)", "writer->write_value(writer->user, KAI_U32, (Kai_Value){.u32 = Value}, (Kai_Write_Format){0})"},
	{"_write_s32", "(Value)", "writer->write_value(writer->user, KAI_S32, (Kai_Value){.s32 = Value}, (Kai_Write_Format){0})"},
	{"_write_u64", "(Value)", "writer->write_value(writer->user, KAI_U64, (Kai_Value){.u64 = Value}, (Kai_Write_Format){0})"},
	{"_write_s64", "(Value)", "writer->write_value(writer->user, KAI_S64, (Kai_Value){.s64 = Value}, (Kai_Write_Format){0})"},
	{"_write_f64", "(Value)", "writer->write_value(writer->user, KAI_F64, (Kai_Value){.f64 = Value}, (Kai_Write_Format){0})"},
	{"_write", "(C_String)",  "writer->write_string(writer->user, KAI_STRING(C_String))"},
	{"_write_string", "(...)", "writer->write_string(writer->user, __VA_ARGS__)"},
	{"_set_color", "(Color)",  "if (writer->set_color != NULL) writer->set_color(writer->user, Color)"},
	{"_write_fill", "(Char,Count)", "writer->write_value(writer->user, KAI_FILL, (Kai_Value){0}, (Kai_Write_Format){.min_count = Count, .fill_character = (Kai_u8)Char})"},
	{"_next_character_equals", "(C)", "( (context->cursor+1) < context->source.count && C == context->source.data[context->cursor+1] )"},
	{"_allocate", "(Old,New_Size,Old_Size)", "allocator->heap_allocate(allocator->user, Old, New_Size, Old_Size)"},
	{"_free", "(Ptr,Size)", "allocator->heap_allocate(allocator->user, Ptr, 0, Size)"},
	{"array_destroy", "(ARRAY)", "(allocator->heap_allocate(allocator->user, (ARRAY)->data, 0, (ARRAY)->capacity * sizeof((ARRAY)->data[0])), (ARRAY)->count = 0, (ARRAY)->capacity = 0, (ARRAY)->data = NULL)"},
	{"array_reserve", "(ARRAY,NEW_CAPACITY)", "kai_raw_array_reserve((Kai_Raw_Dynamic_Array*)(ARRAY), NEW_CAPACITY, allocator, sizeof((ARRAY)->data[0]))"},
	{"array_resize", "(ARRAY,NEW_SIZE)", "kai_raw_array_resize((Kai_Raw_Dynamic_Array*)(ARRAY), NEW_SIZE, allocator, sizeof((ARRAY)->data[0]))"},
	{"array_grow", "(ARRAY,COUNT)", "kai_raw_array_grow((Kai_Raw_Dynamic_Array*)(ARRAY), COUNT, allocator, sizeof((ARRAY)->data[0]))"},
	{"array_push", "(ARRAY,...)", "(kai_raw_array_grow((Kai_Raw_Dynamic_Array*)(ARRAY), 1, allocator, sizeof((ARRAY)->data[0])), (ARRAY)->data[(ARRAY)->count++] = (__VA_ARGS__))"},
	{"array_pop", "(ARRAY)", "(ARRAY)->data[--(ARRAY)->count]"},
	{"array_last", "(ARRAY)", "(ARRAY)->data[(ARRAY)->count - 1]"},
	{"array_insert", "(ARRAY)", "TODO"},
	{"array_insert_n", "(ARRAY)", "TODO"},
	{"array_remove", "(ARRAY)", "TODO"},
	{"array_remove_n", "(ARRAY)", "TODO"},
	{"array_remove_swap", "(ARRAY)", "TODO"},
	{"table_set", "(T,KEY,...)", "do{ Kai_u32 _; kai_raw_hash_table_emplace_key((Kai_Raw_Hash_Table*)(T), KEY, &_, allocator, sizeof (T)->values[0]); (T)->values[_] = (__VA_ARGS__); }while(0)"},
	{"table_find", "(T,KEY)", "kai_raw_hash_table_find((Kai_Raw_Hash_Table*)(T), KEY)"},
};

Macro internal_macros[] = {
	{"_explore", "(Expr,IS_LAST)", "(kai__tree_traversal_push(context, IS_LAST),kai__write_tree(context, Expr),kai__tree_traversal_pop(context))"},
	{"_linked_list_append", "(L,P)", "((L).head == NULL? ((L).head = (P)):((L).last->next = (P)), (L).last = (P))"},
	{"_unexpected", "(Where, Context)", "kai__error_unexpected(parser, &parser->tokenizer.current_token, KAI_STRING(Where), KAI_STRING(Context))"},
	{"_expect", "(EXPR, Where, Context)", "if (!(EXPR)) return kai__unexpected(Where, Context)"},
	{"_next_token", "()", "kai_tokenizer_next(&parser->tokenizer)"},
	{"_peek_token", "()", "kai_tokenizer_peek(&parser->tokenizer)"},
	{"_dump_memory", "(ARRAY)", "do { for (Kai_u32 i = 0; i < (ARRAY).count * sizeof((ARRAY).data[0]); ++i) printf(\"%02x \", ((Kai_u8*)((ARRAY).data))[i]); printf(\"\\n\"); } while (0)"}, // TODO: delete this
};

char** g_internal_macros = NULL;

const char* intrinsics[] = {
	"clz32", "ctz32", "clz64", "ctz64",
	"u128_low", "u128_high", "u128_multiply"
};

Kai_Allocator g_allocator = {0};
Kai_Writer g_writer = {0};

typedef struct {
    Kai_string *items;
    size_t count;
    size_t capacity;
} String_Array;

typedef enum {
	Identifier_Type_Invalid,
	Identifier_Type_Macro,    // KAI_
	Identifier_Type_Type,     // Kai_
	Identifier_Type_Struct,   // Kai_
	Identifier_Type_Function, // kai_
} Identifier_Type;

typedef struct {
	char* key;
	Identifier_Type value;
} Identifier_Map;

typedef struct {
	char* key;
	char* value;
} Function_Decl_Map;

typedef struct {
	char* key;
	Kai_Expr_Struct* value;
} Struct_Map;

typedef struct {
	char* key;
	bool value;
} Bool_Map;

Identifier_Map* g_identifier_map = NULL;
Function_Decl_Map* g_function_cache = NULL;
char** g_internal_functions = NULL;
Struct_Map* g_struct_map = NULL;
Bool_Map* g_generated_slice_types = NULL;
Bool_Map* g_generated_dynarray_types = NULL;
Bool_Map* g_generated_hashtable_types = NULL;

typedef struct {
	Kai_Expr* expr;
} Variable_Type;

typedef struct {
	char* key;
	Variable_Type value;
} Variable_Map;

typedef struct {
	Variable_Map* variable_map;
} Scope;

Scope* g_scopes = NULL;
Kai_Expr* g_current_decl_type = NULL;
Kai_Syntax_Tree* g_current_tree = NULL;

const char* prim_types[] = {
	"u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "f32", "f64",
};

const char** g_imports = NULL;

Kai_u8 temp_cstr_buffer[1024];

const char* temp_cstr_from_string(Kai_string s)
{
	ASSERT(s.count < sizeof(temp_cstr_buffer));
	memcpy(temp_cstr_buffer, s.data, s.count);
	temp_cstr_buffer[s.count] = 0;
	return (const char*)temp_cstr_buffer;
}

const char* format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf((char*)temp_cstr_buffer, sizeof(temp_cstr_buffer), format, args);
    va_end(args);
    return (const char*)temp_cstr_buffer;
}

const char* temp_cstr_upper(Kai_string s)
{
	ASSERT(s.count < sizeof(temp_cstr_buffer));
	for (Kai_u32 i = 0; i < s.count; ++i)
	{
		temp_cstr_buffer[i] = ('a' <= s.data[i] && s.data[i] <= 'z')? s.data[i] - ('a' - 'A') : s.data[i];
	}
	temp_cstr_buffer[s.count] = 0;
	return (const char*)temp_cstr_buffer;
}

const char* clone_string(Kai_string s)
{
	char* new = malloc(s.count+1);
	memcpy(new, s.data, s.count);
	new[s.count] = 0;
	return new;
}

const char* executable(const char* name)
{
#if _WIN32
	return temp_sprintf("%s.exe", name);
#else
	return name;
#endif
}

Variable_Type lookup(const char* var_name)
{
	for (int i = arrlen(g_scopes) - 1; i >= 0; --i)
	{
		Scope scope = g_scopes[i];
		int k = shgeti(scope.variable_map, var_name);
		if (k == -1) continue;
		return scope.variable_map[k].value;
	}
	return (Variable_Type){0};
}

void define(const char* name, Variable_Type type)
{
	Scope scope = arrlast(g_scopes);
	shput(scope.variable_map, name, type);
	arrlast(g_scopes) = scope;
}

const char* expr_string(Kai_Expr* expr)
{
	if (expr == NULL) return "NULL";
	switch (expr->id)
	{
    case KAI_EXPR_IDENTIFIER: return "IDENTIFIER";
    case KAI_EXPR_STRING: return "STRING";
    case KAI_EXPR_NUMBER: return "NUMBER";
    case KAI_EXPR_UNARY: return "UNARY";
    case KAI_EXPR_BINARY: return "BINARY";
    case KAI_EXPR_PROCEDURE_TYPE: return "PROCEDURE_TYPE";
    case KAI_EXPR_PROCEDURE_CALL: return "PROCEDURE_CALL";
    case KAI_EXPR_PROCEDURE: return "PROCEDURE";
    case KAI_EXPR_CODE: return "CODE";
    case KAI_EXPR_STRUCT: return "STRUCT";
    case KAI_EXPR_ENUM: return "ENUM";
    case KAI_EXPR_ARRAY: return "ARRAY";
    case KAI_STMT_RETURN: return "RETURN";
    case KAI_STMT_DECLARATION: return "DECLARATION";
    case KAI_STMT_ASSIGNMENT: return "ASSIGNMENT";
    case KAI_STMT_IF: return "IF";
    case KAI_STMT_WHILE: return "WHILE";
    case KAI_STMT_FOR: return "FOR";
    case KAI_STMT_CONTROL: return "CONTROL";
    case KAI_STMT_COMPOUND: return "COMPOUND";
	default: return "UNKNOWN"; 
	}
}

Kai_Expr* resolve_type(Kai_Expr* type)
{
	//printf("resolving %s {%s}\n", expr_string(type), temp_cstr_from_string(type->source_code));
	switch (type->id)
	{
	default: break;

	case KAI_EXPR_IDENTIFIER: {
		const char* name = temp_cstr_from_string(type->source_code);
		Kai_Expr_Struct* str = shget(g_struct_map, name);
		return (void*)str;
	} break;

	case KAI_EXPR_UNARY: {
		Kai_Expr_Unary* una = (void*)type;
		return resolve_type(una->expr);
	}

	case KAI_EXPR_STRUCT: {
		return type;
	}
	}

	return NULL;
}

Kai_Stmt_Declaration* find_struct_member(Kai_Expr_Struct* str, Kai_string name)
{
	for (Kai_Stmt* field = str->head; field != NULL; field = field->next)
	{
		if (field->flags & KAI_FLAG_EXPR_USING)
		{
			ASSERT(field->id == KAI_EXPR_IDENTIFIER);
			Kai_Expr_Struct* expr = (void*)resolve_type(field);
			ASSERT(expr->id == KAI_EXPR_STRUCT);
			Kai_Stmt_Declaration* decl = find_struct_member(expr, name);
			if (decl != NULL) return decl;
		}
		else
		{
			ASSERT(field->id == KAI_STMT_DECLARATION);
			Kai_Stmt_Declaration* decl = (void*)field;
			if (kai_string_equals(field->name, name))
				return decl;
		}
	}
	return NULL;
}

Variable_Type type_of_expression(Kai_Expr* expr)
{
	switch (expr->id)
	{
	default: break;

	case KAI_EXPR_IDENTIFIER: {
		const char* name = temp_cstr_from_string(expr->source_code);
		//printf("name(%s)\n", name);
		return lookup(name);
	} break;
	
	case KAI_EXPR_BINARY: {
		Kai_Expr_Binary* bin = (void*)expr;
		if (bin->op != '.')
			return type_of_expression(bin->op == KAI_MULTI('-','>',,) ? bin->right : bin->left);
		
		ASSERT(bin->right != NULL && bin->right->id == KAI_EXPR_IDENTIFIER);
		//printf("looking for %s", temp_cstr_from_string(bin->right->source_code));
		//printf(" in %s\n", temp_cstr_from_string(bin->left->source_code));
		Kai_Expr* left_type = type_of_expression(bin->left).expr;
		if (left_type==NULL) break;

		left_type = resolve_type(left_type);

		// Find member
		ASSERT(left_type->id == KAI_EXPR_STRUCT);
		Kai_Expr_Struct* str = (void*)left_type;
		Kai_Stmt_Declaration* decl = find_struct_member(str, bin->right->source_code);
		if (decl != NULL)
			return (Variable_Type){.expr = decl->type};
	} break;
	}
	return (Variable_Type){0};
}

bool is_pointer(Kai_Expr* expr)
{
	if (expr == NULL) return false;
	switch (expr->id)
	{
	default: break;

	case KAI_EXPR_UNARY: {
		Kai_Expr_Unary* una = (void*)expr;
		return una->op == '*';
	} break;
	}
	return false;
}

bool is_type_expression(Kai_Expr* expr)
{
	if (expr == NULL) return false;
	switch (expr->id)
	{
	default: 				  return false;
	case KAI_EXPR_IDENTIFIER: return true;
	case KAI_EXPR_UNARY:      return true;
	}
}

void generate_all_macros(String_Builder* builder)
{
	{
		time_t current_time = time(NULL);
		struct tm *local_time_info = gmtime(&current_time);
		char buffer[80]; // Buffer to store the formatted string
		strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", local_time_info);
		sb_appendf(builder, "#define KAI_BUILD_DATE %s // YMD HMS (UTC)\n", buffer);
	}
	for (int i = 0; i < len(macros); ++i)
	{
		if (macros[i].name[0] == '$')
		{
			read_entire_file("src/macros.h", builder);
			sb_append(builder, "\n");
			continue;
		}
		if (strcmp(macros[i].name, "VERSION_STRING") == 0)
		{
			if (strlen(VERSION_EXTRA) == 0)
				macros[i].value = "\"" MACRO_EXPSTR(VERSION_MAJOR) "." MACRO_EXPSTR(VERSION_MINOR) "." MACRO_EXPSTR(VERSION_PATCH) "\"";
			else
				macros[i].value = "\"" MACRO_EXPSTR(VERSION_MAJOR) "." MACRO_EXPSTR(VERSION_MINOR) "." MACRO_EXPSTR(VERSION_PATCH) " " VERSION_EXTRA "\"";
		}
		shput(g_identifier_map, macros[i].name, Identifier_Type_Macro);
		sb_appendf(builder, "#define KAI_%s%s %s\n", macros[i].name, macros[i].args, macros[i].value);
	}
	sb_append_cstr(builder, "\n");
	
	for (int i = 0; i < len(function_macros); ++i)
	{
		shput(g_identifier_map, function_macros[i].name, Identifier_Type_Function);
		sb_appendf(builder, "#define kai_%s%s %s\n", function_macros[i].name, function_macros[i].args, function_macros[i].value);
	}
	sb_append_cstr(builder, "\n");

	sb_append_cstr(builder,
		"#ifndef KAI_API\n"
		"#define KAI_API(RETURN) extern RETURN\n"
		"#endif\n\n"
	);
	
	sb_append_cstr(builder, "#define KAI_INTERNAL inline static\n\n");

	String_Builder internal = {0};
	for (int i = 0; i < len(internal_macros); ++i)
	{
		shput(g_identifier_map, internal_macros[i].name, Identifier_Type_Function);
		sb_appendf(&internal, "#define kai_%s%s %s\n", internal_macros[i].name, internal_macros[i].args, internal_macros[i].value);
	}
	sb_append_null(&internal);
	arrpush(g_internal_macros, internal.items);
}

#define STRUCT_TEMPLATE "typedef struct { %s } Kai_%s;\n"

void generate_all_primitive_types(String_Builder* builder)
{
	for (int i = 8; i <= 64; i *= 2)
	{
		sb_appendf(builder, "typedef%suint%i_t Kai_u%i;\n", i==8?"  ":" ", i, i);
		shput(g_identifier_map, format("u%i", i), Identifier_Type_Type);
		sb_appendf(builder, "typedef %sint%i_t Kai_s%i;\n", i==8?"  ":" ", i, i);
		shput(g_identifier_map, format("s%i", i), Identifier_Type_Type);
	}
	sb_append_cstr(builder, "typedef    float Kai_f32;\n");
	sb_append_cstr(builder, "typedef   double Kai_f64;\n");
	shput(g_identifier_map, "f32", Identifier_Type_Type);
	shput(g_identifier_map, "f64", Identifier_Type_Type);

	for (int len = 2; len <= 4; ++len)
	{
		for (int i = 0; i < len(prim_types); ++i)
		{
			const char* t = prim_types[i];
			sb_appendf(builder, "typedef struct { Kai_%s%sx, y", t, i<2?"  ":" ");
			if (len >= 3) sb_append_cstr(builder, ", z");
			if (len >= 4) sb_append_cstr(builder, ", w");
			sb_appendf(builder, "; } Kai_vector%i_%s;\n", len, t);
			shput(g_identifier_map, format("vector%i_%s", len, t), Identifier_Type_Struct);
		}
	}
	sb_append_cstr(builder, "\n");
	
	sb_append_cstr(builder, "typedef Kai_u8 Kai_bool;\n");
	shput(g_identifier_map, "bool", Identifier_Type_Type);
	sb_append_cstr(builder, "enum {\n    KAI_FALSE = 0,\n    KAI_TRUE = 1,\n};\n");

	sb_append_cstr(builder, "typedef  intptr_t Kai_int;\n");
	shput(g_identifier_map, "int", Identifier_Type_Type);
	sb_append_cstr(builder, "typedef uintptr_t Kai_uint;\n");
	shput(g_identifier_map, "uint", Identifier_Type_Type);

	// String
	sb_appendf(builder, STRUCT_TEMPLATE, "Kai_u32 count; Kai_u8* data;", "string");
	shput(g_identifier_map, "string", Identifier_Type_Struct);
	shput(g_struct_map, "string", NULL);
	
	// String
	sb_appendf(builder, "typedef const char* Kai_%s;\n", "cstring");
	shput(g_identifier_map, "cstring", Identifier_Type_Type);
	
	sb_append_cstr(builder, "\n");
}

void write_decl(String_Builder* builder, Kai_Stmt_Declaration* decl)
{
	printf("%.*s;\n", decl->name.count, decl->name.data);
	sb_appendf(builder, "KAI_API(void) kai_%.*s();\n", decl->name.count, decl->name.data);
}

const char* map_binary_operator(Kai_u32 op)
{
    switch (op)
    {
    case '<':  return "<";
    case '>':  return ">";
    case '+':  return "+";
    case '-':  return "-";
    case '*':  return "*";
    case '/':  return "/";
    case '%':  return "%";
    case '.':  return ".";
	case '&': return "&";
	case '^': return "^";
	case '|': return "|";
    case KAI_MULTI('&', '&',,): return "&&";
    case KAI_MULTI('|', '|',,): return "||";
    case KAI_MULTI('=', '=',,): return "==";
    case KAI_MULTI('!', '=',,): return "!=";
    case KAI_MULTI('<', '=',,): return "<=";
    case KAI_MULTI('>', '=',,): return ">=";
    case KAI_MULTI('<', '<',,): return "<<";
    case KAI_MULTI('>', '>',,): return ">>";
    case KAI_MULTI('-', '>',,): return NULL; // special case
    case '[': return NULL; // special case
    default:   return "BINOP";
    }
}

#define TOP_PRECEDENCE 100
int binary_operator_precedence(Kai_u32 op)
{
    switch (op)
    {
		case '.':  return 1;
		case '[':  return 1;
		case '(':  return 1;
		case '!': return 2;
		case KAI_MULTI('-', '>',,): return 2;
		case '*':  return 3;
		case '/':  return 3;
		case '%':  return 3;
		case '+':  return 4;
		case '-':  return 4;
		case KAI_MULTI('<', '<',,): return 5;
		case KAI_MULTI('>', '>',,): return 5;
		case KAI_MULTI('<', '=',,): return 6;
		case KAI_MULTI('>', '=',,): return 6;
		case '<':  return 6;
		case '>':  return 6;
		case KAI_MULTI('!', '=',,): return 7;
		case KAI_MULTI('=', '=',,): return 10; // (was 7) silence warning
		case '&': return 10; // (was 9) silence warning
		case '^': return 10; // (was 9) silence warning
		case '|': return 10;
		case KAI_MULTI('&', '&',,): return 11;
		case KAI_MULTI('|', '|',,): return 11; // (was 12) silence warning
		default: return TOP_PRECEDENCE;
    }
}

enum {
	NONE      = 0x00,
	NO_RENAME = 0x01,
	NOT_TYPE  = 0x02,
	KEEP_ALL_PARENTHESIS = 0x04,
};
Identifier_Type generate_expression(String_Builder* builder, Kai_Expr* expr, int prec, int flags);

void generate_struct_literal(String_Builder* builder, Kai_Expr* expr)
{
	ASSERT(expr->id == KAI_EXPR_LITERAL);
	Kai_Expr_Literal* lit = (void*)expr;

	sb_append(builder, "{");
	for (Kai_Expr* m = lit->head; m != NULL; m = m->next)
	{
		if (m->id == KAI_STMT_ASSIGNMENT)
		{
			Kai_Stmt_Assignment* ass = (void*)m;
			sb_append(builder, ".");
			generate_expression(builder, ass->left, TOP_PRECEDENCE, NO_RENAME);
			sb_append(builder, " = ");
			generate_expression(builder, ass->expr, TOP_PRECEDENCE, NOT_TYPE);
		}
		else
		{
			generate_expression(builder, m, TOP_PRECEDENCE, NONE);
		}
		if (m->next != NULL) sb_append(builder, ", ");
	}
	sb_append(builder, "}");
}

bool try_generate_builtin_array_procedure(String_Builder* builder, Kai_Expr_Procedure_Call* call)
{
	ASSERT(call->proc->id == KAI_EXPR_IDENTIFIER);

	// array_reserve(*pp, 100);
    // => _array_reserve(*pp -> *Raw_Dynamic_Array, 100, allocator, sizeof(*pp.data[0]))

	if (kai_string_equals(call->proc->source_code, KAI_STRING("array_reserve")))
	{
		Kai_Expr* arg0 = call->arg_head; ASSERT(arg0 != NULL);
		Kai_Expr* arg1 = arg0->next;     ASSERT(arg1 != NULL);

		sb_append(builder, "kai__array_reserve((Kai_Raw_Dynamic_Array*)(");
		generate_expression(builder, arg0, TOP_PRECEDENCE, NOT_TYPE);
		sb_append(builder, "), ");
		generate_expression(builder, arg1, TOP_PRECEDENCE, NOT_TYPE);
		sb_append(builder, ", allocator, sizeof(");

	}
	return false;
}

Identifier_Type generate_expression(String_Builder* builder, Kai_Expr* expr, int prec, int flags)
{
	ASSERT(expr != NULL);
	switch (expr->id)
	{
	case KAI_EXPR_IDENTIFIER: {
		if (kai_string_equals(expr->source_code, KAI_STRING("true")))
		{
			sb_append(builder, "KAI_TRUE");
			return Identifier_Type_Invalid;
		}
		else if (kai_string_equals(expr->source_code, KAI_STRING("false")))
		{
			sb_append(builder, "KAI_FALSE");
			return Identifier_Type_Invalid;
		}
		else if (kai_string_equals(expr->source_code, KAI_STRING("null")))
		{
			sb_append(builder, "NULL");
			return Identifier_Type_Invalid;
		}
		const char* name = temp_cstr_from_string(expr->source_code);
		Identifier_Type type = Identifier_Type_Invalid;
		if (!(flags & NO_RENAME))
		{
			type = shget(g_identifier_map, name);
			switch (type)
			{
			default: break;
			case Identifier_Type_Macro:    { sb_append(builder, "KAI_"); } break;
			case Identifier_Type_Type:     { sb_append(builder, "Kai_"); } break;
			case Identifier_Type_Struct:   { sb_append(builder, "Kai_"); } break;
			case Identifier_Type_Function: { sb_append(builder, "kai_"); } break;
			}
		}
		sb_appendf(builder, "%s", name);
		return type;
	} break;

	case KAI_EXPR_NUMBER: {
		Kai_Expr_Number* num = (void*)expr;
		
		if (kai_number_is_integer(num->value))
			sb_appendf(builder, "%llu", kai_number_to_u64(num->value));
		else
			sb_appendf(builder, "%g", kai_number_to_f64(num->value));
	} break;
	
	case KAI_EXPR_STRING: {
		Kai_Expr_String* str = (void*)expr;
		sb_appendf(builder, "%s", temp_cstr_from_string(str->source_code));
	} break;

	case KAI_EXPR_ARRAY: {
		Kai_Expr_Array* arr = (void*)expr;
		if (arr->rows != NULL)
		{
			ASSERT(arr->rows->id == KAI_EXPR_IDENTIFIER);
			generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
			sb_append(builder, "_HashTable");
			return Identifier_Type_Struct;
		}
		else
		{
			if (arr->flags & KAI_FLAG_ARRAY_DYNAMIC)
			{
				generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
				sb_append(builder, "_DynArray");
				return Identifier_Type_Struct;
			}
			else
			{
				generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
				sb_append(builder, "_Slice");
				return Identifier_Type_Struct;
			}
		}
	} break;
	
	case KAI_EXPR_UNARY: {
		Kai_Expr_Unary* una = (void*)expr;

		int new_prec = binary_operator_precedence(una->op);
		bool parenthesis = flags & KEEP_ALL_PARENTHESIS || una->op == '~' || una->op == KAI_MULTI('-','>',,);

		if (una->op == KAI_MULTI('-','>',,)) {
			da_append(builder, '(');
			generate_expression(builder, g_current_decl_type, TOP_PRECEDENCE, NONE);
			da_append(builder, ')');
		}
		else if (una->op != '*' || (flags & NOT_TYPE)) {
            if (una->op == '[') {
                da_append(builder, '*'); // dereference
            }
			else
				da_append(builder, una->op == '*'? '&' : una->op);
		}
		if (parenthesis) da_append(builder, '(');
		generate_expression(builder, una->expr, new_prec, flags);
		if (parenthesis) da_append(builder, ')');
		if (una->op == '*' && !(flags & NOT_TYPE)) da_append(builder, una->op);
	} break;
	
	case KAI_EXPR_BINARY: {
		Kai_Expr_Binary* bin = (void*)expr;
		int op = bin->op;
		const char* op_str = map_binary_operator(op);
		int new_flags = flags;
		if (op == '.')
		{
			new_flags |= NO_RENAME;
			if (shgeti(g_struct_map, temp_cstr_from_string(bin->left->source_code)) != -1)
			{
				op = '$';
			}
			else if (is_pointer(type_of_expression(bin->left).expr))
			{
				op_str = "->";
			}
		}
		int new_prec = binary_operator_precedence(op);
		if (new_prec >= prec || (flags & KEEP_ALL_PARENTHESIS)) da_append(builder, '(');
		if (op_str == NULL)
		{
			switch (op)
			{
			case KAI_MULTI('-', '>',,): {
				da_append(builder, '(');
				generate_expression(builder, bin->right, TOP_PRECEDENCE, NONE);
				da_append(builder, ')');
				da_append(builder, '(');
				generate_expression(builder, bin->left, TOP_PRECEDENCE, flags);
				da_append(builder, ')');
			} break;

			case '[': {
				int new_prec = binary_operator_precedence(op);
				generate_expression(builder, bin->left, new_prec, new_flags);
				
				if (bin->left->id == KAI_EXPR_IDENTIFIER)
				{
					const char* name = temp_cstr_from_string(bin->left->source_code);
					Variable_Type type = lookup(name);
					//printf("%s => %p\n", name, type.expr);
					if (type.expr != NULL && type.expr->id == KAI_EXPR_ARRAY)
					{
						Kai_Expr_Array* arr = (void*)(type.expr);
						if (arr->rows == 0)
						{
							sb_append(builder, ".data");
						}
					}
				}

				da_append(builder, '[');
				generate_expression(builder, bin->right, TOP_PRECEDENCE, NONE);
				da_append(builder, ']');
			} break;

			default: {
				ASSERT(op_str != NULL);
			}
			}
		}
		else if (op == '$')
		{
			sb_append(builder, "(");
			generate_expression(builder, bin->left, TOP_PRECEDENCE, NONE);
			sb_append(builder, ")");
			generate_struct_literal(builder, bin->right);
		}
		else
		{
			generate_expression(builder, bin->left, new_prec == 5 ? 1:new_prec, flags);
			sb_append(builder, op_str);
			generate_expression(builder, bin->right, new_prec == 5 ? 1:new_prec, new_flags);
		}
		if (new_prec >= prec || (flags & KEEP_ALL_PARENTHESIS)) da_append(builder, ')');
	} break;
	
	case KAI_EXPR_PROCEDURE_CALL: {
		Kai_Expr_Procedure_Call* call = (void*)expr;

		bool copy_source = false;
		int flags = NOT_TYPE;
		if (call->proc->id == KAI_EXPR_IDENTIFIER
		&& kai_string_equals(call->proc->source_code, KAI_STRING("C")))
		{
			copy_source = true;
		}
		if (call->proc->id == KAI_EXPR_IDENTIFIER
		&& (kai_string_equals(call->proc->source_code, KAI_STRING("sizeof"))
		||  kai_string_equals(call->proc->source_code, KAI_STRING("LINKED_LIST"))))
		{
			flags &= (~NOT_TYPE);
		}
		//if (call->proc->id == KAI_EXPR_IDENTIFIER)
		//{
		//	if (try_generate_builtin_array_procedure(builder, call))
		//		break;
		//}
		if (!copy_source)
		{	
			generate_expression(builder, call->proc, TOP_PRECEDENCE, NONE);
			sb_append(builder, "(");
		}
		for (Kai_Stmt* arg = call->arg_head; arg != NULL; arg = arg->next)
		{
			if (copy_source)
			{
				ASSERT(arg->id == KAI_EXPR_STRING);
				Kai_Expr_String* str = (void*)arg;
				sb_append_buf(builder, str->value.data, str->value.count);
			}
			else
			{
				generate_expression(builder, arg, TOP_PRECEDENCE, flags);
			}
			if (arg->next != NULL && !copy_source)
				sb_append(builder, ", ");
		}
		if (!copy_source) sb_append(builder, ")");
	} break;
	
	case KAI_EXPR_STRUCT: {
		Kai_Expr_Struct* str = (void*)expr;

		if (str->flags & KAI_FLAG_STRUCT_UNION)
			sb_append(builder, "union { " );
		else
			sb_append(builder, "struct { " );
		for (Kai_Stmt* field = str->head; field != NULL; field = field->next)
		{
			if (field->id == KAI_STMT_DECLARATION)
			{
				Kai_Stmt_Declaration* decl = (void*)field;
				ASSERT(decl->type != NULL);
				generate_expression(builder, decl->type, TOP_PRECEDENCE, NONE);
				sb_appendf(builder, " %s; ", temp_cstr_from_string(decl->name));
			}
		}
		sb_append(builder, "}");
		return Identifier_Type_Struct;
	} break;

	default:
		break;
	}
	return Identifier_Type_Invalid;
}

void generate_all_struct_typedefs(String_Builder* builder, Kai_Stmt_Compound* stmt)
{
	Kai_Stmt* current = stmt->head;
	int count = 0;
	for (Kai_Stmt* current = stmt->head; current != NULL; current = current->next)
	{
		if (current->id != KAI_STMT_DECLARATION) continue;
		Kai_Stmt_Declaration* decl = (void*)current;

		if (decl->expr == NULL)
			continue;

		switch (decl->expr->id)
		{
			case KAI_EXPR_STRUCT: {
				const char* t = temp_cstr_from_string(decl->name);
				sb_appendf(builder, "typedef %s Kai_%s Kai_%s;\n",
					decl->expr->flags & KAI_FLAG_STRUCT_UNION? "union":"struct", t, t);
				shput(g_identifier_map, t, Identifier_Type_Struct);
				shput(g_struct_map, t, (void*)decl->expr);
				count += 1;
			} break;
			
			case KAI_EXPR_ENUM: {
				Kai_Expr_Enum* _enum = (void*)decl->expr;
				sb_append(builder, "typedef ");
				generate_expression(builder, _enum->type, TOP_PRECEDENCE, NONE);
				const char* t = temp_cstr_from_string(decl->name);
				sb_appendf(builder, " Kai_%s;\n", t);
				shput(g_identifier_map, t, Identifier_Type_Type);
				count += 1;
			} break;

			default: break;
		}
	}
	if (count != 0)
		sb_append_cstr(builder, "\n");
}

void generate_all_non_struct_typedefs(String_Builder* builder, Kai_Stmt_Compound* stmt)
{
	Kai_Stmt* current = stmt->head;
	int count = 0;
	for (Kai_Stmt* current = stmt->head; current != NULL; current = current->next)
	{
		if (current->id != KAI_STMT_DECLARATION) continue;
		Kai_Stmt_Declaration* decl = (void*)current;

		if (decl->expr == NULL)
			continue;

		switch (decl->expr->id)
		{
			case KAI_EXPR_STRUCT: break;
			case KAI_EXPR_ENUM: break;
			case KAI_EXPR_PROCEDURE: break;
			case KAI_EXPR_ARRAY:
			case KAI_EXPR_PROCEDURE_CALL: {
				sb_append(builder, "typedef ");
				generate_expression(builder, decl->expr, TOP_PRECEDENCE, KEEP_ALL_PARENTHESIS);
				const char* name = temp_cstr_from_string(decl->name);
				sb_appendf(builder, " Kai_%s;\n", name);
				shput(g_identifier_map, name, Identifier_Type_Struct);
			} break;

			case KAI_EXPR_PROCEDURE_TYPE: {
				Kai_Expr_Procedure_Type* proc = (void*)decl->expr;
				sb_append(builder, "typedef ");
				
				String_Builder def = {0};
				Kai_Expr* in_out = proc->in_out_expr;

				String_Builder temp = {0};
				forn (proc->in_count) {
					generate_expression(&temp, in_out, TOP_PRECEDENCE, NONE);
					sb_appendf(&temp, " %.*s", (int)in_out->name.count, in_out->name.data);
					if (i != (Kai_u32)(proc->in_count - 1))
						sb_append(&temp, ", ");
					in_out = in_out->next;
				}

				forn (proc->out_count) {
					generate_expression(&def, in_out, TOP_PRECEDENCE, NONE);
					in_out = in_out->next;
				}
				if (proc->out_count == 0) {
					sb_append(&def, "void");
				}

				const char* t = temp_cstr_from_string(decl->name);
				sb_appendf(&def, " Kai_%s(", t);
				if (temp.count) sb_append_buf(&def, temp.items, temp.count);
				else            sb_append(&def, "void");
				sb_append(&def, ")");

				sb_free(temp);
				sb_appendf(builder, "%.*s;\n", (int)def.count, def.items);
				shput(g_identifier_map, t, Identifier_Type_Type);
				count += 1;
			} break;

			default: {
				if (decl->expr->id == KAI_EXPR_UNARY)
				{
					Kai_Expr_Unary* una = (void*)decl->expr;
					if (una->op == '.')
						break;
				}

				count += 1;
				if (is_type_expression(decl->expr))
				{
					sb_append(builder, "typedef ");
					generate_expression(builder, decl->expr, TOP_PRECEDENCE, NONE);
					const char* t = temp_cstr_from_string(decl->name);
					sb_appendf(builder, " Kai_%s;\n", t);
					shput(g_identifier_map, t, Identifier_Type_Type);
				}
				else
				{
					const char* name = temp_cstr_from_string(decl->name);
					bool internal = name[0] == '_';
					String_Builder def = {0};
					sb_appendf(&def, "#define KAI_%s ", name);
					shput(g_identifier_map, name, Identifier_Type_Macro);
					generate_expression(&def, decl->expr, TOP_PRECEDENCE, KEEP_ALL_PARENTHESIS);
					sb_append(&def, "\n");

					if (!internal)
						sb_append_buf(builder, def.items, def.count);
					else {
						sb_append_null(&def);
						arrpush(g_internal_macros, def.items);
					}
					break;
				}
			}
		}
	}
	if (count != 0)
		sb_append_cstr(builder, "\n");
}

void generate_all_internal_macros(String_Builder* builder)
{
	for (int i = 0; i < arrlen(g_internal_macros); ++i)
		sb_append(builder, g_internal_macros[i]);
	sb_append(builder, "\n");
}

void generate_all_data_structure_types(String_Builder* builder, Kai_Stmt* stmt)
{
	switch (stmt->id)
	{
	default: break;
	case KAI_EXPR_IDENTIFIER: break;
	case KAI_EXPR_NUMBER: break;

	case KAI_EXPR_ARRAY: {
		Kai_Expr_Array* arr = (void*)stmt;
		ASSERT(arr->expr->id == KAI_EXPR_IDENTIFIER);
		Kai_string name = arr->expr->source_code;

		const char* t = temp_cstr_from_string(name);
		if (arr->rows == NULL)
		{
			if (arr->flags & KAI_FLAG_ARRAY_DYNAMIC) {
				if (!shget(g_generated_dynarray_types, t)) {
					sb_append(builder, "typedef KAI_DYNAMIC_ARRAY(");
					generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
					sb_appendf(builder, ") Kai_%s_DynArray;\n", t);
					shput(g_identifier_map, t, Identifier_Type_Struct);
					shput(g_generated_dynarray_types, t, true);
				}
			}
			else {
				if (!shget(g_generated_slice_types, t)) {
					sb_append(builder, "typedef KAI_SLICE(");
					generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
					sb_appendf(builder, ") Kai_%s_Slice;\n", t);
					shput(g_identifier_map, t, Identifier_Type_Struct);
					shput(g_generated_slice_types, t, true);
				}
			}
		}
		else if (arr->rows->id == KAI_EXPR_IDENTIFIER)
		{
			if (!shget(g_generated_hashtable_types, t)) {
				sb_append(builder, "typedef KAI_HASH_TABLE(");
				generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
				sb_appendf(builder, ") Kai_%s_HashTable;\n", t);
				shput(g_identifier_map, t, Identifier_Type_Struct);
				shput(g_generated_hashtable_types, t, true);
			}
		}
	} break;
	
	case KAI_EXPR_UNARY: {
		Kai_Expr_Unary* una = (void*)stmt;
		generate_all_data_structure_types(builder, una->expr);
	} break;
	
	case KAI_EXPR_BINARY: {
		Kai_Expr_Binary* bin = (void*)stmt;
		generate_all_data_structure_types(builder, bin->left);
		generate_all_data_structure_types(builder, bin->right);
	} break;
	
	case KAI_EXPR_PROCEDURE_CALL: {
		Kai_Expr_Procedure_Call* call = (void*)stmt;
		for (Kai_Stmt* arg = call->arg_head; arg != NULL; arg = arg->next)
		{
			generate_all_data_structure_types(builder, arg);
		}
	} break;
	
	case KAI_EXPR_STRUCT: {
		Kai_Expr_Struct* str = (void*)stmt;
		for (Kai_Stmt* field = str->head; field != NULL; field = field->next)
		{
			generate_all_data_structure_types(builder, field);
		}
	} break;

	case KAI_STMT_COMPOUND: {
		Kai_Stmt_Compound* comp = (void*)stmt;
		for (Kai_Stmt* current = comp->head; current != NULL; current = current->next)
		{
			generate_all_data_structure_types(builder, current);
		}
	} break;
	
	case KAI_STMT_DECLARATION: {
		Kai_Stmt_Declaration* decl = (void*)stmt;
		if (decl->type != NULL)
			generate_all_data_structure_types(builder, decl->type);
		if (decl->expr != NULL)
			generate_all_data_structure_types(builder, decl->expr);
	} break;
	
	case KAI_STMT_ASSIGNMENT: {
		Kai_Stmt_Assignment* ass = (void*)stmt;
		generate_all_data_structure_types(builder, ass->left);
		generate_all_data_structure_types(builder, ass->expr);
	} break;
	
	case KAI_STMT_RETURN: {
		Kai_Stmt_Return* ret = (void*)stmt;
		if (ret->expr != NULL)
			generate_all_data_structure_types(builder, ret->expr);
	} break;
	
	case KAI_STMT_IF: {
		Kai_Stmt_If* _if = (void*)stmt;
		generate_all_data_structure_types(builder, _if->expr);
		if (_if->then_body != NULL)
			generate_all_data_structure_types(builder, _if->then_body);
		if (_if->else_body != NULL)
			generate_all_data_structure_types(builder, _if->else_body);
	} break;
	
	case KAI_STMT_WHILE: {
		Kai_Stmt_While* whi = (void*)stmt;
		generate_all_data_structure_types(builder, whi->expr);
		if (whi->body != NULL)
			generate_all_data_structure_types(builder, whi->body);
	} break;
	
	case KAI_EXPR_PROCEDURE: {
		Kai_Expr_Procedure* pro = (void*)stmt;
		//generate_all_data_structure_types(builder, pro->in_out_expr);
		generate_all_data_structure_types(builder, pro->body);
	} break;
	}
}

void generate_all_struct_members(String_Builder* builder, Kai_Expr_Struct* _struct)
{
	if (_struct == NULL) return;
	for (Kai_Stmt* field = _struct->head; field != NULL; field = field->next)
	{
		if (field->id == KAI_STMT_DECLARATION)
		{
			Kai_Stmt_Declaration* decl = (void*)field;
			
			ASSERT(decl->type != NULL);
			tab(1);
			Kai_Expr_Array* arr = (void*)decl->type;
			if (decl->type->id == KAI_EXPR_ARRAY && arr->rows != NULL)
			{
				if (arr->rows->id == KAI_EXPR_IDENTIFIER)
					generate_expression(builder, decl->type, TOP_PRECEDENCE, NONE);
				else generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
			}
			else generate_expression(builder, decl->type, TOP_PRECEDENCE, NONE);
			
			if (decl->type->id == KAI_EXPR_ARRAY && arr->rows != NULL && arr->rows->id == KAI_EXPR_NUMBER) {
				Kai_Expr_Number* num = (void*)arr->rows;
				sb_appendf(builder, " %s[%llu];\n", temp_cstr_from_string(decl->name), kai_number_to_u64(num->value));
			}
			else sb_appendf(builder, " %s;\n", temp_cstr_from_string(decl->name));
		}
		else if (field->flags == KAI_FLAG_EXPR_USING)
		{
			ASSERT(field->id == KAI_EXPR_IDENTIFIER);
			Kai_Expr_Struct* _struct = shget(g_struct_map, temp_cstr_from_string(field->source_code));
			generate_all_struct_members(builder, _struct);
		}
	}
}

Kai_bool enum_excludes_name(Kai_string name)
{
	if (kai_string_equals(name, KAI_STRING("Expr_Id"))) return true;
	if (kai_string_equals(name, KAI_STRING("Expr_Flags"))) return true;
	if (kai_string_equals(name, KAI_STRING("Special_Kind"))) return true;
	if (kai_string_equals(name, KAI_STRING("Control_Kind"))) return true;
	if (kai_string_equals(name, KAI_STRING("Compile_Flags"))) return true;
	if (kai_string_equals(name, KAI_STRING("Node_Flags"))) return true;
	if (kai_string_equals(name, KAI_STRING("Token_Id"))) return true;
	if (kai_string_equals(name, KAI_STRING("Result"))) return true;
	if (kai_string_equals(name, KAI_STRING("Primitive_Type"))) return true;
	return false;
}

void generate_all_struct_definitions(String_Builder* builder, Kai_Stmt_Compound* stmt)
{
	Kai_Stmt* current = stmt->head;
	for (Kai_Stmt* current = stmt->head; current != NULL; current = current->next)
	{
		if (current->id != KAI_STMT_DECLARATION) continue;
		Kai_Stmt_Declaration* decl = (void*)current;

		if (decl->expr == NULL)
			continue;

		if (decl->expr->id == KAI_EXPR_STRUCT)
		{
			Kai_Expr_Struct* str = (void*)decl->expr;
			
			sb_appendf(builder, "%s Kai_%.*s {\n", str->flags & KAI_FLAG_STRUCT_UNION? "union":"struct",
				decl->name.count, decl->name.data);
			generate_all_struct_members(builder, str);
			sb_append(builder, "};\n\n");
		}
		else if (decl->expr->id == KAI_EXPR_ENUM)
		{
			Kai_Expr_Enum* _enum = (void*)decl->expr;
			
			const char* name = temp_cstr_from_string(decl->name);
			sb_appendf(builder, "// Type: Kai_%s\n", name);
			sb_append(builder, "enum {\n");
			for (Kai_Stmt* field = _enum->head; field != NULL; field = field->next)
			{
				ASSERT(field->id == KAI_STMT_ASSIGNMENT);
				Kai_Stmt_Assignment* ass = (void*)field;
				
				tab(1);
				if (enum_excludes_name(decl->name))
					sb_append(builder, "KAI_");
				else
					sb_appendf(builder, "KAI_%s_", temp_cstr_upper(decl->name));
				generate_expression(builder, ass->left, TOP_PRECEDENCE, NONE);
				ASSERT(ass->op == '=');
				sb_append(builder, " = ");
				generate_expression(builder, ass->expr, TOP_PRECEDENCE, NONE);
				sb_append(builder, ",\n");
			}
			sb_append(builder, "};\n\n");
		}
	}
}

void generate_all_function_definitions(String_Builder* builder, Kai_Stmt_Compound* stmt, int insert_newline_after)
{
	Kai_Stmt* current = stmt->head;
	int count = 0;
	for (Kai_Stmt* current = stmt->head; current != NULL; current = current->next)
	{
		if (current->id != KAI_STMT_DECLARATION) continue;
		Kai_Stmt_Declaration* decl = (void*)current;
		
		if (decl->expr == NULL || decl->expr->id != KAI_EXPR_PROCEDURE)
			continue;
			
		Kai_Expr_Procedure* proc = (void*)decl->expr;

		const char* t = temp_cstr_from_string(decl->name);
		shput(g_identifier_map, t, Identifier_Type_Function);

		bool is_internal = t[0] == '_';

		String_Builder def = {0};
		if (is_internal) sb_append(&def, "KAI_INTERNAL ");
		else sb_append(&def, "KAI_API(");
		
		Kai_Expr* in_out = proc->in_out_expr;

		String_Builder temp = {0};
		forn (proc->in_count) {
			generate_expression(&temp, in_out, TOP_PRECEDENCE, NONE);
			sb_appendf(&temp, " %.*s", (int)in_out->name.count, in_out->name.data);
			if (i != (Kai_u32)(proc->in_count - 1))
				sb_append(&temp, ", ");
			in_out = in_out->next;
		}

		forn (proc->out_count) {
			generate_expression(&def, in_out, TOP_PRECEDENCE, NONE);
			in_out = in_out->next;
		}
		if (proc->out_count == 0) {
			sb_append(&def, "void");
		}

		t = temp_cstr_from_string(decl->name);
		sb_appendf(&def, "%skai_%s(", is_internal? " ":") ", t);
		if (temp.count) sb_append_buf(&def, temp.items, temp.count);
		else            sb_append(&def, "void");
		sb_append(&def, ")");

		sb_free(temp);

		if (!is_internal)
		{
			sb_appendf(builder, "%.*s;\n", (int)def.count, def.items);
			count += 1;
		}
		
		sb_append_null(&def);
		shput(g_function_cache, t, def.items);
		if (is_internal)
			arrpush(g_internal_functions, def.items);
	}
	if (count != 0 && insert_newline_after)
		sb_append_cstr(builder, "\n");
}

bool g_go_through_case = false;

void generate_statement(String_Builder* builder, Kai_Stmt* stmt, int depth, int increase_depth_on_non_compound)
{
	if (increase_depth_on_non_compound && stmt->id != KAI_STMT_COMPOUND)
		depth += 1;
	switch (stmt->id)
	{
	case KAI_STMT_COMPOUND: {
		Kai_Stmt_Compound* comp = (void*)stmt;
		tab(depth); sb_append(builder, "{\n");
		for (Kai_Stmt* current = comp->head; current != NULL; current = current->next)
		{
			generate_statement(builder, current, depth+1, false);
		}
		tab(depth); sb_append(builder, "}\n");
	} break;
	
	case KAI_STMT_CONTROL: {
		Kai_Stmt_Control* con = (void*)stmt;
		if (con->kind != KAI_CONTROL_THROUGH) tab(depth);
		switch (con->kind)
		{
			case KAI_CONTROL_CASE: {
				if (!g_go_through_case) sb_append(builder, "break; ");
				if (con->expr == NULL)
					sb_append(builder, "default:\n");
				else {
					sb_append(builder, "case ");
					generate_expression(builder, con->expr, TOP_PRECEDENCE, NONE);
					sb_append(builder, ":\n");
				}
				g_go_through_case = false;
			} break;
			case KAI_CONTROL_BREAK:    sb_append(builder, "break;\n"); break;
			case KAI_CONTROL_CONTINUE: sb_append(builder, "continue;\n"); break;
			case KAI_CONTROL_DEFER:    sb_append(builder, "defer;\n"); break;
			case KAI_CONTROL_THROUGH:  g_go_through_case = true; break;
		}

	} break;
	
	case KAI_STMT_DECLARATION: {
		Kai_Stmt_Declaration* decl = (void*)stmt;
		Identifier_Type type = Identifier_Type_Invalid;
		tab(depth);
		bool is_array = false;
		Kai_Expr_Array* arr = (void*)decl->type;
		if (decl->type->id == KAI_EXPR_ARRAY)
		{
			is_array = arr->rows != NULL && arr->rows->id == KAI_EXPR_NUMBER;
			if (!is_array)
			{
				type = generate_expression(builder, decl->type, TOP_PRECEDENCE, NONE);
			}
			else
			{
				generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
			}
		}
		else
		{
			type = generate_expression(builder, decl->type, TOP_PRECEDENCE, NONE);
		}
		const char* name = temp_cstr_from_string(decl->name);
		sb_appendf(builder, " %s", name);
		if (is_array) {
			ASSERT(arr->rows->id == KAI_EXPR_NUMBER);
			Kai_Expr_Number* num = (void*)arr->rows;
			sb_appendf(builder, "[%llu]", kai_number_to_u64(num->value));
		}
		sb_append(builder, " = ");
		define(name, (Variable_Type){.expr = decl->type});
		if (decl->expr == NULL)
		{
			sb_append(builder, type == Identifier_Type_Struct || is_array? "{0}":"0");
		}
		else
		{
			g_current_decl_type = decl->type;
			generate_expression(builder, decl->expr, TOP_PRECEDENCE, NOT_TYPE);
			g_current_decl_type = NULL;
		}
		sb_append(builder, ";\n");
	} break;
	
	case KAI_STMT_ASSIGNMENT: {
		Kai_Stmt_Assignment* ass = (void*)stmt;
		tab(depth);
		generate_expression(builder, ass->left, TOP_PRECEDENCE, NONE);
		const char* op = "ASSIGN";
		switch (ass->op)
		{
			case '=' : op = "="  ;break;
			case KAI_MULTI('|', '=',,): op = "|=" ;break;
			case KAI_MULTI('&', '=',,): op = "&=" ;break;
			case KAI_MULTI('+', '=',,): op = "+=" ;break;
			case KAI_MULTI('-', '=',,): op = "-=" ;break;
			case KAI_MULTI('*', '=',,): op = "*=" ;break;
			case KAI_MULTI('/', '=',,): op = "/=" ;break;
		}
		sb_appendf(builder, " %s ", op);
		generate_expression(builder, ass->expr, TOP_PRECEDENCE, NOT_TYPE);
		sb_append(builder, ";\n");
	} break;
	
	case KAI_STMT_RETURN: {
		Kai_Stmt_Return* ret = (void*)stmt;
		tab(depth); sb_append(builder, "return ");
		if (ret->expr == NULL)
		{
			da_last(builder) = ';';
			da_append(builder, '\n');
			break;
		}
		generate_expression(builder, ret->expr, TOP_PRECEDENCE, NOT_TYPE);
		sb_append(builder, ";\n");
	} break;
	
	case KAI_STMT_IF: {
		Kai_Stmt_If* _if = (void*)stmt;
		tab(depth);
		if (_if->flags & KAI_FLAG_IF_CASE) {
			g_go_through_case = false;
			sb_append(builder, "switch (");
		}
		else sb_append(builder, "if (");
		generate_expression(builder, _if->expr, TOP_PRECEDENCE, NONE);
		sb_append(builder, ")\n");
		generate_statement(builder, _if->then_body, depth, true);
		if (_if->else_body != NULL)
		{
			tab(depth); sb_append(builder, "else\n");
			if (_if->else_body->id == KAI_STMT_IF)
				depth -= 1;
			generate_statement(builder, _if->else_body, depth, true);
		}
	} break;
	
	case KAI_STMT_WHILE: {
		Kai_Stmt_While* whi = (void*)stmt;
		tab(depth); sb_append(builder, "while (");
		generate_expression(builder, whi->expr, TOP_PRECEDENCE, NONE);
		sb_append(builder, ")\n");
		generate_statement(builder, whi->body, depth, true);
	} break;
	
	case KAI_STMT_FOR: {
		Kai_Stmt_For* _for = (void*)stmt;
		tab(depth); sb_append(builder, "for (Kai_u32 ");
		sb_append_buf(builder, _for->iterator_name.data, _for->iterator_name.count);
		sb_append(builder, " = ");
		generate_expression(builder, _for->from, TOP_PRECEDENCE, NONE);
		sb_append(builder, "; ");
		sb_append_buf(builder, _for->iterator_name.data, _for->iterator_name.count);
		if (_for->flags & KAI_FLAG_FOR_LESS_THAN)
			sb_append(builder, " < ");
		else sb_append(builder, " <= ");
		generate_expression(builder, _for->to, TOP_PRECEDENCE, NONE);
		sb_append(builder, "; ++");
		sb_append_buf(builder, _for->iterator_name.data, _for->iterator_name.count);
		sb_append(builder, ")\n");
		generate_statement(builder, _for->body, depth, true);
	} break;

	default: {
		tab(depth); generate_expression(builder, stmt, TOP_PRECEDENCE, NONE);
		sb_append(builder, ";\n");
	} break;
	}
}

void generate_procedure(String_Builder* builder, Kai_Expr_Procedure* proc)
{
	Scope scope = (Scope){0};
	Kai_Expr* in = proc->in_out_expr;
	for (int i = 0; i < proc->in_count; ++i, in = in->next)
	{
		const char* name = temp_cstr_from_string(in->name);
		Variable_Type type = {.expr = in};
		shput(scope.variable_map, name, type);
	}
	arrpush(g_scopes, scope);
	generate_statement(builder, proc->body, 0, false);
	arrpop(g_scopes);
}

void generate_constant(String_Builder* builder, Kai_Stmt_Declaration* decl)
{
	ASSERT(decl->type != NULL);
	if (decl->type->id == KAI_EXPR_ARRAY)
	{
		ASSERT(decl->expr->id == KAI_EXPR_UNARY);
		Kai_Expr_Array* arr = (void*)decl->type;
		Kai_Expr_Unary* una = (void*)decl->expr;
		ASSERT(una->expr != NULL && una->expr->id == KAI_EXPR_LITERAL);
		Kai_Expr_Literal* lit = (void*)una->expr;
		sb_append(builder, "static ");
		generate_expression(builder, arr->expr, TOP_PRECEDENCE, NONE);
		const char* id = temp_cstr_from_string(decl->name);
		ASSERT(arr->rows != NULL && arr->rows->id == KAI_EXPR_NUMBER);
		Kai_Expr_Number* num = (void*)arr->rows;
		sb_appendf(builder, " kai_%s[%llu] = {\n", id, kai_number_to_u64(num->value));
		shput(g_identifier_map, id, Identifier_Type_Function);
		tab(1);
		Kai_u32 last = builder->count;
		for (Kai_Expr* expr = lit->head; expr != NULL; expr = expr->next)
		{
			generate_expression(builder, expr, TOP_PRECEDENCE, NONE);
			if (expr->next == NULL)
			{
				sb_append(builder, "\n");
				break;
			}
			sb_append(builder, ", ");
			if (builder->count - last > 80)
			{
				sb_append(builder, "\n");
				tab(1);
				last = builder->count;
			}
		}
		sb_append(builder, "};\n");
	}
	else {
		generate_statement(builder, (Kai_Stmt*)decl, 0, KAI_FALSE);
	}
}

void generate_all_imports(String_Builder* builder, Kai_Stmt_Compound* stmt)
{
	Kai_Stmt* current = stmt->head;
	for (Kai_Stmt* current = stmt->head; current != NULL; current = current->next)
	{
		if (current->id != KAI_EXPR_IMPORT) continue;
		const char* name = temp_cstr_from_string(current->name);
		sb_appendf(builder, "#include <%s>\n", name);	
	}
}

void generate_all_internal_function_definitions(String_Builder* builder)
{
	for (int i = 0; i < arrlen(g_internal_functions); ++i)
	{
		sb_append(builder, g_internal_functions[i]);
		sb_append(builder, ";\n");
	}
	sb_append(builder, "\n");
}

void generate_all_function_implementations(String_Builder* builder, Kai_Stmt_Compound* stmt)
{
	Kai_Stmt* current = stmt->head;
	for (Kai_Stmt* current = stmt->head; current != NULL; current = current->next)
	{
		if (current->id != KAI_STMT_DECLARATION) continue;
		Kai_Stmt_Declaration* decl = (void*)current;

		if (decl->expr == NULL)
		{
			generate_constant(builder, decl);
			continue;
		}
		
		if (decl->expr->id == KAI_EXPR_UNARY)
		{
			Kai_Expr_Unary* una = (void*)decl->expr;
			if (una->op != '.') continue;

			generate_constant(builder, decl);
			sb_append(builder, "\n");
			continue;
		}
		
		if (decl->expr->id != KAI_EXPR_PROCEDURE)
			continue;

		Kai_Expr_Procedure* proc = (void*)decl->expr;
		const char* name = temp_cstr_from_string(decl->name);	
		const char* def = shget(g_function_cache, name);
		sb_appendf(builder, "%s\n", def);
		generate_procedure(builder, proc);
		sb_append(builder, "\n");
	}
}

Kai_Syntax_Tree create_tree_from_file(const char* path)
{
	String_Builder builder = {0};
	exit_on_fail(read_entire_file(path, &builder));

	Kai_Error error = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = g_allocator,
		.error = &error,
		.source = {
			.name = kai_string_from_c(path),
			.contents = {
				.data = (Kai_u8*)builder.items,
				.count = builder.count,
			},
		},
	};

	uint64_t start = nanos_since_unspecified_epoch();
	Kai_Syntax_Tree tree = {0};
	if (kai_create_syntax_tree(&info, &tree) != KAI_SUCCESS)
	{
		kai_write_error(&g_writer, &error);
		exit(1);
	}
	uint64_t end = nanos_since_unspecified_epoch();
	String_Builder msg = {0};
	sb_appendf(&msg, "Parsed file \"\x1b[92m%s\x1b[0m\"", path);
	int pad = 20 - strlen(path);
	if (pad < 0) pad = 0;
	forn(pad) da_append(&msg, ' ');
	sb_appendf(&msg, " in \x1b[94m%f\x1b[0m ms", (double)(end - start)/(NANOS_PER_SEC/1000));
	sb_append_null(&msg);
	nob_log(INFO, "%s", msg.items);
	sb_free(msg);
	return tree;
}

int test_index(const char* name)
{
	ASSERT('0' <= name[0] && name[0] <= '9');
	ASSERT('0' <= name[1] && name[1] <= '9');
	return (name[0] - '0') * 10 + (name[1] - '0');
}

void run_tests(void)
{
	minimal_log_level = NO_LOGS;
	set_current_dir("tests");
	{
		printf("------ Tests ----------------------------------------------------------------\n");

		Cmd cmd = {0};

		const char* *test_names = NULL;
		arrsetlen(test_names, 100);
		memset(test_names, 0, 100 * sizeof(test_names[0]));

		File_Paths files = {0};
		exit_on_fail(read_entire_dir(".", &files));
		forn (files.count)
		{
			const char* path = files.items[i];
			String_View path_sv = sv_from_cstr(path);
			if (!sv_end_with(path_sv, ".c")) continue;
			const char* name = clone_string((Kai_string){.data = (Kai_u8*)path, .count = path_sv.count-2});
			test_names[test_index(name)] = name;
		}

		forn (arrlen(test_names)) SCOPED_TEMP()
		{
			const char* name = test_names[i];
			if (name == NULL) continue;
			printf("  %s\n", name);
			const char* output = executable(name);
			nob_cc(&cmd);
			nob_cc_flags(&cmd);
			nob_cc_inputs(&cmd, temp_sprintf("%s.c", name));
			nob_cc_output(&cmd, temp_sprintf("../bin/%s", output));
			exit_on_fail(cmd_run_sync_and_reset(&cmd));
			cmd_append(&cmd, temp_sprintf("../bin/%s", output));
			exit_on_fail(cmd_run_sync_and_reset(&cmd));
		}
		
		printf("-----------------------------------------------------------------------------\n");
	}
	set_current_dir("..");
	minimal_log_level = 0;
}

bool compile_debug = false;

#define nob_cc_debug(cmd) cmd_append(cmd, "-g")

void compile_command_line_tool(void)
{
	set_current_dir("kai");
	SCOPED_TEMP()
	{
		Cmd cmd = {0};
		const char* output = executable("kai");
		nob_cc(&cmd);
		nob_cc_flags(&cmd);
        if (compile_debug) nob_cc_debug(&cmd);
		nob_cc_inputs(&cmd, "main.c");
		nob_cc_output(&cmd, temp_sprintf("../bin/%s", output));
		exit_on_fail(cmd_run_sync_and_reset(&cmd));
	}
	set_current_dir("..");
}

void compile_playground(void)
{
	set_current_dir("playground");
	SCOPED_TEMP()
	{
		Cmd cmd = {0};
		cmd_append(&cmd, executable("clang"));
		cmd_append(&cmd, "-Wall", "-Wextra");
		cmd_append(&cmd, "-O2");
		cmd_append(&cmd, "--target=wasm32", "-D__WASM__", "-nostdlib", "-Wl,--no-entry", "-Wl,--export-dynamic"/* , "-Wl,--allow-undefined" */);
		cmd_append(&cmd, "lib.c");
		cmd_append(&cmd, "-o", "lib.wasm");
		exit_on_fail(cmd_run_sync_and_reset(&cmd));
	}
	set_current_dir("..");
}

const char* source_files[] = {
	"src/core.kai",
	"src/parser.kai",
	"src/compiler.kai",
};

typedef struct {
	Kai_string name;
	const char** imports;
	Kai_Syntax_Tree tree;
	const char* extra;
} Module;

Module modules[] = {
	{KAI_CONST_STRING("writer"), NULL, {0},
	"#if defined(KAI_PLATFORM_WINDOWS)\n"
	"__declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);\n"
	"KAI_INTERNAL void kai__setup_utf8_stdout(void) {\n"
	"    SetConsoleOutputCP(65001);\n"
	"    setlocale(LC_CTYPE, \".UTF8\");\n"
	"}\n"
	"#else\n"
	"KAI_INTERNAL void kai__setup_utf8_stdout(void) {\n"
	"    setlocale(LC_CTYPE, \".UTF8\");\n"
	"}\n"
	"#endif\n"
	"#if defined(KAI_PLATFORM_WINDOWS)\n"
	"KAI_INTERNAL FILE* kai__stdc_file_open(char const* path, char const* mode) {\n"
	"    FILE* handle = NULL;\n"
	"    fopen_s(&handle, path, mode); // this is somehow more safe? :/\n"
	"    return (void*)handle;\n"
	"}\n"
	"#else\n"
	"#    define kai__stdc_file_open fopen\n"
	"#endif\n\n"
	},
	{KAI_CONST_STRING("memory"), NULL, {0},
"#if defined(KAI_PLATFORM_LINUX) || defined(KAI_PLATFORM_APPLE)\n"
"#include <sys/mman.h> // -> mmap\n"
"#include <unistd.h>   // -> getpagesize\n"
"KAI_INTERNAL void* kai__memory_platform_allocate(void* user, void* ptr, Kai_u32 size, Kai_u32 op)\n"
"{\n"
"	Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(user);\n"
"	switch (op) {\n"
"	case KAI_MEMORY_COMMAND_ALLOCATE_WRITE_ONLY: {\n"
"        void* new_ptr = mmap(NULL, size, PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);\n"
"        if (new_ptr == MAP_FAILED)\n"
"            return NULL;\n"
"        metadata->total_allocated += size;\n"
"        return new_ptr;\n"
"	}\n"
"	case KAI_MEMORY_COMMAND_SET_EXECUTABLE: {\n"
"        kai_assert(mprotect(ptr, size, PROT_EXEC) == 0);\n"
"		return NULL;\n"
"	}\n"
"	case KAI_MEMORY_COMMAND_FREE: {\n"
"        kai_assert(munmap(ptr, size) == 0);\n"
"        metadata->total_allocated -= size;\n"
"	}\n"
"	}\n"
"    return NULL;\n"
"}\n"
"#define kai__page_size() (Kai_u32)sysconf(_SC_PAGESIZE)\n"
"#elif defined(KAI_PLATFORM_WINDOWS)\n"
"typedef unsigned short      WORD;\n"
"typedef unsigned long       DWORD;\n"
"typedef int                 BOOL;\n"
"typedef uintptr_t           SIZE_T;\n"
"typedef struct _SYSTEM_INFO SYSTEM_INFO;\n"
"__declspec(dllimport) void* __stdcall VirtualAlloc(void* lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);\n"
"__declspec(dllimport) BOOL __stdcall VirtualProtect(void* lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD* lpflOldProtect);\n"
"__declspec(dllimport) BOOL __stdcall VirtualFree(void* lpAddress, SIZE_T dwSize, DWORD dwFreeType);\n"
"__declspec(dllimport) void __stdcall GetSystemInfo(SYSTEM_INFO* lpSystemInfo);\n"
"KAI_INTERNAL void* kai__memory_platform_allocate(void* user, void* ptr, Kai_u32 size, Kai_u32 op)\n"
"{\n"
"	Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(user);\n"
"	switch (op) {\n"
"	case KAI_MEMORY_COMMAND_ALLOCATE_WRITE_ONLY: {\n"
"		void* result = VirtualAlloc(NULL, size, 0x1000|0x2000, 0x04);\n"
"		kai_assert(result != NULL);\n"
"		metadata->total_allocated += size;\n"
"		return result;\n"
"	}\n"
"	case KAI_MEMORY_COMMAND_SET_EXECUTABLE: {\n"
"		DWORD old;\n"
"		kai_assert(VirtualProtect(ptr, size, 0x10, &old) != 0);\n"
"		return NULL;\n"
"	}\n"
"	case KAI_MEMORY_COMMAND_FREE: {\n"
"		kai_assert(VirtualFree(ptr, 0, 0x8000) != 0);\n"
"		metadata->total_allocated -= size;\n"
"	}\n"
"	}\n"
"    return NULL;\n"
"}\n"
"KAI_INTERNAL Kai_u32 kai__page_size(void)\n"
"{\n"
"    struct {\n"
"        DWORD dwOemId;\n"
"        DWORD dwPageSize;\n"
"        int _padding[10];\n"
"    } info;\n"
"    GetSystemInfo((struct _SYSTEM_INFO*)&info);\n"
"    return (Kai_u32)info.dwPageSize;\n"
"}\n"
"#else\n"
"#	error \"[KAI] No memory allocator implemented for the current platform :(\"\n"
"#endif\n"
	},
};

int main(int argc, char** argv)
{
#ifndef DEBUG // Debugger does not like this
#ifdef USE_GENERATED
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "kai.h");
#else
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "3rd-party/kai.h");
#endif
#endif
	kai_memory_create(&g_allocator);
	g_writer = kai_writer_stdout();

	shdefault(g_identifier_map, Identifier_Type_Invalid);
	shdefault(g_struct_map, NULL);

	Kai_Syntax_Tree* trees = NULL;

	for (int i = 0; i < len(intrinsics); ++i)
	{
		const char* k = format("intrinsics_%s", intrinsics[i]);
		shput(g_identifier_map, k, Identifier_Type_Function);
	}
	shput(g_identifier_map, "assert",      Identifier_Type_Function);
	shput(g_identifier_map, "fatal_error", Identifier_Type_Function);
	shput(g_identifier_map, "u128",        Identifier_Type_Type);

	shput(g_identifier_map, "_setup_utf8_stdout",        Identifier_Type_Function);
	shput(g_identifier_map, "_stdc_file_open",           Identifier_Type_Function);
	shput(g_identifier_map, "_memory_platform_allocate", Identifier_Type_Function);
	shput(g_identifier_map, "_page_size",                Identifier_Type_Function);

	String_Builder builder = {0};
	exit_on_fail(read_entire_file("src/comments/header.h", &builder));
	sb_append(&builder, "#ifndef KAI__H\n#define KAI__H\n\n");
	sb_append(&builder, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");
	sb_append(&builder,
		"#include <stdint.h> // --> uint32, uint64, ...\n"
		"#include <stddef.h> // --> NULL\n");
	for (int i = 0; i < len(modules); ++i)
	{
		modules[i].tree = create_tree_from_file(format("src/%s.kai", modules[i].name.data));
		const char* upper_name = temp_cstr_upper(modules[i].name);
		sb_appendf(&builder, "#ifndef KAI_DONT_USE_%s_API\n", upper_name);
		generate_all_imports(&builder, &modules[i].tree.root);
		sb_append(&builder, "#endif\n");
	}
	sb_append(&builder, "\n");
	generate_all_macros(&builder);
	generate_all_primitive_types(&builder);
	for (int i = 0; i < len(source_files); ++i)
	{
		Kai_Syntax_Tree tree = create_tree_from_file(source_files[i]);
		generate_all_struct_typedefs(&builder, &tree.root);
		arrput(trees, tree);
	}
	for (int i = 0; i < arrlen(trees); ++i)
		generate_all_non_struct_typedefs(&builder, &trees[i].root);
	for (int i = 0; i < arrlen(trees); ++i)
		generate_all_data_structure_types(&builder, (Kai_Stmt*)&trees[i].root);
	sb_append(&builder, "\n");
	for (int i = 0; i < arrlen(trees); ++i)
		generate_all_struct_definitions(&builder, &trees[i].root);
	for (int i = 0; i < arrlen(trees); ++i)
		generate_all_function_definitions(&builder, &trees[i].root, KAI_TRUE);
	for (int i = 0; i < len(modules); ++i)
	{
		const char* upper_name = temp_cstr_upper(modules[i].name);
		sb_appendf(&builder, "#ifndef KAI_DONT_USE_%s_API\n", upper_name);
		generate_all_struct_typedefs(&builder, &modules[i].tree.root);
		generate_all_data_structure_types(&builder, (Kai_Stmt*)&modules[i].tree.root);
		generate_all_non_struct_typedefs(&builder, &modules[i].tree.root);
		generate_all_struct_definitions(&builder, &modules[i].tree.root);
		generate_all_function_definitions(&builder, &modules[i].tree.root, KAI_FALSE);
		sb_append(&builder, "#endif\n\n");
	}
	sb_append(&builder, "#ifdef KAI_IMPLEMENTATION\n\n");
	generate_all_internal_macros(&builder);
	exit_on_fail(read_entire_file("src/intrinsics.h", &builder));
	generate_all_internal_function_definitions(&builder);
	for (int i = 0; i < arrlen(trees); ++i)
		g_current_tree = &trees[i], generate_all_function_implementations(&builder, &trees[i].root);
	for (int i = 0; i < len(modules); ++i)
	{
		const char* upper_name = temp_cstr_upper(modules[i].name);
		sb_appendf(&builder, "#ifndef KAI_DONT_USE_%s_API\n\n", upper_name);
		sb_append(&builder, modules[i].extra);
		g_current_tree = &trees[i], generate_all_function_implementations(&builder, &modules[i].tree.root);
		sb_append(&builder, "#endif\n");
	}
	sb_append(&builder, "#endif // KAI_IMPLEMENTATION\n\n");
	sb_append(&builder, "#ifdef __cplusplus\n}\n#endif\n\n");
	sb_append(&builder, "#endif // KAI__H\n");
	exit_on_fail(read_entire_file("src/comments/footer.h", &builder));
	write_entire_file("kai.h", builder.items, builder.count);
	nob_log(INFO, "Generated \"kai.h\"");

	exit_on_fail(mkdir_if_not_exists("bin"));

    bool want_test = false;

    for (int i = 0; i < argc; ++i)
    {
             if (strcmp(argv[i], "test") == 0) want_test = true;
        else if (strcmp(argv[i], "debug") == 0) compile_debug = true;
    }
    if (want_test) run_tests();
	compile_command_line_tool();
	compile_playground();
	return 0;
}

/* TODO: When type checking is complete
	Kai_Import import;
	import.name = KAI_STRING("_memory_platform_allocate");
	import.type = KAI_STRING("(*void, *void, u32, Memory_Command)");
	import.value.ptr = (void*)"memory_platform_allocate.c";
	import.name = KAI_STRING("_memory_page_size");
	import.type = KAI_STRING("() -> u32");
	import.value.ptr = (void*)"memory_page_size.c";
*/
