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

#define PARSE_NO_PRINT (1<<0)
void help_parse()
{
    printf(
        "Usage: kai parse [FLAGS] <files...>\n"
        "\n"
        "FLAGS:\n"
        "   -p, --no-print   Do not print AST\n"
        "\n"
    );
}

#define COMPILE_NO_PRINT     (1<<0)
#define COMPILE_NO_CODE_GEN  (1<<1)
void help_compile()
{
    printf(
        "Usage: kai compile [FLAGS] <files...>\n"
        "\n"
        "FLAGS:\n"
        "   -p, --no-print       Do not print compilation results\n"
        "   -c, --no-code-gen    Do not generate machine code\n"
        "\n"
    );
}

int help(int argc, char** argv)
{
    if (argc > 0)
    {
        for (int i = 0; i < argc; ++i)
        {
            if (argv[i][0] == '-') continue;
            if (strcmp(argv[i], "parse") == 0)   { help_parse(); continue; }
            if (strcmp(argv[i], "compile") == 0) { help_compile(); continue; }
            printf("no help info for \"%s\"\n", argv[i]);
        }
        return 1;
    }
    printf(
        "Usage: kai command [arguments...]\n"
        "\n"
        "   Commands:\n"
        "      help      Print help and exit (-h, --help, -?)\n"
        "      version   Print version and exit\n"
        "      token     Tokenize file\n"
        "      parse     Parse file\n"
        "      bind      Generate host language bindings\n"
        "\n"
        "Examples:\n"
        "    kai help <command>\n"
        "\n"
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
        hash_table_iterate(program.variable_table, i)
        {
            Kai_string name = program.variable_table.keys[i];
            Kai_Variable var = program.variable_table.values[i];
            kai__write_string(name);
            kai__write(" = ");
            kai_write_value(writer, program.data.data + var.location, var.type);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write(" [");
            kai__set_color(KAI_WRITE_COLOR_TYPE);
            kai_write_type(writer, var.type);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write("]\n");
        }
    }
	if (error.result != KAI_SUCCESS) {
		kai_write_error(writer, &error);
	}
	return error.result != KAI_SUCCESS;
}

typedef enum {
    Language_C,
    Language_Cpp,
    Language_Invalid,
} Language;

int bind(int argc, char** argv)
{
    if (argc <= 0) {
        nob_log(ERROR, "No language provided");
        return 1;
    }
    const char* lang_str = argv[0];
    Language lang = Language_Invalid;
    if (argc <= 1) return error_no_source_provided();
    if      (strcmp(lang_str, "C")   == 0) lang = Language_C;
    else if (strcmp(lang_str, "C++") == 0) lang = Language_Cpp;

    if (lang == Language_Invalid) {
        nob_log(ERROR, "Language not supported (lang = %s)", lang_str);
        return 1;
    }

    nob_log(INFO, "binding generation not yet supported :(");
    return 0;
}

int main(int argc, char** argv)
{
    if (argc <= 1) return help(0, 0);
    argc -= 2;
    argv += 2;
    stdout_writer = kai_writer_stdout();
    kai_memory_create(&allocator);
    if (strcmp(argv[-1], "token"  ) == 0) return token(argc, argv);
    if (strcmp(argv[-1], "parse"  ) == 0) return parse(argc, argv);
    if (strcmp(argv[-1], "compile") == 0) return compile(argc, argv);
    if (strcmp(argv[-1], "bind"   ) == 0) return bind(argc, argv);
    if (strcmp(argv[-1], "help"   ) == 0
    ||  strcmp(argv[-1], "--help" ) == 0
    ||  strcmp(argv[-1], "-?"     ) == 0
    ||  strcmp(argv[-1], "-h"     ) == 0) help(argc, argv), exit(0); // No error
    if (strcmp(argv[-1], "version") == 0)
    {
        Kai_string version = kai_version_string();
        printf("\x1b[1;97mKai v%.*s\x1b[0;90m (build %s)\x1b[0m\n",
            (int)(version.count), version.data, S2(KAI_BUILD_DATE));
        exit(0);
    }
    return help(0, 0);
}
