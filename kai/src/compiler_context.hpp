#pragma once
#include <kai/memory.h>

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
