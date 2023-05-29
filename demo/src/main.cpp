﻿#include "kai/generation.h"
#include <iostream>
#include <bitset>
#include <vector>
#include <chrono>
#include <ranges>
#define NOMINMAX
#include <Windows.h>
#include <excpt.h>
using namespace std::chrono_literals;

inline double time_took_ms = 0.0;

struct timer {
	LARGE_INTEGER start;

	timer() {
		QueryPerformanceCounter(&start);
	}
	~timer() {
		LARGE_INTEGER end;
		QueryPerformanceCounter(&end);

		time_took_ms = static_cast<double>(
			((end.QuadPart - start.QuadPart) * 1'000'000) / freq.QuadPart
		) / 1000.0;
	}

private:
	static inline LARGE_INTEGER freq = []() { LARGE_INTEGER f; QueryPerformanceFrequency(&f); return f; }();
};

#define view(S) std::string_view( (char*)S.data, S.count )
#define range(N) (int i = 0; i < (N); ++i)

void print_error(kai_Error_Info& err) {
	std::cout << view(err.file) << ':' << err.loc.line << " --> ";
	switch (err.value)
	{
	case kai_Result_Error_Syntax:     std::cout << "Syntax Error"; break;
	case kai_Result_Error_Type_Check: std::cout << "Type Check Error"; break;
	default:break;
	}
	std::cout << ": ";
	std::cout << view(err.what);
	std::cout.put('\n');

	char line_number_str[32];
	itoa((int)err.loc.line, line_number_str, 10);

	int len = strlen(line_number_str);

	for range(len) std::cout.put(' ');
	std::cout << " |\n";

	std::cout << std::string_view(line_number_str, len);
	std::cout << " |";

	char* start_of_line = (char*)err.loc.string.data;
	while (start_of_line > (char*)err.loc.source) {
		auto ch = start_of_line[-1];
		if (ch == '\n' || ch == '\r')
			break;
		start_of_line--;
	}

	{ // print line with error
		auto str = start_of_line;
		while (1) {
			if(*str == '\t')
			std::cout.put(' '), str++;
			else std::cout.put(*str++);

			if (*str == '\0' || *str == '\n' || *str == '\r')
				break;
		}
		std::cout.put('\n');
	}

	for range(len) std::cout.put(' ');
	std::cout << " |";

	for range((char*)err.loc.string.data - start_of_line) std::cout.put(' ');

	std::cout.put('^');

	// do "~~~" for error location string length
	for range(err.loc.string.count - 1) std::cout.put('~');

	std::cout.put(' ');
	std::cout << view(err.context);
	std::cout.put('\n');
}

template <typename Fn, typename...Args>
void print_result(Fn& fn, Args&&...args) {
	__try {
		std::cout << fn(std::forward<Args>(args)...);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		std::cout << "an error occured\n";
	}
}

int main() {

	SetConsoleOutputCP( CP_UTF8 );
	setlocale( LC_ALL, ".UTF8" );

	std::cout << kai_version_string();
	std::cout << '\n';
	

	kai_result result;

	kai_Module mod;
	kai_Error_Info error;
	kai_Lib_create_memory(&mod.memory);

	static kai_str source_code = kai_static_string(
		u8R"KAICODE(
function_that_does_stuff :: (a: int, b: int) -> int {
	n := a + b + 1;
	k := a * b;
	ret (n + k) / 2;
}
)KAICODE");

	std::cout << view(source_code);
	std::cout << '\n';

	{
		timer t;

		kai_Syntax_Tree_Create_Info info;
		info.source     = source_code;
		info.module     = &mod;
		info.filename   = kai_static_string("main.kai");
		info.error_info = &error;
		result = kai_create_syntax_tree(&info);
	}

	if KAI_FAILED(result) {
		print_error(error);
	}
	else kai_Lib_print_syntax_tree(&mod);

	std::cout << "\nParsing took: " << time_took_ms << " ms\n";

	kai_Program program;
	if (!KAI_FAILED(result))
	{
		timer t;

		kai_Program_Create_Info info;
		info.module     = &mod;
		info.error_info = &error;
		result = kai_create_program(&info, &program);
	}

	std::cout << "\nCompiling took: " << time_took_ms << " ms\n";

	if (!KAI_FAILED(result))
	{
		auto proc_ptr = kai_find_procedure(program, "function_that_does_stuff", "(s32, s32) -> s32");

		using fn_Type = kai_u64(kai_u64, kai_u64);

		auto proc = reinterpret_cast<fn_Type*>(proc_ptr);

		kai_u64
			a = 5, b = 6;

		std::cout << '\n';
		std::cout << "function_that_does_stuff(" << a << ", " << b << ") = ";
		print_result(proc, a, b);

		kai_destroy_program(program);
	}

#if 0
	auto _kai_print_int = [](kai_int n) { printf("%i\n", n); };
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

	std::cout << '\n' << '\n';
#if 0
	if (!KAI_FAILED(result))
	{
		error.file = kai_static_string("mylevel/main.kai");
		error.loc.line = 12;
		error.loc.source = (kai_u8*)"	num := (foo + 4) * 4";
		error.loc.string = { 3, error.loc.source + 9 };
		error.value = kai_Result_Error_Type_Check;
		error.what = kai_static_string("Undeclared identifier \"foo\"");
		error.context = kai_static_string("");

		print_error(error);
		std::cout << '\n' << '\n';
	}
#endif

	kai_Lib_destroy_memory(&mod.memory);
	return 0;
}