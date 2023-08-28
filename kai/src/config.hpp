#pragma once
#include <kai/core.h>
#define DEBUG 1

#if DEBUG
// Rust got nothing on C
extern void panic();
#include <cstdio>
#define assert(Z,...)           if(!(Z)) panic_with_message(__VA_ARGS__)
#define panic_with_message(...) printf(__VA_ARGS__), panic()
#define debug_log(S)            std::cout << '[' << __FUNCTION__ << ']' << " " S "\n";
#else
#define assert(Z,...)           (void)0
#define panic_with_message(...) (void)0
#define debug_log(S)            (void)0
#endif

#define NO_DISCARD [[nodiscard]]

// how is something like this not built into C++ already?
#define range(N) (std::remove_const_t<decltype(N)> i = 0; i < N; ++i)
#define iterate(CONTAINER) (auto it = (CONTAINER).begin(); it != (CONTAINER).end(); ++it)
#define loop while(1)
#define view(S) std::string_view( (char*)S.data, S.count )

#define XPRIMITIVE_TYPES \
X( u8, kai_u8 ) X(u16, kai_u16) X(u32, kai_u32) X(u64, kai_u64) \
X( s8, kai_s8 ) X(s16, kai_s16) X(s32, kai_s32) X(s64, kai_s64) \
X(f32, kai_f32) X(f64, kai_f64)

#define X(NAME,TYPE) using NAME = TYPE;
XPRIMITIVE_TYPES
#undef  X

struct Compiler_Context {
    kai_Memory memory;
    kai_Error  error;

    void reset(kai_Memory* Memory) {
        memory = *Memory;
        error  = {};
    }
};

inline thread_local Compiler_Context context;

#define ctx_alloc(Size) \
context.memory.alloc(context.memory.user, Size);

#define ctx_alloc_T(Type) \
(Type*)context.memory.alloc(context.memory.user, sizeof(Type));

#define ctx_alloc_array(Type, Count) \
(Type*)context.memory.alloc(context.memory.user, sizeof(Type) * Count);
