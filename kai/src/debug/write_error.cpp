#include <cstdio>
#include <algorithm>
#include <kai/debug.h>
#include "../config.hpp"
#include "../builtin_types.hpp"

static constexpr char const* result_string_map[kai_Result_COUNT] = {
	"Success"
,	"Syntax Error"
,	"Semantic Error"
,	"Type Cast Error"
,	"Type Check Error"
,	"Info"

,	"Fatal Error"
,	"Internal Error"
};

int number_of_digits(kai_int x) {
	if (x <            10) return 1;
	if (x <           100) return 2;
	if (x <         1'000) return 3;
	if (x <        10'000) return 4;
	if (x <       100'000) return 5;
	if (x <     1'000'000) return 6;
	if (x <    10'000'000) return 7;
	if (x <   100'000'000) return 8;
	if (x < 1'000'000'000) return 9;

	// WHO THE FUCK GAVE THIS COMPILER OVER 1 BILLION LINES OF CODE???
	return 10;
}

u8 const* advance_to_line(u8 const* source, kai_int line) {
	--line;
	while(line > 0) {
		if(*source++ == '\n') --line;
	}
	return source;
}

void write_source_code(kai_Debug_String_Writer* writer, u8 const* src) {
	while (*src != 0 && *src != '\n') {
		if(*src == '\t')
			_write_char(' ');
		else
			_write_char(*src);
		++src;
	}
}

void write_source_code_count(kai_Debug_String_Writer* writer, u8 const* src, kai_int count) {
	while (*src != 0 && *src != '\n' && count != 0) {
		_write_char(' ');
		++src;
		--count;
	}
}

void kai_debug_write_error(kai_Debug_String_Writer* writer, kai_Error* error) {
	char temp[256];

write_error_message:
	if (error->result >= kai_Result_COUNT) {
		_write("compiler internal error: result not implemented\n");
		return;
	}

	// ----------------- Write Error Message -----------------

	_set_color(kai_debug_color_important_2);
	_write_string(error->location.file_name);
	_set_color(kai_debug_color_primary);
#if KAI_SHOW_LINE_NUMBER_WITH_FILE
	_write_char(':');
	_set_color(kai_debug_color_important_2);
	_write_format("%u", error->location.line);
	_set_color(kai_debug_color_primary);
#endif
	_write(" --> ");
	if(error->result != kai_Result_Error_Info) _set_color(kai_debug_color_important);
	_write(result_string_map[error->result]);
	if (error->result != kai_Result_Error_Info) _set_color(kai_debug_color_primary);
	_write(": ");
	_write_string(error->message);
	_write_char('\n');

	// ------------------ Write Source Code ------------------

	auto digits = number_of_digits(error->location.line);
	for_n(digits) _write_char(' ');
	_write("  |\n");

	_write_format(" %u", error->location.line);
	_write(" | ");

	auto begin = advance_to_line(error->location.source, error->location.line);

	write_source_code(writer, begin);
	_write_char('\n');

	for_n(digits) _write_char(' ');
	_write("  | ");

	write_source_code_count(writer, begin,
		kai_int(error->location.string.data - begin)
	);

	_set_color(kai_debug_color_important);
	_write_char('^');
	auto n = std::max((kai_int)0, error->location.string.count - 1);
	for_n(n) _write_char('~');

	_write_char(' ');
	_write_string(error->context);

	_write_char('\n');
	_set_color(kai_debug_color_primary);

	// ----------------------- Repeat ------------------------

	if (error->next != nullptr) {
		error = error->next;
		goto write_error_message;
	}
}

