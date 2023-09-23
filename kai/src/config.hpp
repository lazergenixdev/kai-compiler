#pragma once
#include <type_traits>
#include <kai/core.h>

// ==================<< Shortcuts >>===================

#define for_n(N)            for (std::remove_const_t<decltype(N)> i = 0; i < N; ++i)
#define for_iter(CONTAINER) for (auto it = (CONTAINER).begin(); it != (CONTAINER).end(); ++it)
#define SV(S)               ::std::string_view{reinterpret_cast<char*>(S.data), static_cast<size_t>(S.count)}


// ===================<< MACROS >>=====================

#define no_discard [[nodiscard]]

#if   defined(_MSC_VER)
#   define CXX_FUNCTION __FUNCSIG__
#   define debug_break() __debugbreak()
#   define unreachable() (__assume(false))
#elif defined(__GNUC__)
#   define CXX_FUNCTION __PRETTY_FUNCTION__
#   define debug_break() __builtin_trap()
#   define unreachable() (__builtin_unreachable())
#else
[[noreturn]] inline void unreachable_impl() {}
#   define unreachable() (unreachable_impl())
#endif

#define ENABLE_DEV_MODE 1
#if ENABLE_DEV_MODE
#   define ENABLE_DEBUG_PRINT 1
#   include <cstdio>
#   define assert(Z,...)           if(!(Z)) panic_with_message(__VA_ARGS__)
#   define panic_with_message(...) printf(__VA_ARGS__), panic()
#   define debug_log(MSG)          printf("[%s:%i] %s\n", __FUNCTION__, __LINE__, MSG);
[[noreturn]] extern void panic(); // Rust got nothing on C
#else
#   define assert(Z,...)           (void)0
#   define panic_with_message(...) (void)0
#   define debug_log(S)            (void)0
#   define panic()                 (void)0
#endif


// ====================<< Types >>=====================

#define XPRIMITIVE_TYPES \
X( u8, kai_u8 ) X(u16, kai_u16) X(u32, kai_u32) X(u64, kai_u64) \
X( s8, kai_s8 ) X(s16, kai_s16) X(s32, kai_s32) X(s64, kai_s64) \
X(f32, kai_f32) X(f64, kai_f64)

#define X(NAME,TYPE) using NAME = TYPE;
XPRIMITIVE_TYPES
#undef  X


// =================<< Parse Tokens >>=================

#define token_2(STR) kai_u32(               STR[1] << 8 | STR[0])
#define token_3(STR) kai_u32(STR[2] << 16 | STR[1] << 8 | STR[0])


// =================<< Debug Writer >>=================

#define _write(CSTRING)       writer->write_c_string(writer->user, CSTRING)
#define _write_string(STRING) writer->write_string(writer->user, STRING)
#define _write_char(CHAR)     writer->write_char(writer->user, CHAR)
#define _set_color(COLOR)     if(writer->set_color) writer->set_color(writer->user, COLOR)

// Must have a buffer declared as "char temp[COUNT]"
#define _write_format(...) {                                                            \
    kai_int count = snprintf(temp, sizeof(temp), __VA_ARGS__);                          \
	count = std::min(count, (kai_int)sizeof(temp) - 1);                                 \
    writer->write_string(writer->user, kai_str{.count = count, .data = (kai_u8*)temp}); \
} (void)0
