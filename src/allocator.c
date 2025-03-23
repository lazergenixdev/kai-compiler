#include "kai/kai.h"
#include "config.h"

#define KAI__MINIMUM_ARENA_BUCKET_SIZE 0x40000

/// @return The smallest number number n such that n*m >= minimum
Kai_u32 kai__closest_multiple(Kai_u32 minimum, Kai_u32 m) {
    Kai_u32 t = minimum / m;
    return (t*m < minimum)? t + 1 : t;
}

void kai__create_dynamic_arena_allocator(Kai__Dynamic_Arena_Allocator* arena, Kai_Memory_Allocator* allocator) {
    arena->allocator = *allocator;
    arena->bucket_size = kai__closest_multiple(KAI__MINIMUM_ARENA_BUCKET_SIZE, allocator->page_size) * allocator->page_size;
    arena->current_allocated = sizeof(Kai__Arena_Bucket*);
    arena->current_bucket = allocator->allocate(allocator->user, arena->bucket_size, KAI_MEMORY_ACCESS_READ_WRITE);
}

void kai__dynamic_arena_allocator_free_all(Kai__Dynamic_Arena_Allocator* arena) {
    Kai__Arena_Bucket* bucket = arena->current_bucket;
    while (bucket) {
        Kai__Arena_Bucket* prev = bucket->prev;
        arena->allocator.free(arena->allocator.user, bucket, arena->bucket_size);
        bucket = prev;
    }
    arena->current_bucket = 0;
    arena->current_allocated = 0;
}

void kai__destroy_dynamic_arena_allocator(Kai__Dynamic_Arena_Allocator* arena) {
    kai__dynamic_arena_allocator_free_all(arena);
    arena->bucket_size = 0;
    arena->allocator = (Kai_Memory_Allocator) {};
}

void* kai__arena_allocate(Kai__Dynamic_Arena_Allocator* arena, Kai_u32 size) {    
    if (size > arena->bucket_size) return 0;
    
    if (arena->current_allocated + size > arena->bucket_size) {
        Kai__Arena_Bucket* new_bucket = arena->allocator.allocate(arena->allocator.user, arena->bucket_size, KAI_MEMORY_ACCESS_READ_WRITE);
        if (new_bucket == 0) return 0; // Bubble down failure
        new_bucket->prev = arena->current_bucket;
        arena->current_bucket = new_bucket;
        arena->current_allocated = sizeof(Kai__Arena_Bucket*);
    }

    Kai_u8* bytes = (Kai_u8*)arena->current_bucket;
    void* ptr = bytes + arena->current_allocated;
    arena->current_allocated += size;
    return ptr;
}