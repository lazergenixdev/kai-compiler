#ifndef CONFIG_H
#define CONFIG_H

#include "kai/kai.h"
#include <string.h>  // --> memset, memcpy
#include <stdio.h>   // --> printf

#define MACRO_PRAGMA(X) _Pragma(#X)

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

// ==============================<< Shortcuts >>===============================

#define for_n(N)  for (Kai_int i = 0; i < (Kai_int)(N); ++i)
#define for_(I,N) for (Kai_int I = 0; I < (Kai_int)(N); ++I)
#define count_of(ARR) (sizeof(ARR)/sizeof(ARR[0]))

// =============================<< MACROS >>===================================

#if defined(KAI_DISABLE_ASSERTS)
#   define kai__assert(EXPR)
#else
#   define kai__assert(EXPR) \
        if (!(EXPR)) printf("%s:%i: \x1b[91mAssertion Failed\x1b[0m (\x1b[94m%s\x1b[0m)\n", __FILE__, __LINE__, #EXPR), exit(1)
#endif

#if defined(KAI__COMPILER_MSVC)
#    define FUNCTION __FUNCSIG__
#else
#    define FUNCTION __PRETTY_FUNCTION__
#endif

#if defined(KAI__COMPILER_MSVC)
#   define KAI_CONSTANT_STRING(S) {(Kai_u8*)(S), sizeof(S)-1}
#else
#   define KAI_CONSTANT_STRING(S) KAI_STRING(S)
#endif

#define panic_with_message(...) print_location(), printf(__VA_ARGS__), panic()

#define assert(Z, ...) if (!(Z)) panic_with_message(__VA_ARGS__)
#define log(L,F,N,...) \
    printf("%s: [%s:%i] ", L, F, N), \
    printf(__VA_ARGS__), \
    putchar('\n')

#define print_location() printf("in (%s:%i)\n", __FUNCTION__, __LINE__)

extern void panic(void);

#define UNIMPLEMENTED(...) (void)sizeof(__VA_ARGS__), printf("%s is unimplemented :(", FUNCTION), panic()

// ==========================<< DEBUG WRITER >>================================

#define kai__write(CSTRING)       writer->write_c_string(writer->user, CSTRING)
#define kai__write_string(STRING) writer->write_string(writer->user, STRING)
#define kai__write_char(CHAR)     writer->write_char(writer->user, CHAR)
#define kai__set_color(COLOR)     if (writer->set_color) writer->set_color(writer->user, COLOR)

// Must have a buffer declared as "char temp[..]"
#define kai__write_format(...) {                                                              \
    int count = snprintf(temp, sizeof(temp), __VA_ARGS__);                            \
    count = (count < sizeof(temp)-1) ? count : sizeof(temp)-1;                               \
    writer->write_string(writer->user, (Kai_str){.count = (Kai_u32)count, .data = (Kai_u8*)temp}); \
} (void)0

#define str_insert_string(Dest, String) \
memcpy((Dest).data + (Dest).count, String, sizeof(String)-1), (Dest).count += sizeof(String)-1

#define str_insert_str(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data, (Src).count), (Dest).count += (Src).count

#define str_insert_std(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data(), (Src).size()), (Dest).count += (Src).size()

// =============================<< TYPES >>====================================

#define X(TYPE, NAME) typedef TYPE NAME;
	KAI_X_PRIMITIVE_TYPES
#undef X


#endif // CONFIG_H
