#pragma once
#include <kai/core.h>
#define DEBUG 1

#if DEBUG
// Rust got nothing on C
extern void panic();
#include <cstdio>
#define assert(Z,...)           if(!(Z)) panic_with_message(__VA_ARGS__)
#define panic_with_message(...) printf(__VA_ARGS__), panic()
#else
#define assert(Z,...)           (void)0
#define panic_with_message(...) (void)0
#endif

// how is something like this not built into C++ already?
#define range(N) (decltype(N) i = 0; i < N; ++i)

#define XPRIMITIVE_TYPES \
X( u8, kai_u8 ) X(u16, kai_u16) X(u32, kai_u32) X(u64, kai_u64) \
X( s8, kai_s8 ) X(s16, kai_s16) X(s32, kai_s32) X(s64, kai_s64) \
X(f32, kai_f32) X(f64, kai_f64)

#define X(NAME,TYPE) using NAME = TYPE;
XPRIMITIVE_TYPES
#undef  X
