#include <kai/memory.h>
#include <string.h> // --> memcpy
#include "platform.h"
#include "config.h"

#define MINIMUM_TEMP_MEMORY_SIZE 1024 * 64
#define MINIMUM_BUCKET_SIZE      1024 * 64

typedef struct Memory_Bucket {
    struct Memory_Bucket* prev;
} Memory_Bucket;

typedef struct Memory_Impl {
    Memory_Bucket* current_bucket;
    u64            current_allocated;

    u64 page_size;
    u32 temp_page_count;
    u32 bucket_page_count;
    u32 bucket_count;
} Memory_Impl;

#define allocate_bucket(I) \
    os_allocate(I->page_size*I->bucket_page_count, OS_READ_WRITE)

Kai_ptr memory_allocate(Kai_ptr user, Kai_int size) {
    Memory_Impl* impl = (Memory_Impl*)user;
    
    if (impl->current_allocated + size > impl->bucket_page_count * impl->page_size) {
        Memory_Bucket* new_bucket = allocate_bucket(impl);
    	if (new_bucket == OS_FAILED) panic_with_message("Failed to allocate memory!");
        new_bucket->prev = impl->current_bucket;
        impl->current_bucket = new_bucket;
        impl->current_allocated = sizeof(Memory_Bucket*);
        impl->bucket_count += 1;
    }

    Kai_ptr p = (u8*)impl->current_bucket + impl->current_allocated;
    impl->current_allocated += size;
    return p;
}

//! @return: The smallest number number n such that n*m >= minimum
u64 closest_multiple(u64 minimum, u64 m) {
    u64 t = minimum / m;
    return (t*m < minimum)? t + 1 : t;
}

void kai_create_memory(Kai_Memory* out) {
	u64 page_size = os_page_size();
	u64 temp_page_count = closest_multiple(MINIMUM_TEMP_MEMORY_SIZE + sizeof(Memory_Impl), page_size);
    Memory_Impl* impl = (Memory_Impl*)os_allocate(temp_page_count*page_size, OS_READ_WRITE);
    if (impl == OS_FAILED) panic_with_message("Failed to allocate memory!");
    impl->page_size = page_size;
	impl->temp_page_count = temp_page_count;
    impl->bucket_page_count = closest_multiple(MINIMUM_BUCKET_SIZE, page_size);
    impl->bucket_count = 1;
    impl->current_bucket = allocate_bucket(impl);
    if (impl->current_bucket == OS_FAILED) panic_with_message("Failed to allocate memory!");
    impl->current_bucket->prev = NULL;
    impl->current_allocated = sizeof(Memory_Bucket*);

    out->user           = impl;
    out->temperary_size = temp_page_count*page_size - sizeof(impl);
    out->temperary      = impl + sizeof(impl);
    out->alloc          = &memory_allocate;
}

void kai_reset_memory(Kai_Memory* mem) {
    (void)mem;
    UNIMPLEMENTED();
}

#define zero_memory(P) \
    memset(P, 0, sizeof(*P))

void kai_destroy_memory(Kai_Memory* mem) {
    if (mem->user) {
        Memory_Impl* impl = mem->user;
        Memory_Bucket* bucket = impl->current_bucket;
        while (bucket) {
            Memory_Bucket* next = bucket->prev;
            os_free(bucket, impl->page_size*impl->bucket_page_count);
            bucket = next;
        }
        os_free(mem->user, mem->temperary_size + sizeof(Memory_Impl));
    }
    zero_memory(mem);
}

u64 kai_memory_usage(Kai_Memory* mem) {
    if (!mem) return 0;
    Memory_Impl* impl = mem->user;
    return impl->bucket_count*impl->bucket_page_count*impl->page_size;
}

