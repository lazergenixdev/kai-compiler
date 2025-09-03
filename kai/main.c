// Command line utility

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../3rd-party/nob.h"
#define KAI_IMPLEMENTATION
#include "../kai.h"

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

int help(int argc, char** argv)
{
    printf("help command %i %p\n", argc, argv);
    return 0;
}

int token(int argc, char** argv)
{
    if (argc == 0) return help(argc, argv);
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
    if (argc == 0) return help(argc, argv);
    Kai_Error error = {0};
	Kai_Syntax_Tree tree = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = allocator,
		.error = &error,
		.source = load_source_file(argv[0]),
	};
	kai_create_syntax_tree(&info, &tree);
	kai_write_expression(writer, (Kai_Expr*)&tree.root);
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
    if (strcmp(argv[-1], "token") == 0)
        return token(argc, argv);
    if (strcmp(argv[-1], "parse") == 0)
        return parse(argc, argv);
    return help(argc, argv);
}