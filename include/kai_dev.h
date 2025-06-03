/**
 *  FILE IS ONLY MEANT FOR DEVELOPING THE KAI LANGUAGE,
 *  THIS IS NOT AN OFFICIAL PART OF THE API!
 */
#ifndef KAI_DEV__H
#define KAI_DEV__H
#define KAI_USE_DEBUG_API
#include "kai.h"

#if !defined(KAI__PLATFORM_WASM)
#include <stdio.h> // --> printf
#include <stdlib.h>
#endif

// TODO: create 'dev' API (basically just formatted printing)

#if defined(KAI__PLATFORM_WASM)
extern void __wasm_console_log(const char* message, int value);
#endif


#if defined(KAI__PLATFORM_WASM)
#   define panic_with_message(...) kai__fatal_error("Panic", #__VA_ARGS__, __FILE__, __LINE__)
#else
#   define print_location() printf("in (%s:%i)\n", __FILE__, __LINE__)
#   define panic_with_message(...) print_location(), printf(__VA_ARGS__), panic()
#endif

#if !defined(KAI__PLATFORM_WASM)
static void panic(void) {
    puts("\nPanic triggered. Now exiting...");
    exit(1);
}
#endif

inline void dev_dump_memory(Kai_String_Writer* writer, void* data, Kai_u32 count)
{
    Kai_u8* bytes = data;
    Kai_u32 k = 0;
    for (;;) {
        for (int i = 0; i < 16; ++i) {
            if (k >= count)
                return;

            char map[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                          '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            kai__write_char(map[(bytes[k] >> 4) & 0x0F]);
            kai__write_char(map[(bytes[k] >> 0) & 0x0F]);
            kai__write_char(' ');

            k += 1;
        }
        kai__write_char('\n');
    }
}

#endif //KAI_DEV__H
