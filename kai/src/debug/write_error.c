#include <kai/debug.h>
#include "../config.h"
#include <stdio.h>
#include <inttypes.h>

typedef Kai_u8 u8;

Kai_int max(Kai_int a, Kai_int b) {
    return a > b? a : b;
}

static char const* const result_string_map[KAI_RESULT_COUNT] = {
    "Success",

    "Syntax Error",
    "Semantic Error",
    "Type Cast Error",
    "Type Check Error",
    "Info",

    "Fatal Error",
    "Internal Error",
};

int number_of_digits(Kai_int x) {
    if (x <         10) return 1;
    if (x <        100) return 2;
    if (x <       1000) return 3;
    if (x <      10000) return 4;
    if (x <     100000) return 5;
    if (x <    1000000) return 6;
    if (x <   10000000) return 7;
    if (x <  100000000) return 8;
    if (x < 1000000000) return 9;
    return 0;
}

u8 const* advance_to_line(u8 const* source, Kai_int line) {
    --line;
    while (line > 0) {
        if (*source++ == '\n') --line;
    }
    return source;
}

void write_source_code(Kai_Debug_String_Writer* writer, u8 const* src) {
    while (*src != 0 && *src != '\n') {
        if (*src == '\t')
            _write_char(' ');
        else
            _write_char(*src);
        ++src;
    }
}

void write_source_code_count(Kai_Debug_String_Writer* writer, u8 const* src, Kai_int count) {
    while (*src != 0 && *src != '\n' && count != 0) {
        _write_char(' ');
        ++src;
        --count;
    }
}


void kai_debug_write_error(Kai_Debug_String_Writer* writer, Kai_Error* error) {
    char temp[256];

write_error_message:
    if (error->result >= KAI_RESULT_COUNT) {
        _write("[Invalid result value]\n");
        return;
    }
    else if (error->result == KAI_SUCCESS) {
        _write("[Success]\n");
        return;
    }

    // ------------------------- Write Error Message --------------------------

    _set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
    _write_string(error->location.file_name);
    _set_color(KAI_DEBUG_COLOR_PRIMARY);
#if KAI_SHOW_LINE_NUMBER_WITH_FILE
    _write_char(':');
    _set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
    _write_format("%u", error->location.line);
    _set_color(KAI_DEBUG_COLOR_PRIMARY);
#endif
    _write(" --> ");
    if (error->result != KAI_ERROR_INFO) _set_color(KAI_DEBUG_COLOR_IMPORTANT);
    _write(result_string_map[error->result]);
    if (error->result != KAI_ERROR_INFO) _set_color(KAI_DEBUG_COLOR_PRIMARY);
    _write(": ");
    _set_color(KAI_DEBUG_COLOR_SECONDARY);
    _write_string(error->message);
    _write_char('\n');

    // -------------------------- Write Source Code ---------------------------

    if (error->result == KAI_ERROR_FATAL || error->result == KAI_ERROR_INTERNAL)
        goto repeat;

    _set_color(KAI_DEBUG_COLOR_DECORATION);
    Kai_int digits = number_of_digits(error->location.line);
    for_n(digits) _write_char(' ');
    _write("  |\n");

    _write_format(" %" PRIi64, error->location.line);
    _write(" | ");

    Kai_u8 const* begin = advance_to_line(error->location.source, error->location.line);

    _set_color(KAI_DEBUG_COLOR_PRIMARY);
    write_source_code(writer, begin);
    _write_char('\n');

    _set_color(KAI_DEBUG_COLOR_DECORATION);
    for_n(digits) _write_char(' ');
    _write("  | ");

    write_source_code_count(writer, begin,
        (Kai_int)(error->location.string.data - begin)
    );

    _set_color(KAI_DEBUG_COLOR_IMPORTANT);
    _write_char('^');
    Kai_int n = max(0, error->location.string.count - 1);
    for_n(n) _write_char('~');

    _write_char(' ');
    _write_string(error->context);

    _write_char('\n');
    _set_color(KAI_DEBUG_COLOR_PRIMARY);

    // -------------------------------- Repeat --------------------------------

repeat: 
    if (error->next) {
        error = error->next;
        goto write_error_message;
    }
}

