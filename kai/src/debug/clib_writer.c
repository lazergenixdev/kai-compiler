#include <kai/debug.h>
#include <stdio.h>
#include <locale.h>

char const* colors[KAI_DEBUG_COLOR_COUNT] = {
    "\x1b[0;37m",
    "\x1b[1;97m",
    "\x1b[1;91m",
    "\x1b[1;94m",
    "\x1b[0;90m",
};

void _kai_write_string(Kai_ptr user, Kai_str string) {
    (void)user;
    fwrite(string.data, 1, string.count, stdout);
}
void _kai_write_c_string(Kai_ptr user, char const* string) {
    (void)user;
    printf("%s", string);
}
void _kai_write_char(Kai_ptr user, Kai_u8 c) {
    (void)user;
    putchar(c);
}
void _kai_set_color(Kai_ptr user, Kai_Debug_Color color) {
    (void)user;
    printf("%s", colors[color]);
}

Kai_Debug_String_Writer* kai_debug_clib_writer() {
    setlocale( LC_CTYPE, ".UTF8" ); // <- tf is this?
    static Kai_Debug_String_Writer writer = {
        .write_string   = _kai_write_string,
        .write_c_string = _kai_write_c_string,
        .write_char     = _kai_write_char,
        .set_color      = _kai_set_color,
        .user           = NULL
    };
    return &writer;
}

