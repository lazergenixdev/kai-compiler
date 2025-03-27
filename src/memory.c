#define KAI_USE_MEMORY_API
#include "config.h"
#include "stdlib.h" // -> malloc, free

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

Kai_Result kai_create_memory(Kai_Memory_Allocator* Memory)
{
    Memory->allocate = kai__memory_allocate;
    Memory->free = kai__memory_free;
    Memory->set_access = kai__memory_set_access;
    Memory->page_size = kai__page_size();
    Memory->user = malloc(sizeof(Kai__Memory_Internal));

    if (!Memory->user) {
		return KAI_MEMORY_ERROR_OUT_OF_MEMORY;
    }

    Kai__Memory_Internal* internal = Memory->user;
    internal->debug_level = KAI_MEMORY_DEBUG_OFF;
    internal->total_allocated = 0;
	return KAI_SUCCESS;
}

Kai_Result kai_destroy_memory(Kai_Memory_Allocator* Memory)
{
    Kai__Memory_Internal* internal = Memory->user;
    if (internal->total_allocated != 0) {
		//  panic_with_message("Some allocations were not freed! (amount=%u B)", internal->total_allocated);
		return KAI_MEMORY_ERROR_MEMORY_LEAK;
    }
    free(Memory->user);
    *Memory = (Kai_Memory_Allocator) {0};
	return KAI_SUCCESS;
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
