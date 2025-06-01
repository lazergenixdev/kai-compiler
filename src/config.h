#ifndef CONFIG_H
#define CONFIG_H

#include "kai.h"
#include <stdio.h>   // --> printf


#define MACRO_PRAGMA(X) _Pragma(#X)

#ifdef __WASM__
extern void __wasm_console_log(const char* message, int value);
#endif

#if defined(KAI__COMPILER_MSVC)
#    define KAI__MSVC_DISABLE_WARNING(WARNING) MACRO_PRAGMA(warning(disable : WARNING))
#else
#    define KAI__MSVC_DISABLE_WARNING(WARNING)
#endif

#if defined(KAI__COMPILER_GNU) || defined(KAI__COMPILER_CLANG)
#    define KAI__CLANG_DISABLE_WARNING(WARNING) MACRO_PRAGMA(GCC diagnostic ignored WARNING)
#else
#    define KAI__CLANG_DISABLE_WARNING(WARNING)
#endif

KAI__CLANG_DISABLE_WARNING("-Wmultichar")

// =============================<< MACROS >>===================================

#ifndef __WASM__
#define panic_with_message(...) print_location(), printf(__VA_ARGS__), panic()
#else
#define panic_with_message(MESSAGE) kai__fatal_error("Panic", MESSAGE, __FILE__, __LINE__)
#endif

#define print_location() printf("in (%s:%i)\n", __FILE__, __LINE__)

// ==========================<< DEBUG WRITER >>================================

#define kai__write(CSTRING)       writer->write_c_string(writer->user, CSTRING)
#define kai__write_string(STRING) writer->write_string(writer->user, STRING)
#define kai__write_char(CHAR)     writer->write_char(writer->user, CHAR)
#define kai__set_color(COLOR)     if (writer->set_color != NULL) writer->set_color(writer->user, COLOR)

#ifndef __WASM__
// Must have a buffer declared as "char temp[..]"
#define kai__write_format(...) {                                                              \
    int __count__ = snprintf(temp, sizeof(temp), __VA_ARGS__);                            \
    __count__ = (__count__ < sizeof(temp)-1) ? __count__ : sizeof(temp)-1;                               \
    writer->write_string(writer->user, (Kai_str){.count = (Kai_u32)__count__, .data = (Kai_u8*)temp}); \
} (void)0
#else
#define kai__write_format(...)    (void)0
#endif

#ifndef __WASM__
#include <stdlib.h>

static void panic(void) {
    puts("\nPanic triggered. Now exiting...");
    exit(1);
}
#endif

#endif // CONFIG_H
