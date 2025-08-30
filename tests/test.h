#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../3rd-party/nob.h"
#define KAI_IMPLEMENTATION
#include "../kai.h"

#define assert_true(EXPR) if (!(EXPR)) printf("\e[91mTest Failed\e[0m: %s (\e[92m%s:%i\e[0m)\n", #EXPR, __FILE__, __LINE__), exit(1)
#define assert_no_error() \
  if (default_error()->result != KAI_SUCCESS) \
    printf("\e[91mTest Failed\e[0m: (\e[92m%s:%i\e[0m)\n", __FILE__, __LINE__), kai_write_error(default_writer(), default_error()), exit(1)

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

#define EXAMPLE(NAME,SOURCE) \
  Kai_Source example_##NAME = {.name = KAI_STRING("example_" #NAME),.contents = KAI_STRING(SOURCE)}

EXAMPLE(simple,
    "A :: 123;\n"
    "B :: A + 1;\n"
    "C :: A - B;\n"
    "D :: B + C;\n"
    "\n"
);
