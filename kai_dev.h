/**
 *  FILE IS ONLY MEANT FOR DEVELOPING THE KAI LANGUAGE,
 *  THIS IS NOT AN OFFICIAL PART OF THE API!
 */
#ifndef KAI_DEV__H
#define KAI_DEV__H
#define KAI_USE_FATAL_ERROR

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

extern void __wasm_console_log(const char* message, int value);
extern void __wasm_write_string(Kai_ptr User, Kai_str String);
extern void __wasm_write_value(Kai_ptr User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format format);
extern void __wasm_set_color(Kai_ptr User, Kai_Write_Color Color_Index);

static Kai_String_Writer debug_writer = {
	.write_string   = &__wasm_write_string,
	.write_value    = &__wasm_write_value,
	.set_color      = &__wasm_set_color,
};
#else
#   define print_location() printf("in (%s:%i)\n", __FILE__, __LINE__)
#   define panic_with_message(...) print_location(), printf(__VA_ARGS__), panic()
#	define debug_writer (*kai_writer_stdout())
#endif

#if !defined(KAI__PLATFORM_WASM)
static inline void panic(void) {
    puts("\nPanic triggered. Now exiting...");
    exit(1);
}
#endif

#include "kai.h"

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

#if !defined(KAI__PLATFORM_WASM)
extern inline char const *kai__file(char const *cs)
{
    Kai_str s = kai_string_from_c(cs);
    Kai_u32 i = s.count - 1;
    while (i > 0) {
        if (cs[i] == '/' || cs[i] == '\\')
            return cs + i + 1;
        i -= 1;
    }
    return cs;
}
extern inline void kai__fatal_error(char const *Desc, char const *Message, char const *File, int Line)
{
    printf("\x1b[91m%s\x1b[0m: %s\nin \x1b[92m%s:%i\x1b[0m", Desc, Message,
            kai__file(File), Line);
    exit(1);
}
#endif

////////////////////////////////////////////////////////
// TODO?
typedef struct {
    Kai_s64 last_modified;
    Kai_str path;
    Kai_str source_code;
} Kai_Script_Status;

typedef struct {
    Kai_Allocator                 allocator;
    KAI__ARRAY(Kai_Script_Status) scripts;
} Kai_Script_Manager;
////////////////////////////////////////////////////////

#endif //KAI_DEV__H
