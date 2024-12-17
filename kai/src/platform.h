#ifndef PLATFORM_H
#define KAI_PLATFORM_UNIX 1
#define KAI_PLATFORM_WINDOWS 0

// API:
//     os_allocate(size, flags) -> void* | OS_FAILED
//     os_free(data, size) -> void
//     os_page_size() -> int
//
//     ALLOCATE FLAGS:
//         OS_READ
//         OS_WRITE
//         OS_EXECUTE

///////////////////////////////////////////////////////////////////////////////
// IMPLEMENATION:

//-----------------------------------------------------------------------------
#if KAI_PLATFORM_UNIX
#include <sys/mman.h> // -> mmap
#include <unistd.h>   // -> getpagesize
#define OS_FAILED     MAP_FAILED
#define OS_READ_WRITE PROT_READ|PROT_WRITE
#define OS_EXECUTE    PROT_EXEC

#define os_allocate(SIZE, FLAGS) \
    mmap(NULL, SIZE, FLAGS, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0)
#define os_free(DATA, SIZE) \
    munmap(DATA, SIZE)
#define os_page_size() \
    sysconf(_SC_PAGESIZE)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
#elif KAI_PLATFORM_WINDOWS
// I want FAST compilation speeds,
// so including Windows.h just isn't an option.
typedef unsigned long  DWORD;
typedef int            BOOL;
typedef DWORD         *PDWORD;
typedef void          *LPVOID;
typedef uintptr_t      SIZE_T;
#define WINAPI __stdcall

LPVOID WINAPI VirtualAlloc(
	LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD flAllocationType,
    DWORD flProtect
);
BOOL WINAPI VirtualProtect(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
);
BOOL WINAPI VirtualFree(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD dwFreeType
);
typedef struct {
	DWORD dwOemId;
	DWORD dwPageSize;
	void* padding[8];
} SYSTEM_INFO;
void GetSystemInfo(SYSTEM_INFO* lpSystemInfo);

#define OS_FAILED     NULL
#define OS_READ_WRITE 0x04
#define OS_EXECUTE    0x10

#include <stdlib.h>
#define os_allocate(SIZE, FLAGS) \
    VirtualAlloc(NULL, SIZE, 0x1000|0x2000, FLAGS)
#define os_free(DATA, SIZE) \
    VirtualFree(DATA, 0, 0)
static Kai_u64 os_page_size() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (Kai_u64)info.dwPageSize;
}

//-----------------------------------------------------------------------------

#else
#    error "what platform are we???????"
#endif // KAI_PLATFORM_*
#endif // PLATFORM_H

