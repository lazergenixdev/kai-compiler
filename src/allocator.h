#include "kai/kai.h"

void kai__create_dynamic_arena_allocator(Kai__Dynamic_Arena_Allocator* arena, Kai_Memory_Allocator* allocator);
void kai__destroy_dynamic_arena_allocator(Kai__Dynamic_Arena_Allocator* arena);
void* kai__arena_allocate(Kai__Dynamic_Arena_Allocator* arena, Kai_u32 size);

typedef struct {
    int a;
} Kai__Fixed_Arena_Allocator;

typedef struct {
    int a;
} Kai__Heap_Allocator;
