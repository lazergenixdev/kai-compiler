#pragma once
#include <kai/memory.h>

static bool error_internal_impl(kai_str const& message);
#define error_internal(message) error_internal_impl(KAI_STR(message))

template <typename T>
struct Temperary_Array {
    T* base;
    u32 count;

    void copy_to(void* destination) {
        memcpy(destination, base, sizeof(T)*count);
    }
};

struct Compiler_Context {
    kai_Memory memory;
    kai_Error  error;

    void reset(kai_Memory* Memory) {
        memory = *Memory;
        error  = {};
    }

    template <typename T>
    bool has_stack_overflow(Temperary_Array<T> const& arr) {
        auto end = reinterpret_cast<kai_u8*>(arr.base + arr.count);
        if (end > (kai_u8*)memory.temperary + memory.temperary_size) {
            return error_internal("ran out of temperary memory!");
        }
        return false;
    }
};

inline thread_local Compiler_Context context;


#define ctx_alloc(Size) \
context.memory.alloc(context.memory.user, Size);

#define ctx_alloc_T(Type) \
(Type*)context.memory.alloc(context.memory.user, sizeof(Type));

#define ctx_alloc_array(Type, Count) \
(Type*)context.memory.alloc(context.memory.user, sizeof(Type) * Count);


#define str_insert_string(Dest, String) \
memcpy((Dest).data + (Dest).count, String, sizeof(String)-1), (Dest).count += sizeof(String)-1

#define str_insert_str(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data, (Src).count), (Dest).count += (Src).count

#define str_insert_std(Dest, Src) \
memcpy((Dest).data + (Dest).count, (Src).data(), (Src).size()), (Dest).count += (Src).size()


static bool error_internal_impl(kai_str const& message) {
    auto& err = context.error;
    err.result          = kai_Result_Error_Internal;
    err.location.string = {};
    err.location.line   = 0;
    err.message         = message;
    return true;
}
