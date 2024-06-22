#include <Windows.h>
#include "kai/kai.h"
#include "kai/memory.h"
#include "kai/debug.h"

#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <chrono>
using namespace std::chrono_literals;

inline double time_took_ms = 0.0;
using namespace std::chrono;

struct timer {
	high_resolution_clock::time_point start;
	timer() {
		start = high_resolution_clock::now();
	}
	~timer() {
		auto stop = high_resolution_clock::now();
		time_took_ms = (double)duration_cast<microseconds>(stop - start).count()/1e3;
	}
};

#define view(S) std::string_view( (char*)S.data, S.count )
#define range(N) (int i = 0; i < (N); ++i)

template <typename Fn, typename...Args>
void print_result(Fn& fn, Args&&...args) {
//	__try {
		std::cout << fn(std::forward<Args>(args)...);
//	}
//	__except(EXCEPTION_EXECUTE_HANDLER) {
//		std::cout << "an error occured\n";
//	}
}

struct File {
	void* data;
	Kai_u64 size;

	~File() {
		free(data);
	}
};

File read_entire_file(char const* filename) {
	using io = std::ios;
	if (auto f = std::ifstream(filename, io::binary)) {
		Kai_u64 size;
		void* data;
		
		// get file size
		f.seekg(0, io::end);
		size = f.tellg();

		// init output
		if (size) data = malloc(size);

		// read
		f.seekg(0, io::beg);
		f.read((char*)data, size);

		return File{data, size};
	}
	return {};
}

void _kai_print_impl(Kai_str s) {
	std::cout << view(s) << '\n';
}

//#define FILENAME "test.kai"
#define FILENAME "constants.kai"
//#define FILENAME "generated.kai"
//#define FILENAME "simple.kai"
//#define FILENAME "fibo.kai"

#define FILEPATH "scripts/" FILENAME

int main() {
	/////////////////////////////////////////////////
	// Setup
	SetConsoleOutputCP( CP_UTF8 );
	setlocale( LC_ALL, ".UTF8" );

	std::cout << kai_get_version_string().data;
	std::cout << '\n';

	/////////////////////////////////////////////////
	// Load Source File
	File source_file = read_entire_file(FILEPATH);

	if (source_file.data == nullptr) {
		std::cout << "failed to read file: \"" << FILEPATH << "\"\n";
		exit(0);
	}
	Kai_str source_code;
	source_code.data = (Kai_u8*)source_file.data;
	source_code.count = (Kai_int)source_file.size;
	std::cout << '\n' << view(source_code) << '\n';
	

	/////////////////////////////////////////////////
	// Create Abstract Syntax Tree
	kai::Memory memory;
	Kai_Result result;
	Kai_AST tree;
	tree.source_filename.data = (Kai_u8*)FILEPATH;
	tree.source_filename.count = sizeof(FILEPATH) - 1;
	Kai_Error error = {};
	{
		timer t;

		Kai_Syntax_Tree_Create_Info info;
		info.source_code = source_code;
		info.memory = memory;
		info.error  = &error;
		result = kai_create_syntax_tree(&info, &tree);
	}
	if KAI_FAILED(result) {
		kai_debug_write_error(kai_debug_clib_writer(), &error);
		return 0;
	}
	else kai_debug_write_syntax_tree(kai_debug_clib_writer(), &tree);

	/////////////////////////////////////////////////
	// Print some statistics
	std::cout << "\nParsing took: " << time_took_ms << " ms\n";
	std::cout << "\nMemory Usage: " << kai_memory_usage(&memory) << " bytes\n";

//	return 0;

	auto print_ref = Kai_Native_Procedure {
		.address = reinterpret_cast<void*>(&_kai_print_impl),
		.name = KAI_STR("print"),
		.signature = KAI_STR("(str)")
	};


	/////////////////////////////////////////////////
	// Create program
	Kai_Program program;
	if (!KAI_FAILED(result))
	{
		timer t;

		Kai_Program_Create_Info info;
		info.trees  = &tree;
		info.memory = memory;
		info.error  = &error;
		info.native_procedures = &print_ref;
		info.native_procedure_count = 1;
		result = kai_create_program(&info, &program);
	}

	if (KAI_FAILED(result)) {
		auto n = &error;
		while (n != nullptr) {
			n->location.source    = source_code.data;
			n->location.file_name = tree.source_filename;
			n = n->next;
		}
		kai_debug_write_error(kai_debug_clib_writer(), &error);
	}

	std::cout << "\nCompiling took: " << time_took_ms << " ms\n";

	if (!KAI_FAILED(result))
	{
		auto proc_ptr = kai_find_procedure(program, "add", "(s64, s64) -> s64");

		using fn_Type = Kai_u64 (Kai_s64, Kai_s64);

		auto proc = reinterpret_cast<fn_Type KAI_CALL*>(proc_ptr);

		Kai_s64
			a = 4, b = 9;

		std::cout << '\n';
		std::cout << "add(" << a << ", " << b << ") = ";
		std::cout << proc(a, b);
		//print_result(proc, a, b);
		std::cout << '\n';

		kai_destroy_program(program);
	}
	return 0;

#if 0
	auto _kai_print_int = [](Kai_int n) { printf("%i\n", n); };
	auto _kai_print_f32 = [](kai_f32 n) { printf("%f\n", n); };

	struct kai_External {
		void* proc_addr;
		char const* name;
		char const* signature;
	};

	kai_External external_functions[] = {
		{_kai_print_int, "print", "(int) -> void"},
		{_kai_print_f32, "print", "(f32) -> void"}
	};

	typedef void(*fn_On_Update)(kai_f32, kai_ptr);

	fn_On_Update on_update_proc = kai_find_procedure(program, "on_update", "(f32, *void) -> void");
	if (on_update_proc == nullptr) {
		printf("Error\n");
		return;
	}

	on_update_proc(0.0167, nullptr);
#endif

	return 0;
}
