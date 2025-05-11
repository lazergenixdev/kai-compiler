#define KAI_USE_MEMORY_API
#include "config.h"

typedef struct {
    Kai_u32 debug_level;
    Kai_u32 total_allocated;
} Kai__Memory_Internal;

#if defined(KAI__PLATFORM_LINUX) || defined(KAI__PLATFORM_APPLE)
#include <sys/mman.h> // -> mmap
#include <unistd.h>   // -> getpagesize

void* kai__memory_allocate(Kai_ptr user, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ)?    PROT_READ  : 0;
    flags |= (access & KAI_MEMORY_ACCESS_WRITE)?   PROT_WRITE : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)? PROT_EXEC  : 0;
    void* ptr = mmap(NULL, size, flags, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
    Kai__Memory_Internal* internal = user;
    if (internal->debug_level == KAI_MEMORY_DEBUG_VERBOSE)
    {
        printf("[KAI] Allocated %i bytes\n", size);
    }
    internal->total_allocated += size;
    return (ptr == MAP_FAILED) ? NULL : ptr;
}

void kai__memory_free(Kai_ptr user, Kai_ptr ptr, Kai_u32 size)
{
    munmap(ptr, size);
    Kai__Memory_Internal* internal = user;
    if (internal->debug_level == KAI_MEMORY_DEBUG_VERBOSE)
    {
        printf("[KAI] Freed %i bytes\n", size);
    }
    internal->total_allocated -= size;
}

void kai__memory_set_access(Kai_ptr user, Kai_ptr ptr, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ)?    PROT_READ  : 0;
    flags |= (access & KAI_MEMORY_ACCESS_WRITE)?   PROT_WRITE : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)? PROT_EXEC  : 0;
    mprotect(ptr, size, flags);
    Kai__Memory_Internal* internal = user;
    if (internal->debug_level == KAI_MEMORY_DEBUG_VERBOSE)
    {
        printf("[KAI] Changed Access of [%p,%p) to %x\n", ptr, (Kai_u8*)ptr + size, access);
    }
}

#define kai__page_size() sysconf(_SC_PAGESIZE)

#elif defined(KAI__PLATFORM_WINDOWS)
typedef unsigned long  DWORD;
typedef int            BOOL;
typedef DWORD         *PDWORD;
typedef void          *LPVOID;
typedef uintptr_t      SIZE_T;
#define WINAPI __stdcall
typedef struct {
	DWORD dwOemId;
	DWORD dwPageSize;
	void* padding[8];
} SYSTEM_INFO;

LPVOID WINAPI VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
BOOL WINAPI VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
BOOL WINAPI VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
void GetSystemInfo(SYSTEM_INFO* lpSystemInfo);

void* kai__memory_allocate(Kai_ptr user, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ_WRITE)? 0x04 : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)?    0x10 : 0;
    void* ptr = VirtualAlloc(NULL, size, 0x1000|0x2000, flags);
    if (!ptr) return NULL;
    Kai__Memory_Internal* internal = user;
    internal->total_allocated += size;
    return ptr;
}

void kai__memory_free(Kai_ptr user, Kai_ptr ptr, Kai_u32 size)
{
    VirtualFree(ptr, 0, 0);
    Kai__Memory_Internal* internal = user;
    internal->total_allocated -= size;
}

void kai__memory_set_access(Kai_ptr user, Kai_ptr ptr, Kai_u32 size, Kai_u32 access)
{
	(void)user;
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ_WRITE)? 0x04 : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)?    0x10 : 0;
	DWORD old;
    BOOL r = VirtualProtect(ptr, size, flags, &old);
	printf("virtual protect with %i -> %i\n", flags, r);
}

Kai_u32 kai__page_size()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (Kai_u32)info.dwPageSize;
}
#else
extern void* __wasm_allocate(Kai_u32 size);
extern void* __wasm_free(void* ptr);

void* kai__memory_allocate(Kai_ptr user, Kai_u32 size, Kai_u32 access)
{
    __wasm_console_log("kai__memory_allocate", size);
    return __wasm_allocate(size);
}

void kai__memory_free(Kai_ptr user, Kai_ptr ptr, Kai_u32 size)
{
    __wasm_console_log("kai__memory_free", size);
    __wasm_free(ptr);
}

void kai__memory_set_access(Kai_ptr user, Kai_ptr ptr, Kai_u32 size, Kai_u32 access)
{
    __wasm_console_log("kai__memory_set_access", size);
}

Kai_u32 kai__page_size()
{
    return 64 * 1024; // 64 KiB
}
#endif

#ifndef __WASM__
#include "stdlib.h" // -> malloc, free

static void* kai__memory_heap_allocate(void* user, void* old_ptr, Kai_u32 new_size, Kai_u32 old_size)
{
    void* ptr = NULL;
    if (new_size == 0) {
        free(old_ptr);
    } else {
        kai__assert(old_ptr != NULL || old_size == 0);
        ptr = realloc(old_ptr, new_size);
        if (ptr != NULL && old_ptr == NULL) {
            kai__memory_zero(ptr, new_size);
        }
    }
    Kai__Memory_Internal* internal = user;
    if (internal->debug_level == KAI_MEMORY_DEBUG_VERBOSE)
    {
        printf("[KAI] Heap allocated %i bytes\n", (int)new_size - (int)old_size);
    }
    internal->total_allocated += new_size;
    internal->total_allocated -= old_size;
    return ptr;
}
#else
static Kai_u8* scratch = 0;
static Kai_u32 offset = 0;
static void* kai__memory_heap_allocate(void* user, void* old_ptr, Kai_u32 new_size, Kai_u32 old_size)
{
    Kai_u32 ptr = offset;
    offset += new_size;
    for (int i = 0; i < old_size; ++i) {
        scratch[ptr + i] = ((Kai_u8*)old_ptr)[i];
    }
    __wasm_console_log("heap allocated", new_size);
    return scratch + ptr;
}
#endif

Kai_Result kai_create_memory(Kai_Allocator* Memory)
{
    kai__assert(Memory != 0);
	
    Memory->allocate      = kai__memory_allocate;
    Memory->free          = kai__memory_free;
    Memory->heap_allocate = kai__memory_heap_allocate;
    Memory->set_access    = kai__memory_set_access;
    Memory->page_size     = kai__page_size();
#ifndef __WASM__
	Memory->user          = malloc(sizeof(Kai__Memory_Internal));
#else
	scratch = __wasm_allocate(kai__page_size() * 2);
	offset = 0;
    Memory->user          = 0;
#endif
    if (!Memory->user) {
		return KAI_MEMORY_ERROR_OUT_OF_MEMORY;
    }

    Kai__Memory_Internal* internal = Memory->user;
    internal->debug_level = KAI_MEMORY_DEBUG_OFF;
    internal->total_allocated = 0;
	return KAI_SUCCESS;
}

Kai_Result kai_destroy_memory(Kai_Allocator* Memory)
{
    Kai__Memory_Internal* internal = Memory->user;
    if (internal->total_allocated != 0) {
		return KAI_MEMORY_ERROR_MEMORY_LEAK;
    }
    #ifndef __WASM__
    free(Memory->user);
    #endif
    *Memory = (Kai_Allocator) {0};
	return KAI_SUCCESS;
}

void kai_memory_set_debug(Kai_Allocator* Memory, Kai_u32 debug_level)
{
    Kai__Memory_Internal* internal = Memory->user;
    internal->debug_level = debug_level;
}

Kai_u32 kai_memory_usage(Kai_Allocator* Memory)
{
    Kai__Memory_Internal* internal = Memory->user;
    return internal->total_allocated;
}
