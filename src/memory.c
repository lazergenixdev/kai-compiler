#define KAI_USE_MEMORY_API
#include "config.h"
#include "stdlib.h" // -> malloc, free

#define KAI_PLATFORM_UNIX     0
#define KAI_PLATFORM_WINDOWS  1

#if defined(_WIN32)
#   define KAI_CURRENT_PLATFORM  KAI_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__APPLE__)
#   define KAI_CURRENT_PLATFORM  KAI_PLATFORM_UNIX
#else
#   error "unknown platform"
#endif

typedef struct {
    Kai_u32 debug_level;
    Kai_u32 total_allocated;
} Kai__Memory_Internal;

#if KAI_CURRENT_PLATFORM == KAI_PLATFORM_UNIX
#include <sys/mman.h> // -> mmap
#include <unistd.h>   // -> getpagesize

void* kai__memory_allocate(Kai_ptr user, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ)?    PROT_READ  : 0;
    flags |= (access & KAI_MEMORY_ACCESS_WRITE)?   PROT_WRITE : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)? PROT_EXEC  : 0;
    void* ptr = mmap(NULL, size, flags, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    Kai__Memory_Internal* internal = user;
    internal->total_allocated += size;
    return ptr;
}

void kai__memory_free(Kai_ptr user, Kai_ptr ptr, Kai_u32 size)
{
    munmap(ptr, size);
    Kai__Memory_Internal* internal = user;
    internal->total_allocated -= size;
}

void kai__memory_set_access(Kai_ptr user, Kai_ptr ptr, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ)?    PROT_READ  : 0;
    flags |= (access & KAI_MEMORY_ACCESS_WRITE)?   PROT_WRITE : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)? PROT_EXEC  : 0;
    mprotect(ptr, size, flags);
}

#define kai__page_size() sysconf(_SC_PAGESIZE)

#elif KAI_CURRENT_PLATFORM == KAI_PLATFORM_WINDOWS
typedef unsigned long  DWORD;
typedef int            BOOL;
typedef DWORD         *PDWORD;
typedef void          *LPVOID;
typedef uintptr_t      SIZE_T;
#define WINAPI __stdcall

LPVOID WINAPI VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
BOOL WINAPI VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
BOOL WINAPI VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
typedef struct {
	DWORD dwOemId;
	DWORD dwPageSize;
	void* padding[8];
} SYSTEM_INFO;
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
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ_WRITE)? 0x04 : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)?    0x10 : 0;
    VirtualProtect(ptr, size, flags, NULL);
}

Kai_u32 kai__page_size()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (Kai_u32)info.dwPageSize;
}

#endif

void kai_create_memory(Kai_Memory_Allocator* Memory)
{
    Memory->allocate = kai__memory_allocate;
    Memory->free = kai__memory_free;
    Memory->set_access = kai__memory_set_access;
    Memory->page_size = kai__page_size();
    Memory->user = malloc(sizeof(Kai__Memory_Internal));

    if (!Memory->user) {
        panic_with_message("malloc failed!");
    }

    Kai__Memory_Internal* internal = Memory->user;
    internal->debug_level = KAI_MEMORY_DEBUG_OFF;
    internal->total_allocated = 0;
}

void kai_destroy_memory(Kai_Memory_Allocator* Memory)
{
    Kai__Memory_Internal* internal = Memory->user;
    if (internal->total_allocated) {
        panic_with_message("Some allocations were not freed! (amount=%u B)", internal->total_allocated);
    }
    free(Memory->user);
    *Memory = (Kai_Memory_Allocator) {};
}

void kai_memory_set_debug(Kai_Memory_Allocator* Memory, Kai_u32 debug_level)
{
    Kai__Memory_Internal* internal = Memory->user;
    internal->debug_level = debug_level;
}

Kai_u32 kai_memory_usage(Kai_Memory_Allocator* Memory)
{
    Kai__Memory_Internal* internal = Memory->user;
    return internal->total_allocated;
}
