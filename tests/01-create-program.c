#include "test.h"

#define MAKE_SLICE(L) {.data = L, .count = sizeof(L)/sizeof(L[0])}

int main()
{
    Kai_Program program = {0};
    Kai_Source sources[1] = {
        example_simple,
    };
    Kai_Program_Create_Info info = {
        .allocator = default_allocator(),
        .error = default_error(),
        .sources = MAKE_SLICE(sources),
    };
    kai_create_program(&info, &program);
    assert_no_error();

    kai_find_variable(&program, KAI_STRING("A"));
}