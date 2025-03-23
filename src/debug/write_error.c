#define KAI_USE_DEBUG_API
#include "../config.h"
#include <stdio.h>
#include <inttypes.h>

Kai_int kai__max(Kai_int a, Kai_int b) {
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

int kai__base10_digit_count(Kai_int x) {
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

u8 const* kai__advance_to_line(u8 const* source, Kai_int line) {
    --line;
    while (line > 0) {
        if (*source++ == '\n') --line;
    }
    return source;
}

void kai__write_source_code(Kai_Debug_String_Writer* writer, u8 const* src) {
    while (*src != 0 && *src != '\n') {
        if (*src == '\t')
            kai__write_char(' ');
        else
            kai__write_char(*src);
        ++src;
    }
}

void kai__write_source_code_count(Kai_Debug_String_Writer* writer, u8 const* src, Kai_int count) {
    while (*src != 0 && *src != '\n' && count != 0) {
        kai__write_char(' ');
        ++src;
        --count;
    }
}


void kai_debug_write_error(Kai_Debug_String_Writer* writer, Kai_Error* error) {
    char temp[256];

write_error_message:
    if (error->result >= KAI_RESULT_COUNT) {
        kai__write("[Invalid result value]\n");
        return;
    }
    else if (error->result == KAI_SUCCESS) {
        kai__write("[Success]\n");
        return;
    }

    // ------------------------- Write Error Message --------------------------

    kai__set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
    kai__write_string(error->location.file_name);
    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
#if KAI_SHOW_LINE_NUMBER_WITH_FILE
    kai__write_char(':');
    kai__set_color(KAI_DEBUG_COLOR_IMPORTANT_2);
    kai__write_format("%u", error->location.line);
    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
#endif
    kai__write(" --> ");
    if (error->result != KAI_ERROR_INFO) kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
    kai__write(result_string_map[error->result]);
    if (error->result != KAI_ERROR_INFO) kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
    kai__write(": ");
    kai__set_color(KAI_DEBUG_COLOR_SECONDARY);
    kai__write_string(error->message);
    kai__write_char('\n');

    // -------------------------- Write Source Code ---------------------------

    if (error->result == KAI_ERROR_FATAL || error->result == KAI_ERROR_INTERNAL)
        goto repeat;

    kai__set_color(KAI_DEBUG_COLOR_DECORATION);
    Kai_int digits = kai__base10_digit_count(error->location.line);
    for_n(digits) kai__write_char(' ');
    kai__write("  |\n");

    kai__write_format(" %" PRIi32, error->location.line);
    kai__write(" | ");

    Kai_u8 const* begin = kai__advance_to_line(error->location.source, error->location.line);

    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);
    kai__write_source_code(writer, begin);
    kai__write_char('\n');

    kai__set_color(KAI_DEBUG_COLOR_DECORATION);
    for_n(digits) kai__write_char(' ');
    kai__write("  | ");

    kai__write_source_code_count(writer, begin,
        (Kai_int)(error->location.string.data - begin)
    );

    kai__set_color(KAI_DEBUG_COLOR_IMPORTANT);
    kai__write_char('^');
    Kai_int n = kai__max(0, error->location.string.count - 1);
    for_n(n) kai__write_char('~');

    kai__write_char(' ');
    kai__write_string(error->context);

    kai__write_char('\n');
    kai__set_color(KAI_DEBUG_COLOR_PRIMARY);

    // -------------------------------- Repeat --------------------------------

repeat: 
    if (error->next) {
        error = error->next;
        goto write_error_message;
    }
}

