
// Public API

#ifndef KAI_API
#define KAI_API(R) extern R
#endif

// Detect Compiler

#if defined(__clang__)
#	define KAI_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#	define KAI_COMPILER_GNU
#elif defined(_MSC_VER)
#	define KAI_COMPILER_MSVC
#else
#	error "[KAI] Compiler not *officially* supported, but feel free to remove this line"
#endif

// Detect Platform

#if defined(__wasm__)
#   define KAI_PLATFORM_WASM
#elif defined(_WIN32)
#	define KAI_PLATFORM_WINDOWS
#elif defined(__APPLE__)
#   define KAI_PLATFORM_APPLE
#elif defined(__linux__)
#   define KAI_PLATFORM_LINUX
#else
#	define KAI_PLATFORM_UNKNOWN
#	pragma message("[KAI] warning: Platform not recognized! (KAI__PLATFORM_UNKNOWN defined)")
#endif

// Detect Architecture

#if defined(__wasm__)
#	define KAI_MACHINE_WASM// God bless your soul 🙏
#elif defined(__x86_64__) || defined(_M_X64)
#	define KAI_MACHINE_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#	define KAI_MACHINE_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
#	define KAI_MACHINE_ARM64
#else
#	error "[KAI] Architecture not supported!"
#endif

// C++ Struct vs C Structs

#if defined(__cplusplus)
#    define KAI_STRUCT(X) X
#else
#    define KAI_STRUCT(X) (X)
#endif

// Ensure correct encoding

#if defined(KAI_COMPILER_MSVC)
#	define KAI_UTF8(LITERAL) u8##LITERAL
#else
#	define KAI_UTF8(LITERAL) LITERAL
#endif

// Fatal errors & Assertions

#ifndef kai_assert
#define kai_assert(EXPR) if (!(EXPR)) kai_fatal_error("Assertion Failed", #EXPR)
#endif

#ifndef kai_fatal_error
#define kai_fatal_error(DESC, MESSAGE) \
    (kai__debug_print_stacktrace(), printf("[\x1b[92mkai.h:%i\x1b[0m] \x1b[91m%s\x1b[0m: %s\n", __LINE__, DESC, MESSAGE), exit(1))
#endif

#ifndef kai_unreachable
#define kai_unreachable() kai_fatal_error("Assertion Failed", "Unreachable was reached! D:")
#endif

#ifndef kai__todo
#define kai__todo(...)                                       \
do { char __message__[1024] = {0};                           \
    int __length__ = snprintf(__message__, sizeof(__message__), __VA_ARGS__); \
    snprintf(__message__ + __length__, sizeof(__message__) - __length__, " (%s)", __func__); \
    kai_fatal_error("TODO", __message__);                    \
} while (0)
#endif

// Hash Table API

// K: key type, T: pointer to hash table, KEY: key value
#define kai_table_set(K,T,KEY,...)                                                                         \
do {                                                                                                       \
    kai_raw_table_grow((Kai_Raw_Hash_Table*)(T), allocator, sizeof((T)->keys[0]), sizeof((T)->values[0])); \
    Kai_u64 __hash__ = kai_hash_ ## K(KEY);                                                                \
    Kai_u32 __index__ = (Kai_u32)(__hash__);                                                               \
    while (kai_raw_table_next_match((Kai_Raw_Hash_Table*)(T), __hash__, &__index__))                       \
        if (kai_ ## K ## _equals(KEY, (T)->keys[__index__]))                                               \
            break;                                                                                         \
    (T)->keys[__index__] = KEY;                                                                            \
    (T)->values[__index__] = __VA_ARGS__;                                                                  \
} while(0)

#define kai_table_find(K,T,KEY) kai_raw_table_find_ ## K((Kai__ ## K ## _HashTable*)(T), KEY)
