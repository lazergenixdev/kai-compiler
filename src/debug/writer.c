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

#if defined(KAI__PLATFORM_WINDOWS)
// including Windows.h results in a compiler warning :/
int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);
#endif

Kai_Debug_String_Writer* kai_debug_stdout_writer(void)
{
#if defined(KAI__PLATFORM_WINDOWS)
	SetConsoleOutputCP(65001);
#endif
    setlocale(LC_CTYPE, ".UTF8");
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
    if (user == NULL) return;
    fwrite(string.data, 1, string.count, user);
}
void kai__file_writer_write_c_string(Kai_ptr user, char const* string) {
    if (user == NULL) return;
    fprintf(user, "%s", string);
}
void kai__file_writer_write_char(Kai_ptr user, Kai_u8 c) {
    if (user == NULL) return;
    fputc(c, user);
}
void kai__file_writer_set_color(Kai_ptr user, Kai_Debug_Color color) {
    (void)user, (void)color;
    //fprintf(user, "%s", kai__term_debug_colors[color]);
}

#if defined(KAI__COMPILER_MSVC)
static inline FILE* stdc_file_open(char const* path, char const* mode) {
    FILE* handle = NULL;
    fopen_s(&handle, path, mode); // this is somehow more safe? :/
    return handle;
}
#else
#    define stdc_file_open fopen
#endif

void kai_debug_open_file_writer(Kai_Debug_String_Writer* writer, const char* path)
{
    *writer = (Kai_Debug_String_Writer) {
        .write_string   = kai__file_writer_write_string,
        .write_c_string = kai__file_writer_write_c_string,
        .write_char     = kai__file_writer_write_char,
        .set_color      = kai__file_writer_set_color,
        .user           = stdc_file_open(path, "wb"),
    };
}

void kai_debug_close_file_writer(Kai_Debug_String_Writer* writer)
{
    fclose(writer->user);
}
