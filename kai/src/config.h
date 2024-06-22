#ifndef CONFIG_H
#include <kai/core.h>
#include <string.h>  // --> memset, memcpy

// ==============================<< Shortcuts >>===============================

#define for_n(N)  for (Kai_int i = 0; i < (Kai_int)(N); ++i)
#define for_(I,N) for (Kai_int I = 0; I < (Kai_int)(N); ++I)
#define zero_memory(P) memset(P, 0, sizeof(*P))


// =============================<< MACROS >>===================================

#if   defined(_MSC_VER)
#    define FUNCTION __FUNCSIG__
#    define debug_break() __debugbreak()
#elif defined(__GNUC__)
#    define FUNCTION __PRETTY_FUNCTION__
#    define debug_break() __builtin_trap()
#else
#    error "what compiler???"
#endif // Compiler Detect

// 0 -> no debug output
// 1 -> basic output
// 2 -> more detailed output
// 3 -> VERBOSE
#define TRACE_LEVEL 1

#if TRACE_LEVEL > 0
#    include <stdio.h> // --> printf
#    define D_INFO(...) log("info", FUNCTION, __LINE__, __VA_ARGS__)
#    define panic_with_message(...) print_location(), printf(__VA_ARGS__), panic()
#else
#    define D_INFO(...)
#    define panic_with_message(...) panic()
#endif
#if TRACE_LEVEL > 1
#    define D_DETAIL(...) log("info", FUNCTION, __LINE__, __VA_ARGS__)
#else
#    define D_DETAIL(...)
#endif
#if TRACE_LEVEL > 2
#    define D_VERBOSE(...) log("info", FUNCTION, __LINE__, __VA_ARGS__)
#else
#    define D_VERBOSE(...)
#endif

#define assert(Z, ...) if (!(Z)) panic_with_message(__VA_ARGS__)
#define log(L,F,N,...) \
    printf("%s: [%s:%i] ", L, F, N), \
    printf(__VA_ARGS__), \
    putchar('\n')

#define print_location() printf("in (" __FUNCTION__ ":%i)\n", __LINE__)

extern void panic();

#define UNIMPLEMENTED() printf("%s is unimplemented :(", FUNCTION), panic()

// ==========================<< DEBUG WRITER >>================================

#define _write(CSTRING)       writer->write_c_string(writer->user, CSTRING)
#define _write_string(STRING) writer->write_string(writer->user, STRING)
#define _write_char(CHAR)     writer->write_char(writer->user, CHAR)
#define _set_color(COLOR)     if (writer->set_color) writer->set_color(writer->user, COLOR)

// Must have a buffer declared as "char temp[..]"
#define _write_format(...) {                                                              \
    Kai_int count = snprintf(temp, sizeof(temp), __VA_ARGS__);                            \
    count = count < sizeof(temp)-1? count : sizeof(temp)-1;                               \
    writer->write_string(writer->user, (Kai_str){.count = count, .data = (Kai_u8*)temp}); \
} (void)0

#define str_insert_string(Dest, String) \
memcpy((Dest).data + (Dest).count, String, sizeof(String)-1), (Dest).count += sizeof(String)-1

#define str_insert_str(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data, (Src).count), (Dest).count += (Src).count

#define str_insert_std(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data(), (Src).size()), (Dest).count += (Src).size()

// =============================<< TYPES >>====================================

#define XPRIMITIVE_TYPES \
X( u8, Kai_u8 ) X(u16, Kai_u16) X(u32, Kai_u32) X(u64, Kai_u64) \
X( s8, Kai_s8 ) X(s16, Kai_s16) X(s32, Kai_s32) X(s64, Kai_s64) \
X(f32, Kai_f32) X(f64, Kai_f64)

#define X(NAME,TYPE) typedef TYPE NAME;
XPRIMITIVE_TYPES
#undef  X

#endif // CONFIG_H

