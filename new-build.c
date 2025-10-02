// Temporary file, that will eventually be the new build.c!
#define USE_GENERATED
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0
#define VERSION_EXTRA "alpha"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "3rd-party/nob.h"
#define STB_DS_IMPLEMENTATION
#include "3rd-party/stb_ds.h"
#define KAI_IMPLEMENTATION
#   ifdef USE_GENERATED
#       include "kai.h"
#   else
#       include "3rd-party/kai.h"
#endif

#define sb_append nob_sb_append_cstr
#define sb_append_string(builder, string) nob_sb_append_buf(builder, string.data, string.count)
#define ASSERT NOB_ASSERT
#define global static
#define len(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))
#define forn(LEN) for (int i = 0; i < (LEN); ++i)
#define tab(N) for (int i = 0; i < N; ++i) sb_append(builder, "    ")
#define MAKE_SLICE(L) {.data = L, .count = sizeof(L)/sizeof(L[0])}
#define MACRO_STRING(S) #S
#define MACRO_EXPSTR(S) MACRO_STRING(S)
#define TOP_PRECEDENCE 100
#define KAI_MULTI(A,B,C,D) ((Kai_u32)(A) | (Kai_u32)(B << 8) | (Kai_u32)((C+0) << 16) | (Kai_u32)((D+0) << 24))

typedef void (*Expr_Visitor)(String_Builder* builder, Kai_Expr* expr);

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
} String_Map;

#define S KAI_CONST_STRING
static struct {
    Kai_Import import;
    Kai_string value;
} g_macros[] = {
    {.import = {.name = S("VERSION_STRING"), .type = S("string")}},
    {.import = {.name = S("VERSION_MAJOR"),  .type = S("u32"), .value = {.u32 = VERSION_MAJOR}}, .value = S(MACRO_EXPSTR(VERSION_MAJOR))},
    {.import = {.name = S("VERSION_MINOR"),  .type = S("u32"), .value = {.u32 = VERSION_MINOR}}, .value = S(MACRO_EXPSTR(VERSION_MINOR))},
    {.import = {.name = S("VERSION_PATCH"),  .type = S("u32"), .value = {.u32 = VERSION_PATCH}}, .value = S(MACRO_EXPSTR(VERSION_PATCH))},
};
#undef S

global Identifier_Map* g_identifier_map;
global String_Map* g_function_def_cache; // cache for convenience, not speed
global Kai_Writer stdout_writer;

static inline String_Builder* temp_builder()
{
    global String_Builder builder = {0};
    builder.count = 0;
    return &builder;
}

static inline char upper(char c)
{
    if ('a' <= c && c <= 'z')
        return c + ('A' - 'a');
    return c;
}

global Kai_u8 cstr_buffer[1024];
static inline const char* cstr(Kai_string s)
{
	ASSERT(s.count < sizeof(cstr_buffer));
	memcpy(cstr_buffer, s.data, s.count);
	cstr_buffer[s.count] = 0;
	return (const char*)cstr_buffer;
}
static inline const char* cstr_upper(Kai_string s)
{
	ASSERT(s.count < sizeof(cstr_buffer));
    for (Kai_uint i = 0; i < s.count; ++i)
        cstr_buffer[i] = upper(s.data[i]);
	cstr_buffer[s.count] = 0;
	return (const char*)cstr_buffer;
}

static inline Kai_Source load_source_file(const char* path)
{
	String_Builder builder = {0};
	
	if (!read_entire_file(path, &builder))
		exit(1);
	
	return (Kai_Source) {
		.name = kai_string_from_c(path),
		.contents = {
			.data = (Kai_u8*)(builder.items),
			.count = (Kai_u32)(builder.count)
		}
	};
}

static inline uint64_t build_date()
{
    // NOTE: this function is super sketchy, but eh it works
    time_t current_time = time(NULL);
    struct tm *local_time_info = gmtime(&current_time);
    char buffer[80]; // Buffer to store the formatted string
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", local_time_info);
    uint64_t n = 0;
    for (int i = 0; buffer[i] != 0; ++i) {
        uint64_t d = buffer[i] - '0';
        n = n * 10 + d;
    }
    return n;
}

void generate_all_primitive_types(String_Builder* builder)
{
	for (int i = 8; i <= 64; i *= 2)
	{
		sb_appendf(builder, "typedef%suint%i_t Kai_u%i;\n", i==8?"  ":" ", i, i);
		shput(g_identifier_map, temp_sprintf("u%i", i), Identifier_Type_Type);
		sb_appendf(builder, "typedef %sint%i_t Kai_s%i;\n", i==8?"  ":" ", i, i);
		shput(g_identifier_map, temp_sprintf("s%i", i), Identifier_Type_Type);
	}
	sb_append_cstr(builder, "typedef    float Kai_f32;\n");
	sb_append_cstr(builder, "typedef   double Kai_f64;\n");
	shput(g_identifier_map, "f32", Identifier_Type_Type);
	shput(g_identifier_map, "f64", Identifier_Type_Type);

	for (int len = 2; len <= 4; ++len)
	{
        const char* types[] = {
            "u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "f32", "f64",
        };
		for (int i = 0; i < len(types); ++i)
		{
			const char* t = types[i];
			sb_appendf(builder, "typedef struct { Kai_%s%sx, y", t, i<2?"  ":" ");
			if (len >= 3) sb_append_cstr(builder, ", z");
			if (len >= 4) sb_append_cstr(builder, ", w");
			sb_appendf(builder, "; } Kai_vector%i_%s;\n", len, t);
			shput(g_identifier_map, temp_sprintf("vector%i_%s", len, t), Identifier_Type_Struct);
		}
	}

	sb_append_cstr(builder, "typedef Kai_u8 Kai_bool;\n");
	shput(g_identifier_map, "bool", Identifier_Type_Type);
	sb_append_cstr(builder, "enum { KAI_FALSE = 0, KAI_TRUE = 1 };\n");

	sb_append_cstr(builder, "typedef  intptr_t Kai_int;\n");
	shput(g_identifier_map, "int", Identifier_Type_Type);
	sb_append_cstr(builder, "typedef uintptr_t Kai_uint;\n");
	shput(g_identifier_map, "uint", Identifier_Type_Type);

	// String
	sb_appendf(builder, "typedef struct { Kai_uint count; Kai_u8* data; } Kai_string;\n");
	shput(g_identifier_map, "string", Identifier_Type_Struct);
	
	// C String
	sb_appendf(builder, "typedef const char* Kai_%s;\n", "cstring");
	shput(g_identifier_map, "cstring", Identifier_Type_Type);
}

void for_each_node(String_Builder* builder, Kai_Program* program, Expr_Visitor visitor)
{
    Kai_Syntax_Tree_Slice trees = program->code.trees;
    forn (trees.count)
    {
        Kai_Stmt_Compound root = trees.data[i].root;
        Kai_Stmt* current = root.head;
        while (current) {
            visitor(builder, current);
            current = current->next;
        }
    }
}

#define WT_NO_PREFIX (1<<0)
void write_type(String_Builder* builder, Kai_Type type, Kai_u32 flags)
{
    if (!(flags & WT_NO_PREFIX))
        sb_append(builder, "Kai_");
    switch (type->id)
    {
        break;case KAI_TYPE_ID_INTEGER: {
            Kai_Type_Info_Integer* info = (Kai_Type_Info_Integer*)type;
            sb_appendf(builder, "%s%u", info->is_signed? "s" : "u", info->bits);
        }
        break;case KAI_TYPE_ID_ARRAY: {
            Kai_Type_Info_Array* info = (Kai_Type_Info_Array*)type;
            if (info->rows != 0) {
                sb_appendf(builder, "vector%u_", info->rows);
                write_type(builder, info->sub_type, WT_NO_PREFIX);
            }
        }
        break;case KAI_TYPE_ID_STRING: {
            sb_append(builder, "string");
        }
        break;default: {
            sb_appendf(builder, "[todo %i]", type->id);
        }
    }
}

Kai_Tag* find_tag(Kai_Expr* expr, Kai_string name)
{
    for (Kai_Tag* tag = expr->tag; tag != NULL; tag = tag->next)
        if (kai_string_equals(tag->name, name))
            return tag;
    return NULL;
}

void write_inline_comment(String_Builder* builder, Kai_Expr* expr)
{
    for (Kai_Tag* tag = expr->tag; tag != NULL; tag = tag->next)
    {
        if (kai_string_equals(tag->name, KAI_STRING("comment")))
        {
            ASSERT(tag->expr != NULL);
            ASSERT(tag->expr->id == KAI_EXPR_STRING);
            Kai_string comment = ((Kai_Expr_String*)tag->expr)->value;
            sb_appendf(builder, " // %.*s", (int)comment.count, comment.data);
            break;
        }
    }
}

void generate_procedure_definition(String_Builder* builder, Kai_Expr* expr)
{
    if (expr->id != KAI_STMT_DECLARATION) return;
    Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)expr;
    if (d->expr == NULL) return;
    if (d->expr->id != KAI_EXPR_PROCEDURE) return;

    Kai_string name = d->name;
    Kai_Expr_Procedure* p = (Kai_Expr_Procedure*)d->expr;
    ASSERT(p->this_type->id == KAI_TYPE_ID_PROCEDURE);
    Kai_Type_Info_Procedure* type = (Kai_Type_Info_Procedure*)p->this_type;
    ASSERT(type->outputs.count <= 1);

    String_Builder def = {0};
    if (d->flags & KAI_FLAG_DECL_EXPORT)
    {
        sb_append(&def, "KAI_API(");
        if (type->outputs.count == 0)
            sb_append(&def, "void");
        else
            write_type(&def, type->outputs.data[0], 0);
        sb_appendf(&def, ") kai_%.*s(", (int)name.count, name.data);
    }
    else
    {
        sb_append(&def, "KAI_INTERNAL ");
        if (type->outputs.count == 0)
            sb_append(&def, "void");
        else
            write_type(&def, type->outputs.data[0], 0);
        sb_appendf(&def, " kai__%.*s(", (int)name.count, name.data);
    }

    Kai_Expr* current = p->in_out_expr;
    if (type->inputs.count == 0)
        sb_append(&def, "void");
    else forn (type->inputs.count) {
        write_type(&def, type->inputs.data[i], 0);
        da_append(&def, ' ');
        sb_append_string(&def, current->name);
        if (i != type->inputs.count - 1)
            sb_append(&def, ", ");

        current = current->next;
    }
    sb_append(&def, ")");
    sb_append_null(&def);

    shput(g_function_def_cache, cstr(name), def.items);
    if (d->flags & KAI_FLAG_DECL_EXPORT)
        sb_appendf(builder, "%s;\n", def.items);
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

#define GE_REQUIRE_LITERAL_TYPE  (1<<0)
#define GE_TYPE                  (1<<1)
void generate_expression(String_Builder* builder, Kai_Expr* expr, int prec, Kai_u32 flags)
{
    ASSERT(expr != NULL);
	switch (expr->id)
	{
	break;case KAI_EXPR_IDENTIFIER: {
        if (kai_string_equals(expr->source_code, KAI_STRING("void")))
        {
            sb_append(builder, "void");
            return;
        }
        // Special case to allow this to still be a C string
        if (kai_string_equals(expr->source_code, KAI_STRING("VERSION_STRING")))
        {
            sb_append(builder, "KAI_STRING(KAI_VERSION_STRING)");
            return;
        }
        const char* prefix = "";
        const char* name = cstr(expr->source_code);
        Identifier_Type identifier_type = shget(g_identifier_map, name);
        switch (identifier_type) {
        break;case Identifier_Type_Macro:    prefix = "KAI_";
        break;case Identifier_Type_Function: prefix = "kai_";
        break;case Identifier_Type_Type:     prefix = "Kai_";
        break;case Identifier_Type_Struct:   prefix = "Kai_";
        break;default: {
            if (flags & GE_TYPE)
                prefix = "Kai_";
        }
        }
        sb_appendf(builder, "%s%s", prefix, name);
    }
    break;case KAI_EXPR_NUMBER: {
        Kai_Expr_Number* n = (Kai_Expr_Number*)expr;
        // TODO: not always representable as integer
        sb_appendf(builder, "%llu", kai_number_to_u64(n->value));
    }
    break;case KAI_EXPR_LITERAL: {
        Kai_Expr_Literal* l = (Kai_Expr_Literal*)expr;
        if (flags&GE_REQUIRE_LITERAL_TYPE) {
            sb_append(builder, "(");
            write_type(builder, l->this_type, 0);
            sb_append(builder, ")");
        }
        sb_append(builder, "{");
		for (Kai_Stmt* current = l->head; current != NULL; current = current->next)
        {
            generate_expression(builder, current, TOP_PRECEDENCE, 0);
            if (current->next != NULL) sb_append(builder, ", ");
        }
        sb_append(builder, "}");
    }
    break;case KAI_EXPR_UNARY: {
        Kai_Expr_Unary* u = (Kai_Expr_Unary*)expr;
        if (flags&GE_TYPE) {
            ASSERT(u->op == '*');
            generate_expression(builder, u->expr, TOP_PRECEDENCE, flags);
            da_append(builder, '*');
        }
        else {
            sb_append(builder, "[unary]");
        }
    }
    break;case KAI_EXPR_BINARY: {
        Kai_Expr_Binary* b = (Kai_Expr_Binary*)expr;
        sb_append(builder, "(");
        generate_expression(builder, b->left, TOP_PRECEDENCE, flags);
        sb_append(builder, map_binary_operator(b->op));
        generate_expression(builder, b->right, TOP_PRECEDENCE, flags);
        sb_append(builder, ")");
    }
    break;default: {
        sb_append(builder, "[expr]");
    }
    }
}

void generate_statement(String_Builder* builder, Kai_Stmt* stmt, int depth, int increase_depth_on_non_compound)
{
    ASSERT(stmt != NULL);
    global int go_through_case = false;
	if (increase_depth_on_non_compound && stmt->id != KAI_STMT_COMPOUND)
		depth += 1;
	switch (stmt->id)
	{
	case KAI_STMT_COMPOUND: {
		Kai_Stmt_Compound* comp = (void*)stmt;
		tab(depth); sb_append(builder, "{\n");
		for (Kai_Stmt* current = comp->head; current != NULL; current = current->next)
			generate_statement(builder, current, depth+1, 0);
		tab(depth); sb_append(builder, "}\n");
	} break;
	
	case KAI_STMT_CONTROL: {
		Kai_Stmt_Control* con = (void*)stmt;
		if (con->kind != KAI_CONTROL_THROUGH) tab(depth);
		switch (con->kind)
		{
			case KAI_CONTROL_CASE: {
				if (!go_through_case) sb_append(builder, "break; ");
				if (con->expr == NULL)
					sb_append(builder, "default:\n");
				else {
					sb_append(builder, "case ");
					generate_expression(builder, con->expr, TOP_PRECEDENCE, 0);
					sb_append(builder, ":\n");
				}
				go_through_case = false;
			} break;
			case KAI_CONTROL_BREAK:    sb_append(builder, "break;\n"); break;
			case KAI_CONTROL_CONTINUE: sb_append(builder, "continue;\n"); break;
			case KAI_CONTROL_DEFER:    sb_append(builder, "defer;\n"); break;
			case KAI_CONTROL_THROUGH:  go_through_case = true; break;
		}

	} break;
	
	case KAI_STMT_DECLARATION: {
		Kai_Stmt_Declaration* decl = (void*)stmt;
		tab(depth);
		sb_append(builder, "decl;\n");
	} break;
	
	case KAI_STMT_ASSIGNMENT: {
		Kai_Stmt_Assignment* ass = (void*)stmt;
		tab(depth);
		generate_expression(builder, ass->left, TOP_PRECEDENCE, 0);
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
		generate_expression(builder, ass->expr, TOP_PRECEDENCE, 0);
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
		generate_expression(builder, ret->expr, TOP_PRECEDENCE, GE_REQUIRE_LITERAL_TYPE);
		sb_append(builder, ";\n");
	} break;
	
	case KAI_STMT_IF: {
		Kai_Stmt_If* _if = (void*)stmt;
		tab(depth);
		if (_if->flags & KAI_FLAG_IF_CASE) {
			go_through_case = false;
			sb_append(builder, "switch (");
		}
		else sb_append(builder, "if (");
		generate_expression(builder, _if->expr, TOP_PRECEDENCE, 0);
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
		generate_expression(builder, whi->expr, TOP_PRECEDENCE, 0);
		sb_append(builder, ")\n");
		generate_statement(builder, whi->body, depth, true);
	} break;
	
	case KAI_STMT_FOR: {
		Kai_Stmt_For* _for = (void*)stmt;
		tab(depth); sb_append(builder, "for (Kai_u32 ");
		sb_append_buf(builder, _for->iterator_name.data, _for->iterator_name.count);
		sb_append(builder, " = ");
		generate_expression(builder, _for->from, TOP_PRECEDENCE, 0);
		sb_append(builder, "; ");
		sb_append_buf(builder, _for->iterator_name.data, _for->iterator_name.count);
		if (_for->flags & KAI_FLAG_FOR_LESS_THAN)
			sb_append(builder, " < ");
		else sb_append(builder, " <= ");
		generate_expression(builder, _for->to, TOP_PRECEDENCE, 0);
		sb_append(builder, "; ++");
		sb_append_buf(builder, _for->iterator_name.data, _for->iterator_name.count);
		sb_append(builder, ")\n");
		generate_statement(builder, _for->body, depth, true);
	} break;

	default: {
		tab(depth); generate_expression(builder, stmt, TOP_PRECEDENCE, 0);
		sb_append(builder, ";\n");
	} break;
	}
}

void generate_typedef(String_Builder* builder, Kai_Expr* expr)
{
    if (expr->id != KAI_STMT_DECLARATION) return;
    Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)expr;
    if (d->expr == NULL) return;
    if (d->expr->id == KAI_EXPR_STRUCT)
    {
        const char* name = cstr(d->name);
        sb_appendf(builder, "typedef struct Kai_%s Kai_%s;\n", name, name);
    }
    else if (d->expr->id == KAI_EXPR_ENUM)
    {
        Kai_Expr_Enum* e = (Kai_Expr_Enum*)d->expr;
        sb_append(builder, "typedef ");
        generate_expression(builder, e->type, TOP_PRECEDENCE, GE_TYPE);
        sb_appendf(builder, " Kai_%s;\n", cstr(d->name));
    }
}

void generate_enum(String_Builder* builder, Kai_Expr* expr)
{
    if (expr->id != KAI_STMT_DECLARATION) return;
    Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)expr;
    if (d->expr == NULL) return;
    if (d->expr->id != KAI_EXPR_ENUM) return;
    Kai_Expr_Enum* e = (Kai_Expr_Enum*)d->expr;
    bool long_names = false;
    if (find_tag(expr, KAI_STRING("long_names")) != NULL) {
        long_names = true;
    }
    
    // Calculate max width for padding
    Kai_uint max_width = 0;
    for (Kai_Stmt* current = e->head; current != NULL; current = current->next)
    {
        ASSERT(current->id == KAI_STMT_DECLARATION);
        Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)current;
        if (d->name.count > max_width)
            max_width = d->name.count;
    }

    sb_appendf(builder, "enum /* Kai_%.*s */ {\n", (int)d->name.count, d->name.data);
    for (Kai_Stmt* current = e->head; current != NULL; current = current->next)
    {
        Kai_Stmt_Declaration* ed = (Kai_Stmt_Declaration*)current;
        tab(1);
        sb_append(builder, "KAI_");
        if (long_names)
            sb_appendf(builder, "%s_", cstr_upper(d->name));
        sb_append_string(builder, ed->name);
        for (Kai_uint i = ed->name.count; i < max_width; ++i)
            da_append(builder, ' ');
        sb_append(builder, " = ");
        generate_expression(builder, ed->expr, TOP_PRECEDENCE, 0);
        sb_append(builder, ",");
        write_inline_comment(builder, current);
        sb_append(builder, "\n");
    }
    sb_append(builder, "};\n\n");
}

void generate_struct(String_Builder* builder, Kai_Expr* expr)
{
    if (expr->id != KAI_STMT_DECLARATION) return;
    Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)expr;
    if (d->expr == NULL) return;
    if (d->expr->id != KAI_EXPR_STRUCT) return;
    Kai_Expr_Struct* s = (Kai_Expr_Struct*)d->expr;
    
    // Calculate max width for padding
    Kai_uint max_width = 0;
    String_Builder temp = {0};
    for (Kai_Stmt* current = s->head; current != NULL; current = current->next)
    {
        Kai_Stmt_Declaration* sd = (Kai_Stmt_Declaration*)current;
        ASSERT(sd->type != NULL);
        Kai_uint prev = temp.count;
        generate_expression(&temp, sd->type, TOP_PRECEDENCE, GE_TYPE);
        Kai_uint count = temp.count - prev;
        if (count > max_width)
            max_width = count;
        temp.count = 0;
    }

    sb_appendf(builder, "struct Kai_%.*s {\n", (int)d->name.count, d->name.data);
    for (Kai_Stmt* current = s->head; current != NULL; current = current->next)
    {
        Kai_Stmt_Declaration* sd = (Kai_Stmt_Declaration*)current;
        tab(1);
        Kai_uint prev = builder->count;
        generate_expression(builder, sd->type, TOP_PRECEDENCE, GE_TYPE);
        Kai_uint count = builder->count - prev;
        da_append(builder, ' ');
        for (Kai_uint i = count; i < max_width; ++i)
            da_append(builder, ' ');
        sb_append_string(builder, sd->name);
        sb_append(builder, ";\n");
    }
    sb_append(builder, "};\n\n");
}

void generate_all_internal_procedure_definitions(String_Builder* builder)
{
    forn (shlen(g_function_def_cache))
    {
        const char* def = g_function_def_cache[i].value;
        if (def[4] == 'I') // HACK
            sb_appendf(builder, "%s;\n", def);
    }
}

void generate_procedure_implementation(String_Builder* builder, Kai_Expr* expr)
{
    if (expr->id != KAI_STMT_DECLARATION) return;
    Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)expr;
    if (d->expr == NULL) return;
    
    const char* def = shget(g_function_def_cache, cstr(d->name));
    if (def == NULL) return;

    ASSERT(d->expr->id == KAI_EXPR_PROCEDURE);
    Kai_Expr_Procedure* p = (Kai_Expr_Procedure*)d->expr;

    sb_appendf(builder, "%s\n", def);
    generate_statement(builder, p->body, 0, 0);
    sb_appendf(builder, "\n");
}

void print_ast(String_Builder* builder, Kai_Expr* expr)
{
    kai_write_expression(&stdout_writer, expr, 0);
}

static uint64_t current_build_date;

int main(int argc, char** argv)
{
#ifndef DEBUG // Debugger does not like this
#ifdef USE_GENERATED
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "kai.h");
#else
	NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "3rd-party/kai.h");
#endif
#endif
    Kai_Allocator allocator = {0};
    kai_memory_create(&allocator);
    stdout_writer = kai_writer_stdout();
    Kai_Writer* writer = &stdout_writer;
    current_build_date = build_date();

    Kai_Source sources[] = {
        load_source_file("src/new-core.kai"),
    };
    Kai_Import imports[1+4] = {
        {.name = KAI_CONST_STRING("BUILD_DATE"), .type = KAI_CONST_STRING("u64"), .value = {.u64 = current_build_date}},
    };
    forn (len(g_macros)) {
        if (kai_string_equals(g_macros[i].import.name, KAI_STRING("VERSION_STRING"))) {
			if (strlen(VERSION_EXTRA) == 0)
				g_macros[i].value = KAI_STRING("\"" MACRO_EXPSTR(VERSION_MAJOR) "." MACRO_EXPSTR(VERSION_MINOR) "." MACRO_EXPSTR(VERSION_PATCH) "\"");
			else
				g_macros[i].value = KAI_STRING("\"" MACRO_EXPSTR(VERSION_MAJOR) "." MACRO_EXPSTR(VERSION_MINOR) "." MACRO_EXPSTR(VERSION_PATCH) " " VERSION_EXTRA "\"");
            g_macros[i].import.value.string = g_macros[i].value;
        }
        imports[1+i] = g_macros[i].import;
    }
    Kai_Error error = {};
    Kai_Program_Create_Info program_ci = {
        .sources = MAKE_SLICE(sources),
        .allocator = allocator,
        .error = &error,
        .imports = MAKE_SLICE(imports),
        .options = {.flags = KAI_COMPILE_NO_CODE_GEN},
        .debug_writer = writer,
    };
    Kai_Program program = {0};
    if (kai_create_program(&program_ci, &program)) {
        kai_write_error(writer, &error);
        return 1;
    }
    //for_each_node(NULL, &program, print_ast); // DEBUG
    String_Builder builder = {0};
	read_entire_file("src/comments/header.h", &builder);
	sb_append(&builder, "#ifndef KAI__H\n#define KAI__H\n\n");
	sb_append(&builder, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");
	sb_append(&builder,
		"#include <stdint.h>\n"
		"#include <stddef.h>\n");
	sb_append(&builder, "\n");
    sb_appendf(&builder, "#define KAI_BUILD_DATE %llu\n", current_build_date);
    forn (len(g_macros)) {
        Kai_string name = g_macros[i].import.name;
        Kai_string value = g_macros[i].value;
        const char* cname = temp_sprintf("%.*s", (int)name.count, name.data);
        shput(g_identifier_map, cname, Identifier_Type_Macro);
        sb_appendf(&builder, "#define KAI_%s %.*s\n", cname, (int)value.count, value.data);
    }
	sb_append(&builder, "#define KAI_INTERNAL inline static\n");
	read_entire_file("src/macros.h", &builder);
	sb_append(&builder, "\n");
    sb_append(&builder, "#define KAI_STRING(LITERAL) KAI_STRUCT(Kai_string){.count = (Kai_u32)(sizeof(LITERAL)-1), .data = (Kai_u8*)(LITERAL)}\n");
	sb_append(&builder, "\n");
    generate_all_primitive_types(&builder);
    for_each_node(&builder, &program, generate_typedef);
	sb_append(&builder, "\n");
    for_each_node(&builder, &program, generate_procedure_definition);
    sb_append(&builder, "\n");
    for_each_node(&builder, &program, generate_enum);
    for_each_node(&builder, &program, generate_struct);
	sb_append(&builder, "#ifdef KAI_IMPLEMENTATION\n\n");
	read_entire_file("src/intrinsics.h", &builder);
	read_entire_file("src/ldexp.c", &builder);
    generate_all_internal_procedure_definitions(&builder);
	sb_append(&builder, "\n");
    for_each_node(&builder, &program, generate_procedure_implementation);
	sb_append(&builder, "#endif // KAI_IMPLEMENTATION\n\n");
	sb_append(&builder, "#ifdef __cplusplus\n}\n#endif\n\n");
	sb_append(&builder, "#endif // KAI__H\n");
	read_entire_file("src/comments/footer.h", &builder);
	write_entire_file("new-kai.h", builder.items, builder.count);
    return 0;
}