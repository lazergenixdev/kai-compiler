// Command line utility

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../3rd-party/nob.h"
#define KAI_IMPLEMENTATION
#include "../kai.h"
#define S1(x) #x
#define S2(x) S1(x)

#define hash_table_iterate(Table, Iter_Var)                               \
  for (Kai_u32 Iter_Var = 0; Iter_Var < (Table).capacity; ++Iter_Var)     \
    if ((Table).occupied[Iter_Var / 64] & ((Kai_u64)1 << (Iter_Var % 64)))

#define space(N) for (int __i = 0; __i < (int)(N); ++__i) putchar(' ')

typedef struct {
    const char* name;
    int (*cmd)(int argc, char** argv);
} Command;

Kai_Writer stdout_writer = {0};
Kai_Writer* writer = &stdout_writer;
Kai_Allocator allocator = {0};
const char* program_name = "kai";

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

int help(int argc, char** argv)
{
    (void)argc, (void)argv;
    printf(
        "Usage: %s command [arguments...]\n"
        "\n"
        "   Commands:\n"
        "      help      Print help and exit (-h, --help, -?)\n"
        "      version   Print version and exit\n"
        "      token     Tokenize file\n"
        "      parse     Parse file\n"
        "\n"
    ,   program_name
    );
    return 1;
}

int error_no_source_provided()
{
    nob_log(ERROR, "No input files provided");
    return 1;
}

int token(int argc, char** argv)
{
    if (argc == 0) return error_no_source_provided();
    Kai_Source source = load_source_file(argv[0]);
    Kai_Tokenizer tokenizer = {
        .source = source.contents,
        .line_number = 1,
        .string_arena = {
            .data = allocator.heap_allocate(allocator.user, NULL, source.contents.count, 0),
            .size = source.contents.count,
        },
    };
    Kai_Token* token = kai_tokenizer_next(&tokenizer);
    while (token->id != KAI_TOKEN_END)
    {
        kai_write_token(writer, *token);
        kai_tokenizer_next(&tokenizer);
        kai__write(" ");
    }
    return 0;
}

#define PARSE_NO_PRINT (1<<0) // -p

int parse(int argc, char** argv)
{
    if (argc == 0) return error_no_source_provided();
    Kai_u32 parse_options = 0;
    Kai_u32 source_start = 0;
    for (int i = 0; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // -> Possible Flag
            if (strcmp(argv[i]+1, "p") == 0)
                parse_options |= PARSE_NO_PRINT;
        }
        else {
            source_start = i;
            break;
        }
    }
    Kai_Error error = {0};
	Kai_Syntax_Tree tree = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = allocator,
		.error = &error,
		.source = load_source_file(argv[source_start]),
	};
	kai_create_syntax_tree(&info, &tree);
    if (!(parse_options & PARSE_NO_PRINT))
	    kai_write_expression(writer, (Kai_Expr*)&tree.root, 0);
	if (error.result != KAI_SUCCESS) {
		kai_write_error(writer, &error);
	}
	return error.result != KAI_SUCCESS;
}

void write_value_no_code_gen(void* data, Kai_Type type)
{
    switch (type->id)
    {
    case KAI_TYPE_ID_PROCEDURE: {
        Kai_Expr* expr = *(Kai_Expr**)data;
        printf("\n");
        kai_write_expression(writer, expr, 1);
    } break;
    case KAI_TYPE_ID_INTEGER: {
        union { Kai_u64 u; Kai_s64 s; } value;
        Kai_Type_Info_Integer* i = (Kai_Type_Info_Integer*)type;
        if (i->is_signed)
        switch (i->bits)
        {
        case 8:  value.s = *(Kai_s8*)data;
        case 16: value.s = *(Kai_s16*)data;
        case 32: value.s = *(Kai_s32*)data;
        case 64: value.s = *(Kai_s64*)data;
        }
        else switch (i->bits)
        {
        case 8:  value.u = *(Kai_u8*)data;
        case 16: value.u = *(Kai_u16*)data;
        case 32: value.u = *(Kai_u32*)data;
        case 64: value.u = *(Kai_u64*)data;
        }
        printf(" = ");
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
        if (i->is_signed) printf("%lli", value.s);
        else              printf("%llu", value.u);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        printf(" [");
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT_2);
        kai_write_type(writer, type);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        printf("]\n");
    } break;
    case KAI_TYPE_ID_FLOAT: {
        Kai_Type_Info_Float* f = (Kai_Type_Info_Float*)type;
        printf(" = ");
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
        switch (f->bits)
        {
        case 32: printf("%f", *(Kai_f32*)data); break;
        case 64: printf("%f", *(Kai_f64*)data); break;
        }
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        printf(" [");
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT_2);
        kai_write_type(writer, type);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        printf("]\n");
    } break;
    default: {
        printf("[unknown value type]\n");
    } break;
    }
}

#define COMPILE_NO_PRINT     (1<<0) // -p
#define COMPILE_SHOW_EXPORTS (1<<1) // -e
#define COMPILE_NO_CODE_GEN  (1<<2) // -c

int compile(int argc, char** argv)
{
    if (argc == 0) return error_no_source_provided();
    Kai_u32 parse_options = 0;
    Kai_u32 source_start = 0;
    for (int i = 0; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // -> Possible Flag
            if (strcmp(argv[i]+1, "p") == 0)
                parse_options |= COMPILE_NO_PRINT;
            else if (strcmp(argv[i]+1, "e") == 0)
                parse_options |= COMPILE_SHOW_EXPORTS;
        }
        else {
            source_start = i;
            break;
        }
    }
    Kai_Error error = {0};
	Kai_Program program = {0};
    Kai_Source source = load_source_file(argv[source_start]);
    Kai_Program_Create_Info info = {
        .allocator = allocator,
        .error = &error,
        .sources = { .count = 1, .data = &source },
		.options = { .flags = KAI_COMPILE_NO_CODE_GEN },
    };
    kai_create_program(&info, &program);

    if (!(parse_options & COMPILE_NO_PRINT))
    {
        if (parse_options & COMPILE_SHOW_EXPORTS)
        {
            printf("\x1b[1;97mName");
            space(32 - 4);
            printf("Type\x1b[0m\n");
        }
        hash_table_iterate(program.variable_table, i)
        {
            Kai_string name = program.variable_table.keys[i];
            Kai_Variable var = program.variable_table.values[i];
            printf("%.*s", name.count, name.data);
            if (parse_options & COMPILE_SHOW_EXPORTS)
            {
                space(32 - name.count);
                kai_write_type(writer, var.type);
                printf("\n");
            }
            else write_value_no_code_gen(program.data.data + var.location, var.type);
        }
    }
	if (error.result != KAI_SUCCESS) {
		kai_write_error(writer, &error);
	}
	return error.result != KAI_SUCCESS;
}

int main(int argc, char** argv)
{
    if (argc <= 1) return help(argc, argv);
    argc -= 2;
    argv += 2;
    stdout_writer = kai_writer_stdout();
    kai_memory_create(&allocator);
    if (strcmp(argv[-1], "token"  ) == 0) return token(argc, argv);
    if (strcmp(argv[-1], "parse"  ) == 0) return parse(argc, argv);
    if (strcmp(argv[-1], "compile") == 0) return compile(argc, argv);
    if (strcmp(argv[-1], "help"   ) == 0
    ||  strcmp(argv[-1], "--help" ) == 0
    ||  strcmp(argv[-1], "-?"     ) == 0
    ||  strcmp(argv[-1], "-h"     ) == 0) help(argc, argv), exit(0);
    if (strcmp(argv[-1], "version") == 0)
    {
        Kai_string version = kai_version_string();
        printf("\x1b[1;97mKai v%.*s\x1b[0;90m (build %s)\x1b[0m\n", version.count, version.data, S2(KAI_BUILD_DATE));
        exit(0);
    }
    return help(argc, argv);
}