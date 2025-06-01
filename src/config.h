#ifndef CONFIG_H
#define CONFIG_H

#include "kai.h"
// TODO: remove
#if !defined(KAI__PLATFORM_WASM)
#include <stdio.h>   // --> printf
#endif


#define MACRO_PRAGMA(X) _Pragma(#X)

#ifdef __WASM__
extern void __wasm_console_log(const char* message, int value);
#endif

// =============================<< MACROS >>===================================

#ifndef __WASM__
#define panic_with_message(...) print_location(), printf(__VA_ARGS__), panic()
#else
#define panic_with_message(MESSAGE) kai__fatal_error("Panic", MESSAGE, __FILE__, __LINE__)
#endif

#define print_location() printf("in (%s:%i)\n", __FILE__, __LINE__)

// ==========================<< DEBUG WRITER >>================================

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
