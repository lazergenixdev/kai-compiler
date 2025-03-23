#define KAI_USE_DEBUG_API
#include "../config.h"
#include <stdio.h>
#include <locale.h>

char const* kai__term_debug_colors [KAI_DEBUG_COLOR_COUNT] = {
    [KAI_DEBUG_COLOR_PRIMARY]     = "\x1b[0;37m",
    [KAI_DEBUG_COLOR_SECONDARY]   = "\x1b[1;97m",
    [KAI_DEBUG_COLOR_IMPORTANT]   = "\x1b[1;91m",
    [KAI_DEBUG_COLOR_IMPORTANT_2] = "\x1b[1;94m",
    [KAI_DEBUG_COLOR_DECORATION]  = "\x1b[0;90m",
};

void kai__stdout_writer_write_string(Kai_ptr user, Kai_str string) {
    (void)user;
    fwrite(string.data, 1, string.count, stdout);
}
void kai__stdout_writer_write_c_string(Kai_ptr user, char const* string) {
    (void)user;
    printf("%s", string);
}
void kai__stdout_writer_write_char(Kai_ptr user, Kai_u8 c) {
    (void)user;
    putchar(c);
}
void kai__stdout_writer_set_color(Kai_ptr user, Kai_Debug_Color color) {
    (void)user;
    printf("%s", kai__term_debug_colors[color]);
}

Kai_Debug_String_Writer* kai_debug_stdout_writer(void)
{
    setlocale( LC_CTYPE, ".UTF8" ); // <- tf is this?
    static Kai_Debug_String_Writer writer = {
        .write_string   = kai__stdout_writer_write_string,
        .write_c_string = kai__stdout_writer_write_c_string,
        .write_char     = kai__stdout_writer_write_char,
        .set_color      = kai__stdout_writer_set_color,
        .user           = NULL
    };
    return &writer;
}

void kai__file_writer_write_string(Kai_ptr user, Kai_str string) { 
    fwrite(string.data, 1, string.count, user);
}
void kai__file_writer_write_c_string(Kai_ptr user, char const* string) {
    fprintf(user, "%s", string);
}
void kai__file_writer_write_char(Kai_ptr user, Kai_u8 c) {
    fputc(c, user);
}
void kai__file_writer_set_color(Kai_ptr user, Kai_Debug_Color color) {
    (void)user, (void)color;
    //fprintf(user, "%s", kai__term_debug_colors[color]);
}

void kai_debug_open_file_writer(Kai_Debug_String_Writer* writer, const char* path)
{
    *writer = (Kai_Debug_String_Writer) {
        .write_string   = kai__file_writer_write_string,
        .write_c_string = kai__file_writer_write_c_string,
        .write_char     = kai__file_writer_write_char,
        .set_color      = kai__file_writer_set_color,
        .user           = fopen(path, "wb"),
    };
}

void kai_debug_close_file_writer(Kai_Debug_String_Writer* writer)
{
    fclose(writer->user);
}
