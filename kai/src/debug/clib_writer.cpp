#include <kai/debug.h>
#include <cstdio>
#include <clocale>

namespace clib {
	char const* colors[kai_debug_color_COUNT] = {
		"\x1b[0m",
		"\x1b[1;97m",
		"\x1b[1;91m",
		"\x1b[1;94m",
	};

	void write_string(kai_ptr user, kai_str string) {
		fwrite(string.data, 1, string.count, stdout);
	}
	void write_c_string(kai_ptr user, char const* string) {
		printf("%s", string);
	}
	void write_char(kai_ptr user, kai_u8 c) {
		putchar(c);
	}
	void set_color(kai_ptr user, kai_debug_color_enum color) {
		printf("%s", colors[color]);
	}
	
	kai_Debug_String_Writer writer = {
		.write_string   = write_string,
		.write_c_string = write_c_string,
		.write_char     = write_char,
		.set_color      = set_color,
		.user           = nullptr,
	};
}

kai_Debug_String_Writer* kai_debug_clib_writer() {
	setlocale(LC_CTYPE, ".UTF8");
	return &clib::writer;
}

