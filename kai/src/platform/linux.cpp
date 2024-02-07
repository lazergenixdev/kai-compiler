#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>
#include <kai/memory.h>
#include "../config.hpp"
#include "../program.hpp"

static constexpr kai_int minimum_temerary_memory_size = 1024 * 1;
static constexpr kai_int minimum_bucket_size          = 1024 * 1;

struct Memory_Bucket {
	kai_ptr data;
	static kai_int _size;

	Memory_Bucket(kai_int Size) {
		_size = Size;
		data = mmap(nullptr, Size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		assert(data != MAP_FAILED, "error: failed to allocate memory [mmap]");
	}

	~Memory_Bucket() {
		munmap(data, _size);
	}
};
kai_int Memory_Bucket::_size = 0;

struct Memory_Impl {
	std::vector<Memory_Bucket> buckets;

	kai_int page_size;
	kai_int temp_num_pages;
	kai_int bucket_num_pages;

	kai_u32 bucket_allocated_size; // number of bytes allocated on the last bucket by the compiler
	kai_u32 bucket_index;


	Memory_Impl(kai_int ps, kai_int tnp):
		page_size(ps), temp_num_pages(tnp)
	{
		buckets.reserve(1024);
		
		bucket_num_pages = minimum_bucket_size / page_size;
		if( bucket_num_pages * page_size < minimum_bucket_size ) {
			++bucket_num_pages;
		}
	//	printf("DEBUG: %i %i\n", bucket_num_pages, page_size);

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

kai_ptr allocate_linux(kai_ptr user, kai_int Size) {
	#if 0
	return malloc(Size);
	#else
	auto impl = reinterpret_cast<Memory_Impl*>(user);

	// If the size we are allocating is bigger than the bucket size, we are screwed!
	assert(Size < impl->bucket_num_pages * impl->page_size, "error: bucket is too small!");

	// we want a new bucket
	if( impl->bucket_allocated_size + Size > impl->bucket_num_pages * impl->page_size ) {
		impl->alloc_bucket();
	}

	auto ptr = (kai_u8*)impl->buckets[impl->bucket_index].data + impl->bucket_allocated_size;
	impl->bucket_allocated_size += (kai_u32)Size;
	return ptr;
	#endif
}

void kai_create_memory(kai_Memory* mem) {
	mem->user           = new Memory_Impl(getpagesize(), 0);
	mem->temperary_size = 10'000;
	mem->temperary      = malloc(mem->temperary_size);
	mem->alloc          = &allocate_linux;
}


void kai_reset_memory( kai_Memory* mem ) {
	
}

void kai_destroy_memory( kai_Memory* mem ) {
	auto impl = reinterpret_cast<Memory_Impl*>(mem->user);
	std::destroy_at(impl);

	free(mem->temperary);
	mem->alloc     = nullptr;
	mem->temperary = nullptr;
	mem->user      = nullptr;
	mem->temperary_size = 0;
}

kai_u64 kai_memory_usage(kai_Memory* mem) {
	kai_u64 total = 0;
	return total;
}

kai_Program init_program(Machine_Code code) {
	auto program = new kai_Program_Impl;

	void* mem = mmap(
		nullptr,
		1 << 16,
		PROT_WRITE, // only want write access
		MAP_ANONYMOUS | MAP_PRIVATE,
		0,
		0
	);

	if(mem == MAP_FAILED)
		panic_with_message("failed to allocate memory [error = %i]", errno);

	memcpy(mem, code.data, code.size);

	program->executable_memory      = mem;
	program->executable_memory_size = code.size;

	if(0 != mprotect(mem, code.size, PROT_EXEC))
		panic_with_message("mprotect failed [error = %i]", errno);

	return program;
}

void kai_destroy_program(kai_Program program)
{
	munmap(program->executable_memory, program->executable_memory_size);
	delete program;
}
