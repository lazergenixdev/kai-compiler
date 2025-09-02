#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../3rd-party/nob.h"
#define KAI_IMPLEMENTATION
#include "../kai.h"

#define FAIL(FMT,...) printf("\x1b[91mTest Failed\x1b[0m: " FMT " (\x1b[92m%s:%i\x1b[0m)\n", __VA_ARGS__, __FILE__, __LINE__), exit(1)
#define assert_true(EXPR) if (!(EXPR)) printf("\x1b[91mTest Failed\x1b[0m: %s (\x1b[92m%s:%i\x1b[0m)\n", #EXPR, __FILE__, __LINE__), exit(1)
#define assert_no_error() \
  if (default_error()->result != KAI_SUCCESS) \
    printf("\x1b[91mTest Failed\x1b[0m: (\x1b[92m%s:%i\x1b[0m)\n", __FILE__, __LINE__), kai_write_error(default_writer(), default_error()), exit(1)

#define MAKE_SLICE(L) {.data = L, .count = sizeof(L)/sizeof(L[0])}

static inline Kai_Allocator default_allocator()
{
    Kai_Allocator allocator = {0};
    kai_memory_create(&allocator);
    return allocator;
}
static inline Kai_Error* default_error()
{
    static Kai_Error error = {0};
    return &error;
}
static inline Kai_Writer* default_writer()
{
    static Kai_Writer writer = {0};
    writer = kai_writer_stdout();
    return &writer;
}

static inline Kai_Source load_source_file(const char* path)
{
	String_Builder builder = {0};
	
	if (!read_entire_file(path, &builder))
		FAIL("Failed to read \"%s\"", path);
	
	return (Kai_Source) {
		.name = kai_string_from_c(path),
		.contents = {
			.data = (Kai_u8*)(builder.items),
			.count = (Kai_u32)(builder.count)
		}
	};
}
