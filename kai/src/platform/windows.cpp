#include <kai/memory.h>
#include <vector>
#include <cassert>

// TODO: Memory allocater kinda trash, there is a fixed upper bound on the max allocation size, which is trash
// TODO: Error handling would be kinda good?

// Windows Garbage:
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS     - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES - VK_*
#define NOWINMESSAGES     - WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      - SM_*
#define NOMENUS           - MF_*
#define NOICONS           - IDI_*
#define NOKEYSTATES       - MK_*
#define NOSYSCOMMANDS     - SC_*
#define NORASTEROPS       - Binary and Tertiary raster ops
#define NOSHOWWINDOW      - SW_*
#define OEMRESOURCE       - OEM Resource values
#define NOATOM            - Atom Manager routines
#define NOCLIPBOARD       - Clipboard routines
#define NOCOLOR           - Screen colors
#define NOCTLMGR          - Control and Dialog routines
#define NODRAWTEXT        - DrawText() and DT_*
#define NOGDI             - All GDI defines and routines
#define NOKERNEL          - All KERNEL defines and routines
#define NOUSER            - All USER defines and routines
#define NONLS             - All NLS defines and routines
#define NOMB              - MB_* and MessageBox()
#define NOMEMMGR          - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        - typedef METAFILEPICT
#define NOMINMAX          - Macros min(a,b) and max(a,b)
#define NOMSG             - typedef MSG and associated routines
#define NOOPENFILE        - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          - SB_* and scrolling routines
#define NOSERVICE         - All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           - Sound driver routines
#define NOTEXTMETRIC      - typedef TEXTMETRIC and associated routines
#define NOWH              - SetWindowsHook and WH_*
#define NOWINOFFSETS      - GWL_*, GCL_*, associated routines
#define NOCOMM            - COMM driver routines
#define NOKANJI           - Kanji support stuff.
#define NOHELP            - Help engine interface.
#define NOPROFILER        - Profiler interface.
#define NODEFERWINDOWPOS  - DeferWindowPos routines
#define NOMCX             - Modem Configuration Extensions
#include <Windows.h>

static constexpr kai_int minimum_temerary_memory_size = 1024 * 256;
static constexpr kai_int minimum_bucket_size          = 1024 * 256;

struct Memory_Bucket {
	kai_ptr data;

	Memory_Bucket(kai_int Size) {
		data = VirtualAlloc(nullptr, Size, MEM_COMMIT, PAGE_READWRITE);
		assert(data != nullptr);
	}

	~Memory_Bucket() {
		auto result = VirtualFree(data, 0, MEM_RELEASE);
		assert(result != 0);
	}
};

struct Memory_Win32_Impl {
	std::vector<Memory_Bucket> buckets;

	kai_int page_size;
	kai_int temp_num_pages;
	kai_int bucket_num_pages;

	kai_u32 bucket_allocated_size; // number of bytes allocated on the last bucket by the compiler
	kai_u32 bucket_index;


	Memory_Win32_Impl(kai_int ps, kai_int tnp):
		page_size(ps), temp_num_pages(tnp)
	{
		buckets.reserve(1024);
		
		bucket_num_pages = minimum_bucket_size / page_size;
		if( bucket_num_pages * page_size < minimum_bucket_size ) {
			++bucket_num_pages;
		}

		buckets.emplace_back(bucket_num_pages * page_size);
		bucket_allocated_size = 0;
		bucket_index = 0;
	}

	void alloc_bucket() {
		++bucket_index;
		while (buckets.size() <= bucket_index)
			buckets.emplace_back(bucket_num_pages * page_size);
		bucket_allocated_size = 0;
	}
};


kai_ptr allocate_win32(kai_ptr user, kai_int Size) {
	auto impl = reinterpret_cast<Memory_Win32_Impl*>(user);

	// If the size we are allocating is bigger than the bucket size, we are screwed!
	assert(Size < impl->bucket_num_pages * impl->page_size);
//	if (Size < impl->bucket_num_pages * impl->page_size) return malloc(Size);

	// we want a new bucket
	if( impl->bucket_allocated_size + Size > impl->bucket_num_pages * impl->page_size ) {
		impl->alloc_bucket();
	}

	auto ptr = (kai_u8*)impl->buckets[impl->bucket_index].data + impl->bucket_allocated_size;
	impl->bucket_allocated_size += (kai_u32)Size;
	return ptr;
}

void kai_create_memory(kai_Memory* mem) {
	kai_int page_size;
	{
		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);
		page_size = sys_info.dwPageSize;
	}

	auto temp_num_pages = minimum_temerary_memory_size / page_size;

	// make temp size bigger if it is smaller than the minimum
	if( temp_num_pages * page_size < minimum_temerary_memory_size ) {
		++temp_num_pages;
	}

	// Allocate user memory
	void* user = VirtualAlloc(nullptr, temp_num_pages * page_size, MEM_COMMIT, PAGE_READWRITE);

	if(!user) {
		// we die now :)
		exit(1);
		return;
	}

	// Init our memory implementation
	new(user) Memory_Win32_Impl(page_size, temp_num_pages);

	mem->user           = user;
	mem->temperary      = (kai_u8*)user + sizeof(Memory_Win32_Impl);
	mem->temperary_size = temp_num_pages * page_size - sizeof(Memory_Win32_Impl);
	mem->alloc          = &allocate_win32;
}


void kai_reset_memory( kai_Memory* mem ) {
	auto impl = reinterpret_cast<Memory_Win32_Impl*>(mem->user);
	impl->bucket_index          = 0;
	impl->bucket_allocated_size = 0;
}

void kai_destroy_memory( kai_Memory* mem ) {
	if( mem->user == nullptr ) return; // our work is done!

	auto impl = reinterpret_cast<Memory_Win32_Impl*>(mem->user);

	// call destructor
	std::destroy_at(impl);

	auto result = VirtualFree(mem->user, 0, MEM_RELEASE);

	assert(result != 0);

	mem->alloc     = nullptr;
	mem->temperary = nullptr;
	mem->user      = nullptr;
	mem->temperary_size = 0;
}

kai_u64 kai_memory_usage(kai_Memory* mem)
{
	auto impl = reinterpret_cast<Memory_Win32_Impl*>(mem->user);

	kai_u64 total = 0;
	if (impl->buckets.size() > 1) {
		total = impl->bucket_num_pages * impl->page_size * (impl->buckets.size() - 1);
	}
	total += impl->bucket_allocated_size;

	return total;
}

#include "../program.hpp"

kai_Program init_program(Machine_Code code) {
	auto program = new kai_Program_Impl;

	kai_int allocation_size;
	{
		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);
		kai_int page_size = sys_info.dwPageSize;

		allocation_size = ((code.size / page_size) + 1) * page_size;
	}

	// allocate memory
	auto mem = VirtualAlloc(nullptr, allocation_size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

	assert(mem != nullptr);

	// write into the memory
	memcpy(mem, code.data, code.size);

	// make memory Execute only
	DWORD _;
	SIZE_T dwSize = allocation_size;
	auto r = VirtualProtect(mem, dwSize, PAGE_EXECUTE, &_);

	assert(r != 0);

	program->executable_memory      = mem;
	program->executable_memory_size = allocation_size;

	FlushInstructionCache(GetModuleHandle(nullptr), nullptr, 0);

	return program;
}

void kai_destroy_program(kai_Program program)
{
	auto result = VirtualFree(program->executable_memory, 0, MEM_RELEASE);

	assert(result != 0);

	delete program;
}
