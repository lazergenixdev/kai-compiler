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

Kai_string load_file_to_string(const char* path)
{
	String_Builder builder = {0};
	read_entire_file(path, &builder);
	return (Kai_string) {.data = (Kai_u8*)(builder.items), .count = builder.count};
}

int help(int argc, char** argv)
{
    printf("help command %i %p\n", argc, argv);
    return 0;
}

int parse(int argc, char** argv)
{
    if (argc == 0) return help(argc, argv);

	Kai_Writer writer = kai_writer_stdout();
    Kai_Allocator allocator;
	kai_memory_create(&allocator);

    Kai_Error error = {0};
	Kai_Syntax_Tree tree = {0};
	Kai_Syntax_Tree_Create_Info info = {
		.allocator = allocator,
		.error = &error,
		.source = {
            .name = KAI_STRING("[internal]"),
            .contents = load_file_to_string(argv[0]),
        },
	};
	if (kai_create_syntax_tree(&info, &tree) != KAI_SUCCESS) {
        kai_write_error(&writer, &error);
    }
	kai_write_expression(&writer, (Kai_Expr*)&tree.root);
	return error.result != KAI_SUCCESS;
}

int main(int argc, char** argv)
{
    if (argc <= 1) return help(argc, argv);
    argc -= 2;
    argv += 2;
    if (strcmp(argv[-1], "parse") == 0)
        return parse(argc, argv);
    return help(argc, argv);
}