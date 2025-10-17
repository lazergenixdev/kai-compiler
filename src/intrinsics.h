static inline Kai_u32 kai_intrinsics_clz32(Kai_u32 value)
{
#if defined(KAI_COMPILER_CLANG) || defined(KAI_COMPILER_GNU)
    return __builtin_clz(value);
#elif defined(KAI_COMPILER_MSVC)
    DWORD index;
    if (_BitScanReverse(&index, value) == 0)
        return 32;
    return (Kai_u32)(31 - index);
#endif
}

static inline Kai_u32 kai_intrinsics_ctz32(Kai_u32 value)
{
#if defined(KAI_COMPILER_CLANG) || defined(KAI_COMPILER_GNU)
    return __builtin_ctz(value);
#elif defined(KAI_COMPILER_MSVC)
    DWORD index;
    if (_BitScanForward(&index, value) == 0)
        return 32;
    return (Kai_u32)(index);
#endif
}

static inline Kai_u32 kai_intrinsics_clz64(Kai_u64 value)
{
#if defined(KAI_COMPILER_CLANG) || defined(KAI_COMPILER_GNU)
    return __builtin_clzll(value);
#elif defined(KAI_COMPILER_MSVC)
    DWORD index;
    if (_BitScanReverse64(&index, value) == 0)
        return 64;
    return (Kai_u32)(63 - index);
#endif
}

static inline Kai_u32 kai_intrinsics_ctz64(Kai_u64 value)
{
#if defined(KAI_COMPILER_CLANG) || defined(KAI_COMPILER_GNU)
    return __builtin_ctzll(value);
#elif defined(KAI_COMPILER_MSVC)
    DWORD index;
    if (_BitScanForward64(&index, value) == 0)
        return 64;
    return (Kai_u32)(index);
#endif
}

// 128 bit integers (unsigned)

// Always use fallback when compiling for WASM
#if defined(__wasm__) && !defined(KAI_NO_INTRINSIC_128)
#	define KAI_NO_INTRINSIC_128
#endif

#if !defined(KAI_NO_INTRINSIC_128) && (defined(KAI_COMPILER_CLANG) || defined(KAI_COMPILER_GNU))
    typedef unsigned __int128 Kai_u128;
#   define kai_intrinsics_u128_low(Value) (Kai_u64)(Value)
#   define kai_intrinsics_u128_high(Value) (Kai_u64)(Value >> 64)
#   define kai_intrinsics_u128_multiply(A,B) ((Kai_u128)(A) * (Kai_u128)(B))
#elif !defined(KAI_NO_INTRINSIC_128) && defined(KAI_COMPILER_MSVC)
    typedef struct { unsigned __int64 low, high; } Kai_u128;
#   define kai_intrinsics_u128_low(Value) (Value).low
#   define kai_intrinsics_u128_high(Value) (Value).high
    static inline Kai_u128 kai_intrinsics_u128_multiply(Kai_u64 A, Kai_u64 B) {
        Kai_u128 r;
        r.low = _umul128(A, B, &r.high);
        return r;
    }
#else // fallback
    typedef struct { Kai_u64 low, high; } Kai_u128;
#   define kai_intrinsics_u128_low(Value) (Value).low
#   define kai_intrinsics_u128_high(Value) (Value).high
    static inline Kai_u128 kai_intrinsics_u128_multiply(Kai_u64 A, Kai_u64 B) {
    // https://www.codeproject.com/Tips/618570/UInt-Multiplication-Squaring
        Kai_u64 a1 = (A & 0xffffffff);
        Kai_u64 b1 = (B & 0xffffffff);
        Kai_u64 t = (a1 * b1);
        Kai_u64 w3 = (t & 0xffffffff);
        Kai_u64 k = (t >> 32);

        A >>= 32;
        t = (A * b1) + k;
        k = (t & 0xffffffff);
        Kai_u64 w1 = (t >> 32);

        B >>= 32;
        t = (a1 * B) + k;
        k = (t >> 32);

        return (Kai_u128) {
            .high = (A * B) + w1 + k,
            .low = (t << 32) + w3,
        };
    }
#endif

// TODO: only dev builds
#if defined(KAI_PLATFORM_APPLE) || defined(KAI_PLATFORM_LINUX)
#include "execinfo.h"
void kai__debug_print_stacktrace(void) {
    void *buffer[128];
    int nptrs = backtrace(buffer, sizeof(buffer)/sizeof(void*));
    char **symbols = backtrace_symbols(buffer, nptrs);

    if (symbols == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    printf("Stack trace:\n");
    for (int i = 1; i < nptrs; i++) {
        printf("%s\n", symbols[i]);
    }

    free(symbols);
}
#else
void kai__debug_print_stacktrace(void) {}
#endif

#if !defined(KAI_PLATFORM_WASM)
#define __env_console_log(...) (void)0
#endif

