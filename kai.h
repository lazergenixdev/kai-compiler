/**
 *  < Kai >---< Scripting Language >
 *
 * @author:       lazergenixdev@gmail.com
 * @development:  https://github.com/lazergenixdev/kai-compiler
 * @license:      GNU GPLv3 (see end of file)
 */
/*
--- INCLUDE OPTIONS -------------------------------------------------------------------------------------

    define  KAI_IMPLEMENTATION  in one compilation unit (.c/.cpp file) to use the library

    define  KAI_DONT_USE_MEMORY_API  to disable default memory allocator
    define  KAI_DONT_USE_WRITER_API  to disable stdout and file writer
    define  KAI_DONT_USE_CPP_API     to disable C++ API (experimental)

    define  KAI_HAVE_FATAL_ERROR     to use a costum error function on fatal errors

        Function Signature:
        void kai__fatal_error(char const* Desc, char const* Message, int Line);

    NOTE: When not using these APIs, no C standard libraries are used!
          (except stdint and stddef, but these can trivially be replaced)

--- CONVENTIONS -----------------------------------------------------------------------------------------

    Types:           begin with "Kai_"
    Functions:       begin with "kai_"
    Enums/Macros:    begin with "KAI_"
    Internal API:    begin with "KAI__" or "kai__" or "Kai__"

    This header is divided into sections with `KAI__SECTION` for convenience
        KAI__SECTION_SETUP
        KAI__SECTION_BUILTIN_TYPES
        KAI__SECTION_PLATFORM_INTRINSICS
        KAI__SECTION_TYPE_INFO_STRUCTS
        KAI__SECTION_CORE_STRUCTS
        KAI__SECTION_INTERNAL_STRUCTS
        KAI__SECTION_SYNTAX_TREE
        KAI__SECTION_PROGRAM
        KAI__SECTION_CORE_API
        KAI__SECTION_NUMBER_API
        KAI__SECTION_INTERNAL_API
        KAI__SECTION_BYTECODE_API
        KAI__SECTION_MEMORY_API
        KAI__SECTION_WRITER_API
        KAI__SECTION_CPP_API

---------------------------------------------------------------------------------------------------------

    CODE STYLE:

    I have one very simple rule, "always" choose formatting that increases visibility

    This means:
        - Strict following of 'CONVENTIONS' above
            - Note: Macros that act like functions are allowed to look like functions
        - No complicated macros (anything that makes the code NOT look like C)
        - Align similar lines together
		
	OTHER:

	- Allocator is assumed to always succeed
		-> this prevents an explosions of checks for each allocation

*/
#ifndef KAI__H
#define KAI__H
#include <stdint.h> // --> uint32, uint64, ...
#include <stddef.h> // --> NULL
#ifndef KAI_DONT_USE_MEMORY_API
#include <stdlib.h> // --> realloc, free
#endif
#ifndef KAI_DONT_USE_WRITER_API
#include <stdio.h>  // --> stdout, fopen, fclose
#include <locale.h> // --> setlocale(UTF8)
#endif
#ifndef KAI_DONT_USE_CPP_API
#ifdef __cplusplus
#include <string>
#include <fstream>
#endif
#endif

#ifndef KAI__SECTION_SETUP

#define KAI_VERSION_MAJOR 0
#define KAI_VERSION_MINOR 1
#define KAI_VERSION_PATCH 0
#define KAI__MAKE_VERSION_STRING_HELPER(X,Y,Z) #X "." #Y "." #Z
#define KAI__MAKE_VERSION_STRING(X,Y,Z) KAI__MAKE_VERSION_STRING_HELPER(X,Y,Z) // macros suck
#define KAI_VERSION_STRING KAI__MAKE_VERSION_STRING(KAI_VERSION_MAJOR,KAI_VERSION_MINOR,KAI_VERSION_PATCH)

// Detect Compiler

#if defined(__clang__)
#	define KAI__COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#	define KAI__COMPILER_GNU
#elif defined(_MSC_VER)
#	define KAI__COMPILER_MSVC
#else
#	error "[KAI] Compiler not *officially* supported, but feel free to remove this line"
#endif

// Detect Platform

#if defined(__WASM__)
#   define KAI__PLATFORM_WASM
#elif defined(_WIN32)
#	define KAI__PLATFORM_WINDOWS
#elif defined(__APPLE__)
#   define KAI__PLATFORM_APPLE
#elif defined(__linux__)
#   define KAI__PLATFORM_LINUX
#else
#	define KAI__PLATFORM_UNKNOWN
#	pragma message("[KAI] warning: Platform not recognized! (KAI__PLATFORM_UNKNOWN defined)")
#endif

// Detect Architecture

#if defined(__WASM__)
#	define KAI__MACHINE_WASM// God bless your soul 🙏
#elif defined(__x86_64__) || defined(_M_X64)
#	define KAI__MACHINE_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#	define KAI__MACHINE_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
#	define KAI__MACHINE_ARM64
#else
#	error "[KAI] Architecture not supported!"
#endif

#if !defined(KAI_API)
#	define KAI_API(RETURN_TYPE) extern RETURN_TYPE
#endif

#if defined(__cplusplus)
#    define KAI_STRUCT(X) X
#else
#    define KAI_STRUCT(X) (X)
#endif

#if defined(KAI__COMPILER_MSVC)
#	define KAI_UTF8(STRING) u8##STRING
#else
#	define KAI_UTF8(STRING) STRING
#endif

// Helper Macros

#define KAI_BOOL(EXPR) ((Kai_bool)((EXPR) ? KAI_TRUE : KAI_FALSE))

#if defined(KAI__COMPILER_MSVC)
#   define KAI_CONSTANT_STRING(S) {(Kai_u8*)(S), sizeof(S)-1}
#else
#   define KAI_CONSTANT_STRING(S) KAI_STRING(S)
#endif
#define KAI_STRING(LITERAL) KAI_STRUCT(Kai_string){(Kai_u8*)(LITERAL), sizeof(LITERAL)-1}
#define KAI_EMPTY_STRING KAI_STRUCT(Kai_string){0}

#endif

#if defined(KAI__COMPILER_GNU) || defined(KAI__COMPILER_CLANG)
#	pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmultichar" // ? this is a feature, why warning??
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers" // ? sometimes you just want default zero, seriously clang, what is wrong with you?
#elif defined(KAI__COMPILER_MSVC)
#	pragma warning(push) // Ex: pragma warning(disable: 4127)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KAI__SECTION_BUILTIN_TYPES

enum {
    KAI_FALSE = 0,
    KAI_TRUE  = 1,
};

#define KAI_X_PRIMITIVE_TYPES \
    X( int8_t   , s8  , S8  ) \
    X( int16_t  , s16 , S16 ) \
    X( int32_t  , s32 , S32 ) \
    X( int64_t  , s64 , S64 ) \
    X( uint8_t  , u8  , U8  ) \
    X( uint16_t , u16 , U16 ) \
    X( uint32_t , u32 , U32 ) \
    X( uint64_t , u64 , U64 ) \
    X( float    , f32 , F32 ) \
    X( double   , f64 , F64 )

#define X(TYPE, NAME, _) typedef TYPE Kai_##NAME;
    KAI_X_PRIMITIVE_TYPES
#undef X

// 128 bit integers (unsigned)

#if defined(KAI__PLATFORM_WASM)
    typedef struct { Kai_u64 low, high; } Kai_u128;
#   define kai__u128_low(Value) (Value).low
#   define kai__u128_high(Value) (Value).high
    static inline Kai_u128 kai__u128_multiply(Kai_u64 A, Kai_u64 B) {
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
#elif defined(KAI__COMPILER_CLANG) || defined(KAI__COMPILER_GNU)
    typedef unsigned __int128 Kai_u128;
#   define kai__u128_low(Value) (Kai_u64)(Value)
#   define kai__u128_high(Value) (Kai_u64)(Value >> 64)
#   define kai__u128_multiply(A,B) ((Kai_u128)(A) * (Kai_u128)(B))
#elif defined(KAI__COMPILER_MSVC)
    typedef struct { unsigned __int64 low, high; } Kai_u128;
#   define kai__u128_low(Value) (Value).low
#   define kai__u128_high(Value) (Value).high
    static inline Kai_u128 kai__u128_multiply(Kai_u64 A, Kai_u64 B) {
        Kai_u128 r;
        r.low = _umul128(A, B, &r.high);
        return r;
    }
#endif

typedef uint8_t   Kai_bool;
typedef void*     Kai_ptr;
typedef  intptr_t Kai_int;
typedef uintptr_t Kai_uint;

typedef struct Kai_Type_Info* Kai_Type;

#define X(TYPE, NAME, _)                                         \
    typedef struct { TYPE x, y;          } Kai_vector2_##NAME;   \
    typedef struct { TYPE x, y, z;       } Kai_vector3_##NAME;   \
    typedef struct { TYPE x, y, z, w;    } Kai_vector4_##NAME;   \
    typedef struct { TYPE elements[2*2]; } Kai_matrix2x2_##NAME; \
    typedef struct { TYPE elements[3*3]; } Kai_matrix3x3_##NAME; \
    typedef struct { TYPE elements[4*4]; } Kai_matrix4x4_##NAME;
    KAI_X_PRIMITIVE_TYPES
#undef X

typedef struct {
    Kai_u8* data;
    Kai_u32 count;
} Kai_string;

typedef struct {
    Kai_u32 count;
    Kai_u32 offset;
} Kai_range;

typedef struct {
    Kai_ptr data;
    Kai_u32 count;
} Kai_slice;

#endif
#ifndef KAI__SECTION_PLATFORM_INTRINSICS

static inline Kai_u32 kai_intrinsic_clz(Kai_u64 Value)
{
#if defined(KAI__COMPILER_CLANG) || defined(KAI__COMPILER_GNU)
    return __builtin_clzll(Value);
#elif defined(KAI__COMPILER_MSVC)
    Kai_u32 index;
    if (_BitScanReverse64(&index, Value) == 0)
        return 64;
    return 63 - index;
#endif
}
static inline Kai_u32 kai_intrinsic_ctz(Kai_u64 Value)
{
#if defined(KAI__COMPILER_CLANG) || defined(KAI__COMPILER_GNU)
    return __builtin_ctzll(Value);
#elif defined(KAI__COMPILER_MSVC)
    Kai_u32 index;
    if (_BitScanForward64(&index, Value) == 0)
        return 64;
    return index;
#endif
}

#endif
#ifndef KAI__SECTION_TYPE_INFO_STRUCTS

enum {
    KAI_TYPE_TYPE      = 0, // No Type_Info struct for this.
    KAI_TYPE_INTEGER   = 1,
    KAI_TYPE_FLOAT     = 2,
    KAI_TYPE_POINTER   = 3,
    KAI_TYPE_PROCEDURE = 4,
    KAI_TYPE_ARRAY     = 5,
    KAI_TYPE_STRUCT    = 6,
};

typedef struct Kai_Type_Info {
    Kai_u8 type;
} Kai_Type_Info;

typedef struct {
    Kai_u8   type;
    Kai_u8   bits; // 8, 16, 32, 64
    Kai_bool is_signed;
} Kai_Type_Info_Integer;

typedef struct {
    Kai_u8 type;
    Kai_u8 bits; // 32, 64
} Kai_Type_Info_Float;

typedef struct {
    Kai_u8   type;
    Kai_Type sub_type;
} Kai_Type_Info_Pointer;

typedef struct {
    Kai_u8    type;
    Kai_u8    in_count;
    Kai_u8    out_count;
    Kai_Type* sub_types; // list of parameters types, then return types
} Kai_Type_Info_Procedure;

typedef struct {
    Kai_u8     type;
    Kai_u32    rows;
    Kai_u32    cols;
    Kai_Type   sub_type;
} Kai_Type_Info_Array;

typedef struct {
    Kai_u8       type;
    Kai_u32      field_count;
    Kai_Type   * field_types;
    Kai_string * field_names;
} Kai_Type_Info_Struct;

// TODO: move to implementation
#define KAI__X_TYPE_INFO_DEFINITIONS                                                                                  \
    X(Kai_Type_Info         , kai__type_info_type , { .type = KAI_TYPE_TYPE })                                        \
    X(Kai_Type_Info_Integer , kai__type_info_s8   , { .type = KAI_TYPE_INTEGER, .bits = 8,  .is_signed = KAI_TRUE })  \
    X(Kai_Type_Info_Integer , kai__type_info_s16  , { .type = KAI_TYPE_INTEGER, .bits = 16, .is_signed = KAI_TRUE })  \
    X(Kai_Type_Info_Integer , kai__type_info_s32  , { .type = KAI_TYPE_INTEGER, .bits = 32, .is_signed = KAI_TRUE })  \
    X(Kai_Type_Info_Integer , kai__type_info_s64  , { .type = KAI_TYPE_INTEGER, .bits = 64, .is_signed = KAI_TRUE })  \
    X(Kai_Type_Info_Integer , kai__type_info_u8   , { .type = KAI_TYPE_INTEGER, .bits = 8,  .is_signed = KAI_FALSE }) \
    X(Kai_Type_Info_Integer , kai__type_info_u16  , { .type = KAI_TYPE_INTEGER, .bits = 16, .is_signed = KAI_FALSE }) \
    X(Kai_Type_Info_Integer , kai__type_info_u32  , { .type = KAI_TYPE_INTEGER, .bits = 32, .is_signed = KAI_FALSE }) \
    X(Kai_Type_Info_Integer , kai__type_info_u64  , { .type = KAI_TYPE_INTEGER, .bits = 64, .is_signed = KAI_FALSE }) \
    X(Kai_Type_Info_Float   , kai__type_info_f32  , { .type = KAI_TYPE_FLOAT, .bits = 32 })                           \
    X(Kai_Type_Info_Float   , kai__type_info_f64  , { .type = KAI_TYPE_FLOAT, .bits = 64 })

#define X(TYPE, NAME, ...) extern TYPE NAME;
    KAI__X_TYPE_INFO_DEFINITIONS
#undef X

#endif
#ifndef KAI__SECTION_CORE_STRUCTS

enum {
    //         ID   SIZE
    KAI_U8   =  0 | (1 << 4),
    KAI_U16  =  1 | (2 << 4),
    KAI_U32  =  2 | (4 << 4),
    KAI_U64  =  3 | (8 << 4),
    KAI_S8   =  4 | (1 << 4),
    KAI_S16  =  5 | (2 << 4),
    KAI_S32  =  6 | (4 << 4),
    KAI_S64  =  7 | (8 << 4),
    KAI_F32  =  8 | (4 << 4),
    KAI_F64  =  9 | (8 << 4),
    KAI_TYPE = 10 | (sizeof(void*) << 4),

    // Special type for Writer to
    // only use the fill_character + min_count
    KAI_FILL = 0x800,
};

typedef union {
#define X(TYPE, NAME, _) Kai_##NAME NAME;
    KAI_X_PRIMITIVE_TYPES
#undef X
} Kai_Value;

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- String Writer ---------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

enum {
    KAI_COLOR_PRIMARY,
    KAI_COLOR_SECONDARY,
    KAI_COLOR_IMPORTANT,
    KAI_COLOR_IMPORTANT_2,
    KAI_COLOR_DECORATION,
    KAI__COLOR_COUNT,
};
typedef Kai_u32 Kai_Write_Color;

enum {
    KAI_WRITE_FLAG_NONE        = 0,
    KAI_WRITE_FLAG_HEXIDECIMAL = 1 << 0,
};
typedef Kai_u32 Kai_Write_Flags;

typedef struct {
    Kai_u32 flags;
    Kai_u32 min_count; // default is 0 (no minimum)
    Kai_u32 max_count; // default is 0 (no maximum)
    Kai_u8  fill_character;
} Kai_Write_Format;

typedef void Kai_P_Write_String (void* User, Kai_string String);
typedef void Kai_P_Write_Value  (void* User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format Format);
typedef void Kai_P_Set_Color    (void* User, Kai_Write_Color Color);

typedef struct {
    Kai_P_Write_String * write_string;
    Kai_P_Write_Value  * write_value;
    Kai_P_Set_Color    * set_color;
    void               * user;
} Kai_Writer;

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Base Memory Allocator -------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

enum {
    KAI_MEMORY_READ       = 1 << 0,
    KAI_MEMORY_WRITE      = 1 << 1,
    KAI_MEMORY_EXECUTE    = 1 << 2,
    KAI_MEMORY_READ_WRITE = KAI_MEMORY_READ | KAI_MEMORY_WRITE,
};

enum {
	KAI_MEMORY_ALLOCATE_WRITE_ONLY, // input: Size
	KAI_MEMORY_SET_EXECUTABLE,      // input: Ptr, Size
	KAI_MEMORY_FREE,                // input: Ptr, Size
};

typedef void* Kai_P_Memory_Heap_Allocate (void* User, void* Ptr, Kai_u32 New_Size, Kai_u32 Old_Size);
typedef void* Kai_P_Memory_JIT           (void* User, void* Ptr, Kai_u32 Size, Kai_u32 Operation);

typedef struct {
    Kai_P_Memory_JIT           * jit;
    Kai_P_Memory_Heap_Allocate * heap_allocate;
    void                       * user;
    Kai_u32                      page_size;
} Kai_Allocator;

typedef struct {
    void    * data;
    Kai_u32   size;
} Kai_Memory;

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Errors ----------------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

enum {
    KAI_SUCCESS = 0,
	KAI_ERROR_MEMORY,   // allocator failed
    KAI_ERROR_SYNTAX,
    KAI_ERROR_SEMANTIC,
    KAI_ERROR_INFO,
    KAI_ERROR_FATAL,    // means compiler bug probably
    KAI_ERROR_INTERNAL, // means compiler error unrelated to source code (e.g. out of memory)
    KAI__RESULT_COUNT,
};
typedef Kai_u32 Kai_Result;

typedef struct {
    Kai_string file_name;
    Kai_string string;
    Kai_u8* source; // source code for this file // TODO: *u8 -> string
    Kai_u32 line;
} Kai_Location;

typedef struct Kai_Error {
    Kai_Result         result;
    Kai_Location       location;
    Kai_string         message;
    Kai_string         context;
    Kai_Memory         memory;
    struct Kai_Error * next;
} Kai_Error;

#endif
#ifndef KAI__SECTION_INTERNAL_STRUCTS

#define KAI__SLICE(T) \
struct {              \
    Kai_u32 count;    \
    T*      elements; \
}

#define KAI__ARRAY(T) \
struct {              \
    Kai_u32 count;    \
    Kai_u32 capacity; \
    T*      elements; \
}

#define KAI__SMALL_ARRAY(T,N) \
struct {                      \
    Kai_u32 count;            \
    Kai_u32 capacity;         \
    union {                   \
        T*  elements;         \
        T   storage[N];       \
    };                        \
}

#define KAI__HASH_TABLE(T) \
struct {                   \
    Kai_u32      count;    \
    Kai_u32      capacity; \
    Kai_u64    * occupied; \
    Kai_u64    * hashes;   \
    Kai_string * keys;     \
    T          * values;   \
}

#define KAI__LINKED_LIST(T) \
struct {                    \
    T head;                 \
    T last;                 \
}

typedef KAI__ARRAY(Kai_u8) Kai_Byte_Array;

#define KAI__MINIMUM_ARENA_BUCKET_SIZE 0x40000

typedef struct Kai__Arena_Bucket {
    struct Kai__Arena_Bucket* prev;
} Kai__Arena_Bucket;

typedef struct {
    Kai__Arena_Bucket*  current_bucket;
    Kai_u32             current_allocated;
    Kai_u32             bucket_size;
    Kai_Allocator       allocator;
} Kai__Arena_Allocator;

// Number type used for number literals, I call it the dual mantissa number.
//  - abs(value) = (n/d) * 2^e
//  - (161 bit) + 31 padding
typedef struct {
    Kai_u64 n;
    Kai_u64 d; // never 0 (except when n == 0)
    Kai_u32 is_neg;
    Kai_s32 e;
} Kai_Number;

#endif
#ifndef KAI__SECTION_SYNTAX_TREE

typedef struct Kai_Expr_Base* Kai_Expr; // Expression Nodes
typedef struct Kai_Expr_Base* Kai_Stmt; // Statement Nodes

#define KAI__X_AST_NODE_IDS         \
    X(KAI_EXPR_IDENTIFIER     , 0)  \
    X(KAI_EXPR_STRING         , 1)  \
    X(KAI_EXPR_NUMBER         , 2)  \
    X(KAI_EXPR_UNARY          , 3)  \
    X(KAI_EXPR_BINARY         , 4)  \
    X(KAI_EXPR_PROCEDURE_TYPE , 5)  \
    X(KAI_EXPR_PROCEDURE_CALL , 6)  \
    X(KAI_EXPR_PROCEDURE      , 7)  \
    X(KAI_EXPR_CODE           , 8)  \
    X(KAI_EXPR_STRUCT         , 9)  \
    X(KAI_EXPR_ARRAY          , 10) \
    X(KAI_STMT_RETURN         , 11) \
    X(KAI_STMT_DECLARATION    , 12) \
    X(KAI_STMT_ASSIGNMENT     , 13) \
    X(KAI_STMT_IF             , 14) \
    X(KAI_STMT_FOR            , 15) \
    X(KAI_STMT_DEFER          , 16) \
    X(KAI_STMT_COMPOUND       , 17) \

typedef enum {
#define X(NAME,VALUE) NAME = VALUE,
    KAI__X_AST_NODE_IDS
#undef X
} Kai_Node_ID;

enum {
    KAI_DECL_FLAG_CONST = 1 << 0,
    KAI_DECL_FLAG_USING = 1 << 1,
};

#define KAI_BASE_MEMBERS    \
    Kai_u8     id;          \
    Kai_u8     flags;       \
    Kai_string source_code; \
    Kai_string name;        \
    Kai_Expr   next;        \
    Kai_u32    line_number
//	Kai_Type_Info* _type;

typedef struct Kai_Expr_Base {
    KAI_BASE_MEMBERS;
} Kai_Expr_Base;

typedef struct {
    KAI_BASE_MEMBERS;
} Kai_Expr_Identifier;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Number value;
} Kai_Expr_Number;

typedef struct {
    KAI_BASE_MEMBERS;
} Kai_Expr_String;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr expr;
    Kai_u32  op;
} Kai_Expr_Unary;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr left;
    Kai_Expr right;
    Kai_u32  op;
} Kai_Expr_Binary;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr  proc;
    Kai_Expr  arg_head;
    Kai_u8    arg_count;
} Kai_Expr_Procedure_Call;
 
typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr  in_out_expr;
    Kai_u8    in_count;
    Kai_u8    out_count;
} Kai_Expr_Procedure_Type;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr in_out_expr;
    Kai_Stmt body;
    Kai_u8   in_count;
    Kai_u8   out_count;

    Kai_u32 _scope;
} Kai_Expr_Procedure;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_u32  rows;
    Kai_u32  cols;
    Kai_Expr expr;
} Kai_Expr_Array;

typedef struct {
    KAI_BASE_MEMBERS;
	Kai_u32  field_count;
    Kai_Stmt head;
} Kai_Expr_Struct;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr expr; // optional
} Kai_Stmt_Return;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr expr;
    Kai_Expr type; // optional
} Kai_Stmt_Declaration;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr left;
    Kai_Expr expr;
} Kai_Stmt_Assignment;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Stmt head;

    Kai_u32 _scope;
} Kai_Stmt_Compound;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Expr expr;
    Kai_Stmt then_body;
    Kai_Stmt else_body;
} Kai_Stmt_If;

typedef struct {
    KAI_BASE_MEMBERS;
    Kai_Stmt   body;
    Kai_Expr   from;
    Kai_Expr   to; // optional (interates through `from` if this is null)
    Kai_string iterator_name;
} Kai_Stmt_For;

typedef struct {
    Kai_Stmt_Compound    root;
    Kai_string           source_filename; // TODO: rename
    Kai__Arena_Allocator allocator; // TODO: arena for each syntax tree? ok or no?
} Kai_Syntax_Tree;

typedef struct {
    Kai_string     source_code; // input
    Kai_Allocator  allocator;   // input
    Kai_Error*     error;       // output [optional]
} Kai_Syntax_Tree_Create_Info;

#endif
#ifndef KAI__SECTION_PROGRAM

typedef struct {
    Kai_u32 Interpreter_Max_Step_Count; // default = 1000000
    Kai_u32 Interpreter_Max_Call_Depth; // default = 1024
} Kai_Compile_Options;

typedef struct {
    char const*  name;
    void const*  address;
    Kai_u8       input_count; // TODO: more than this for typechecking
} Kai_Native_Procedure;

typedef struct {
    struct {
		Kai_string code;
	} source;
    Kai_Allocator          allocator;
    Kai_Error            * error;
    Kai_Native_Procedure * native_procedures;
    Kai_u32                native_procedure_count;
	Kai_Compile_Options    options;
} Kai_Program_Create_Info;

typedef struct {
    void*                     platform_machine_code;
    Kai_u32                   code_size;
    KAI__HASH_TABLE(Kai_uint) procedure_table;
    Kai_Allocator             allocator;
} Kai_Program;

#endif
#ifndef KAI__SECTION_CORE_API

KAI_API (Kai_vector3_u32) kai_version        (void);
KAI_API (Kai_string)      kai_version_string (void);

KAI_API (Kai_bool)   kai_string_equals (Kai_string A, Kai_string B);
KAI_API (Kai_string) kai_string_from_c (char const* String);

KAI_API (void)  kai_destroy_error  (Kai_Error* Error, Kai_Allocator* Allocator);

/// @param out_Syntax_Tree Must be freed using @ref(kai_destroy_syntax_tree)
KAI_API (Kai_Result) kai_create_syntax_tree  (Kai_Syntax_Tree_Create_Info* Info, Kai_Syntax_Tree* out_Syntax_Tree);
KAI_API (void)       kai_destroy_syntax_tree (Kai_Syntax_Tree* Syntax_Tree);

// TODO: use tagged union instead of specialized functions
KAI_API (Kai_Result) kai_create_program  (Kai_Program_Create_Info* Info, Kai_Program* out_Program);
KAI_API (void)       kai_destroy_program (Kai_Program Program);

//! @note With WASM backend you cannot get any function pointers
KAI_API (void*) kai_find_procedure (Kai_Program Program, Kai_string Name, Kai_Type opt_Type);

KAI_API (void) kai_write_syntax_tree (Kai_Writer* Writer, Kai_Syntax_Tree* Tree);
KAI_API (void) kai_write_error       (Kai_Writer* Writer, Kai_Error* Error);
KAI_API (void) kai_write_type        (Kai_Writer* Writer, Kai_Type Type);
KAI_API (void) kai_write_expression  (Kai_Writer* Writer, Kai_Expr Expr);

#endif
#ifndef KAI__SECTION_NUMBER_API

KAI_API (void) kai_write_number(Kai_Writer* Writer, Kai_Number Number);
KAI_API (Kai_s32) kai_number_compare(Kai_Number A, Kai_Number B); // -1 (less than), 0 (equal), 1 (greater than)
KAI_API (Kai_Number) kai_number_normalize(Kai_Number A);

// Arithmetic Operations

KAI_API (Kai_Number) kai_number_neg(Kai_Number A); // -A
KAI_API (Kai_Number) kai_number_inv(Kai_Number A); // 1 / A
KAI_API (Kai_Number) kai_number_add(Kai_Number A, Kai_Number B); // A + B
KAI_API (Kai_Number) kai_number_sub(Kai_Number A, Kai_Number B); // A - B
KAI_API (Kai_Number) kai_number_mul(Kai_Number A, Kai_Number B); // A * B
KAI_API (Kai_Number) kai_number_div(Kai_Number A, Kai_Number B); // A / B
KAI_API (Kai_Number) kai_number_pow_int(Kai_Number A, Kai_u32 Exp);

// Conversions

#define kai_number_is_integer(Number) ((Number).d == 1 && (Number).e >= 0)
KAI_API (Kai_f64) kai_number_to_f64(Kai_Number Number);

// Parsing

KAI_API (Kai_Number) kai_number_parse_whole(Kai_string Text, Kai_u32* Offset, Kai_u32 Base);
KAI_API (Kai_Number) kai_number_parse_decimal(Kai_string Text, Kai_u32* Offset); // ! Base 10 only
KAI_API (Kai_Number) kai_number_parse_exponent(Kai_string Text, Kai_u32* Offset); // ! Base 10 only

#endif
#ifndef KAI__SECTION_BYTECODE_API

// Bytecode Operations (BOP)
enum Kai_Bytecode_Op {
	// TODO: does compare need to be separate?
    KAI_BOP_NOP              = 0,  //  nop
    KAI_BOP_COPY             = 1,  //  %0 <- %1
    KAI_BOP_LOAD_CONSTANT    = 2,  //  %0 <- load_constant.u32 8
    KAI_BOP_MATH             = 3,  //  %8 <- mul.u32 %0, %1
    KAI_BOP_MATH_CONSTANT    = 4,  //  %8 <- mul.u32 %0, %1
    KAI_BOP_COMPARE          = 5,  //  %3 <- compare.gt.s64 %0, %3
    KAI_BOP_COMPARE_CONSTANT = 6,  //  %3 <- compare.gt.s64 %0, 420
    KAI_BOP_BRANCH           = 7,  //  branch %5 {0x43}
    KAI_BOP_JUMP             = 8,  //  jump 0x43
    KAI_BOP_CALL             = 9,  //  %4, %5 <- call {0x34} (%1, %2)
    KAI_BOP_RETURN           = 10, //  ret %1, %2
    KAI_BOP_NATIVE_CALL      = 11, //  %4 <- native_call {0x100f3430} (%1, %2)
    KAI_BOP_LOAD             = 12, //  %5 <- u64 [%1 + 0x24]
    KAI_BOP_STORE            = 13, //  u32 [%2 + 0x8] <- %5
    KAI_BOP_STACK_ALLOC      = 14, //  %6 <- stack_alloc 48
    KAI_BOP_STACK_FREE       = 15, //  stack_free 48
    KAI_BOP_CHECK_ADDRESS    = 99, //  check_address.u32 (%1 + 0x8)
};

enum Kai_Bytecode_Math {
    KAI_MATH_ADD = 0,
    KAI_MATH_SUB = 1,
    KAI_MATH_MUL = 2,
    KAI_MATH_DIV = 3,
};

enum Kai_Bytecode_Cmp {
    KAI_CMP_LT = 0,
    KAI_CMP_GE = 1,
    KAI_CMP_GT = 2,
    KAI_CMP_LE = 3,
    KAI_CMP_EQ = 4,
    KAI_CMP_NE = 5,
};

enum {
    KAI_BC_ERROR_BRANCH   = 1, // bcs_set_branch
    KAI_BC_ERROR_OVERFLOW = 2, // bytecode instruction went outside of buffer
};

enum {
    KAI_INTERP_FLAGS_DONE       = 1 << 0, // returned from call stack
    KAI_INTERP_FLAGS_OVERFLOW   = 1 << 1, // the next bytecode instruction went outside of buffer
    KAI_INTERP_FLAGS_INCOMPLETE = 1 << 2, // a bytecode instruction being executed was not able to be fully read
    KAI_INTERP_FLAGS_INVALID    = 1 << 3, // instruction was invalid (e.g. %0xFFFFFFF <- 69)
};

typedef Kai_u32 Kai_Reg;

typedef struct {
	Kai_Allocator*     allocator;
    KAI__ARRAY(Kai_u8) code;
} Kai_Bytecode_Encoder;

typedef struct {
    Kai_u8* bytecode;
    Kai_u32 cursor;
	Kai_u32 count;
} Kai_Bytecode_Decoder;

typedef struct {
    Kai_Reg base_register;
    Kai_u32 return_address;
} Kai_BC_Procedure_Frame;

typedef struct {
    // Bytecode to Execute
    Kai_u8                  * bytecode;
    Kai_u32                   count;

    // State of Execution
    Kai_u32                   pc;
    Kai_u32                   flags;

    // Virtual Registers
    Kai_Value               * registers;
    Kai_u32                   max_register_count;
    Kai_Reg                   frame_max_register_written; // used to determine next base register

    // Call Frame Information
    Kai_BC_Procedure_Frame  * call_stack;
    Kai_u32                   call_stack_count;
    Kai_u32                   max_call_stack_count;
    Kai_Reg                 * return_registers; // where to place returned values
    Kai_u32                   return_registers_count;
    Kai_u32                   max_return_value_count;
    
    // Native Functions
    Kai_Native_Procedure    * native_procedures;
    Kai_u32                   native_procedure_count;

    // Stack Memory
    Kai_u8                  * stack;
    Kai_u32                   stack_size;
    Kai_u32                   max_stack_size;

    void                    * scratch_buffer; // at least 255 x 4 Bytes

	Kai_Allocator           * allocator;
} Kai_Interpreter;

typedef struct {
    Kai_u32         max_register_count;
    Kai_u32         max_call_stack_count;
    Kai_u32         max_return_value_count;
    Kai_u32         max_stack_size;
    Kai_Allocator * allocator;
} Kai_Interpreter_Create_Info;

typedef struct {
    Kai_u8               * data;
    Kai_u32                count;
    Kai_range              range;
    Kai_u8                 ret_count;
    Kai_u8                 arg_count;
    Kai_Native_Procedure * natives;
    Kai_u32                native_count;
    Kai_u32              * branch_hints;
    Kai_u32                branch_count;
} Kai_Bytecode;

KAI_API (void) kai_encode_load_constant    (Kai_Bytecode_Encoder* Encoder, Kai_u8 Type, Kai_Reg Reg_Dst, Kai_Value Value);
KAI_API (void) kai_encode_math             (Kai_Bytecode_Encoder* Encoder, Kai_u8 Type, Kai_u8 Operation, Kai_Reg Reg_dst, Kai_Reg Reg_Src1, Kai_Reg Reg_Src2);
KAI_API (void) kai_encode_math_constant    (Kai_Bytecode_Encoder* Encoder, Kai_u8 Type, Kai_u8 Operation, Kai_Reg Reg_dst, Kai_Reg Reg_Src1, Kai_Value Value);
KAI_API (void) kai_encode_compare          (Kai_Bytecode_Encoder* Encoder, Kai_u8 Type, Kai_u8 Comparison, Kai_Reg Reg_dst, Kai_Reg Reg_Src1, Kai_Reg Reg_Src2);
KAI_API (void) kai_encode_compare_constant (Kai_Bytecode_Encoder* Encoder, Kai_u8 Type, Kai_u8 Comparison, Kai_Reg Reg_dst, Kai_Reg Reg_Src1, Kai_Value Value);
KAI_API (void) kai_encode_branch_location  (Kai_Bytecode_Encoder* Encoder, Kai_u32 Location, Kai_Reg Reg_Src);
KAI_API (void) kai_encode_branch           (Kai_Bytecode_Encoder* Encoder, Kai_u32* Branch, Kai_Reg Reg_Src);
KAI_API (void) kai_encode_jump_location    (Kai_Bytecode_Encoder* Encoder, Kai_u32 Location);
KAI_API (void) kai_encode_jump             (Kai_Bytecode_Encoder* Encoder, Kai_u32* Branch);
KAI_API (void) kai_encode_call             (Kai_Bytecode_Encoder* Encoder, Kai_u32* Branch, Kai_u8 Ret_Count, Kai_Reg* Reg_Ret, Kai_u8 Arg_Count, Kai_Reg* Reg_Arg);
KAI_API (void) kai_encode_return           (Kai_Bytecode_Encoder* Encoder, Kai_u8 Count, Kai_Reg* Regs);
KAI_API (void) kai_encode_native_call      (Kai_Bytecode_Encoder* Encoder, Kai_u8 Use_Dst, Kai_Reg Reg_Dst, Kai_Native_Procedure* Proc, Kai_Reg* Reg_Src);
KAI_API (void) kai_encode_load             (Kai_Bytecode_Encoder* Encoder, Kai_Reg Reg_Dst, Kai_u8 Type, Kai_Reg Reg_Addr, Kai_u32 Offset);
KAI_API (void) kai_encode_store            (Kai_Bytecode_Encoder* Encoder, Kai_Reg Reg_Src, Kai_u8 Type, Kai_Reg Reg_Addr, Kai_u32 Offset);
KAI_API (void) kai_encode_check_address    (Kai_Bytecode_Encoder* Encoder, Kai_u8 Type, Kai_Reg Reg_Addr, Kai_u32 Offset);
KAI_API (void) kai_encode_stack_alloc      (Kai_Bytecode_Encoder* Encoder, Kai_Reg Reg_Dst, Kai_u32 Size);
KAI_API (void) kai_encode_stack_free       (Kai_Bytecode_Encoder* Encoder, Kai_u32 Size);

KAI_API (Kai_Result) kai_encoder_set_branch (Kai_Bytecode_Encoder* Encoder, Kai_u32 Branch, Kai_u32 Location);

#define kai_interp_run(Interpreter, Max_Step_Count) \
    for (int __iteration__ = 0; kai_interp_step(Interpreter) && ++__iteration__ < Max_Step_Count;)

/// @brief Execute one bytecode instruction
/// @return 1 if done executing or error, 0 otherwise
int kai_interp_step(Kai_Interpreter* interp);

KAI_API (void) kai_interp_create  (Kai_Interpreter_Create_Info* Info, Kai_Interpreter* out_Interp);
KAI_API (void) kai_interp_destroy (Kai_Interpreter* Interp);

KAI_API (void) kai_interp_reset       (Kai_Interpreter* Interp, Kai_u32 location);
KAI_API (void) kai_interp_set_input   (Kai_Interpreter* Interp, Kai_u32 index, Kai_Value value);
KAI_API (void) kai_interp_push_output (Kai_Interpreter* Interp, Kai_Reg reg);

// TODO: rename -> kai_interp_load_from_array(Kai_Interpreter* Interp, Kai__Byte_Array* array)
KAI_API (void) kai_interp_load_from_encoder (Kai_Interpreter* Interp, Kai_Bytecode_Encoder* Encoder);
KAI_API (void) kai_interp_load_from_memory  (Kai_Interpreter* Interp, void* Code, Kai_u32 Size);

KAI_API (Kai_Result) kai_bytecode_to_string (Kai_Bytecode* Bytecode, Kai_Writer* Writer);
KAI_API (Kai_Result) kai_bytecode_to_c      (Kai_Bytecode* Bytecode, Kai_Writer* Writer);

#endif
#ifndef KAI_DONT_USE_MEMORY_API // KAI__SECTION_MEMORY_API

enum {
    KAI_MEMORY_ERROR_OUT_OF_MEMORY  = 1, //! @see kai_memory_create implementation
    KAI_MEMORY_ERROR_MEMORY_LEAK    = 2, //! @see kai_memory_destroy implementation
};

KAI_API (Kai_Result) kai_memory_create  (Kai_Allocator* out_Allocator);
KAI_API (Kai_Result) kai_memory_destroy (Kai_Allocator* Allocator);
KAI_API (Kai_u64)    kai_memory_usage   (Kai_Allocator* Allocator);

#endif
#ifndef KAI_DONT_USE_WRITER_API // KAI__SECTION_WRITER_API

KAI_API (Kai_Writer*) kai_writer_stdout (void);

KAI_API (void) kai_writer_file_open  (Kai_Writer* out_Writer, const char* Path);
KAI_API (void) kai_writer_file_close (Kai_Writer* Writer);

#endif
#ifndef KAI__SECTION_INTERNAL_API

// type ID -> sizeof(type)
#define kai__typeid_to_size(T) ((T) >> 4)

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Linked List API -------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__linked_list_append(List, Ptr)      \
  if (List.head == NULL) List.head = Ptr;       \
  else                   List.last->next = Ptr; \
  List.last = Ptr

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Array API -------------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__array_iterate(Array, Var)                                         \
  for (Kai_u32 Var = 0; Var < (Array).count; ++Var)

#define kai__array_iterate_reverse(Array, Var)                                 \
  for (Kai_u32 Var = (Array).count - 1; Var != (Kai_u32)-1; --Var)

#define kai__array_clear(Ptr_Array)                                            \
  (Ptr_Array)->count = 0

#define kai__array_reserve(Ptr_Array, Size)                                    \
  kai__array_reserve_stride(Ptr_Array, Size, allocator,                        \
                            sizeof((Ptr_Array)->elements[0]))

#define kai__array_resize(Ptr_Array, Size)                                     \
  kai__array_reserve_stride(Ptr_Array, Size, allocator,                        \
                            sizeof((Ptr_Array)->elements[0]));                 \
  (Ptr_Array)->count = Size

#define kai__array_destroy(Ptr_Array)                                          \
  kai__array_destroy_stride(Ptr_Array, allocator,                              \
                            sizeof((Ptr_Array)->elements[0]))

#define kai__array_append(Ptr_Array, ...)                                      \
  kai__array_grow_stride(Ptr_Array, (Ptr_Array)->count + 1, allocator,         \
                         sizeof((Ptr_Array)->elements[0])),                    \
      (Ptr_Array)->elements[(Ptr_Array)->count++] = (__VA_ARGS__)

#define kai__array_grow(Ptr_Array, Amount)                                     \
  kai__array_grow_stride(Ptr_Array, (Ptr_Array)->count + Amount, allocator,    \
                         sizeof((Ptr_Array)->elements[0]))

//////////////////////////////////////////////////////////////////////////////
// Only for byte arrays + no bounds checking
#define kai__push_u8(Array, value) \
  (Array).elements[(Array).count++] = value

#define kai__push_u32(Array, value) \
  *(Kai_u32*)((Array).elements + (Array).count) = value, (Array).count += 4

#define kai__push_address(Array, value) \
  *(Kai_uint*)((Array).elements + (Array).count) = (Kai_uint)value, (Array).count += sizeof(Kai_uint)

#define kai__push_value(Array, Type, Value) \
    kai__memory_copy((Array).elements + (Array).count, &value, kai__typeid_to_size(type)), \
    (Array).count += kai__typeid_to_size(type);
//////////////////////////////////////////////////////////////////////////////

KAI_API (void) kai__array_reserve_stride(void* Array, Kai_u32 Size, Kai_Allocator* allocator, Kai_u32 Stride);
KAI_API (void) kai__array_grow_stride(void* Array, Kai_u32 Min_Size, Kai_Allocator* allocator, Kai_u32 Stride);
KAI_API (void) kai__array_shrink_to_fit_stride(void* Array, Kai_Allocator* allocator, Kai_u32 Stride);
KAI_API (void) kai__array_destroy_stride(void* Ptr_Array, Kai_Allocator* allocator, Kai_u32 Stride);

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Hash Table API --------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__bit_array_size(Bit_Count)                                         \
  (((((Bit_Count)-1) / 64) + 1) * sizeof(Kai_u64))

#define kai__bit_array_get(Array, Index)                                       \
    ((Array)[Index>>6] & ((Kai_u64)1 << (Index&63))

#define kai__bit_array_set(Array, Index)                                       \
    ((Array)[Index>>6] |= ((Kai_u64)1 << (Index&63)))

#define kai__bit_array_remove(Array, Index)                                    \
    ((Array)[Index>>6] &=~ ((Kai_u64)1 << (Index&63)))

#define kai__hash_table_destroy(Table)                                         \
  kai__hash_table_destroy_stride(&(Table), allocator, sizeof((Table).values[0]))

#define kai__hash_table_find(Ptr_Table, Key)                                   \
  kai__hash_table_find_stride(Ptr_Table, Key, sizeof((Ptr_Table)->values[0]))

#define kai__hash_table_remove_index(Table, Index)                             \
  kai__hash_table_remove_index_stride(&(Table), Index, sizeof((Table).values[0]))

#define kai__hash_table_get(Table, Key, out_Value)                             \
  kai__hash_table_get_stride(&(Table), Key, out_Value, sizeof(*out_Value),     \
                             sizeof((Table).values[0]))

#define kai__hash_table_get_str(Table, Key, out_Value)                         \
  kai__hash_table_get_stride(&(Table), KAI_STRING(Key), out_Value,             \
                             sizeof(*out_Value), sizeof((Table).values[0]))

#define kai__hash_table_iterate(Table, Iter_Var)                               \
  for (Kai_u32 Iter_Var = 0; Iter_Var < (Table).capacity; ++Iter_Var)          \
    if ((Table).occupied[Iter_Var / 64] & ((Kai_u64)1 << (Iter_Var % 64)))

#define kai__hash_table_emplace(Ptr_Table, Key, ...)                           \
  do {                                                                         \
    Kai_u32 __index__; kai__hash_table_emplace_key_stride(Ptr_Table,           \
        Key, &__index__, allocator, sizeof((Ptr_Table)->values[0]));           \
    (Ptr_Table)->values[__index__] = (__VA_ARGS__);                            \
  } while (0)

// Only insert key and get index of insertion
// Return true => new key was inserted
#define kai__hash_table_emplace_key(Ptr_Table, Key, out_Index)                 \
  kai__hash_table_emplace_key_stride(Ptr_Table, Key, out_Index, allocator,     \
                                     sizeof((Ptr_Table)->values[0]))
  
KAI_API (Kai_u64)  kai__string_hash(Kai_string In);
KAI_API (void)     kai__hash_table_grow(void* Table, Kai_Allocator* allocator, Kai_u32 Elem_Size);
KAI_API (Kai_bool) kai__hash_table_emplace_key_stride(void* Table, Kai_string Key, Kai_u32* out_Index, Kai_Allocator* Allocator, Kai_u32 Elem_Size);
KAI_API (void*)    kai__hash_table_find_stride(void* Table, Kai_string Key, Kai_u32 Elem_Size);
KAI_API (Kai_bool) kai__hash_table_get_stride(void* Table, Kai_string Key, void* out_Value, Kai_u32 Value_Size, Kai_u32 Elem_Size);
KAI_API (void)     kai__hash_table_destroy_stride(void* Table, Kai_Allocator* allocator, Kai_u32 Elem_Size);
KAI_API (Kai_bool) kai__hash_table_remove_index_stride(void* Table, Kai_u32 Index, Kai_u32 Elem_Size);

#endif
#ifndef KAI_DONT_USE_CPP_API // KAI__SECTION_CPP_API
#ifdef __cplusplus

namespace Kai {
    enum Result {
        Success = KAI_SUCCESS,
    };

    struct String : public Kai_string {
        String(const char* s): Kai_string(kai_str_from_cstring(s)) {}
        String(std::string s) {
            data = (Kai_u8*)s.data();
            count = (Kai_u32)s.size();
        }
        std::string string() {
            return std::string((char*)data, count);
        }
    };
    
#ifndef KAI_DONT_USE_MEMORY_API
#	define KAI_ALLOCATOR_SET_DEFAULT = Default_Allocator
    struct Default_Allocator : public Kai_Allocator {
        Default_Allocator() {
            kai_create_memory(this);
        }
        ~Default_Allocator() {
            kai_destroy_memory(this);
        }
    };
#else
#	define KAI_ALLOCATOR_SET_DEFAULT
#endif

    struct Error : public Kai_Error {
        void print() {
            kai_write_error(kai_writer_stdout(), this);
        }
    };

    template <typename Allocator KAI_ALLOCATOR_SET_DEFAULT>
    struct Program {
        Kai_Program handle;
        Allocator allocator;
        Error error;

        void add_native_procedure(const char* name, void* address) {

        }

        Result compile_from_file(std::string const& path) {
            std::ifstream f { path };
            f.seekg(0, std::ios::end);
            auto size = f.tellg();
            f.seekg(0, std::ios::beg);
            std::string source(size, '\0');
            f.read(source.data(), source.size());
            return (Result)kai_create_program_from_source(String(source), &allocator, &error, &handle);
        }

        template <typename T>
        T* find_procedure(const char* name) {
            return reinterpret_cast<T*>(kai_find_procedure(handle, name, nullptr));
        }
    };
}
#endif
#endif

#ifdef KAI_IMPLEMENTATION
#define KAI__DEBUG_DEPENDENCY_GRAPH    0
#define KAI__DEBUG_COMPILATION_ORDER   0
#define KAI__DISABLE_PEEPHOLES         0

#ifndef KAI__SECTION_IMPLEMENTATION_STRUCTS

typedef struct {
    Kai_u32  size;
    Kai_u32  capacity;
    Kai_u8 * data;
    Kai_u32  offset;
} Kai__Dynamic_Buffer;

#define X(TYPE, NAME, ...) TYPE NAME = __VA_ARGS__;
    KAI__X_TYPE_INFO_DEFINITIONS
#undef X

static struct {
    Kai_string name;
    Kai_Type   type;
} kai__builtin_types [] = {
    { KAI_CONSTANT_STRING("type"), (Kai_Type)&kai__type_info_type },
    { KAI_CONSTANT_STRING("s8"),   (Kai_Type)&kai__type_info_s8   },
    { KAI_CONSTANT_STRING("s16"),  (Kai_Type)&kai__type_info_s16  },
    { KAI_CONSTANT_STRING("s32"),  (Kai_Type)&kai__type_info_s32  },
    { KAI_CONSTANT_STRING("s64"),  (Kai_Type)&kai__type_info_s64  },
    { KAI_CONSTANT_STRING("u8"),   (Kai_Type)&kai__type_info_u8   },
    { KAI_CONSTANT_STRING("u16"),  (Kai_Type)&kai__type_info_u16  },
    { KAI_CONSTANT_STRING("u32"),  (Kai_Type)&kai__type_info_u32  },
    { KAI_CONSTANT_STRING("u64"),  (Kai_Type)&kai__type_info_u64  },
    { KAI_CONSTANT_STRING("f32"),  (Kai_Type)&kai__type_info_f32  },
    { KAI_CONSTANT_STRING("f64"),  (Kai_Type)&kai__type_info_f64  },
};

#endif
#ifndef KAI__SECTION_IMPLEMENTATION_INTERNAL

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Convienence -----------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__unused(...) (void)(__VA_ARGS__)

#define kai__count(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

#define kai__for_n(N) for (Kai_u32 i = 0; i < (N); ++i)

#define kai__allocate(Old, New_Size, Old_Size) \
    allocator->heap_allocate(allocator->user, Old, New_Size, Old_Size)

#define kai__free(Ptr,Size) \
    allocator->heap_allocate(allocator->user, Ptr, 0, Size)

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- String Writer ---------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__write(C_String) \
    writer->write_string(writer->user, KAI_STRING(C_String))

#define kai__write_string(...) \
    writer->write_string(writer->user, __VA_ARGS__)

#define kai__write_char(Char) \
    writer->write_string(writer->user, (Kai_string){.data = (Kai_u8[]){Char}, .count = 1})

#define kai__set_color(Color) \
    if (writer->set_color != NULL) writer->set_color(writer->user, Color)

#define kai__write_fill(Char, Count)                                        \
    writer->write_value(writer->user, KAI_FILL, (Kai_Value){0},             \
        (Kai_Write_Format){.fill_character = (Kai_u8)Char, .min_count = Count})

#define kai__write_u32(Value) \
    writer->write_value(writer->user, KAI_U32, (Kai_Value){.u32 = Value}, (Kai_Write_Format){0})

#define kai__write_s32(Value) \
    writer->write_value(writer->user, KAI_S32, (Kai_Value){.s32 = Value}, (Kai_Write_Format){0})

#define kai__write_u64(Value) \
    writer->write_value(writer->user, KAI_U64, (Kai_Value){.u64 = Value}, (Kai_Write_Format){0})

#define kai__write_s64(Value) \
    writer->write_value(writer->user, KAI_S64, (Kai_Value){.s64 = Value}, (Kai_Write_Format){0})

#define kai__write_f64(Value) \
    writer->write_value(writer->user, KAI_F64, (Kai_Value){.f64 = Value}, (Kai_Write_Format){0})

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- (Fatal) Error Handling + Debug Assertions -----------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define KAI__RED(S)    "\x1b[91m" S "\x1b[0m"
#define KAI__GREEN(S)  "\x1b[92m" S "\x1b[0m"
#define KAI__YELLOW(S) "\x1b[93m" S "\x1b[0m"
#define KAI__BLUE(S)   "\x1b[94m" S "\x1b[0m"

#ifndef kai__debug
#define kai__debug(...) printf(__VA_ARGS__)
#endif
#ifndef kai__debug_writer
#define kai__debug_writer kai_writer_stdout()
#endif

#define kai__todo(...)                                       \
do { char __message__[1024] = {0};                           \
    snprintf(__message__, sizeof(__message__), __VA_ARGS__); \
    KAI__FATAL_ERROR("TODO", __message__);                   \
} while (0)

#ifdef KAI_HAVE_FATAL_ERROR
extern void kai__fatal_error(char const* Desc, char const* Message, int Line);
#else
static void kai__fatal_error(char const* Desc, char const* Message, int Line)
{
    printf("[\x1b[92mkai.h:%i\x1b[0m] \x1b[91m%s\x1b[0m: %s\n", Line, Desc, Message);
    exit(1);
}
#endif

#define KAI__FATAL_ERROR(Desc, Message) kai__fatal_error(Desc, Message, __LINE__);
#define kai__assert(EXPR) if (!(EXPR)) KAI__FATAL_ERROR("Assertion Failed", #EXPR)
#define kai__unreachable() KAI__FATAL_ERROR("Assertion Failed", "Unreachable was reached! D:")
#define kai__check_allocation(PTR) if ((PTR) == NULL) KAI__FATAL_ERROR("Allocation Error", #PTR " was null")
        
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Basic Memory Operations -----------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

static inline void kai__memory_copy(void* Dst, void* Src, Kai_u32 Size)
{
    for (Kai_u32 i = 0; i < Size; ++i)
    {
        ((Kai_u8*)Dst)[i] = ((Kai_u8*)Src)[i];
    }
}
static inline void kai__memory_zero(void* Dst, Kai_u32 Size)
{
    for (Kai_u32 i = 0; i < Size; ++i)
    {
        ((Kai_u8*)Dst)[i] = 0;
    }
}
static inline void kai__memory_fill(void* Dst, Kai_u8 Data, Kai_u32 Size)
{
    for (Kai_u32 i = 0; i < Size; ++i)
    {
        ((Kai_u8*)Dst)[i] = Data;
    }
}
static inline Kai_u32 kai__string_copy(Kai_u8* Dst, char const* Src, Kai_u32 Dst_Size)
{
    Kai_u32 i = 0;
    for (; i < Dst_Size && Src[i] != '\0'; ++i)
    {
        ((Kai_u8*)Dst)[i] = ((Kai_u8*)Src)[i];
    }
    return i;
}

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Basic Math Functions --------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

static inline Kai_u32 kai__ceil_div(Kai_u32 Num, Kai_u32 Den)
{
    return (Num + Den - 1) / Den;
}
static inline Kai_u32 kai__ceil_div_fast(Kai_u64 Num, Kai_u32 Exp)
{
    return (Kai_u32)( (Num + ((Kai_u64)1 << Exp) - 1) >> Exp );
}
static inline Kai_u32 kai__min_u32(Kai_u32 A, Kai_u32 B)
{
    return A < B ? A : B;
}
static inline Kai_u32 kai__max_u32(Kai_u32 A, Kai_u32 B)
{
    return A < B ? B : A;
}
static inline Kai_u64 kai__gcd(Kai_u64 A, Kai_u64 B)
{
    Kai_u64 r;
    do {
        r = A % B;
        A = B;
        B = r;
    } while (r != 0);
    return A;
}
static inline double kai__ldexp(Kai_f64 x, int n)
{
    // Copyright (C) 1993,2004 Sun Microsystems
	union { double f; uint64_t i; } u;
	double y = x;

	if (n > 1023) {
		y *= 0x1p1023;
		n -= 1023;
		if (n > 1023) {
			y *= 0x1p1023;
			n -= 1023;
			if (n > 1023)
				n = 1023;
		}
	} else if (n < -1022) {
		/* make sure final n < -53 to avoid double
		   rounding in the subnormal range */
		y *= 0x1p-1022 * 0x1p53;
		n += 1022 - 53;
		if (n < -1022) {
			y *= 0x1p-1022 * 0x1p53;
			n += 1022 - 53;
			if (n < -1022)
				n = -1022;
		}
	}
	u.i = (uint64_t)(0x3ff+n)<<52;
	x = y * u.f;
	return x;
}

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Dynamic Arena Allocator -----------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

static inline void kai__arena_allocator_create(Kai__Arena_Allocator* Arena, Kai_Allocator* Allocator)
{
    kai__assert(Arena != NULL);
    kai__assert(Allocator != NULL);

    Arena->allocator = *Allocator;
    Arena->bucket_size = kai__ceil_div(KAI__MINIMUM_ARENA_BUCKET_SIZE, Allocator->page_size)
                       * Allocator->page_size;
    Arena->current_allocated = sizeof(Kai__Arena_Bucket*);
    Arena->current_bucket = Allocator->heap_allocate(Allocator->user, NULL, Arena->bucket_size, 0);
}
static inline void kai__arena_allocator_free_all(Kai__Arena_Allocator* Arena)
{
    kai__assert(Arena != NULL);
    Kai__Arena_Bucket* bucket = Arena->current_bucket;

    while (bucket)
    {
        Kai__Arena_Bucket* prev = bucket->prev;
        Arena->allocator.heap_allocate(Arena->allocator.user, bucket, 0, Arena->bucket_size);
        bucket = prev;
    }
    Arena->current_bucket = NULL;
    Arena->current_allocated = 0;
}
static inline void kai__arena_allocator_destroy(Kai__Arena_Allocator* Arena)
{
    kai__arena_allocator_free_all(Arena);
    Arena->bucket_size = 0;
    Arena->allocator = (Kai_Allocator) {0};
}
static inline void* kai__arena_allocate(Kai__Arena_Allocator* Arena, Kai_u32 Size)
{    
    kai__assert(Arena != NULL);

    if (Size > Arena->bucket_size)
        KAI__FATAL_ERROR("Arena Allocator", "Object size greater than bucket size (incorrect usage)");
    
    if (Arena->current_allocated + Size > Arena->bucket_size)
    {
        Kai__Arena_Bucket* new_bucket = Arena->allocator.heap_allocate(
            Arena->allocator.user, NULL,
            Arena->bucket_size, 0
        );

        // Bubble failure to caller
        if (new_bucket == NULL)
            return NULL;
        
        new_bucket->prev = Arena->current_bucket;
        Arena->current_bucket = new_bucket;
        Arena->current_allocated = sizeof(Kai__Arena_Bucket*);
    }
    Kai_u8* bytes = (Kai_u8*)Arena->current_bucket;
    void* ptr = bytes + Arena->current_allocated;
    Arena->current_allocated += Size;
    return ptr;
}

KAI_API (void) kai__array_reserve_stride(void* Array, Kai_u32 Size, Kai_Allocator* allocator, Kai_u32 Stride)
{
    KAI__ARRAY(void)* array = Array;
    
    if (Size <= array->capacity)
        return;

    array->elements = kai__allocate(array->elements, Stride * Size, Stride * array->capacity);
    array->capacity = Size;
}
KAI_API (void) kai__array_grow_stride(void* Array, Kai_u32 Min_Size, Kai_Allocator* allocator, Kai_u32 Stride)
{
    KAI__ARRAY(void)* array = Array;

    if (Min_Size <= array->capacity)
        return;
    
    Kai_u32 new_capacity = kai__max_u32(Min_Size + (Min_Size >> 1), 8);
    array->elements = kai__allocate(array->elements, Stride * new_capacity, Stride * array->capacity);
    array->capacity = new_capacity;
}
KAI_API (void) kai__array_shrink_to_fit_stride(void* Array, Kai_Allocator* allocator, Kai_u32 Stride)
{
    KAI__ARRAY(void)* array = Array;

    if (array->capacity != 0)
    {
        array->elements = kai__allocate(array->elements, Stride * array->count, Stride * array->capacity);
        array->capacity = array->count;
    }
}
KAI_API (void) kai__array_destroy_stride(void* Ptr_Array, Kai_Allocator* allocator, Kai_u32 Stride)
{
    KAI__ARRAY(int)* array = Ptr_Array;
    
    if (array->capacity != 0)
    {
        kai__free(array->elements, Stride * array->capacity);
    }
    array->elements = NULL;
    array->capacity = 0;
    array->count    = 0;
}

KAI_API (Kai_u64) kai__string_hash(Kai_string In)
{
    // http://www.cse.yorku.ca/~oz/hash.html (djb2)
    // " this algorithm (k=33) was first reported by dan bernstein many years ago in comp.lang.c.
    // " another version of this algorithm (now favored by bernstein) uses xor:
    // " hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
    // " (why it works better than many other constants, prime or not) has never been adequately explained. 
    Kai_u64 hash = 5381;
    for (Kai_u32 i = 0; i < In.count; ++i)
        hash = ((hash << 5) + hash) + In.data[i]; /* hash * 33 + c */
    return hash;
}
KAI_API (void) kai__hash_table_grow(void* Table, Kai_Allocator* allocator, Kai_u32 Elem_Size)
{
    kai__assert(Table != NULL);
    kai__assert(allocator != NULL);
    kai__assert(Elem_Size != 0);

    KAI__HASH_TABLE(void)* table = Table;
    
    // Calculate total space required
    Kai_u32 new_capacity  = (table->capacity == 0) ? 8 : table->capacity * 2;
    Kai_u32 occupied_size = kai__bit_array_size(new_capacity);
    Kai_u32 hashes_size   = new_capacity * sizeof(Kai_u64);
    Kai_u32 keys_size     = new_capacity * sizeof(Kai_string);
    Kai_u32 values_size   = new_capacity * Elem_Size;

    void* new_ptr = kai__allocate(NULL, occupied_size + hashes_size + keys_size + values_size, 0);

    // Distribute allocated memory
    Kai_u64*    occupied = new_ptr;
    Kai_u64*    hashes   = (Kai_u64*)((Kai_u8*)occupied + occupied_size);
    Kai_string* keys     = (Kai_string*)((Kai_u8*)hashes + hashes_size);
    Kai_u8*     values   = (Kai_u8*)((Kai_u8*)keys + keys_size);

    Kai_u32 count = 0;

    // Rehash all elements
    kai__hash_table_iterate(*table, i)
    {
        Kai_u64 hash = kai__string_hash(table->keys[i]);
        Kai_u32 mask = new_capacity - 1;
        Kai_u32 index = (Kai_u32)hash & mask;
    
        for (Kai_u32 j = 0; j < new_capacity; ++j)
        {
            Kai_u64 block = occupied[index / 64];
            Kai_u64 bit  = (Kai_u64)1 << (index % 64);
    
            // Do an insertion
            if ((block & bit) == 0)
            {
                occupied[index / 64] |= bit;
                hashes[index] = hash;
                keys[index] = table->keys[i];
                Kai_u8* src = (Kai_u8*)table->values + i * Elem_Size;
                kai__memory_copy(values + index * Elem_Size, src, Elem_Size);
                count += 1;
                break;
            }
    
            index = (index + 1) & mask;
        }
    }

    kai__assert(count == table->count);

    // Free old memory
    if (table->capacity != 0)
    {
        kai__free(table->occupied,
            kai__bit_array_size(table->capacity)
            + table->capacity * (sizeof(Kai_u64) + sizeof(Kai_string) + Elem_Size)
        );
    }

    table->capacity = new_capacity;
    table->occupied = occupied;
    table->hashes   = hashes;
    table->keys     = keys;
    table->values   = values;
}
KAI_API (Kai_bool) kai__hash_table_emplace_key_stride(void* Table, Kai_string Key, Kai_u32* out_Index, Kai_Allocator* Allocator, Kai_u32 Elem_Size)
{
    kai__assert(Table != NULL);
    kai__assert(Allocator != NULL);
    kai__assert(Elem_Size != 0);
    
    KAI__HASH_TABLE(void)* table = Table;

    // Check load factor and grow hash table
    if (4 * table->count >= 3 * table->capacity)
    {
        kai__hash_table_grow(Table, Allocator, Elem_Size);
    }

    Kai_u64 hash = kai__string_hash(Key);
    Kai_u32 mask = table->capacity - 1;
    Kai_u32 index = (Kai_u32)hash & mask;

    for (Kai_u32 i = 0; i < table->capacity; ++i)
    {
        Kai_u64 byte = table->occupied[index / 64];
        Kai_u64 bit  = (Kai_u64)1 << (index % 64);

        // Do an insertion
        if ((byte & bit) == 0)
        {
            table->occupied[index / 64] |= bit;
            table->hashes[index] = hash;
            table->keys[index] = Key;
            table->count += 1;
            *out_Index = index;
            return KAI_TRUE;
        }

        // Emplace an existing item
        if (table->hashes[index] == hash
        &&  kai_string_equals(table->keys[index], Key))
        {
            *out_Index = index;
            return KAI_FALSE;
        }

        index = (index + 1) & mask;
    }

    kai__unreachable();
    return KAI_FALSE;
}
KAI_API (void*) kai__hash_table_find_stride(void* Table, Kai_string Key, Kai_u32 Elem_Size)
{
    kai__assert(Table != NULL);
    kai__assert(Elem_Size != 0);
    
    KAI__HASH_TABLE(void)* table = Table;

    Kai_u64 hash = kai__string_hash(Key);
    Kai_u32 mask = table->capacity - 1;
    Kai_u32 index = (Kai_u32)hash & mask;

    for (Kai_u32 i = 0; i < table->capacity; ++i)
    {
        Kai_u64 block = table->occupied[index / 64];
        Kai_u64 bit   = (Kai_u64)1 << (index % 64);

        if ((block & bit) == 0)
        {
            return NULL; // Slot was empty
        }
        else if (table->hashes[index] == hash
             &&  kai_string_equals(table->keys[index], Key))
        {
            return (Kai_u8*)table->values + Elem_Size * index;
        }

        index = (index + 1) & mask;
    }

    return NULL;
}
KAI_API (Kai_bool) kai__hash_table_get_stride(void* Table, Kai_string Key, void* out_Value, Kai_u32 Value_Size, Kai_u32 Elem_Size)
{
    kai__assert(Table != NULL);
    kai__assert(Elem_Size != 0);

    void* elem_ptr = kai__hash_table_find_stride(Table, Key, Elem_Size);

    if (elem_ptr == NULL)
        return KAI_FALSE;
        
    kai__memory_copy(out_Value, elem_ptr, Value_Size);
    return KAI_TRUE;
}
KAI_API (void) kai__hash_table_destroy_stride(void* Table, Kai_Allocator* allocator, Kai_u32 Elem_Size)
{
    kai__assert(Table != NULL);
    kai__assert(allocator != NULL);
    kai__assert(Elem_Size != 0);

    typedef KAI__HASH_TABLE(void) Hash_Table;
    Hash_Table* table = Table;

    if (table->capacity != 0)
    {
        kai__free(table->occupied,
            kai__bit_array_size(table->capacity)
            + table->capacity * (sizeof(Kai_u64) + sizeof(Kai_string) + Elem_Size)
        );
    }
    *table = (Hash_Table) {0};
}
KAI_API (Kai_bool) kai__hash_table_remove_index_stride(void* Table, Kai_u32 Index, Kai_u32 Elem_Size)
{
    kai__assert(Table != NULL);
    kai__assert(Elem_Size != 0);

    KAI__HASH_TABLE(void)* table = Table;
    kai__assert(Index < table->capacity);

    if (table->occupied[Index/64] & ((Kai_u64)1 << (Index%64)))
    {
        table->occupied[Index/64] &= ~((Kai_u64)1 << (Index%64));
        table->count -= 1;

        Kai_u32 mask = table->capacity - 1;
        Kai_u32 empty = Index;
        Kai_u32 current = (Index + 1) & mask;
        
        for (Kai_u32 i = 0; i < table->capacity; ++i)
        {
            Kai_u64 block = table->occupied[current / 64];
            Kai_u64 bit = (Kai_u64)1 << (current % 64);

            if ((block & bit) == 0)
                break;

            {
                Kai_u32 current_desired_index = table->hashes[current] & mask; // Index where current slot wants to be
                Kai_u32 empty_comparable = current_desired_index > empty ? empty + table->capacity : empty; // Handle wrap-around behavior
                if (empty_comparable < current_desired_index)
                    break;
            }

            // Move current -> empty
            kai__bit_array_set(table->occupied, empty);
            kai__bit_array_remove(table->occupied, current);
            table->hashes[empty] = table->hashes[current];
            table->keys[empty] = table->keys[current];
            kai__memory_copy(
                (Kai_u8*)table->values + empty * Elem_Size,
                (Kai_u8*)table->values + current * Elem_Size,
                Elem_Size
            );

            current = (current + 1) & mask;
        }

        return KAI_TRUE;
    }
    return KAI_FALSE;
}

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Dynamic Buffer API ----------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__dynamic_buffer_append_string(Builder, String) \
    kai__dynamic_buffer_append_a(Builder, String.data, String.count, allocator)

#define kai__dynamic_buffer_append_string_max(Builder, String, Max_Count) \
    kai__dynamic_buffer_append_max_a(Builder, String, Max_Count, allocator)

#define kai__dynamic_buffer_push(Buffer, Size) \
    kai__dynamic_buffer_push_a(Buffer, Size, allocator)
    
#define kai__range_to_data(Range, Memory) \
    (void*)(((Kai_u8*)(Memory).data) + (Range).offset)
    
#define kai__range_to_string(Range, Memory) \
    (KAI_STRUCT(Kai_string) { .count = (Range).count, .data = kai__range_to_data(Range, Memory) })

static inline void kai__dynamic_buffer_append_a(Kai__Dynamic_Buffer* Buffer, void* Data, Kai_u32 Size, Kai_Allocator* Allocator)
{
    kai__array_grow_stride(Buffer, Buffer->size + Size, Allocator, 1);
    kai__memory_copy(Buffer->data + Buffer->size, Data, Size);
    Buffer->size += Size;
}
static inline void kai__dynamic_buffer_append_max_a(Kai__Dynamic_Buffer* Buffer, Kai_string String, Kai_u32 Max_Count, Kai_Allocator* Allocator)
{
    kai__assert(Max_Count > 3);
    Kai_u32 size = (String.count > Max_Count) ? Max_Count : String.count;
    kai__array_grow_stride(Buffer, Buffer->size + size, Allocator, 1);
    if (String.count < Max_Count)
    {
        kai__memory_copy(Buffer->data + Buffer->size, String.data, String.count);
        Buffer->size += String.count;
    }
    else
    {
        kai__memory_copy(Buffer->data + Buffer->size, String.data, Max_Count - 3);
        kai__memory_copy(Buffer->data + Buffer->size + Max_Count - 3, "...", 3);
        Buffer->size += Max_Count;
    }
}
static inline Kai_range kai__dynamic_buffer_next(Kai__Dynamic_Buffer* Buffer)
{
    Kai_range out = {
        .count = Buffer->size - Buffer->offset,
        .offset = Buffer->offset
    };
    Buffer->offset = Buffer->size;
    return out;
}
static inline Kai_range kai__dynamic_buffer_push_a(Kai__Dynamic_Buffer* Buffer, Kai_u32 Size, Kai_Allocator* Allocator)
{
    kai__array_grow_stride(Buffer, Buffer->size + Size, Allocator, 1);
    Kai_range out = {
        .count = Size,
        .offset = Buffer->offset
    };
    Buffer->size += Size;
    Buffer->offset = Buffer->size;
    return out;
}
static inline Kai_Memory kai__dynamic_buffer_release(Kai__Dynamic_Buffer* Buffer)
{
    Kai_Memory memory = {
        .data = Buffer->data,
        .size = Buffer->capacity,
    };
    *Buffer = (Kai__Dynamic_Buffer) {0};
    return memory;
}

#endif
#ifndef KAI__SECTION_IMPLEMENTATION_SIMPLE

KAI_API (Kai_vector3_u32) kai_version(void)
{
    return (Kai_vector3_u32) {
        .x = KAI_VERSION_MAJOR,
        .y = KAI_VERSION_MINOR,
        .z = KAI_VERSION_PATCH,
    };
}
KAI_API (Kai_string) kai_version_string(void)
{
    return KAI_STRING("Kai Compiler v" KAI_VERSION_STRING);
}

KAI_API (Kai_bool) kai_string_equals(Kai_string A, Kai_string B)
{
    if (A.count != B.count)
        return KAI_FALSE;
    for (Kai_u32 i = 0; i < A.count; ++i)
    {
        if (A.data[i] != B.data[i])
            return KAI_FALSE;
    }
    return KAI_TRUE;
}
KAI_API (Kai_string) kai_string_from_c(char const* String)
{
    Kai_u32 len = 0;
    while (String[len] != '\0') ++len;
    return (Kai_string) { .data = (Kai_u8*)String, .count = len };
}

KAI_API (void) kai_destroy_error(Kai_Error* Error, Kai_Allocator* allocator)
{
    while (Error)
    {
        Kai_Error* next = Error->next;
        if (Error->memory.size)
            kai__free(Error->memory.data, Error->memory.size);
        Error = next;
    }
}

KAI_API(void) kai_write_number(Kai_Writer* writer, Kai_Number Number)
{
    // If number is a fraction, write it out as a float
    if (!kai_number_is_integer(Number))
    {
        kai__write_f64(kai_number_to_f64(Number));
        return;
    }

    if (Number.is_neg)
        kai__write_char('-');

    // If number fits into u64, write it out as u64
    if ((Kai_u32)(Number.e) <= kai_intrinsic_clz(Number.n))
    {
        kai__write_u64(Number.n << Number.e);
        return;
    }

    // Might change this to just write as float, this representation
    // is accurate, but don't know how useful it actually is
    kai__write_u64(Number.n);
    kai__write(" * 2^");
    kai__write_s32(Number.e);
}

static inline Kai_u64 kai__mul_with_shift(Kai_u64 A, Kai_u64 B, Kai_s32* Exp, Kai_bool Sub)
{
    Kai_u128 full = kai__u128_multiply(A, B);
    Kai_u64 hi = kai__u128_high(full);
    Kai_u64 lo = kai__u128_low(full);
    
    if (hi == 0) {
        // Fits in 64 bits, no shift needed
        return lo;
    }
    
    // Amount of right shift required to fit in 64 bits
    Kai_s32 shift = 64 - (Kai_s32)(kai_intrinsic_clz(hi));
    
    if (Sub) *Exp -= shift;
    else     *Exp += shift;
    return (hi << (64 - shift)) | (lo >> shift);
}
static inline Kai_u64 kai__add_with_shift(Kai_u64 A, Kai_u64 B, Kai_s32* Exp, Kai_bool Sub)
{
    Kai_u64 sum = A + B;
    Kai_u32 carry = (sum < A);

    if (!carry)
        return sum;

    if (Sub) *Exp -= 1;
    else     *Exp += 1;
    return ((Kai_u64)1 << 63) | (sum >> 1);
}
KAI_API (Kai_Number) kai_number_add_same_exp(Kai_Number A, Kai_Number B)
{
    Kai_s32 t0e = 0, t1e = 0;
    Kai_u64 t0 = kai__mul_with_shift(A.n, B.d, &t0e, 0);
    Kai_u64 t1 = kai__mul_with_shift(B.n, A.d, &t1e, 0);
    Kai_s32 ex = A.e;

    // Exponents have changed, need to make them the same again
    if (t0e != t1e) {
        if (t0e < t1e) {
            t0 >>= (t1e - t0e);
            ex += t1e;
        }
        else {
            t1 >>= (t0e - t1e);
            ex += t0e;
        }
    }

    Kai_u64 de = kai__mul_with_shift(A.d, B.d, &ex, 1);
    Kai_u64 nu;

    // Handle resulting sign, TODO: simplify
    Kai_u32 is_neg = A.is_neg & B.is_neg;
    if (A.is_neg && !B.is_neg) {
        if (t0 > t1) nu = t0 - t1, is_neg = 1; // -5 + 1
        else         nu = t1 - t0, is_neg = 0; // -1 + 5
    }
    else if (B.is_neg && !A.is_neg) {
        if (t0 > t1) nu = t0 - t1, is_neg = 0; // 5 + -1
        else         nu = t1 - t0, is_neg = 1; // 1 + -5
    }
    else {
        nu = kai__add_with_shift(t0, t1, &ex, 0);
    }

    return kai_number_normalize((Kai_Number) {
        .n = nu, .d = de, .e = ex,
        .is_neg = is_neg,
    });
}
KAI_API (void) kai_number_match_exponents(Kai_Number* A, Kai_Number* B)
{
    if (A->e < B->e) {
        // reduce b.e or increase a.e
        while (A->e != B->e && (A->d >> 63) == 0) {
            A->d <<= 1;
            A->e += 1;
        }
        while (A->e != B->e && (B->n >> 63) == 0) {
            B->n <<= 1;
            B->e -= 1;
        }
        // must lose some precision now :(
        Kai_s64 diff = B->e - A->e;
        A->n >>= (Kai_u32)(diff);
        A->e = B->e;
    }
    else /* a.e > b.e */ {
        // reduce a.e or increase b.e
        while (A->e != B->e && (B->d >> 63) == 0) {
            B->d <<= 1;
            B->e += 1;
        }
        while (A->e != B->e && (A->n >> 63) == 0) {
            A->n <<= 1;
            A->e -= 1;
        }
        // must lose some precision now :(
        Kai_s64 diff = A->e - B->e;
        B->n >>= (Kai_u32)(diff);
        B->e = A->e;
    }
}
KAI_API (Kai_s32) kai_number_compare(Kai_Number A, Kai_Number B)
{
    if (A.n == 0 || B.n == 0)
    {
        if (A.n == 0 && B.n == 0) return 0;
        if (A.n == 0)             return B.is_neg ?  1 : -1;
        if (B.n == 0)             return A.is_neg ? -1 :  1;
    }
    if (A.is_neg != B.is_neg)
    {
        return A.is_neg ? -1 : 1;
    }
    Kai_s64 de = A.e - B.e;
    Kai_u128 l = kai__u128_multiply(A.n, B.d);
    Kai_u128 r = kai__u128_multiply(B.n, A.d);
    Kai_u64 lh = kai__u128_high(l);
    Kai_u64 rh = kai__u128_high(r);
    Kai_u64 ll = kai__u128_low(l);
    Kai_u64 rl = kai__u128_low(r);

    if (de < 0)
    {
        // Shift left side
        de = -de;
        ll >>= de;
        if (de <= 64) {
            ll = (lh << (64 - de)) | ll;
            lh >>= de;
        }
        else {
            ll = lh >> (de - 64);
            lh = 0;
        }
    }
    else
    {
        // Shift right side
        rl >>= de;
        if (de <= 64) {
            rl = (rh << (64 - de)) | rl;
            rh >>= de;
        }
        else {
            rl = rh >> (de - 64);
            rh = 0;
        }
    }

    Kai_s32 result = 0;
    if      (lh != rh) result = lh < rh ? -1 : 1;
    else if (ll != rl) result = ll < rl ? -1 : 1;
    else               result = 0;
    return A.is_neg ? -result : result;
}
KAI_API (Kai_Number) kai_number_normalize(Kai_Number A)
{
    Kai_s32 ns = kai_intrinsic_ctz(A.n);
    Kai_s32 ds = kai_intrinsic_ctz(A.d);
    Kai_s32 ex = A.e + (ns - ds);
    Kai_u64 nu = A.n >> ns;
    Kai_u64 de = A.d >> ds;
    Kai_u64 d = kai__gcd(nu, de);
    return (Kai_Number) {
        .n = nu / d,
        .d = de / d,
        .is_neg = A.is_neg,
        .e = ex,
    };
}

KAI_API (Kai_Number) kai_number_neg(Kai_Number A)
{
    return (Kai_Number) {.n = A.n, .d = A.d, .e = A.e, .is_neg = A.is_neg ^ 1};
}
KAI_API (Kai_Number) kai_number_abs(Kai_Number A)
{
    return (Kai_Number) {.n = A.n, .d = A.d, .e = A.e, .is_neg = 0};
}
KAI_API (Kai_Number) kai_number_inv(Kai_Number A)
{
    return (Kai_Number) {.n = A.d, .d = A.n, .e = -A.e, .is_neg = A.is_neg};
}
KAI_API (Kai_Number) kai_number_add(Kai_Number A, Kai_Number B)
{
    if (A.n == 0) return B;
    if (B.n == 0) return A;

    if (A.e == B.e)
        return kai_number_add_same_exp(A, B);

    kai_number_match_exponents(&A, &B);
    return kai_number_add_same_exp(A, B);
}
KAI_API (Kai_Number) kai_number_sub(Kai_Number A, Kai_Number B)
{
    return kai_number_add(A, kai_number_neg(B));
}
KAI_API (Kai_Number) kai_number_mul(Kai_Number A, Kai_Number B)
{
    if (A.n == 0 || B.n == 0)
        return (Kai_Number){0};

    Kai_s32 ex = A.e + B.e;
    Kai_u64 d0 = kai__gcd(A.n, B.d);
    Kai_u64 d1 = kai__gcd(B.n, A.d);
    Kai_u64 nu = kai__mul_with_shift(A.n / d0, B.n / d1, &ex, 0);
    Kai_u64 de = kai__mul_with_shift(A.d / d1, B.d / d0, &ex, 1);

    return kai_number_normalize((Kai_Number) {
        .is_neg = A.is_neg ^ B.is_neg,
        .n = nu, .d = de, .e = ex,
    });
}
KAI_API (Kai_Number) kai_number_div(Kai_Number A, Kai_Number B)
{
    return kai_number_mul(A, kai_number_inv(B));
}
KAI_API (Kai_Number) kai_number_pow_int(Kai_Number A, Kai_u32 Exp)
{
    Kai_Number r = A;
    Kai_s32 i = 30 - (Kai_s32)(kai_intrinsic_clz(Exp));
    if (i >= 0)
    {
        for (Kai_u32 bit = (Kai_u32)1 << i; bit != 0; bit >>= 1)
        {
            r = kai_number_mul(r, r);
            if (Exp & bit)
                r = kai_number_mul(r, A);
        }
    }
    return r;
}

KAI_API (Kai_f64) kai_number_to_f64(Kai_Number Number)
{
    if (Number.n == 0)
        return 0.0;
    
    Kai_f64 val = (Kai_f64)(Number.n) / (Kai_f64)(Number.d);

    if (Number.is_neg)
        val = -val;
    return kai__ldexp(val, Number.e);
}

KAI_API (Kai_Number) kai_number_parse_whole(Kai_string Text, Kai_u32* Offset, Kai_u32 Base)
{
    Kai_Number result = {0};
    // TODO: Better to call normalize, or leave base as is?
    Kai_Number base = kai_number_normalize((Kai_Number){Base, 1});
    Kai_u32 offset = *Offset;

    for (;offset < Text.count; offset += 1)
    {
        Kai_u64 ch = Text.data[offset];
        Kai_u64 d;

        if (ch >= '0' && ch <= '9')
            d = ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            d = (ch - 'A') + 0xA;
        else if (ch >= 'a' && ch <= 'f')
            d = (ch - 'a') + 0xA;
        else if (ch == '_')
            continue;
        else break;

        if (d >= Base)
            break;

        result = kai_number_mul(result, base);
        result = kai_number_add(result, (Kai_Number){d, 1});
    }

    *Offset = offset;
    return result;
}
KAI_API (Kai_Number) kai_number_parse_decimal(Kai_string Text, Kai_u32* Offset)
{
    Kai_Number result = {0};
    Kai_Number power = {1, 5, 0, -1}; // 1 / 10
    Kai_u32 offset = *Offset;
    
    for (;offset < Text.count; offset += 1)
    {
        Kai_u32 d = (Kai_u32)(Text.data[offset] - '0');
        if (d > 9) break;
        
        Kai_Number h = kai_number_mul(power, (Kai_Number){d, 1});
        result = kai_number_add(result, h);
        power = kai_number_mul(power, (Kai_Number){1, 5, 0, -1}); // 1 / 10
    }
    
    *Offset = offset;
    return result;
}
Kai_u64 const kai__powers_of_five[10] = {
    1, 5, 25, 125, 625, 3125, 15625, 78125, 390625, 1953125,
};
KAI_API (Kai_Number) kai_number_parse_exponent(Kai_string Text, Kai_u32* Offset)
{
    Kai_Number result = {1, 1};
    Kai_u32 offset = *Offset;
    
    for (;offset < Text.count; offset += 1)
    {
        Kai_u32 d = (Kai_u32)(Text.data[offset] - '0');
        if (d > 9) break;
        
        result = kai_number_pow_int(result, 10);
        result = kai_number_mul(result, (Kai_Number){kai__powers_of_five[d], 1, 0, d}); // 10^d
    }
    
    *Offset = offset;
    return result;
}

#endif
#ifndef KAI__SECTION_IMPLEMENTATION_WRITER

static inline Kai_Result kai__error_internal(Kai_Error* out_Error, Kai_string Message)
{
    *out_Error = (Kai_Error) {
        .message = Message,
        .result = KAI_ERROR_INTERNAL,
    };
    return KAI_ERROR_INTERNAL;
}

static Kai_string const kai__result_string_map[KAI__RESULT_COUNT] = {
    [KAI_SUCCESS]         = KAI_CONSTANT_STRING("Success"),
    [KAI_ERROR_SYNTAX]    = KAI_CONSTANT_STRING("Syntax Error"),
    [KAI_ERROR_SEMANTIC]  = KAI_CONSTANT_STRING("Semantic Error"),
    [KAI_ERROR_INFO]      = KAI_CONSTANT_STRING("Info"),
    [KAI_ERROR_FATAL]     = KAI_CONSTANT_STRING("Fatal Error"),
    [KAI_ERROR_INTERNAL]  = KAI_CONSTANT_STRING("Internal Error"),
};

static Kai_u32 kai__base10_digit_count(Kai_u32 x)
{
    if (x <         10) return 1;
    if (x <        100) return 2;
    if (x <       1000) return 3;
    if (x <      10000) return 4;
    if (x <     100000) return 5;
    if (x <    1000000) return 6;
    if (x <   10000000) return 7;
    if (x <  100000000) return 8;
    if (x < 1000000000) return 9;
    return 0;
}

static Kai_u8* kai__advance_to_line(Kai_u8 const* source, Kai_int line)
{
    --line;
    while (line > 0)
    {
        if (*source++ == '\n') --line;
    }
    return (Kai_u8*)source;
}


// TODO: Audit code
static void kai__write_source_code(Kai_Writer* writer, Kai_u8* src)
{
    while (*src != 0 && *src != '\n')
    {
        if (*src == '\t')
            kai__write_char(' ');
        else
            kai__write_char(*src);
        ++src;
    }
}

static uint32_t kai__utf8_decode(Kai_u8 **s)
{
    const unsigned char *p = (const unsigned char *)*s;
    uint32_t cp = 0;
    size_t len = 0;

    // No idea if this is right
    if (p[0] < 0x80)
    {
        cp = p[0];
        len = 1;
    }
    else if ((p[0] & 0xE0) == 0xC0)
    {
        cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
        len = 2;
    }
    else if ((p[0] & 0xF0) == 0xE0)
    {
        cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        len = 3;
    }
    else if ((p[0] & 0xF8) == 0xF0)
    {
        cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
             ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        len = 4;
    }
    else
    {
        cp = 0xFFFD; // Invalid byte
        len = 1;     // skip
    }

    *s += len;
    return cp;
}

static int kai__unicode_char_width(Kai_Writer* writer, uint32_t cp, Kai_u8 first, Kai_u8 ch) {
    int count = 0;
    if ((cp == 0)                     // NULL
    ||  (cp < 0x20 && cp != '\t')     // Control not including TAB
    ||  (cp >= 0x7f && cp < 0xa0)     // Control
    ||  (cp >= 0x300 && cp <= 0x36F)) // Combining marks
        return 0;

    // Codepoint that would be the width of 2 ascii characters
    if ((cp >= 0x1100  && cp <= 0x115F)
    ||  (cp >= 0x2329  && cp <= 0x232A)
    ||  (cp >= 0x2E80  && cp <= 0xA4CF)
    ||  (cp >= 0xAC00  && cp <= 0xD7A3)
    ||  (cp >= 0xF900  && cp <= 0xFAFF)
    ||  (cp >= 0xFE10  && cp <= 0xFE19)
    ||  (cp >= 0xFE30  && cp <= 0xFE6F)
    ||  (cp >= 0xFF00  && cp <= 0xFF60)
    ||  (cp >= 0xFFE0  && cp <= 0xFFE6)
    ||  (cp >= 0x1F300 && cp <= 0x1FAFF))
    {
        kai__write_fill(first, 1);
        first = ch;
        count += 1;
    }

    kai__write_fill(first, 1);
    first = ch;
    return count + 1;
}

// TODO: Audit code
static void kai__write_source_code_fill(Kai_Writer* writer, Kai_u8* src, Kai_u8* end, Kai_u8 first, Kai_u8 ch)
{
    while (src < end && *src != 0 && *src != '\n')
    {
        uint32_t cp = kai__utf8_decode(&src);
        kai__unicode_char_width(writer, cp, first, ch);
        first = ch;
    }
}

KAI_API (void) kai_write_error(Kai_Writer* writer, Kai_Error* error)
{
    for (; error != NULL; error = error->next)
    {
        if (error->result == KAI_SUCCESS) {
            kai__write("[Success]\n");
            return;
        }
        if (error->result >= KAI__RESULT_COUNT) {
            kai__write("[Invalid result value]\n");
            return;
        }

        // ------------------------- Write Error Message --------------------------

        kai__set_color(KAI_COLOR_IMPORTANT_2);
        kai__write_string(error->location.file_name);
        kai__set_color(KAI_COLOR_PRIMARY);
    #if KAI_SHOW_LINE_NUMBER_WITH_FILE
        kai__write_char(':');
        kai__set_color(KAI_COLOR_IMPORTANT_2);
        kai__write_u32(error->location.line);
        kai__set_color(KAI_COLOR_PRIMARY);
    #endif
        kai__write(" --> ");
        if (error->result != KAI_ERROR_INFO) kai__set_color(KAI_COLOR_IMPORTANT);
        kai__write_string(kai__result_string_map[error->result]);
        if (error->result != KAI_ERROR_INFO) kai__set_color(KAI_COLOR_PRIMARY);
        kai__write(": ");
        kai__set_color(KAI_COLOR_SECONDARY);
        kai__write_string(error->message);
        kai__write_char('\n');

        // -------------------------- Write Source Code ---------------------------

        if (error->result == KAI_ERROR_FATAL || error->result == KAI_ERROR_INTERNAL)
            continue;

        kai__set_color(KAI_COLOR_DECORATION);
        Kai_u32 digits = kai__base10_digit_count(error->location.line);
        kai__write_fill(' ', digits);
        kai__write("  |\n");

        kai__write_char(' ');
        kai__write_u32(error->location.line);
        kai__write(" | ");

        // TODO: Should start from location.string.data and go back
        Kai_u8* begin = kai__advance_to_line(error->location.source, error->location.line);

        kai__set_color(KAI_COLOR_PRIMARY);
        kai__write_source_code(writer, begin);
        kai__write_char('\n');

        kai__set_color(KAI_COLOR_DECORATION);
        kai__write_fill(' ', digits);
        kai__write("  | ");

        kai__write_source_code_fill(writer, begin, error->location.string.data, ' ', ' ');

        kai__set_color(KAI_COLOR_IMPORTANT);
        //kai__write_char('^');
        kai__write_source_code_fill(writer,
            error->location.string.data,
            error->location.string.data + error->location.string.count,
            '^', '~');

        kai__write_char(' ');
        kai__write_string(error->context);

        kai__write_char('\n');
        kai__set_color(KAI_COLOR_PRIMARY);
    }
}

KAI_API (void) kai_write_type(Kai_Writer* writer, Kai_Type Type)
{
    void* void_Type = Type;
    if (Type == NULL) {
        kai__write("[null]");
        return;
    }
    switch (Type->type)
    {
    default:            { kai__write("[Unknown]"); } break;
    case KAI_TYPE_TYPE: { kai__write("type");      } break;

    case KAI_TYPE_INTEGER: {
        Kai_Type_Info_Integer* info = void_Type;
        kai__write_char(info->is_signed? 's':'u');
        kai__write_u32(info->bits);
    } break;

    case KAI_TYPE_FLOAT: {
        Kai_Type_Info_Float* info = void_Type;
        kai__write_char('f');
        kai__write_u32(info->bits);
    } break;

    case KAI_TYPE_POINTER: {
        Kai_Type_Info_Pointer* info = void_Type;
        kai__write_char('*');
        kai_write_type(writer, info->sub_type);
    } break;

    case KAI_TYPE_PROCEDURE: {
        Kai_Type_Info_Procedure* info = void_Type;
        kai__write("(");
        kai__for_n (info->in_count) {
            kai_write_type(writer, info->sub_types[i]);
            if ((Kai_u8)i < info->in_count - 1)
                kai__write(", ");
        }
        kai__write(") -> (");
        kai__for_n (info->out_count) {
            kai_write_type(writer, info->sub_types[info->in_count+i]);
            if ((Kai_u8)i < info->out_count - 1)
                kai__write(", ");
        }
        kai__write(")");
    } break;
    
    case KAI_TYPE_STRUCT: {
        Kai_Type_Info_Struct* info = void_Type;
		kai__write("struct {");
        for (Kai_u32 i = 0;;) {
			kai__write_string(info->field_names[i]);
			kai__write(":");
            kai_write_type(writer, info->field_types[i]);
            if (++i < info->field_count)
            {
                kai__write("; ");
            }
            else break;
        }
		kai__write("}");
    } break;
    }
}

static Kai_string kai__tree_branches[4] = {
    KAI_CONSTANT_STRING(KAI_UTF8("\u2503   ")),
    KAI_CONSTANT_STRING(KAI_UTF8("\u2523\u2501\u2501 ")),
    KAI_CONSTANT_STRING(KAI_UTF8("    ")),
    KAI_CONSTANT_STRING(KAI_UTF8("\u2517\u2501\u2501 ")),
};

static Kai_string kai__binary_operator_name(Kai_u32 op)
{
    switch (op)
    {
    case '->': return KAI_STRING("cast");
    case '&&': return KAI_STRING("and");
    case '||': return KAI_STRING("or");
    case '==': return KAI_STRING("equals");
    case '<=': return KAI_STRING("less or equal");
    case '>=': return KAI_STRING("greater or equal");
    case '+':  return KAI_STRING("add");
    case '-':  return KAI_STRING("subtract");
    case '*':  return KAI_STRING("multiply");
    case '/':  return KAI_STRING("divide");
    case '.':  return KAI_STRING("member access");
    case '[':  return KAI_STRING("index");
    default:   return KAI_EMPTY_STRING;
    }
}

static Kai_string kai__unary_operator_name(Kai_u32 op)
{
    switch (op)
    {
    case '-':  return KAI_STRING("negate");
    case '*':  return KAI_STRING("pointer to");
    case '[':  return KAI_STRING("array");
    default:   return KAI_EMPTY_STRING;
    }
}

typedef struct {
    Kai_Writer * writer;
    Kai_u64             stack[256]; // 64 * 256 max depth
    Kai_u32             stack_count;
    Kai_string          prefix;
} Kai__Tree_Traversal_Context;

#define kai__push_traversal_stack(B)                                                        \
    { if (B) Context->stack[Context->stack_count/64] |= 1llu << (Context->stack_count%64);  \
      else   Context->stack[Context->stack_count/64] &=~ 1llu << (Context->stack_count%64); \
      Context->stack_count += 1;                                                            \
    } (void)0

#define kai__pop_traversal_stack() \
    Context->stack_count -= 1

#define kai__explore(Expr, Is_Last)                                 \
    { kai__assert(Context->stack_count < sizeof(Context->stack)*8); \
      kai__push_traversal_stack(Is_Last);                           \
      kai__traverse_tree(Context, Expr);                            \
      kai__assert(Context->stack_count != 0);                       \
      kai__pop_traversal_stack(); } (void)0

static void kai__traverse_tree(Kai__Tree_Traversal_Context* Context, Kai_Expr Expr)
{
    Kai_Writer* writer = Context->writer;

    kai__set_color(KAI_COLOR_DECORATION);
    Kai_int last = Context->stack_count - 1;
    kai__for_n (Context->stack_count)
    {
        Kai_u32 k = ((Kai_u32)((Context->stack[i/64] >> i%64) & 1) << 1) | (i == last);
        kai__write_string(kai__tree_branches[k]);
    }

    if (Context->prefix.count > 0) {
        kai__set_color(KAI_COLOR_IMPORTANT_2);
        kai__write_string(Context->prefix);
        kai__write_char(' ');
        Context->prefix = KAI_EMPTY_STRING;
    }
    if (Expr == NULL) {
        kai__set_color(KAI_COLOR_IMPORTANT_2);
        kai__write("null\n");
        kai__set_color(KAI_COLOR_PRIMARY);
        return;
    }

#define kai__write_name() \
    if (Expr->name.count != 0) \
    { \
        kai__set_color(KAI_COLOR_PRIMARY); \
        kai__write(" (name = \""); \
        kai__set_color(KAI_COLOR_IMPORTANT); \
        kai__write_string(Expr->name); \
        kai__set_color(KAI_COLOR_PRIMARY); \
        kai__write("\")"); \
    }

    void* void_Expr = Expr;
    switch (Expr->id) {
        default: {
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("unknown");
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write(" (id = ");
            kai__write_u32(Expr->id);
            kai__write_char(')');
            kai__write_char('\n');
        }
        break; case KAI_EXPR_IDENTIFIER: {
            Kai_Expr_Identifier* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("identifier");
            kai__write_name();
            kai__write(" \"");
            kai__set_color(KAI_COLOR_IMPORTANT);
            kai__write_string(node->source_code);
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write("\"\n");
        }
        break; case KAI_EXPR_STRING: {
            Kai_Expr_String* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("string");
            kai__write_name();
            kai__write(" \"");
            kai__set_color(KAI_COLOR_IMPORTANT);
            kai__write_string(node->source_code);
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write("\"\n");
        }
        break; case KAI_EXPR_NUMBER: {
            Kai_Expr_Number* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("number");
            kai__write_name();
            kai__write(" \"");
            kai__set_color(KAI_COLOR_IMPORTANT);
            kai__write_string(node->source_code);
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write("\"");
            //kai__write_format(" whole: %llu, frac: %llu, den: %hu",
            //	node->value.Whole_Part, node->value.Frac_Part, node->value.Frac_Denom);
            kai__write("\n");
        }
        break; case KAI_EXPR_UNARY: {
            Kai_Expr_Unary* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("unary");
            kai__write_name();
            kai__write(" (op = ");
            kai__set_color(KAI_COLOR_IMPORTANT);
            kai__write_string(kai__unary_operator_name(node->op));
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write(")\n");

            kai__explore(node->expr, 1);
        }
        break; case KAI_EXPR_BINARY: {
            Kai_Expr_Binary* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("binary");
            kai__write_name();
            kai__write(" (op = ");
            kai__set_color(KAI_COLOR_IMPORTANT);
            kai__write_string(kai__binary_operator_name(node->op));
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write(")\n");

            kai__explore(node->left,  0);
            kai__explore(node->right, 1);
        }
        break; case KAI_EXPR_PROCEDURE_TYPE: {
            Kai_Expr_Procedure_Type* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("procedure type");
            kai__write_name();
            kai__write(" (");
            kai__write_u32(node->in_count);
            kai__write(" in, ");
            kai__write_u32(node->out_count);
            kai__write(" out)\n");

            Kai_u32 end = node->in_count + node->out_count - 1;
            Kai_Expr current = node->in_out_expr;

            kai__for_n (node->in_count) {
                Context->prefix = KAI_STRING("in ");
                kai__explore(current, i == end);
                current = current->next;
            }

            kai__for_n (node->out_count) {
                Context->prefix = KAI_STRING("out");
                Kai_int idx = i + node->in_count;
                kai__explore(current, idx == end);
                current = current->next;
            }
        }
        break; case KAI_EXPR_PROCEDURE_CALL: {
            Kai_Expr_Procedure_Call* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("procedure call");
            kai__write_name();
            kai__write_char('\n');

            Context->prefix = KAI_STRING("proc");
            kai__explore(node->proc, node->arg_count == 0);

            Kai_int n = node->arg_count;
            Kai_Expr current = node->arg_head;
            kai__for_n (n) {
                kai__explore(current, i == n - 1);
                current = current->next;
            }
        }
        break; case KAI_EXPR_PROCEDURE: {
            Kai_Expr_Procedure* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("procedure");
            kai__write_name();
            kai__write(" (");
            kai__write_u32(node->in_count);
            kai__write(" in, ");
            kai__write_u32(node->out_count);
            kai__write(" out)\n");

            Kai_Expr current = node->in_out_expr;

            kai__for_n (node->in_count) {
                Context->prefix = KAI_STRING("in ");
                kai__explore(current, 0);
                current = current->next;
            }

            kai__for_n (node->out_count) {
                Context->prefix = KAI_STRING("out");
                kai__explore(current, 0);
                current = current->next;
            }

            kai__explore(node->body, 1);
        }
        break; case KAI_EXPR_STRUCT: {
            Kai_Expr_Struct* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("struct\n");

            Kai_Stmt current = node->head;
            while (current) {
                kai__explore(current, current->next == NULL);
                current = current->next;
            }
        }
        break; case KAI_EXPR_ARRAY: {
            Kai_Expr_Array* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("array type");
            kai__write_name();
            kai__write(" [");
            kai__write_u32(node->rows);
            kai__write("x");
            kai__write_u32(node->cols);
            kai__write("]\n");

            kai__explore(node->expr, 1);
        }
        break; case KAI_STMT_RETURN: {
            Kai_Stmt_Return* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("return\n");
            kai__explore(node->expr, 1);
        }
        break; case KAI_STMT_DECLARATION: {
            Kai_Stmt_Declaration* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("declaration");
            kai__write_name();
            if (node->flags & KAI_DECL_FLAG_CONST) kai__write(" CONST");
            kai__write_char('\n');

            Kai_bool has_expr = (node->expr != NULL);

            if (node->type) {
                Context->prefix = KAI_STRING("type");
                kai__explore(node->type, !has_expr);
            }

            if (has_expr)
                kai__explore(node->expr, 1);
        }
        break; case KAI_STMT_ASSIGNMENT: {
            Kai_Stmt_Assignment* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("assignment\n");

            Context->prefix = KAI_STRING("left ");
            kai__explore(node->left, 0);
            Context->prefix = KAI_STRING("right");
            kai__explore(node->expr, 1);
        }
        break; case KAI_STMT_COMPOUND: {
            Kai_Stmt_Compound* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("compound statement\n");

            Kai_Stmt current = node->head;
            while (current) {
                kai__explore(current, current->next == NULL);
                current = current->next;
            }
        }
        break; case KAI_STMT_IF: {
            Kai_Stmt_If* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("if statement\n");

            Context->prefix = KAI_STRING("expr");
            kai__explore(node->expr, 0);
            kai__explore(node->then_body, node->else_body == NULL);
            if (node->else_body != NULL) {
                kai__explore(node->else_body, 1);
            }
        }
        break; case KAI_STMT_FOR: {
            Kai_Stmt_For* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("for statement");
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write("(iterator name = ");
            kai__set_color(KAI_COLOR_IMPORTANT);
            kai__write_string(node->iterator_name);
            kai__set_color(KAI_COLOR_PRIMARY);
            kai__write(")\n");

            Context->prefix = KAI_STRING("from");
            kai__explore(node->from, 0);

            Context->prefix = KAI_STRING("to");
            kai__explore(node->to, 0);

            kai__explore(node->body, 1);
        }
    }
}

KAI_API (void) kai_write_syntax_tree(Kai_Writer* writer, Kai_Syntax_Tree* tree)
{
    Kai__Tree_Traversal_Context context = {
        .writer = writer,
        .stack_count = 1,
        .stack = { 1 }
    };
    kai__write("Top Level\n");
    kai__traverse_tree(&context, (Kai_Stmt)&tree->root);
}
KAI_API (void) kai_write_expression(Kai_Writer* writer, Kai_Expr expr)
{
    Kai__Tree_Traversal_Context context = { .writer = writer };
    kai__traverse_tree(&context, expr);
}

#endif
#ifndef KAI__SECTION_IMPLEMENTATION_TOKENIZER

#define KAI__X_TOKEN_KEYWORDS \
    X(break      , 0x80)      \
    X(cast       , 0x81)      \
    X(continue   , 0x82)      \
    X(defer      , 0x83)      \
    X(else       , 0x84)      \
    X(for        , 0x85)      \
    X(if         , 0x86)      \
    X(loop       , 0x87)      \
    X(ret        , 0x88)      \
    X(struct     , 0x89)      \
    X(using      , 0x8A)      \
    X(while      , 0x8B)

#define KAI__X_TOKEN_SYMBOLS \
    X('->') X('=>')          \
    X('==') X('!=')          \
    X('<=') X('>=')          \
    X('&&') X('||')          \
    X('<<') X('>>')          \
    X('..') X('---')

enum {
    KAI__TOKEN_END        = 0,
    KAI__TOKEN_IDENTIFIER = 0xC0,
    KAI__TOKEN_DIRECTIVE  = 0xC1,
    KAI__TOKEN_STRING     = 0xC2,
    KAI__TOKEN_NUMBER     = 0xC3,
#define X(NAME,ID) KAI__TOKEN_ ## NAME = ID,
    KAI__X_TOKEN_KEYWORDS
#undef X
};

typedef struct {
    Kai_u32     type;
    Kai_u32     line_number;
    Kai_string  string;
    Kai_Number  number;
} Kai__Token;

typedef struct {
    Kai__Token  current_token;
    Kai__Token  peeked_token;
    Kai_string  source;
    Kai_u32     cursor;
    Kai_u32     line_number;
    Kai_bool    peeking;
    Kai_Allocator allocator;
} Kai__Tokenizer;

// out_String->count: Maximum number of characters
// out_String->count must be >= 3
static inline void kai__token_type_string(Kai_u32 Type, Kai_string* out_String)
{
    switch (Type)
    {
    default:
    {
        kai__assert(Type <= 0xff);
        out_String->data[0] = '\'';
        out_String->data[1] = (char)Type;
        out_String->data[2] = '\'';
    } break;

    case KAI__TOKEN_END:
        out_String->count = kai__string_copy(out_String->data, "end of file", out_String->count);
    break;

    case KAI__TOKEN_IDENTIFIER:
        out_String->count = kai__string_copy(out_String->data, "identifier", out_String->count);
    break;

    case KAI__TOKEN_DIRECTIVE:
        out_String->count = kai__string_copy(out_String->data, "directive", out_String->count);
    break;

    case KAI__TOKEN_STRING:
        out_String->count = kai__string_copy(out_String->data, "string", out_String->count);
    break;

    case KAI__TOKEN_NUMBER:
        out_String->count = kai__string_copy(out_String->data, "number", out_String->count);
    break;

#define X(SYMBOL) \
    case SYMBOL: \
        out_String->count = kai__string_copy(out_String->data, #SYMBOL, out_String->count); \
    break;
    KAI__X_TOKEN_SYMBOLS
#undef X

#define X(NAME,ID) \
    case ID: \
        out_String->count = kai__string_copy(out_String->data, "'" #NAME "'", out_String->count); \
    break;
    KAI__X_TOKEN_KEYWORDS
#undef X
    }
}

#define KAI__CHECK(A,B)                               \
    if (context->source.data[context->cursor] == A) { \
        t->type = B;                                  \
        ++context->cursor;                            \
        ++t->string.count;                            \
        return;                                       \
    }

static inline void parse_multi_token(Kai__Tokenizer* context, Kai__Token* t, Kai_u8 current)
{
    if(context->cursor >= context->source.count) return;

    switch(current)
    {
    case '&': {
        KAI__CHECK('&', '&&')
    }
    case '|': {
        KAI__CHECK('|', '||')
    }
    case '=': {
        KAI__CHECK('=', '==')
    }
    case '>': {
        KAI__CHECK('=', '>=')
        KAI__CHECK('>', '>>')
    }
    case '<': {
        KAI__CHECK('=', '<=')
        KAI__CHECK('>', '<<')
    }
    case '!': {
        KAI__CHECK('=', '!=')
    }
    case '-': {
        KAI__CHECK('>', '->')
        if (context->cursor + 1 < context->source.count &&
            context->source.data[context->cursor] == '-' &&
            context->source.data[context->cursor+1] == '-'
        ) {
            t->type = '---';
            context->cursor += 2;
            t->string.count += 2;
        }
    }
    }
}
#undef KAI__CHECK

enum {
    // Last Bit == 0 means that the "character" can be used in an identifier
    KAI__T_IDENTIFIER = 0,

    // TODO: refactor to somehow not use single characters
    W = 1 << 1 | 1,
    T = 2 << 1 | 1,
    D = 3 << 1 | 1,
    K = 2,
    C = 5 << 1 | 1,
    N = 4,
    S = 7 << 1 | 1,
    Z = 8 << 1 | 1,

    KAI__T_WHITE_SPACE     = W, // white space is ignored
    KAI__T_CHARACTER_TOKEN = T, // example: ! <= != * +
    KAI__T_DIRECTIVE       = D,
    KAI__T_KEYWORD         = K,
    KAI__T_NUMBER          = N,
    KAI__T_COMMENT         = C, // can be comment, or just a divide symbol
    KAI__T_STRING          = S,
    KAI__T_DOT             = Z, // Dot is complicated..
};

static Kai_u8 kai__token_lookup_table[128] = {
    //  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W, // 0
    W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W, // 1
    W,  T,  S,  D,  T,  T,  T,  T,  T,  T,  T,  T,  T,  T,  Z,  C, // 2
    N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  T,  T,  T,  T,  T,  T, // 3
    T,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 4
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  T,  T,  T,  T,  0, // 5
    T,  0,  K,  K,  K,  K,  K,  0,  0,  K,  0,  0,  K,  0,  0,  0, // 6
    0,  0,  K,  K,  0,  K,  0,  K,  0,  0,  0,  T,  T,  T,  T,  W, // 7
};

#define kai__next_character_equals(C) \
( (context->cursor+1) < context->source.count && C == context->source.data[context->cursor+1] )

Kai_string keyword_map[] = {
#define X(NAME, ID) KAI_CONSTANT_STRING(#NAME),
    KAI__X_TOKEN_KEYWORDS
#undef X
};

static Kai_u8 kai__keyword_map[16] = {
    4, 3, 3, 3, 7, 10, 6, 6, 1, 11, 9, 8, 2, 0, 0, 5,
};
static inline int kai__hash_keyword(Kai_string s) {
    if (s.count < 2) return 6;
    return (s.count&3) | ((s.data[0]&2) << 2) | ((s.data[1]&2) << 1);
}

//! @TODO: String Escape \"
//! @TODO: Add "false" and "true" keywords ?
static inline Kai__Token kai__tokenizer_generate(Kai__Tokenizer* context)
{
    Kai__Token token = (Kai__Token) {
        .type = KAI__TOKEN_END,
        .string = {.count = 1},
        .line_number = context->line_number,
    };

    while (context->cursor < context->source.count)
    {
        token.string.data = context->source.data + context->cursor;
        Kai_u8 ch = *token.string.data;

        // treat every multi-byte unicode symbol as just an identifier
        if (ch & 0x80) goto case_identifier;

        switch (kai__token_lookup_table[ch])
        {

        case KAI__T_WHITE_SPACE: {
            if (ch == '\r') {
                // Handle Windows: CR LF
                if (kai__next_character_equals('\n')) ++context->cursor;
                context->line_number += 1;
            }
            else if (ch == '\n') {
                context->line_number += 1;
            }
            token.line_number = context->line_number;
            ++context->cursor;
            break;
        }

        case KAI__T_KEYWORD: {
            token.type = KAI__TOKEN_IDENTIFIER;
            ++context->cursor;
            while (context->cursor < context->source.count) {
                Kai_u8 c = context->source.data[context->cursor];
                if (c < 128 && (kai__token_lookup_table[c]&1)) {
                    break;
                }
                ++context->cursor;
                ++token.string.count;
            }

            // Check if the string we just parsed is a keyword
            int hash = kai__hash_keyword(token.string);
            int keyword_index = kai__keyword_map[hash];
            if (kai_string_equals(keyword_map[keyword_index], token.string)) {
                token.type = 0x80 | keyword_index;
                return token;
            }
            return token;
        }

        case KAI__T_DIRECTIVE: {
            token.type = KAI__TOKEN_DIRECTIVE;
            ++token.string.data; // skip over '#' symbol
            token.string.count = 0;
            ++context->cursor;
            goto parse_identifier;
        }

        case KAI__T_IDENTIFIER: {
        case_identifier:
            token.type = KAI__TOKEN_IDENTIFIER;
            ++context->cursor;
        parse_identifier:
            while (context->cursor < context->source.count) {
                Kai_u8 c = context->source.data[context->cursor];
                if (c < 128 && (kai__token_lookup_table[c]&1)) {
                    break;
                }
                ++context->cursor;
                ++token.string.count;
            }
            return token;
        }

        case KAI__T_STRING: {
            token.type = KAI__TOKEN_STRING;
            token.string.count = 0;
            ++context->cursor;
            ++token.string.data;
            Kai_u32 start = context->cursor;
            while (context->cursor < context->source.count) {
                if (context->source.data[context->cursor] == '\"') {
                    break;
                }
                context->cursor += 1;
            }
            token.string.count = (Kai_u32)(context->cursor - (Kai_uint)start);
            context->cursor += 1;
            return token;
        }

        case KAI__T_NUMBER: {
            ++context->cursor;
            token.type = KAI__TOKEN_NUMBER;

            // Integer
            if (ch == '0' && context->cursor < context->source.count)
            {
                if (context->source.data[context->cursor] == 'b')
                {
                    ++context->cursor;
                    token.number = kai_number_parse_whole(context->source, &context->cursor, 2);
                    goto return_number;
                }
                if (context->source.data[context->cursor] == 'x')
                {
                    ++context->cursor;
                    token.number = kai_number_parse_whole(context->source, &context->cursor, 16);
                    goto return_number;
                }
            }
            else --context->cursor;

            token.number = kai_number_parse_whole(context->source, &context->cursor, 10);

        parse_fraction:
            // Fractional Part
            if (context->cursor < context->source.count
            &&  context->source.data[context->cursor] == '.' && !kai__next_character_equals('.'))
            {
                ++context->cursor;
                Kai_Number decimal = kai_number_parse_decimal(context->source, &context->cursor);
                token.number = kai_number_add(token.number, decimal);
            }

            // Parse Exponential Part
            if (context->cursor < context->source.count
            && (context->source.data[context->cursor] == 'e' || context->source.data[context->cursor] == 'E'))
            {
                Kai_bool is_neg = KAI_FALSE;
                
                ++context->cursor;
                if (context->cursor < context->source.count)
                {
                    if (context->source.data[context->cursor] == '-') { ++context->cursor; is_neg = KAI_TRUE; }
                    if (context->source.data[context->cursor] == '+') ++context->cursor; // skip
                }
                
                Kai_Number exponent = kai_number_parse_exponent(context->source, &context->cursor);
                if (is_neg) exponent = kai_number_inv(exponent);
                token.number = kai_number_mul(token.number, exponent);
            }

        return_number:
            token.string.count = (Kai_u32)((context->source.data + context->cursor) - token.string.data);
            return token;
        }

        case KAI__T_COMMENT: {
            ++context->cursor;
            if (context->source.data[context->cursor] == '/' && context->cursor < context->source.count) {
                // single line comment
                while (context->cursor < context->source.count) {
                    if (context->source.data[context->cursor] == '\r' ||
                        context->source.data[context->cursor] == '\n')
                        break;
                    ++context->cursor;
                }
                break;
            }

            // multi-line comment
            if (context->source.data[context->cursor] == '*') {
                ++context->cursor;
                //! @TODO: need to skip "/*" and "*/" in strings
                int depth = 1;
                while (depth > 0 && context->cursor < context->source.count) {
                    if (context->source.data[context->cursor] == '/'
                    &&  (context->cursor + 1) < context->source.count
                    &&  context->source.data[context->cursor+1] == '*') {
                        context->cursor += 2;
                        depth += 1;
                        continue;
                    }

                    if (context->source.data[context->cursor] == '*'
                    &&  (context->cursor + 1) < context->source.count
                    &&  context->source.data[context->cursor+1] == '/') {
                        context->cursor += 2;
                        depth -= 1;
                        continue;
                    }

                    context->cursor += 1;
                }
                break;
            }

            // otherwise just a divide symbol
            token.type = '/';
            return token;
        }

        case KAI__T_DOT: {
            if (kai__next_character_equals('.')) {
                context->cursor += 2;
                ++token.string.count;
                token.type = '..';
                return token;
            }
            if ((context->cursor + 1) < context->source.count &&
                context->source.data[context->cursor + 1] >= '0' &&
                context->source.data[context->cursor + 1] <= '9') {
                token.type = KAI__TOKEN_NUMBER;
                goto parse_fraction;
            }
            // Not ".." or Number? then single character token
            token.type = (Kai_u32)ch;
            ++context->cursor;
            return token;
        }

        case KAI__T_CHARACTER_TOKEN: {
            token.type = (Kai_u32)ch;
            ++context->cursor;
            parse_multi_token(context, &token, ch);
            return token;
        }

        default: {
            kai__unreachable();
            return token;
        }

        }
    }
    return token;
}

Kai__Token* kai__tokenizer_next(Kai__Tokenizer* context)
{
    if (!context->peeking) {
        context->current_token = kai__tokenizer_generate(context);
        return &context->current_token;
    }
    context->peeking = KAI_FALSE;
    context->current_token = context->peeked_token;
    return &context->current_token;
}
Kai__Token* kai__tokenizer_peek(Kai__Tokenizer* context)
{
    if (context->peeking)
        return &context->peeked_token;
    context->peeking = KAI_TRUE;
    context->peeked_token = kai__tokenizer_generate(context);
    return &context->peeked_token;
}

#endif
#ifndef KAI__SECTION_IMPLEMENTATION_PARSER

typedef struct {
    Kai__Tokenizer         tokenizer;
    Kai__Arena_Allocator   arena;
    Kai_Error            * error;
} Kai__Parser;

static inline Kai_string kai__merge_string(Kai_string A, Kai_string B)
{
    Kai_string left  = A.data < B.data ? A : B;
    Kai_string right = A.data < B.data ? B : A;
    return (Kai_string) {
        .data = left.data,
        .count = (Kai_u32)(right.data + (Kai_uint)right.count - left.data),
    };
}

static inline Kai_Expr kai__parser_create_identifier(Kai__Parser* Parser, Kai__Token token)
{
    Kai_Expr_Identifier* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Identifier));
    node->id = KAI_EXPR_IDENTIFIER;
    node->source_code = token.string;
    node->line_number = token.line_number;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_string(Kai__Parser* Parser, Kai__Token token)
{
    Kai_Expr_String* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_String));
    node->id = KAI_EXPR_STRING;
    node->source_code = token.string;
    node->line_number = token.line_number;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_number(Kai__Parser* Parser, Kai__Token token)
{
    Kai_Expr_Number* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Number));
    node->id = KAI_EXPR_NUMBER;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->value = token.number;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_binary(Kai__Parser* Parser, Kai_Expr left, Kai_Expr right, Kai_u32 op)
{
    Kai_Expr_Binary* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Binary));
    node->id = KAI_EXPR_BINARY;
    node->source_code = kai__merge_string(left->source_code, right->source_code);
    node->line_number = kai__min_u32(left->line_number, right->line_number);
    node->op = op;
    node->left = left;
    node->right = right;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_unary(Kai__Parser* Parser, Kai__Token op_token, Kai_Expr expr)
{
    Kai_Expr_Unary* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Unary));
    node->id = KAI_EXPR_UNARY;
    node->source_code = kai__merge_string(op_token.string, expr->source_code);
    node->line_number = kai__min_u32(op_token.line_number, expr->line_number);
    node->op = op_token.type;
    node->expr = expr;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_array(Kai__Parser* Parser, Kai__Token op_token, Kai_Expr expr, Kai_u32 rows, Kai_u32 cols)
{
    Kai_Expr_Array* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Array));
    node->id = KAI_EXPR_ARRAY;
    node->source_code = kai__merge_string(op_token.string, expr->source_code);
    node->line_number = kai__min_u32(op_token.line_number, expr->line_number);
    node->rows = rows;
    node->cols = cols;
    node->expr = expr;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_struct(Kai__Parser* Parser, Kai__Token token, Kai_u32 field_count, Kai_Stmt body)
{
    Kai_Expr_Struct* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Struct));
    node->id = KAI_EXPR_STRUCT;
    node->source_code = token.string;
    node->line_number = token.line_number;
	node->field_count = field_count;
    node->head = body;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_procedure_type(Kai__Parser* Parser, Kai_Expr in_out, Kai_u8 in_count, Kai_u8 out_count)
{
    Kai_Expr_Procedure_Type* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Procedure_Type));
    node->id = KAI_EXPR_PROCEDURE_TYPE;
    node->source_code = in_out->source_code;
    node->line_number = in_out->line_number;
    node->in_out_expr = in_out;
    node->in_count = in_count;
    node->out_count = out_count;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_procedure_call(Kai__Parser* Parser, Kai_Expr proc, Kai_Expr args, Kai_u8 arg_count)
{
    Kai_Expr_Procedure_Call* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Procedure_Call));
    node->id = KAI_EXPR_PROCEDURE_CALL;
    node->source_code = proc->source_code;
    node->line_number = proc->line_number;
    node->proc = proc;
    node->arg_head = args;
    node->arg_count = arg_count;
    return (Kai_Expr)node;
}
static inline Kai_Expr kai__parser_create_procedure(Kai__Parser* Parser, Kai__Token token, Kai_Expr in_out, Kai_Stmt body, Kai_u8 in_count, Kai_u8 out_count)
{
    Kai_Expr_Procedure* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Expr_Procedure));
    node->id = KAI_EXPR_PROCEDURE;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->in_out_expr = in_out;
    node->in_count = in_count;
    node->out_count = out_count;
    node->body = body;
    return (Kai_Expr)node;
}
static inline Kai_Stmt kai__parser_create_declaration(Kai__Parser* Parser, Kai_string name, Kai_Expr type, Kai_Expr expr, Kai_u8 flags, Kai_u32 line_number)
{
    Kai_Stmt_Declaration* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Stmt_Declaration));
    node->id = KAI_STMT_DECLARATION;
    node->source_code = name;
    node->line_number = line_number;
    node->name = name;
    node->type = type;
    node->expr = expr;
    node->flags = flags;
    return (Kai_Stmt)node;
}
static inline Kai_Stmt kai__parser_create_compound(Kai__Parser* Parser, Kai__Token token, Kai_Stmt body)
{
    Kai_Stmt_Compound* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Stmt_Compound));
    node->id = KAI_STMT_COMPOUND;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->head = body;
    return (Kai_Stmt)node;
}
static inline Kai_Stmt kai__parser_create_return(Kai__Parser* Parser, Kai__Token ret_token, Kai_Expr expr)
{
    Kai_Stmt_Return* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Stmt_Return));
    node->id = KAI_STMT_RETURN;
    node->source_code = ret_token.string;
    node->line_number = ret_token.line_number;
    node->expr = expr;
    return (Kai_Stmt)node;
}
static inline Kai_Stmt kai__parser_create_if(Kai__Parser* Parser, Kai__Token if_token, Kai_Expr expr, Kai_Stmt then_body, Kai_Stmt else_body)
{
    Kai_Stmt_If* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Stmt_If));
    node->id = KAI_STMT_IF;
    node->source_code = if_token.string;
    node->line_number = if_token.line_number;
    node->expr = expr;
    node->then_body = then_body;
    node->else_body = else_body;
    return (Kai_Stmt)node;
}
static inline Kai_Stmt kai__parser_create_for(Kai__Parser* Parser, Kai__Token for_token, Kai_string name, Kai_Expr from, Kai_Expr to, Kai_Stmt body, Kai_u8 flags)
{
    Kai_Stmt_For* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Stmt_For));
    node->id = KAI_STMT_FOR;
    node->source_code = for_token.string;
    node->line_number = for_token.line_number;
    node->body = body;
    node->from = from;
    node->to = to;
    node->iterator_name = name;
    node->flags = flags;
    return (Kai_Stmt)node;
}
static inline Kai_Stmt kai__parser_create_assignment(Kai__Parser* Parser, Kai_Expr left, Kai_Expr expr)
{
    Kai_Stmt_Assignment* node = kai__arena_allocate(&Parser->arena, sizeof(Kai_Stmt_Assignment));
    node->id = KAI_STMT_ASSIGNMENT;
    node->source_code = left->source_code;
    node->line_number = left->line_number;
    node->left = left;
    node->expr = expr;
    return (Kai_Stmt)node;
}

enum {
    KAI__OPERATOR_TYPE_BINARY,
    KAI__OPERATOR_TYPE_INDEX,
    KAI__OPERATOR_TYPE_PROCEDURE_CALL,
};

typedef struct {
    Kai_u32 type;
    Kai_s32 prec;
} Kai__Operator;

#define KAI__PREC_DEFAULT -6942069
#define KAI__PREC_CAST     0x0900
#define KAI__PREC_UNARY    0x1000

Kai__Operator kai__token_type_to_operator(Kai_u32 t)
{
    switch (t)
    {
    case '&&': return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0010};
    case '||': return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0010};
    case '==': return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0040};
    case '<=': return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0040};
    case '>=': return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0040};
    case '<':  return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0040};
    case '>':  return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0040};
    case '+':  return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0100};
    case '-':  return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0100};
    case '*':  return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0200};
    case '/':  return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0x0200};
    case '.':  return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, 0xFFFF};  // member access
    case '->': return (Kai__Operator){KAI__OPERATOR_TYPE_BINARY, KAI__PREC_CAST};
    case '[':  return (Kai__Operator){KAI__OPERATOR_TYPE_INDEX,  0xBEEF};
    case '(':  return (Kai__Operator){KAI__OPERATOR_TYPE_PROCEDURE_CALL, 0xBEEF};
    default:   return (Kai__Operator){0};
    }
}

static inline void* kai__error_unexpected(Kai__Parser* parser, Kai__Token* token, Kai_string where, Kai_string wanted)
{
    Kai_Allocator* allocator = &parser->arena.allocator;

    // Report only the first Syntax Error
    if (parser->error->result != KAI_SUCCESS)
        return NULL;

    Kai__Dynamic_Buffer buffer = {0};
    kai__dynamic_buffer_append_string(&buffer, KAI_STRING("unexpected "));
    Kai_u8 temp [32] = {0};
    Kai_string type = { .data = temp, .count = sizeof(temp) };
    kai__token_type_string(token->type, &type);
    kai__dynamic_buffer_append_string(&buffer, type);
    kai__dynamic_buffer_append_string(&buffer, KAI_STRING(" "));
    kai__dynamic_buffer_append_string(&buffer, where);
    Kai_range message_range = kai__dynamic_buffer_next(&buffer);
    Kai_Memory memory = kai__dynamic_buffer_release(&buffer);

    *parser->error = (Kai_Error) {
        .result = KAI_ERROR_SYNTAX,
        .location = {
            .string = token->string,
            .line = token->line_number,
        },
        .message = kai__range_to_string(message_range, memory),
        .context = wanted,
        .memory = memory,
    };
    
    if (token->type == KAI__TOKEN_STRING)
    {
        parser->error->location.string.data  -= 1; // strings must begin with "
        parser->error->location.string.count += 2;
    }
    else if (token->type == KAI__TOKEN_DIRECTIVE)
    {
        parser->error->location.string.data  -= 1;
        parser->error->location.string.count += 1;
    }
    return NULL;
}

#define kai__unexpected(Where, Context) kai__error_unexpected(parser, &parser->tokenizer.current_token, KAI_STRING(Where), KAI_STRING(Context))
#define kai__expect(EXPR, Where, Context) if (!(EXPR)) return kai__unexpected(Where, Context)

#define kai__next_token()     kai__tokenizer_next(&parser->tokenizer)
#define kai__peek_token()     kai__tokenizer_peek(&parser->tokenizer)
#define kai__parse_type()     kai__parse_type_expression(parser)
#define kai__parse_expr(Prec) kai__parse_expression(parser, Prec)
#define kai__parse_stmt()     kai__parse_statement(parser)
#define kai__parse_proc()     kai__parse_procedure(parser)

Kai_Expr kai__parse_type_expression(Kai__Parser* parser);
Kai_Expr kai__parse_expression(Kai__Parser* parser, Kai_s32 precedence);
Kai_Stmt kai__parse_statement(Kai__Parser* parser);
Kai_Expr kai__parse_procedure(Kai__Parser* parser);
Kai_Expr kai__parse_declaration(Kai__Parser* parser);

Kai_bool kai__is_procedure_next(Kai__Parser* parser)
{
    Kai__Token* p = kai__peek_token();
    if (p->type == ')')
      return KAI_TRUE;

    Kai__Tokenizer state = parser->tokenizer;
    Kai__Token* cur = kai__next_token();
    Kai_bool found = KAI_FALSE;

    // go until we hit ')' or END, searching for ':'
    while (cur->type != KAI__TOKEN_END && cur->type != ')')
    {
        if (cur->type == ':')
        {
            found = KAI_TRUE;
            break;
        }
        kai__next_token();
    }
    parser->tokenizer = state;
    return found;
}
Kai_Expr kai__parse_type_expression(Kai__Parser* parser)
{
	Kai__Token* t = &parser->tokenizer.current_token;

	if (t->type != '(')
		return kai__parse_expr(KAI__PREC_DEFAULT);

	kai__next_token(); // skip '('

	Kai_u8 in_count = 0;
	Kai_u8 out_count = 0;
    KAI__LINKED_LIST(Kai_Expr) in_out = {0};

	// Parse Procedure input types
	if (t->type != ')') for (;;)
    {
		Kai_Expr type = kai__parse_type();
		if (!type)
			return kai__unexpected("in procedure type", "should be a type here");

        kai__linked_list_append(in_out, type);

		kai__expect(in_count != 255, "in procedure type", "too many inputs to procedure");
		in_count += 1;

		kai__next_token();  // get ',' token or ')'

		if (t->type == ')') break;
		if (t->type == ',') kai__next_token();
		else return kai__unexpected("in procedure type", "',' or ')' expected here");
	}

	Kai__Token* peek = kai__peek_token();  // see if we have any returns

	///////////////////////////////////////////////////////////////////////
	// "This code is broken as hell."
	if (peek->type == '->') {
		kai__next_token();  // eat '->'
		kai__next_token();  // get token after

		Kai_bool enclosed_return = KAI_FALSE;
		if (t->type == '(') {
			enclosed_return = KAI_TRUE;
			kai__next_token();
		}

		for (;;) {
			Kai_Expr type = kai__parse_type();
			if (!type)
				return NULL;

            kai__linked_list_append(in_out, type);

			kai__expect(out_count != 255, "in procedure type",
				"too many inputs to procedure");
			out_count += 1;

			kai__peek_token();  // get ',' token or something else

			if (peek->type == ',') {
				kai__next_token();  // get ','
				kai__next_token();  // skip ','
			}
			else break;
		}

		if (enclosed_return) {
			if (peek->type != ')') {
				kai__next_token();
				return kai__unexpected("in procedure type",
					"should be ')' after return types");
			}
			else kai__next_token();
		}
	}
	///////////////////////////////////////////////////////////////////////

    return kai__parser_create_procedure_type(parser, in_out.head, in_count, out_count);
}
Kai_Expr kai__parse_expression(Kai__Parser* parser, Kai_s32 prec)
{
    Kai_Expr left = NULL;
    Kai__Token* t = &parser->tokenizer.current_token;

    switch (t->type)
    {
    default: return NULL;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Single-Token Expressions
    case KAI__TOKEN_IDENTIFIER: left = kai__parser_create_identifier(parser, *t); break;
    case KAI__TOKEN_NUMBER:     left = kai__parser_create_number(parser, *t); break;
    case KAI__TOKEN_STRING:     left = kai__parser_create_string(parser, *t); break;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Parenthesis
    case '(': {
        kai__next_token();
        left = kai__parse_expr(KAI__PREC_DEFAULT);
        kai__expect(left, "in expression", "should be an expression here");
        kai__next_token();
        kai__expect(t->type == ')', "in expression", "an operator or ')' in expression");
    } break;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Array/Slice Types
    case '[': {
        Kai__Token op_token = *t;
		kai__next_token(); // skip '['

		Kai_u32 rows = 1, cols = 1;

		if (t->type != ']') {
			kai__expect(t->type == KAI__TOKEN_NUMBER, "in array type", "should be a number here");
            kai__todo("make sure number is an integer");
            //rows = (Kai_u32)t->number.Whole_Part;
		
			kai__next_token(); // skip number
			if (t->type == ',') {
				kai__next_token(); // skip ','
			    kai__expect(t->type == KAI__TOKEN_NUMBER, "in array type", "should be a number here");
                kai__todo("make sure number is an integer");
				//cols = (Kai_u32)t->number.Whole_Part;
				kai__next_token(); // skip number
			}
		}

		kai__expect(t->type == ']', "in array type", "array");
		kai__next_token(); // skip ']'
        Kai_Expr type = kai__parse_type();
		kai__expect(type, "in array type", "expected type here");
        // Types do not have binary operators (I hope it stays this way), so return early
        return kai__parser_create_array(parser, op_token, type, rows, cols);
    } break;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Unary Operators
    case '-':
    case '+':
    case '*':
    case '/': {
        Kai__Token op_token = *t;
        kai__next_token();
        left = kai__parse_expr(KAI__PREC_UNARY); // TODO: should unary have all same precidence?
        kai__expect(left, "in unary expression", "should be an expression here");
        left = kai__parser_create_unary(parser, op_token, left);
    } break;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Explicit Casting "cast(int) x"
    case KAI__TOKEN_cast: {
        kai__next_token();
        kai__expect(t->type == '(', "in expression", "'(' after cast keyword");
        kai__next_token();
        Kai_Expr type = kai__parse_type();
        kai__expect(type, "in expression", "type");
        kai__next_token();
        kai__expect(t->type == ')', "in expression", "')' after Type in cast expression");
        kai__next_token();
        Kai_Expr expr = kai__parse_expr(KAI__PREC_CAST);
        kai__expect(expr, "in expression", "expression after cast");
        left = kai__parser_create_binary(parser, expr, type, '->');
    } break;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Directives
    case KAI__TOKEN_DIRECTIVE: {
        if (kai_string_equals(KAI_STRING("type"), t->string))
        {
            kai__next_token();
            left = kai__parse_type();
            kai__expect(left, "in expression", "type");
        }
        else if (kai_string_equals(KAI_STRING("Julie"), t->string))
        {
            t->string = KAI_STRING("<3");
            left = kai__parser_create_string(parser, *t);
        }
        else return NULL;
    } break;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Struct
    case KAI__TOKEN_struct: {
        Kai__Token struct_token = *t;
        KAI__LINKED_LIST(Kai_Stmt) body = {0};
		Kai_u32 count = 0;

        kai__next_token(); // skip struct
        kai__next_token(); // skip '{'
        while (t->type != '}')
        {
            Kai_Stmt stmt = kai__parse_stmt();
            kai__expect(stmt, "in struct definition", "expected a statement here");
            kai__linked_list_append(body, stmt);
			count += 1;
            kai__next_token();
        }

        left = kai__parser_create_struct(parser, struct_token, count, body.head);
    } break;
    }

    for (;;)
    {
        Kai__Token *p = kai__peek_token();
        Kai_u32 token_type = p->type;
        Kai__Operator op_info = kai__token_type_to_operator(token_type);

        // handle precedence by returning early
        if (op_info.prec == 0 || op_info.prec < prec)
			return left;

        kai__next_token();
        kai__next_token();

        switch (op_info.type)
        {
        case KAI__OPERATOR_TYPE_BINARY: {
            Kai_Expr right = kai__parse_expr(op_info.prec);
            kai__expect(right, "in binary expression", "should be an expression after binary operator");
            left = kai__parser_create_binary(parser, left, right, token_type);
        } break;

        case KAI__OPERATOR_TYPE_INDEX: {
            Kai_Expr right = kai__parse_expr(KAI__PREC_DEFAULT);
            kai__expect(right, "in index operation", "should be an expression here");
            kai__next_token();
            kai__expect(t->type == ']', "in index operation", "expected ']' here");
            left = kai__parser_create_binary(parser, left, right, token_type);
        } break;

        case KAI__OPERATOR_TYPE_PROCEDURE_CALL: {
            KAI__LINKED_LIST(Kai_Expr) args = {0};
            Kai_u8 arg_count = 0;

            if (t->type != ')') for (;;)
            {
                Kai_Expr expr = kai__parse_expr(KAI__PREC_DEFAULT);
                kai__expect(expr, "in procedure call", "an expression");
                kai__linked_list_append(args, expr);

                kai__expect(arg_count != 255, "in procedure call", "too many inputs to procedure");
                arg_count += 1;

                kai__next_token(); // skip expr
                if (t->type == ')') break;
                if (t->type == ',') kai__next_token(); // skip ','
                else return kai__unexpected("in procedure call", "',' or ')' expected here");
            }

            left = kai__parser_create_procedure_call(parser, left, args.head, arg_count);
        } break;

        default:
            return kai__unexpected("in expression", "expected an operator here");
        }
    }
}
Kai_Expr kai__parse_procedure(Kai__Parser* parser)
{
    Kai__Token* t = &parser->tokenizer.current_token;
    Kai__Token token = *t;

    // sanity check
    kai__expect(t->type == '(', "", "this is likely a compiler bug, sorry :c");
    kai__next_token(); // skip '('

    Kai_u8 in_count = 0;
    Kai_u8 out_count = 0;
    KAI__LINKED_LIST(Kai_Expr) in_out = {0};

    // TODO: this should be a loop, what is this goto??
    if (t->type != ')')
    {
    parse_parameter:
        (void)0;
        Kai_u8 flags = 0;
        if (t->type == KAI__TOKEN_using) {
            flags |= 1; // TODO: KAI_PARAMETER_FLAG_USING;
            kai__next_token();
        }
        kai__expect(t->type == KAI__TOKEN_IDENTIFIER, "in procedure input", "should be an identifier");
        Kai_string name = t->string;
        kai__next_token();
        kai__expect(t->type == ':', "in procedure input", "wanted a ':' here");
        kai__next_token();
        Kai_Expr type = kai__parse_type();
        kai__expect(type, "in procedure input", "should be type");

        type->name = name;
        type->flags = flags;

        kai__linked_list_append(in_out, type);

        kai__expect(in_count != 255, "in procedure call", "too many inputs to procedure");
        in_count += 1;

        kai__next_token();

        switch (t->type) {
        case ')': break;
        case ',': kai__next_token(); goto parse_parameter;
        default: return kai__unexpected("in procedure input", "wanted ')' or ',' here");
        }
    }
    kai__next_token();

    // return value
    if (t->type == '->') {
        kai__next_token();
        Kai_Expr type = kai__parse_type();
        kai__expect(type, "in procedure return type", "should be type");

        kai__linked_list_append(in_out, type);

        kai__expect(out_count != 255, "in procedure call", "too many inputs to procedure");
        out_count += 1;

        kai__next_token();
    }

    Kai_Stmt body = NULL;
    if (t->type == KAI__TOKEN_DIRECTIVE &&
        kai_string_equals(KAI_STRING("native"), t->string)) {
        kai__next_token();
        kai__expect(t->type == ';', "???", "???");
    }
    else {
        body = kai__parse_stmt();
        if (!body) return NULL;
    }

    return kai__parser_create_procedure(parser, token, in_out.head, body, in_count, out_count);
}
Kai_Stmt kai__parse_statement(Kai__Parser* parser)
{
    Kai__Token* t = &parser->tokenizer.current_token;
    switch (t->type)
    {
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Compound Statements
    case '{': {
        Kai__Token token = *t;
        kai__next_token();

        Kai_Stmt head = NULL;
        Kai_Stmt current = NULL;

        // parse statements until we get a '}'
        while (t->type != '}') {
            Kai_Stmt statement = kai__parse_stmt();
            if (!statement)
                return NULL;

            if (!head)
                head = statement;
            else
                current->next = statement;
            current = statement;

            kai__next_token();  // eat ';' (get token after)
        }

        // TODO: is this OK?
        // No need for compound if there is only one statement
        // if (statement_array.count == 1) {
        //    p_tarray_destroy(statement_array);
        //    return *(Kai_Stmt*)((Kai_u8*)parser->memory.temperary +
        //    statement_array.offset);
        //}

        return kai__parser_create_compound(parser, token, head);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Return Statements
    case KAI__TOKEN_ret: {
        Kai__Token ret_token = *t;
        kai__next_token(); // skip 'ret'

        if (t->type == ';')
            return kai__parser_create_return(parser, ret_token, NULL);

        Kai_Expr expr = kai__parse_expr(KAI__PREC_DEFAULT);
        kai__expect(expr, "in return statement", "should be an expression");
        kai__next_token();
        kai__expect(t->type == ';', "after statement", "there should be a ';' before this");

        return kai__parser_create_return(parser, ret_token, expr);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // If Statements
    case KAI__TOKEN_if: {
        Kai__Token if_token = *t;
        kai__next_token(); // skip 'if'
        Kai_Expr expr = kai__parse_expr(KAI__PREC_DEFAULT);
        kai__expect(expr, "in if statement", "should be an expression here");
        kai__next_token();
        Kai_Stmt then_body = kai__parse_stmt();
        if (!then_body)
            return NULL;

        Kai__Token* p = kai__peek_token();
        Kai_Stmt else_body = NULL;
        if (p->type == KAI__TOKEN_else) {
            kai__next_token();
            kai__next_token();
            else_body = kai__parse_stmt();
            if (!else_body)
                return NULL;
        }
        return kai__parser_create_if(parser, if_token, expr, then_body, else_body);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // For Statements
    case KAI__TOKEN_for: {
        Kai__Token for_token = *t;
        kai__next_token(); // skip 'for'
        kai__expect(t->type == KAI__TOKEN_IDENTIFIER, "in for statement",
            "should be the name of the iterator");
        Kai_string iterator_name = t->string;
        kai__next_token();
        kai__expect(t->type == ':', "in for statement", "should be ':' here");
        kai__next_token();
        Kai_Expr from = kai__parse_expr(KAI__PREC_DEFAULT);
        kai__expect(from, "in for statement", "should be an expression here");
        Kai_Expr to = NULL;
        kai__next_token();
        if (t->type == '..') {
            kai__next_token();
            to = kai__parse_expr(KAI__PREC_DEFAULT);
            kai__expect(to, "in for statement", "should be an expression here");
            kai__next_token();
        }
        Kai_Stmt body = kai__parse_stmt();
        if (!body)
            return NULL;
        return kai__parser_create_for(parser, for_token, iterator_name, from, to, body, 0);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Handle Possible Declarations
    case KAI__TOKEN_IDENTIFIER: {
        Kai__Token* p = kai__peek_token();
        // just an expression?
        if (p->type == ':')
            return kai__parse_declaration(parser);
    } /* FALLTHROUGH */
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Expression/Assignment Statements
    default: {
        Kai_Expr expr = kai__parse_expr(KAI__PREC_DEFAULT);
        kai__expect(expr, "in statement", "should be an expression or statement");
        kai__next_token(); // skip expr

        if (t->type == '=') {
            kai__next_token();
            Kai_Expr right = kai__parse_expr(KAI__PREC_DEFAULT);
            kai__expect(right, "in assignment statement", "should be an expression");
            expr = kai__parser_create_assignment(parser, expr, right);
            kai__next_token();
            kai__expect(t->type == ';', "in assignment statement", "should be ';' after expression");
        }
        else kai__expect(t->type == ';', "in expression statement", "should be ';' after expression");
        return expr;
    }
    }
}
Kai_Stmt kai__parse_declaration(Kai__Parser* parser)
{
    Kai__Token* t = &parser->tokenizer.current_token;

    if (t->type != KAI__TOKEN_IDENTIFIER)
        return kai__unexpected("in declaration", "expected an identifier");

    Kai_string name = t->string;
    Kai_u32 line_number = t->line_number;

    kai__next_token(); // skip identifier

    if (t->type != ':')
        return kai__unexpected("in declaration", "expected ':' here");

    kai__next_token(); // skip ':'

    Kai_u8 flags = 0;
    Kai_Expr type = kai__parse_type();

    if (type) kai__next_token(); // skip type if there was one

    Kai_Expr expr = NULL;

    switch (t->type)
    {
    case ':': flags |= KAI_DECL_FLAG_CONST; break;
    case '=': break;
    case ';': goto done_declaration;
    default:
        return kai__unexpected("in declaration", "should be '=', ':', or ';'");
    }
    kai__next_token(); // skip '=', ':', or ';'

    // structs and procedures do not require semicolon
    Kai_bool require_semicolon = KAI_FALSE;

    if (t->type == '(' && kai__is_procedure_next(parser)) {
        expr = kai__parse_proc();
        if (!expr)
            return NULL;
    }
    else {
        require_semicolon = KAI_TRUE;
        if (t->type == KAI__TOKEN_struct)
            require_semicolon = KAI_FALSE;
        expr = kai__parse_expr(KAI__PREC_DEFAULT);
        kai__expect(expr, "in declaration", "should be an expression here");
    }
    
    if (require_semicolon) {
        kai__next_token();
        kai__expect(t->type == ';', "after statement",
            "there should be a ';' before this");
    }
    else {
        Kai__Token* p = kai__peek_token();
        if (p->type == ';')
            kai__next_token(); // go to semicolon
    }

done_declaration:
    return kai__parser_create_declaration(parser, name, type, expr, flags, line_number);
}

KAI_API (Kai_Result) kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* Info, Kai_Syntax_Tree* tree)
{
    Kai__Parser p = {
        .tokenizer = {
            .source = Info->source_code,
            .cursor = 0,
            .line_number = 1,
            .allocator = Info->allocator,
        },
        .error = Info->error,
    };
    kai__arena_allocator_create(&p.arena, &Info->allocator);

    KAI__LINKED_LIST(Kai_Stmt) statements = {0};
    Kai__Token* token = kai__tokenizer_next(&p.tokenizer);
    while (token->type != KAI__TOKEN_END)
    {
        Kai_Stmt statement = kai__parse_declaration(&p);
        if (statement == NULL)
            break;
        kai__linked_list_append(statements, statement);
        kai__tokenizer_next(&p.tokenizer);
    }

    tree->allocator = p.arena;
    tree->root.id = KAI_STMT_COMPOUND;
    tree->root.head = statements.head;

    // TODO: find actual solution for this
    Info->error->location.file_name = tree->source_filename;
    Info->error->location.source = Info->source_code.data;
    return p.error->result;
}
KAI_API (void) kai_destroy_syntax_tree(Kai_Syntax_Tree* tree)
{
    kai__arena_allocator_destroy(&tree->allocator);
    *tree = (Kai_Syntax_Tree) {0};
}

#endif
#ifndef KAI__SECTION_IMPLEMENTATION_BYTECODE

// extern char const* bc__op_to_name [100];

#define kai__make_space(Bytes_Required)             \
	Kai_Allocator* allocator = Encoder->allocator;  \
	kai__array_grow(&Encoder->code, Bytes_Required);

// BYTECODE INSTRUCTION DESCRIPTION SYNTAX:
//
// Basic: [WHAT_IS_STORED]:bits_required
//
// Concatenation: [option1:size1 | option2:size2]:size   [ size = max(size1, size2) ]
// Addition:      [part1  :4     + part2  :4    ]:size   [ size = 4 + 4 ]
//
// S = size of platform (ex: 64-bit -> 64)
// R = size to hold register index (hard-coded to 32 bits)
// $ = size inferred from TYPE
// - = use rest of bits available

// ----------------------------------------------------------------------------------------------
//  LOAD_CONSTANT         [TYPE]:8 [DST]:R [VALUE]:$
//  MATH                  [TYPE]:8 [OPERATION]:8 [DST]:R [LEFT]:R [RIGHT]:R
//  MATH_CONSTANT         [TYPE]:8 [OPERATION]:8 [DST]:R [LEFT]:R [VALUE]:$
//  COMPARE               [TYPE]:8 [COMPARISON]:8 [DST]:R [LEFT]:R [RIGHT]:R
//  COMPARE_CONSTANT      [TYPE]:8 [COMPARISON]:8 [DST]:R [LEFT]:R [VALUE]:$
//  BRANCH                [LOCATION]:32 [SRC]:R
//  JUMP                  [LOCATION]:32
//  CALL                  [RET_COUNT]:8 [ARG_COUNT]:8 [LOCATION]:32 ([DST]:R ...) ([SRC]:R ...)
//  RETURN                [COUNT]:8 ([SRC]:R ...)
//  NATIVE_CALL           [USE_DST]:8 [COUNT]:8 [ADDRESS]:S ([DST]:R) ([SRC]:R ...)
//  LOAD                  [TYPE]:8 [DST]:R [ADDR]:R [OFFSET]:32
//  STORE                 [TYPE]:8 [SRC]:R [ADDR]:R [OFFSET]:32
//  STACK_ALLOC           [SIZE]:32 [DST]:R
//  STACK_FREE            [SIZE]:32
// ----------------------------------------------------------------------------------------------

KAI_API (void) kai_encode_load_constant(Kai_Bytecode_Encoder* Encoder, Kai_u8 type, Kai_Reg reg_dst, Kai_Value value)
{
    kai__make_space(2 + sizeof(Kai_Reg) + kai__typeid_to_size(type));
    kai__push_u8(Encoder->code, KAI_BOP_LOAD_CONSTANT);
    kai__push_u8(Encoder->code, type);
    kai__push_u32(Encoder->code, reg_dst);
    kai__push_value(Encoder->code, type, value);
}
KAI_API (void) kai_encode_math(Kai_Bytecode_Encoder* Encoder, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2)
{
    kai__make_space(3 + sizeof(Kai_Reg) * 3);
    kai__push_u8(Encoder->code, KAI_BOP_MATH);
    kai__push_u8(Encoder->code, type);
    kai__push_u8(Encoder->code, operation);
    kai__push_u32(Encoder->code, reg_dst);
    kai__push_u32(Encoder->code, reg_src1);
    kai__push_u32(Encoder->code, reg_src2);
}
KAI_API (void) kai_encode_math_constant(Kai_Bytecode_Encoder* Encoder, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value)
{
    kai__make_space(3 + sizeof(Kai_Reg) * 2 + kai__typeid_to_size(type));
    kai__push_u8(Encoder->code, KAI_BOP_MATH_CONSTANT);
    kai__push_u8(Encoder->code, type);
    kai__push_u8(Encoder->code, operation);
    kai__push_u32(Encoder->code, reg_dst);
    kai__push_u32(Encoder->code, reg_src1);
    kai__push_value(Encoder->code, type, value);
}
KAI_API (void) kai_encode_compare(Kai_Bytecode_Encoder* Encoder, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2)
{
    kai__make_space(3 + sizeof(Kai_Reg) * 3);
    kai__push_u8(Encoder->code, KAI_BOP_COMPARE);
    kai__push_u8(Encoder->code, type);
    kai__push_u8(Encoder->code, comparison);
    kai__push_u32(Encoder->code, reg_dst);
    kai__push_u32(Encoder->code, reg_src1);
    kai__push_u32(Encoder->code, reg_src2);
}
KAI_API (void) kai_encode_compare_constant(Kai_Bytecode_Encoder* Encoder, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value)
{
    kai__make_space(3 + sizeof(Kai_Reg) * 2 + kai__typeid_to_size(type));
    kai__push_u8(Encoder->code, KAI_BOP_COMPARE_CONSTANT);
    kai__push_u8(Encoder->code, type);
    kai__push_u8(Encoder->code, comparison);
    kai__push_u32(Encoder->code, reg_dst);
    kai__push_u32(Encoder->code, reg_src1);
    kai__push_value(Encoder->code, type, value);
}
KAI_API (void) kai_encode_branch_location(Kai_Bytecode_Encoder* Encoder, Kai_u32 location, Kai_Reg reg_src)
{
    kai__make_space(1 + sizeof(Kai_Reg) * 2);
    kai__push_u8(Encoder->code, KAI_BOP_BRANCH);
    kai__push_u32(Encoder->code, location);
    kai__push_u32(Encoder->code, reg_src);
}
KAI_API (void) kai_encode_branch(Kai_Bytecode_Encoder* Encoder, Kai_u32* branch, Kai_Reg reg_src)
{
    kai__make_space(1 + sizeof(Kai_Reg) * 2);
    kai__push_u8(Encoder->code, KAI_BOP_BRANCH);
    *branch = Encoder->code.count;
    kai__push_u32(Encoder->code, 0xFFFFFFFF); // location not set yet
    kai__push_u32(Encoder->code, reg_src);
}
KAI_API (void) kai_encode_jump_location(Kai_Bytecode_Encoder* Encoder, Kai_u32 location)
{
    kai__make_space(1 + sizeof(Kai_u32));
    kai__push_u8(Encoder->code, KAI_BOP_JUMP);
    kai__push_u32(Encoder->code, location);
}
KAI_API (void) kai_encode_jump(Kai_Bytecode_Encoder* Encoder, Kai_u32* branch)
{
    kai__make_space(1 + sizeof(Kai_u32));
    kai__push_u8(Encoder->code, KAI_BOP_JUMP);
    *branch = Encoder->code.count;
    kai__push_u32(Encoder->code, 0xFFFFFFFF); // location not set yet
}
KAI_API (void) kai_encode_call(Kai_Bytecode_Encoder* Encoder, Kai_u32* branch, Kai_u8 ret_count, Kai_Reg* reg_ret, Kai_u8 arg_count, Kai_Reg* reg_arg)
{
    kai__make_space(3 + sizeof(Kai_u32) + sizeof(Kai_Reg) * (ret_count + arg_count));
    kai__push_u8(Encoder->code, KAI_BOP_CALL);
    kai__push_u8(Encoder->code, ret_count);
    kai__push_u8(Encoder->code, arg_count);
    *branch = Encoder->code.count;
    kai__push_u32(Encoder->code, 0xFFFFFFFF); // location not set yet
    kai__for_n (ret_count) kai__push_u32(Encoder->code, reg_ret[i]);
    kai__for_n (arg_count) kai__push_u32(Encoder->code, reg_arg[i]);
}
KAI_API (void) kai_encode_return(Kai_Bytecode_Encoder* Encoder, Kai_u8 count, Kai_Reg* regs)
{
    kai__make_space(2 + sizeof(Kai_Reg) * count);
    kai__push_u8(Encoder->code, KAI_BOP_RETURN);
    kai__push_u8(Encoder->code, count);
    kai__for_n(count) kai__push_u32(Encoder->code, regs[i]);
}
KAI_API (void) kai_encode_native_call(Kai_Bytecode_Encoder* Encoder, Kai_u8 use_dst, Kai_Reg reg_dst, Kai_Native_Procedure* proc, Kai_Reg* reg_src)
{
    kai__make_space(3 + sizeof(Kai_uint) + (use_dst? sizeof(Kai_Reg):0) + sizeof(Kai_Reg) * proc->input_count);
    kai__push_u8(Encoder->code, KAI_BOP_NATIVE_CALL);
    kai__push_u8(Encoder->code, use_dst? 1:0);
    kai__push_u8(Encoder->code, proc->input_count);
    kai__push_address(Encoder->code, proc->address);
    if (use_dst) kai__push_u32(Encoder->code, reg_dst);
    kai__for_n (proc->input_count) kai__push_u32(Encoder->code, reg_src[i]);
}
KAI_API (void) kai_encode_load(Kai_Bytecode_Encoder* Encoder, Kai_Reg reg_dst, Kai_u8 type, Kai_Reg reg_addr, uint32_t offset)
{
    kai__make_space(2 + sizeof(Kai_Reg) * 2 + sizeof(Kai_u32));
    kai__push_u8(Encoder->code, KAI_BOP_LOAD);
    kai__push_u8(Encoder->code, type);
    kai__push_u32(Encoder->code, reg_dst);
    kai__push_u32(Encoder->code, reg_addr);
    kai__push_u32(Encoder->code, offset);
}
KAI_API (void) kai_encode_store(Kai_Bytecode_Encoder* Encoder, Kai_Reg reg_src, Kai_u8 type, Kai_Reg reg_addr, uint32_t offset)
{
    kai__make_space(2 + sizeof(Kai_Reg) * 2 + sizeof(uint32_t));
    kai__push_u8(Encoder->code, KAI_BOP_STORE);
    kai__push_u8(Encoder->code, type);
    kai__push_u32(Encoder->code, reg_src);
    kai__push_u32(Encoder->code, reg_addr);
    kai__push_u32(Encoder->code, offset);
}
KAI_API (void) kai_encode_stack_alloc(Kai_Bytecode_Encoder* Encoder, Kai_Reg reg_dst, uint32_t size)
{
    kai__make_space(1 + sizeof(Kai_Reg) + sizeof(Kai_u32));
    kai__push_u8(Encoder->code, KAI_BOP_STACK_ALLOC);
    kai__push_u32(Encoder->code, size);
    kai__push_u32(Encoder->code, reg_dst);
}
KAI_API (void) kai_encode_stack_free(Kai_Bytecode_Encoder* Encoder, uint32_t size)
{
    kai__make_space(1 + sizeof(Kai_u32));
    kai__push_u8(Encoder->code, KAI_BOP_STACK_FREE);
    kai__push_u32(Encoder->code, size);
}

#undef kai__make_space

KAI_API (Kai_Result) kai_encoder_set_branch(Kai_Bytecode_Encoder* Encoder, uint32_t branch, uint32_t location)
{
    if (branch >= Encoder->code.count)
		return KAI_BC_ERROR_BRANCH;
    *(Kai_u32*)(Encoder->code.elements + branch) = location;
    return KAI_SUCCESS;
}

Kai_string kai__math_operation_to_string(enum Kai_Bytecode_Math op)
{
    switch (op)
    {
        case KAI_MATH_ADD: return KAI_STRING("+");
        case KAI_MATH_MUL: return KAI_STRING("*");
        case KAI_MATH_DIV: return KAI_STRING("/");
        case KAI_MATH_SUB: return KAI_STRING("-");
        default:           return KAI_STRING("?");
    }
}

#define BC_X_INSTRUCTIONS       \
    X(KAI_BOP_NOP             ) \
    X(KAI_BOP_LOAD_CONSTANT   ) \
    X(KAI_BOP_MATH            ) \
    X(KAI_BOP_MATH_CONSTANT   ) \
    X(KAI_BOP_COMPARE         ) \
    X(KAI_BOP_COMPARE_CONSTANT) \
    X(KAI_BOP_BRANCH          ) \
    X(KAI_BOP_JUMP            ) \
    X(KAI_BOP_CALL            ) \
    X(KAI_BOP_RETURN          ) \
    X(KAI_BOP_NATIVE_CALL     ) \
    X(KAI_BOP_LOAD            ) \
    X(KAI_BOP_STORE           ) \
    X(KAI_BOP_STACK_ALLOC     ) \
    X(KAI_BOP_STACK_FREE      ) \
    X(KAI_BOP_CHECK_ADDRESS   )

char const* bc__op_to_name[] = {
#define X(NAME) [NAME] = #NAME,
    BC_X_INSTRUCTIONS
#undef X
};

#define kai__decoder_iterate(Decoder) \
	while (Decoder.cursor < Decoder.count)

#define kai__decode_load_constant(Decoder, Type_Var, Dst_Var, Value_Var)                                          \
	Kai_u8    Type_Var  = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_Reg   Dst_Var   = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
    Kai_Value Value_Var = *(Kai_Value*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += kai__typeid_to_size(Type_Var)

#define kai__decode_math(Decoder, Type_Var, Op_Var, Dst_Var, Left_Var, Right_Var)                          \
	Kai_u8  Type_Var  = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_u8  Op_Var    = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_Reg Dst_Var   = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Reg Left_Var  = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Reg Right_Var = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg)

#define kai__decode_math_constant(Decoder, Type_Var, Op_Var, Dst_Var, Left_Var, Value_Var)                   \
	Kai_u8    Type_Var  = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_u8    Op_Var    = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_Reg   Dst_Var   = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Reg   Left_Var  = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Value Value_Var = *(Kai_Value*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += kai__typeid_to_size(Type_Var)

#define kai__decode_compare(Decoder, Type_Var, Op_Var, Dst_Var, Left_Var, Right_Var)                       \
	Kai_u8  Type_Var  = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_u8  Op_Var    = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_Reg Dst_Var   = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Reg Left_Var  = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Reg Right_Var = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg)

#define kai__decode_compare_constant(Decoder, Type_Var, Op_Var, Dst_Var, Left_Var, Value_Var)                \
	Kai_u8    Type_Var  = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_u8    Op_Var    = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);   \
	Kai_Reg   Dst_Var   = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Reg   Left_Var  = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	Kai_Value Value_Var = *(Kai_Value*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += kai__typeid_to_size(Type_Var)

#define kai__decode_branch(Decoder, Location_Var, Src_Var) \
	Kai_u32 Location_Var = *(Kai_u32*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u32); \
	Kai_Reg Src_Var = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg)

#define kai__decode_jump(Decoder, Location_Var) \
	Kai_u32 Location_Var = *(Kai_u32*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u32);

#define kai__decode_call(Decoder, Ret_Count_Var, Arg_Count_Var, Location_Var, Regs_Vars)                                         \
	Kai_u8 Ret_Count_Var = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);                      \
	Kai_u8 Arg_Count_Var = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8);                      \
	Kai_u32 Location_Var = *(Kai_u32*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u32);                    \
	kai__for_n (Ret_Count_Var) Regs_Vars[i] = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg); \
	kai__for_n (Arg_Count_Var) Regs_Vars[i] = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg)

#define kai__decode_return(Decoder, Count_Var, Regs_Var) \
	Kai_u8 Count_Var = *(Kai_u8*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_u8); \
	kai__for_n (Count_Var) Regs_Var[i] = *(Kai_Reg*)(Decoder.bytecode + Decoder.cursor); Decoder.cursor += sizeof(Kai_Reg)

Kai_Result kai_bytecode_to_c(Kai_Bytecode* bytecode, Kai_Writer* writer)
{
    //! TODO: use type info
    //! TODO: find all branch locations
    //! TODO: use procedure name and argument count

    #define bcc__indent() kai__write("    ")
    #define bcc__branch_check()                                \
		kai__for_n (bytecode->branch_count) {                  \
			if (decoder.cursor == bytecode->branch_hints[i]) { \
				kai__write("__loc_");                          \
				kai__write_u32(i);                             \
				kai__write(":\n");                             \
				break;                                         \
			}                                                  \
		}    

    kai__write("int32_t func(");
    for (int i = 0;;)
    {
        kai__write("int32_t __");
        kai__write_u32(i);
        if (++i < bytecode->arg_count)
        {
            kai__write(", ");
        }
        else break;
    }
    kai__write(") {\n");

	Kai_Bytecode_Decoder decoder = {
		.bytecode = bytecode->data + bytecode->range.offset,
		.count = bytecode->range.count,
		.cursor = 0,
	};
	Kai_Reg temp_regs[16] = {0};
	char const* cmp_tab[] = { "<", ">=", ">", "<=", "==", "!=" };

	kai__decoder_iterate (decoder)
	{
        bcc__branch_check();
        enum Kai_Bytecode_Op op = decoder.bytecode[decoder.cursor++];

        switch (op)
        {
        case KAI_BOP_LOAD_CONSTANT: {
			kai__decode_load_constant(decoder, type, dst, value);
            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = ");
            kai__write_u32(value.s32);
            kai__write(";\n");
        } break;

        case KAI_BOP_MATH: {
			kai__decode_math(decoder, type, math_op, dst, a, b);
			kai__unused(type);
            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = __");
            kai__write_u32(a);
            kai__write(" ");
            kai__write_string(kai__math_operation_to_string(math_op));
            kai__write(" __");
			kai__write_u32(b);
            kai__write(";\n");
        } break;
		
        case KAI_BOP_MATH_CONSTANT: {
			kai__decode_math_constant(decoder, type, math_op, dst, a, value);
			kai__unused(type);
            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = __");
            kai__write_u32(a);
            kai__write(" ");
            kai__write_string(kai__math_operation_to_string(math_op));
            kai__write(" ");
            kai__write_u32(value.s32);
            kai__write(";\n");
        } break;
		
        case KAI_BOP_COMPARE: {
			kai__decode_compare(decoder, type, comp, dst, a, b);
			kai__unused(type); // TODO
            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = __");
            kai__write_u32(a);
            kai__write(" ");
            kai__write_string(kai_string_from_c(cmp_tab[comp]));
            kai__write(" __");
			kai__write_u32(b);
            kai__write(";\n");
        } break;

        case KAI_BOP_COMPARE_CONSTANT: {
			kai__decode_compare_constant(decoder, type, comp, dst, a, value);
            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = __");
            kai__write_u32(a);
            kai__write(" ");
            kai__write_string(kai_string_from_c(cmp_tab[comp]));
            kai__write(" ");
            kai__write_u32(value.s32);
            kai__write(";\n");
        } break;

        case KAI_BOP_BRANCH: {
			kai__decode_branch(decoder, location, src);
            bcc__indent();
            kai__for_n (bytecode->branch_count) {
                if (location == bytecode->branch_hints[i]) {
                    kai__write("if (__");
                    kai__write_u32(src);
                    kai__write(") goto __loc_");
                    kai__write_u32(i);
                    kai__write(";\n");
                    break;
                }
            }
        } break;
		
        case KAI_BOP_JUMP: {
			kai__decode_jump(decoder, location);
            bcc__indent();
            kai__for_n (bytecode->branch_count) {
                if (location == bytecode->branch_hints[i]) {
                    kai__write("goto __loc_");
                    kai__write_u32(i);
                    kai__write(";\n");
                    break;
                }
            }
        } break;
		
        case KAI_BOP_CALL: {
			kai__decode_call(decoder, ret_count, arg_count, location, temp_regs);
            bcc__indent();
            kai__assert(ret_count <= 1);
            kai__for_n (ret_count) {
				kai__write("int32_t __");
                kai__write_u32(temp_regs[i]);
                if (i != (Kai_u32)(ret_count - 1))
				kai__write(", ");
            }
			kai__write(" = ");
			kai__unused(location);
            kai__write("func("); // TODO: use procedure name
            kai__for_n (arg_count) {
                kai__write("__");
                kai__write_u32(temp_regs[ret_count + i]);
                if (i != (Kai_u32)(arg_count - 1))
                  	kai__write(", ");
            }
            kai__write(");\n");
        } break;

		/*
        case KAI_BOP_NATIVE_CALL: {
            uintptr_t address;
            address = *(uintptr_t*)(data + cursor);
            cursor += sizeof(uintptr_t);

            Kai_u8 use_dst = 0;
            use_dst = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            //Kai_Reg dst = 0;
            if (use_dst) {
                //dst = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
            }

            Kai_u8 input_count;
            input_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            //char const* name = "???";
            kai__for_n (bytecode->native_count) {
                if (address == (uintptr_t)bytecode->natives[i].address) {
                    //name = bytecode->natives[i].name;
                    break;
                }
            }

            bcc__indent();
            if (use_dst) {
                kai__unreachable();
                //bcc__write("int32_t __%d = %s(", dst, name);
            } else {
                kai__unreachable();
                //bcc__write("%s(", name);
            }

            kai__for_n (input_count) {
                //Kai_Reg reg;
                //reg = *(Kai_Reg*)(data + cursor);
                //cursor += sizeof(Kai_Reg);

                kai__unreachable();
                //bcc__write("__%d", reg);

                if (i != (Kai_u32)(input_count - 1)) {
                    kai__unreachable();
                    //bcc__write(", ");
                }
            }
            
            kai__unreachable();
            //bcc__write(");\n");
        } break;
		*/

        case KAI_BOP_RETURN: {
			kai__decode_return(decoder, ret_count, temp_regs);
            bcc__indent();
			kai__write("return");
            kai__for_n (ret_count) {
				kai__write(" __");
                kai__write_u32(temp_regs[i]);
            }
			kai__write(";\n");
        } break;

        default: {
            //kai__todo("unfinished : %s", bc__op_to_name[op]);
            kai__write("BYTECODE ERROR\n}");
            return KAI_SUCCESS;
        } break;
        }
    }

    kai__write("}");
    return KAI_SUCCESS;
}

// TODO: remove these + rename bci__
#define bci__for(N) for (int i = 0; i < (int)N; ++i)
#define __type_to_size(T) ((T) >> 4)

int bci__next_u8(Kai_Interpreter* interp, uint8_t* out) {
    if (interp->pc + sizeof(uint8_t) > interp->count) {
        interp->flags |= KAI_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    *out = interp->bytecode[interp->pc++];
    return 0;
}
int bci__next_u32(Kai_Interpreter* interp, uint32_t* out) {
    if (interp->pc + sizeof(uint32_t) > interp->count) {
        interp->flags |= KAI_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    *out = *(uint32_t*)(interp->bytecode + interp->pc);
    interp->pc += sizeof(uint32_t);
    return 0;
}
int bci__next_address(Kai_Interpreter* interp, uintptr_t* out) {
    if (interp->pc + sizeof(uintptr_t) > interp->count) {
        interp->flags |= KAI_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    *out = *(uintptr_t*)(interp->bytecode + interp->pc);
    interp->pc += sizeof(uintptr_t);
    return 0;
}
int bci__next_value(Kai_Interpreter* interp, uint8_t type, Kai_Value* out) {
    uint8_t size = __type_to_size(type);
    if (interp->pc + size > interp->count) {
        interp->flags |= KAI_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    uint8_t* u8_out = (uint8_t*)(out);
    for (int i = 0; i < size; ++i) {
        u8_out[i] = interp->bytecode[interp->pc + i];
    }
    interp->pc += size;
    return 0;
}
int bci__next_reg(Kai_Interpreter* interp, Kai_Reg* out) {
    if (interp->pc + sizeof(Kai_Reg) > interp->count) {
        interp->flags |= KAI_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    Kai_Reg reg = *(Kai_Reg*)(interp->bytecode + interp->pc);
    // TODO: cannot do bounds check here
    if (reg >= interp->max_register_count) {
        interp->flags |= KAI_INTERP_FLAGS_INVALID;
        return 1;
    }
    *out = reg;
    interp->pc += sizeof(Kai_Reg);
    return 0;
}

#define bci__use_reg(interp, reg) \
    if (interp->frame_max_register_written < reg) \
        interp->frame_max_register_written = reg

int bci__read_register(Kai_Interpreter* interp, Kai_Reg reg, Kai_Value* value) {
    Kai_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
    if (base + reg >= interp->max_register_count) return 1;
    *value = interp->registers[base + reg];
    return 0;
}
int bci__write_register(Kai_Interpreter* interp, Kai_Reg reg, Kai_Value value) {
    Kai_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
    if (base + reg >= interp->max_register_count) return 1;
    bci__use_reg(interp, reg);
    interp->registers[base + reg] = value;
    return 0;
}

uintptr_t bci__call_native_procedure(uintptr_t address, uintptr_t* args, int arg_count) {
    uintptr_t result = 0;
#if defined(__arm64__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
    if (arg_count-- < 0) goto call; asm ( "mov x0, %0" :: "r" (args[0]) : "x0" );
    if (arg_count-- < 0) goto call; asm ( "mov x1, %0" :: "r" (args[1]) : "x1" );
    if (arg_count-- < 0) goto call; asm ( "mov x2, %0" :: "r" (args[2]) : "x2" );
    if (arg_count-- < 0) goto call; asm ( "mov x3, %0" :: "r" (args[3]) : "x3" );
    if (arg_count-- < 0) goto call; asm ( "mov x4, %0" :: "r" (args[4]) : "x4" );
    if (arg_count-- < 0) goto call; asm ( "mov x5, %0" :: "r" (args[5]) : "x5" );
    if (arg_count-- < 0) goto call; asm ( "mov x6, %0" :: "r" (args[6]) : "x6" );
    if (arg_count-- < 0) goto call; asm ( "mov x7, %0" :: "r" (args[7]) : "x7" );

    // TODO: push onto stack with >8 arguments
    if (arg_count > 0) kai__todo("Too many arguments!");

call:
    asm ( "blr %0"      :: "r" (address) : "x30" );
    asm ( "mov %0, x0"  : "=r" (result) );
#pragma GCC diagnostic pop
#else
    kai__unused(address);
    kai__unused(args);
    kai__unused(arg_count);
    kai__unreachable();
//#   error "Dynamic call not implemented for this architecture!"
#endif
    return result;
}

Kai_Value bci__compute_math(uint8_t type, Kai_u8 op, Kai_Value a, Kai_Value b) 
{
#	define BCI__MATH_OPERATION(NAME,A,B)                            \
		switch (op) {                                               \
			case KAI_MATH_ADD: return (Kai_Value) {.NAME = A + B};  \
			case KAI_MATH_SUB: return (Kai_Value) {.NAME = A - B};  \
			case KAI_MATH_MUL: return (Kai_Value) {.NAME = A * B};  \
			case KAI_MATH_DIV: return (Kai_Value) {.NAME = A / B};  \
		}
#   define X(TYPE, ITEM, NAME) case KAI_##NAME: BCI__MATH_OPERATION(ITEM, a.ITEM, b.ITEM)
	switch (type) {
        KAI_X_PRIMITIVE_TYPES
    }
#   undef X
#   undef BCI__MATH_OPERATION
    return (Kai_Value) {0};
}

uint8_t bci__compute_comparison(uint8_t type, uint8_t comp, Kai_Value a, Kai_Value b) {
#   define BCI__COMPARE(A,B)              \
    switch (comp) {                       \
        case KAI_CMP_LT: return A < B;   \
        case KAI_CMP_GE: return A >= B;  \
        case KAI_CMP_GT: return A > B;   \
        case KAI_CMP_LE: return A <= B;  \
        case KAI_CMP_EQ: return A == B;  \
        case KAI_CMP_NE: return A != B;  \
    }
    switch (type) {
#       define X(TYPE, ITEM, NAME) case KAI_##NAME: BCI__COMPARE(a.ITEM, b.ITEM)
        KAI_X_PRIMITIVE_TYPES
#       undef X
    }
    return 0;
#   undef BCI__COMPARE
}

KAI_API (void) kai_interp_create(Kai_Interpreter_Create_Info* Info, Kai_Interpreter* out_Interp)
{
    Kai_u32 required_size =
	       Info->max_register_count     * sizeof(Kai_Value)
        +  Info->max_call_stack_count   * sizeof(Kai_BC_Procedure_Frame)
        +  Info->max_return_value_count * sizeof(Kai_Reg)
        +  256                          * sizeof(Kai_Reg);
	
	Kai_Allocator* allocator = Info->allocator;
	void* memory = kai__allocate(NULL, required_size, 0);

    uint8_t* base = memory;
    out_Interp->registers = (Kai_Value*)base;
    base += Info->max_register_count * sizeof(Kai_Value);
    out_Interp->call_stack = (Kai_BC_Procedure_Frame*)base;
    base += Info->max_call_stack_count * sizeof(Kai_BC_Procedure_Frame);
    out_Interp->return_registers = (Kai_Reg*)base;
    base += Info->max_return_value_count * sizeof(Kai_Reg);
    out_Interp->scratch_buffer = (uint32_t*)base;

    out_Interp->bytecode = 0;
    out_Interp->count = 0;
    out_Interp->flags = 0;
    out_Interp->pc = 0;
    out_Interp->call_stack_count = 0;
    out_Interp->return_registers_count = 0;
    out_Interp->max_register_count = Info->max_register_count;
    out_Interp->max_call_stack_count = Info->max_call_stack_count;
    out_Interp->max_return_value_count = Info->max_return_value_count;
	out_Interp->allocator = allocator;
}

void kai_interp_destroy(Kai_Interpreter* Interp)
{
	if (Interp->registers == NULL)
		return;

	Kai_Allocator* allocator = Interp->allocator;
    Kai_u32 size =
	       Interp->max_register_count     * sizeof(Kai_Value)
        +  Interp->max_call_stack_count   * sizeof(Kai_BC_Procedure_Frame)
        +  Interp->max_return_value_count * sizeof(Kai_Reg)
        +  256                            * sizeof(Kai_Reg);
	kai__free(Interp->registers, size);
}

void kai_interp_load_from_encoder(Kai_Interpreter* Interp, Kai_Bytecode_Encoder* Encoder)
{
    Interp->bytecode = Encoder->code.elements;
    Interp->count    = Encoder->code.count;
}

void kai_interp_load_from_memory(Kai_Interpreter* Interp, void* Code, uint32_t Size)
{	
    Interp->bytecode = Code;
    Interp->count    = Size;
}

void kai_interp_reset(Kai_Interpreter* interp, uint32_t location)
{
    interp->pc = location;
    interp->flags = 0;
    interp->return_registers_count = 0;
    interp->frame_max_register_written = 0;
    interp->stack_size = 0;

    // setup stack for execution
    interp->call_stack_count = 1;
    interp->call_stack[0].base_register = 0;
    interp->call_stack[0].return_address = 0; // final return will end exection, not used.
}

void kai_interp_set_input(Kai_Interpreter* interp, uint32_t index, Kai_Value value)
{
    interp->registers[index] = value;
}

void kai_interp_push_output(Kai_Interpreter* interp, Kai_Reg reg)
{
    interp->return_registers[interp->return_registers_count++] = reg;
}

// return 0 => done
int kai_interp_step(Kai_Interpreter* interp)
{
    if (interp->flags)
    {
        return 1;
    }

    if (interp->pc >= interp->count)
    {
        interp->flags |= KAI_INTERP_FLAGS_OVERFLOW;
        return 0;
    }

    uint8_t operation = interp->bytecode[interp->pc++];
	
    switch (operation)
    {
		
    case KAI_BOP_LOAD_CONSTANT: {
        Kai_Reg dst;
        Kai_u8 type;
        Kai_Value value = {0};

        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_reg(interp, &dst)) return 0;
        if (bci__next_value(interp, type, &value)) return 0;
        if (bci__write_register(interp, dst, value)) return 0;
    } break;

    case KAI_BOP_MATH: {
        Kai_Reg dst;
        Kai_u8 type, math_op;

        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_u8(interp, &math_op)) return 0;
        if (bci__next_reg(interp, &dst)) return 0;

        Kai_Value va, vb;
        {
            Kai_Reg a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }
		{
            Kai_Reg b;
            if (bci__next_reg(interp, &b)) return 0;
            if (bci__read_register(interp, b, &vb)) return 0;
        }

        Kai_Value result = bci__compute_math(type, math_op, va, vb);
        if (bci__write_register(interp, dst, result)) return 0;
    } break;
	
    case KAI_BOP_MATH_CONSTANT: {
        Kai_Reg dst;
        Kai_u8 type, math_op;

        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_u8(interp, &math_op)) return 0;
        if (bci__next_reg(interp, &dst)) return 0;

        Kai_Value va, vb;
        {
            Kai_Reg a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }
		if (bci__next_value(interp, type, &vb)) return 0;

        Kai_Value result = bci__compute_math(type, math_op, va, vb);
        if (bci__write_register(interp, dst, result)) return 0;
    } break;

    case KAI_BOP_COMPARE: {
        Kai_Reg dst;
        Kai_u8 type, comp;
        
        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_u8(interp, &comp)) return 0;
        if (bci__next_reg(interp, &dst)) return 0;
        
        Kai_Value va, vb;
        {
            Kai_Reg a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }
        {
            Kai_Reg b;
            if (bci__next_reg(interp, &b)) return 0;
            if (bci__read_register(interp, b, &vb)) return 0;
        }

        Kai_Value result = {.u8 = bci__compute_comparison(type, comp, va, vb)};
        if (bci__write_register(interp, dst, result)) return 0;
    } break;
	
    case KAI_BOP_COMPARE_CONSTANT: {
        Kai_Reg dst;
        Kai_u8 type, comp;
        
        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_u8(interp, &comp)) return 0;
        if (bci__next_reg(interp, &dst)) return 0;
        
        Kai_Value va, vb;
        {
            uint32_t a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }
    	if (bci__next_value(interp, type, &vb)) return 0;

        Kai_Value result = {.u8 = bci__compute_comparison(type, comp, va, vb)};
        if (bci__write_register(interp, dst, result)) return 0;
    } break;

    case KAI_BOP_BRANCH: {
        uint32_t location;
        Kai_Reg src;

        if (bci__next_u32(interp, &location)) return 0;
        if (bci__next_reg(interp, &src)) return 0;

        Kai_Value value;
        if (bci__read_register(interp, src, &value)) return 0;
        if (value.u8 != 0) interp->pc = location;
    } break;

    case KAI_BOP_JUMP: {
        if (bci__next_u32(interp, &interp->pc)) return 0;
    } break;

    case KAI_BOP_CALL: {
        uint32_t location;
        uint8_t ret_count, arg_count;

        if (bci__next_u8(interp, &ret_count)) return 0;
        if (bci__next_u8(interp, &arg_count)) return 0;
        if (bci__next_u32(interp, &location)) return 0;

        Kai_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
        
        bci__for (ret_count) {
            Kai_Reg reg;
            bci__next_reg(interp, &reg);
            // TODO: bounds check
            interp->return_registers[interp->return_registers_count + (ret_count - 1) - i] = reg;
            bci__use_reg(interp, reg);
        }
        interp->return_registers_count += ret_count;

        Kai_Reg proc_base = base + interp->frame_max_register_written + 1;

        bci__for (arg_count) {
            Kai_Reg reg;
            bci__next_reg(interp, &reg);
            interp->registers[proc_base + i] = interp->registers[base + reg];
        }

        interp->call_stack[interp->call_stack_count++] = (Kai_BC_Procedure_Frame) {
            .return_address = interp->pc,
            .base_register  = proc_base,
        };
        interp->frame_max_register_written = 0;
        interp->pc = location;
    } break;

    case KAI_BOP_RETURN: {
        uint8_t ret_count;
        if (bci__next_u8(interp, &ret_count)) return 0;

        Kai_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
        Kai_Reg ret_base = (interp->call_stack_count <= 1) ? 0
                        : interp->call_stack[interp->call_stack_count - 2].base_register;

        bci__for (ret_count) {
            Kai_Reg src;
            if (bci__next_reg(interp, &src)) return 0;
            // TODO: bounds check
            Kai_Reg dst = interp->return_registers[interp->return_registers_count - 1];
            interp->registers[ret_base + dst] = interp->registers[base + src];
            interp->return_registers_count -= 1;
        }

        interp->call_stack_count -= 1;

        if (interp->call_stack_count == 0) {
            interp->flags |= KAI_INTERP_FLAGS_DONE;
            return 0;
        }

        interp->pc = interp->call_stack[interp->call_stack_count].return_address;
    } break;

    case KAI_BOP_NATIVE_CALL: {
        uintptr_t address;
        uint8_t use_dst;
        Kai_Reg dst = 0;
        uint8_t input_count;

        if (bci__next_u8(interp, &use_dst)) return 0;
        if (bci__next_u8(interp, &input_count)) return 0;
        if (bci__next_address(interp, &address)) return 0;
        if (use_dst && bci__next_reg(interp, &dst)) return 0;

        Kai_Value* args = interp->scratch_buffer;
        bci__for (input_count) {
            Kai_Reg reg;
            if (bci__next_reg(interp, &reg)) return 0;
            if (bci__read_register(interp, reg, args + i)) return 0;
        }

        union {
            uintptr_t in;
            Kai_Value value;
        } result = { .in = bci__call_native_procedure(address, (uintptr_t*)args, input_count) };

        if (use_dst) {
            if (bci__write_register(interp, dst, result.value)) return 0;
        }
    } break;

    case KAI_BOP_LOAD: {
        kai__unreachable();
    } break;

    case KAI_BOP_STORE: {
        kai__unreachable();
    } break;

    case KAI_BOP_STACK_ALLOC: {
        kai__unreachable();
    } break;

    case KAI_BOP_STACK_FREE: {
        kai__unreachable();
    } break;
	 
    default: {
		kai__todo("interpreter : %s", bc__op_to_name[operation]);
        interp->flags |= KAI_INTERP_FLAGS_INCOMPLETE;
        return 0;
    } break;
    }

	return 1;
}

#endif
#ifndef KAI__SECTION_IMPLEMENTATION_COMPILATION

typedef enum {
	KAI__NODE_START       = 0, // same as KAI__NODE_MULTI
	KAI__NODE_RETURN      = 1,
	KAI__NODE_CONSTANT    = 2,
    KAI__NODE_ADD         = 3,
    KAI__NODE_SUB         = 4,
    KAI__NODE_MUL         = 5,
    KAI__NODE_DIV         = 6,
    KAI__NODE_NEGATE      = 7,
    KAI__NODE_SCOPE       = 8,
    KAI__NODE_MULTI       = 9,
    KAI__NODE_PROJECTION  = 10,
    KAI__NODE_COMPARE     = 11,
    KAI__NODE_NOT         = 12,
} Kai__Node_Kind;

typedef enum {
    KAI__NODE_TYPE_BOTTOM      = 0, // Bottom (ALL)
    KAI__NODE_TYPE_TOP         = 1, // Top    (ANY)
    KAI__NODE_TYPE_CONTROL     = 2,
    KAI__NODE_TYPE_SIMPLE      = 3, // End of the Simple Types
    KAI__NODE_TYPE_NUMBER      = 4,
    KAI__NODE_TYPE_PRIMITIVE   = 5, // bool, u8, u16, ...
    KAI__NODE_TYPE_TUPLE       = 6,
} Kai__Node_Type_ID;

typedef enum {
    KAI__NODE_TYPE_IS_CONSTANT = 0x01,
    KAI__NODE_TYPE_IS_BOTTOM   = 0x02,
} Kai__Node_Type_Flags;

typedef struct {
    Kai__Node_Type_ID    id;
    Kai__Node_Type_Flags flags;
} Kai__Node_Type;

#define KAI__TYPE_BOTTOM  (Kai__Node_Type) {.id = KAI__NODE_TYPE_BOTTOM,  .flags = 0}
#define KAI__TYPE_TOP     (Kai__Node_Type) {.id = KAI__NODE_TYPE_TOP,     .flags = 1}
#define KAI__TYPE_CONTROL (Kai__Node_Type) {.id = KAI__NODE_TYPE_CONTROL, .flags = 0}

#define kai__is_simple(TYPE)   ((TYPE).id < KAI__NODE_TYPE_SIMPLE)
#define kai__is_constant(TYPE) ((TYPE).flags & KAI__NODE_TYPE_IS_CONSTANT)

typedef struct Kai__Node Kai__Node;
typedef KAI__HASH_TABLE(Kai_u32) Kai__Symbol_Table;
typedef KAI__ARRAY(Kai__Symbol_Table) Kai__Scope_Stack;

struct Kai__Node {
	Kai_u32                uid;
    Kai__Node_Kind         kind;
    Kai__Node_Type         type;
	KAI__ARRAY(Kai__Node*) inputs;
	KAI__ARRAY(Kai__Node*) outputs;
	Kai_string             source;
    union Kai__Node_Values {
        Kai_Number       number; // constant node / type = Number
        Kai__Scope_Stack scopes; // scope node
        Kai_u32          index; // projection node
        KAI__ARRAY(Kai__Node_Type) types; // multi-node
    } value;
};

// Node graph is also just referred to as just "ir"
typedef struct {
	Kai__Node* start_node;
    Kai__Node* scope;
    Kai__Node* _return_node;
} Kai__Node_Graph;

typedef struct {
    Kai_Allocator          allocator;
    Kai_Syntax_Tree      * tree;
    Kai__Arena_Allocator   node_arena;
    Kai_u32                next_uid;
    Kai__Node_Graph        ir;
    Kai_Writer             debug_writer; // Delete
} Kai__Compiler_Context;

#define KAI_INTERNAL    static inline
#define debug_print(X) printf(#X " = %i\n", (int)(X))

KAI_INTERNAL void kai__write_ir_node(Kai_Writer* writer, Kai__Node* node)
{
	if (node == NULL)
		return;

    // Write node
    kai__write("\t");
    kai__write_u32(node->uid);
    kai__write("[label=\"");
    switch (node->kind) {
        case KAI__NODE_START:    { kai__write("START"); } break;
        case KAI__NODE_RETURN:   { kai__write("RETURN"); } break;
        case KAI__NODE_CONSTANT: { kai_write_number(writer, node->value.number); } break;
        case KAI__NODE_ADD:      { kai__write("+"); } break;
        case KAI__NODE_SUB:      { kai__write("-"); } break;
        case KAI__NODE_MUL:      { kai__write("*"); } break;
        case KAI__NODE_DIV:      { kai__write("/"); } break;
        case KAI__NODE_NEGATE:   { kai__write("-"); } break;
        case KAI__NODE_SCOPE:    { kai__write("SCOPE"); } break;
        case KAI__NODE_MULTI:    { kai__write("multi"); } break;
        case KAI__NODE_PROJECTION: { kai__write_string(node->source); } break;
        case KAI__NODE_COMPARE:  { kai__write("comp"); } break;
        case KAI__NODE_NOT:      { kai__write("!"); } break;
    }
    kai__write("\"");
    if (node->kind == KAI__NODE_CONSTANT)
    {
        kai__write(" style=filled fillcolor=lightblue");
    }
    if (node->kind == KAI__NODE_START || node->kind == KAI__NODE_RETURN)
    {
        kai__write(" style=filled fillcolor=yellow shape=box");
    }
    kai__write("];\n");

    kai__array_iterate (node->inputs, i)
    {
        kai__write_ir_node(writer, node->inputs.elements[i]);
    }

    kai__array_iterate (node->inputs, i)
    {
        kai__write_u32(node->uid);
        kai__write(" -> ");
        kai__write_u32(node->inputs.elements[i]->uid);
        kai__write(";\n");
    }
}
KAI_INTERNAL void kai__write_ir(Kai_Writer* writer, Kai__Node_Graph* graph)
{
	kai__assert(writer != NULL);
	kai__assert(graph != NULL);

    kai__write_ir_node(writer, graph->_return_node);
}

#define kai__node_add_use(Node, Used) kai__array_append(&(Node)->outputs, Used)
#define kai__node_add_input(Node, Input) do { kai__array_append(&(Node)->inputs, Input); kai__node_add_use(Input, Node); } while (0)
#define kai__node_is_unused(Node) ((Node)->outputs.count == 0)

KAI_INTERNAL Kai__Node* kai__new_node(Kai__Compiler_Context* Context, Kai__Node Value)
{
    Kai__Node* node = kai__arena_allocate(&Context->node_arena, sizeof(Kai__Node));
    *node = Value;
    node->uid = ++Context->next_uid;
    return node;
}
KAI_INTERNAL Kai_bool kai__node_delete_use(Kai__Node* Node, Kai__Node* Use)
{
    kai__array_iterate (Node->outputs, i)
    {
        if (Node->outputs.elements[i] == Use)
        {
            Node->outputs.elements[i] = Node->outputs.elements[Node->outputs.count - 1];
            Node->outputs.count -= 1;
        }
    }
    return KAI_BOOL(Node->outputs.count == 0);
}
KAI_INTERNAL void kai__node_free(Kai__Compiler_Context* Context, Kai__Node* Node)
{
    Kai_Allocator* allocator = &Context->allocator;
    kai__array_destroy(&Node->inputs);
    kai__array_destroy(&Node->outputs);
}
KAI_INTERNAL void kai__node_kill(Kai__Compiler_Context* Context, Kai__Node* Node)
{
    kai__assert(kai__node_is_unused(Node));

    // Set all inputs to null, recursively killing unused Nodes
    kai__array_iterate (Node->inputs, i)
    {
        Kai__Node* old_def = Node->inputs.elements[i];
            
        if (old_def != NULL &&  // If the old def exists, remove a def->use edge
            kai__node_delete_use(old_def, Node)) // If we removed the last use, the old def is now dead
            kai__node_kill(Context, old_def);
    }
    Node->inputs.count = 0;
}
KAI_INTERNAL Kai__Node* kai__node_add_def(Kai__Compiler_Context* Context, Kai__Node* Node, Kai__Node* Def)
{
    Kai_Allocator* allocator = &Context->allocator;
    kai__array_append(&Node->inputs, Def);
    if (Def != NULL)
        kai__node_add_use(Def, Node);
    return Def;
}
KAI_INTERNAL Kai__Node* kai__node_set_def(Kai__Compiler_Context* Context, Kai__Node* Node, Kai_u32 Index, Kai__Node* Def)
{
    Kai_Allocator* allocator = &Context->allocator;
    Kai__Node* old_def = Node->inputs.elements[Index];

    if (old_def == Def)
        return Node; // No change
        
    if (Def != NULL)
        kai__node_add_use(Def, Node);

    if (old_def != NULL &&  // If the old def exists, remove a def->use edge
        kai__node_delete_use(old_def, Node)) // If we removed the last use, the old def is now dead
        kai__node_kill(Context, old_def);

    // Set the new_def over the old (killed) edge
    Node->inputs.elements[Index] = Def;
    return Def;
}

#define kai__scope_lookup(Name) \
  kai__scope_update_or_lookup(Context, Name, NULL, Context->ir.scope->value.scopes.count - 1)

#define kai__scope_update(Name, New_Node) \
  kai__scope_update_or_lookup(Context, Name, New_Node, Context->ir.scope->value.scopes.count - 1)

KAI_INTERNAL void kai__compute_type(Kai__Compiler_Context* Context, Kai__Node* node)
{
    kai__unused(Context);
    switch (node->kind) {
    default: {
        debug_print(node->kind);
        KAI__FATAL_ERROR("compute_type", "invalid node type");
    } break;
    case KAI__NODE_CONSTANT: {
        // already have type set for constant nodes
    } break;
    case KAI__NODE_RETURN: {
        node->type = KAI__TYPE_BOTTOM;
    } break;
    case KAI__NODE_PROJECTION: {
        Kai__Node* ctrl = node->inputs.elements[0];
        if (ctrl->type.id == KAI__NODE_TYPE_TUPLE)
        {
            node->type = ctrl->value.types.elements[node->value.index];
            break;
        }
        node->type = KAI__TYPE_BOTTOM;
    } break;
    case KAI__NODE_NEGATE: {
        Kai__Node* input = node->inputs.elements[0];
        if (input->type.id == KAI__NODE_TYPE_NUMBER)
        {
            if (kai__is_constant(input->type))
                node->value.number = kai_number_neg(input->value.number);
            node->type = input->type;
            break;
        }
        node->type = KAI__TYPE_BOTTOM;
    } break;
    case KAI__NODE_ADD: {
        Kai__Node* left  = node->inputs.elements[0];
        Kai__Node* right = node->inputs.elements[1];
        if (left->type.id == KAI__NODE_TYPE_NUMBER && right->type.id == KAI__NODE_TYPE_NUMBER)
        {
            node->type = left->type;
            if (kai__is_constant(left->type) && kai__is_constant(right->type))
            {
                node->value.number = kai_number_add(left->value.number, right->value.number);
                break;
            }
        }
        node->type = KAI__TYPE_BOTTOM;
    } break;
    case KAI__NODE_SUB: {
        Kai__Node* left  = node->inputs.elements[0];
        Kai__Node* right = node->inputs.elements[1];
        if (left->type.id == KAI__NODE_TYPE_NUMBER && right->type.id == KAI__NODE_TYPE_NUMBER)
        {
            node->type = left->type;
            if (kai__is_constant(left->type) && kai__is_constant(right->type))
            {
                node->value.number = kai_number_sub(left->value.number, right->value.number);
                break;
            }
        }
        node->type = KAI__TYPE_BOTTOM;
    } break;
    case KAI__NODE_MUL: {
        Kai__Node* left  = node->inputs.elements[0];
        Kai__Node* right = node->inputs.elements[1];
        if (left->type.id == KAI__NODE_TYPE_NUMBER && right->type.id == KAI__NODE_TYPE_NUMBER)
        {
            node->type = left->type;
            if (kai__is_constant(left->type) && kai__is_constant(right->type))
            {
                node->value.number = kai_number_mul(left->value.number, right->value.number);
                break;
            }
        }
        node->type = KAI__TYPE_BOTTOM;
    } break;
    case KAI__NODE_DIV: {
        Kai__Node* left  = node->inputs.elements[0];
        Kai__Node* right = node->inputs.elements[1];
        if (left->type.id == KAI__NODE_TYPE_NUMBER && right->type.id == KAI__NODE_TYPE_NUMBER)
        {
            node->type = left->type;
            if (kai__is_constant(left->type) && kai__is_constant(right->type))
            {
                node->value.number = kai_number_div(left->value.number, right->value.number);
                break;
            }
        }
        node->type = KAI__TYPE_BOTTOM;
    } break;
    }
}
KAI_INTERNAL Kai__Node* kai__idealize(Kai__Compiler_Context* Context, Kai__Node* node)
{
    kai__unused(Context);
    return node;
}
KAI_INTERNAL Kai__Node* kai__peephole(Kai__Compiler_Context* Context, Kai__Node* node)
{
    Kai_Allocator* allocator = &Context->allocator;
    kai__compute_type(Context, node);

    if (KAI__DISABLE_PEEPHOLES)
        return node;

    if (node->kind != KAI__NODE_CONSTANT && kai__is_constant(node->type))
    {
        kai__node_kill(Context, node);
        node->kind = KAI__NODE_CONSTANT;
        kai__array_append(&node->inputs, Context->ir.start_node);
        return kai__peephole(Context, node);
    }

    return kai__idealize(Context, node);
}
KAI_INTERNAL void kai__scope_define(Kai__Compiler_Context* Context, Kai_string Name, Kai__Node* Node)
{
    Kai_Allocator* allocator = &Context->allocator;
    Kai__Scope_Stack* scopes = &Context->ir.scope->value.scopes;
    Kai__Symbol_Table* table = &scopes->elements[scopes->count - 1];
    Kai_u32 index = 0;
    Kai_bool inserted = kai__hash_table_emplace_key(table, Name, &index);
    kai__assert(inserted == KAI_TRUE);
    table->values[index] = Context->ir.scope->inputs.count;
    kai__node_add_def(Context, Context->ir.scope, Node);
}
KAI_INTERNAL Kai__Node* kai__scope_update_or_lookup(Kai__Compiler_Context* Context, Kai_string Name, Kai__Node* Node, int depth)
{
entry:
    if (depth < 0)
        return NULL;

    Kai__Scope_Stack* scopes = &Context->ir.scope->value.scopes;
    Kai__Symbol_Table* symbols = &scopes->elements[depth];
    Kai_u32* index = kai__hash_table_find(symbols, Name);

    if (index == NULL)
    { // recursively look in parent scope
        depth -= 1;
        goto entry;
    }

    Kai__Node* old = Context->ir.scope->inputs.elements[*index];

    if (Node == NULL)
        return old;

    return kai__node_set_def(Context, Context->ir.scope, *index, Node);
}

KAI_INTERNAL Kai__Node* kai__generate_ir(Kai__Compiler_Context* Context, Kai_Expr Expr)
{
    Kai_Allocator* allocator = &Context->allocator;

    void* void_Expr = Expr;
    switch (Expr->id) {
    default: {
        kai__todo("Implement kai__generate_ir for expr.id = %u", Expr->id);
    }
    break; case KAI_STMT_COMPOUND:
    {
        Kai_Stmt_Compound* node = void_Expr;

        Kai__Scope_Stack* stack = &Context->ir.scope->value.scopes;
        kai__array_append(stack, (Kai__Symbol_Table){0}); // Push

        Kai_Stmt current = node->head;
        while (current) {
            kai__generate_ir(Context, current);
            current = current->next;
        }

        stack->count -= 1; // Pop
    }
    break; case KAI_EXPR_PROCEDURE:
    {
        Kai_Expr_Procedure* node = void_Expr;
        kai__generate_ir(Context, node->body);
    }
    break; case KAI_EXPR_IDENTIFIER:
    {
        Kai_Expr_Identifier* node = void_Expr;
        Kai_string name = node->source_code;
        Kai__Node* data_node = kai__scope_lookup(name);
        kai__assert(data_node != NULL);
        return data_node;
    }
    break; case KAI_EXPR_UNARY:
    {
        Kai_Expr_Unary* node = void_Expr;
        Kai__Node* new_node = kai__new_node(Context, (Kai__Node) { .kind = KAI__NODE_NEGATE, .source = node->source_code });
        Kai__Node* data_node = kai__generate_ir(Context, node->expr);
        kai__node_add_input(new_node, data_node);
        return kai__peephole(Context, new_node);
    }
    break; case KAI_EXPR_BINARY:
    {
        Kai_Expr_Binary* node = void_Expr;
        Kai__Node* new_node = kai__new_node(Context, (Kai__Node) { .source = node->source_code });
        switch (node->op) {
        case '+': new_node->kind = KAI__NODE_ADD; break;
        case '-': new_node->kind = KAI__NODE_SUB; break;
        case '*': new_node->kind = KAI__NODE_MUL; break;
        case '/': new_node->kind = KAI__NODE_DIV; break;
        }
        Kai__Node* left_node = kai__generate_ir(Context, node->left);
        Kai__Node* right_node = kai__generate_ir(Context, node->right);
        kai__node_add_input(new_node, left_node);
        kai__node_add_input(new_node, right_node);
        return kai__peephole(Context, new_node);
    }
    break; case KAI_STMT_DECLARATION:
    {
        Kai_Stmt_Declaration* node = void_Expr;
        Kai__Node* data_node = kai__generate_ir(Context, node->expr);
        kai__scope_define(Context, node->name, data_node);
        return data_node;
    }
    break; case KAI_STMT_ASSIGNMENT:
    {
        Kai_Stmt_Assignment* node = void_Expr;
        kai__assert(node->left->id == KAI_EXPR_IDENTIFIER);
        Kai_string name = node->left->source_code;
        Kai__Node* data_node = kai__generate_ir(Context, node->expr);
        kai__scope_update(name, data_node);
        return data_node;
    }
    break; case KAI_EXPR_NUMBER:
    {
        Kai_Expr_Number* node = void_Expr;
        Kai__Node* new_node = kai__new_node(Context, (Kai__Node) { .kind = KAI__NODE_CONSTANT, .source = node->source_code });
        kai__node_add_input(new_node, Context->ir.start_node);
        new_node->type = (Kai__Node_Type) {.id = KAI__NODE_TYPE_NUMBER, .flags = KAI__NODE_TYPE_IS_CONSTANT};
        new_node->value.number = node->value;
        return kai__peephole(Context, new_node);
    }
    break; case KAI_STMT_RETURN:
    {
        Kai_Stmt_Return* node = void_Expr;
        Kai__Node* data_node = kai__generate_ir(Context, node->expr);
        Kai__Node* new_node = kai__new_node(Context, (Kai__Node) { .kind = KAI__NODE_RETURN, .source = node->source_code });
        kai__node_add_input(new_node, Context->ir.start_node->outputs.elements[0]);
        kai__node_add_input(new_node, data_node);
        Context->ir._return_node = new_node;
        return kai__peephole(Context, new_node);
    }
    }

    return NULL;
}

KAI_API(Kai_Result) kai_create_program(Kai_Program_Create_Info* Info, Kai_Program* out_Program)
{
	kai__unused(out_Program);

    Kai__Compiler_Context context = { 0 };
    kai__arena_allocator_create(&context.node_arena, &Info->allocator);

	Kai_Syntax_Tree_Create_Info info = {
		.allocator = Info->allocator,
		.error = Info->error,
		.source_code = Info->source.code,
	};
	Kai_Syntax_Tree tree;
	kai_create_syntax_tree(&info, &tree);

    context.allocator = Info->allocator;
    context.tree = &tree;

    {
        Kai_Allocator* allocator = &context.allocator;
        Kai_Stmt_Compound* compound = &tree.root;
        kai__assert(compound->head->id == KAI_STMT_DECLARATION);
        Kai_Stmt_Declaration* decl = (void*)compound->head;
        kai__assert(decl->expr != NULL);
        
        context.ir.start_node = kai__new_node(&context, (Kai__Node) {
            .kind = KAI__NODE_START,
            .type = {
                .id = KAI__NODE_TYPE_TUPLE,
            },
        });
        kai__array_append(&context.ir.start_node->value.types, KAI__TYPE_CONTROL);
        kai__array_append(&context.ir.start_node->value.types, KAI__TYPE_BOTTOM);

        context.ir.scope = kai__new_node(&context, (Kai__Node){.kind = KAI__NODE_SCOPE});
        kai__array_append(&context.ir.scope->value.scopes, (Kai__Symbol_Table){0});

        Kai__Node* ctrl_proj = kai__new_node(&context, (Kai__Node){.kind = KAI__NODE_PROJECTION});
        ctrl_proj->source = KAI_STRING("$ctrl");
        ctrl_proj->value.index = 0;
        kai__node_add_input(ctrl_proj, context.ir.start_node);

        Kai__Node* arg_proj = kai__new_node(&context, (Kai__Node){.kind = KAI__NODE_PROJECTION});
        arg_proj->source = KAI_STRING("arg");
        arg_proj->value.index = 1;
        kai__node_add_input(arg_proj, context.ir.start_node);

        kai__scope_define(&context, KAI_STRING("$ctrl"), kai__peephole(&context, ctrl_proj));
        kai__scope_define(&context, KAI_STRING("arg"), kai__peephole(&context, arg_proj));

        kai__generate_ir(&context, decl->expr);
    }

    kai_writer_file_open(&context.debug_writer, "ir.dot");
    {
        Kai_Writer* writer = &context.debug_writer;
        kai__write("digraph {\n");
        kai__write_ir(writer, &context.ir);
        kai__write("}\n");
    }
    kai_writer_file_close(&context.debug_writer);
	
    return kai__error_internal(Info->error, KAI_STRING("reached end :)"));
}

Kai_Result kai__error_redefinition(
    Kai_Error*     out_Error,
    Kai_Allocator* allocator,
    Kai_string     string,
    Kai_u32        line_number,
    Kai_string     original_name,
    Kai_u32        original_line_number)
{
    *out_Error = (Kai_Error) {
        .result = KAI_ERROR_SEMANTIC,
        .location = {
            .line = line_number,
            .string = string,
        },
    };

    Kai__Dynamic_Buffer buffer = {0};

    // First error message	
    {
        kai__dynamic_buffer_append_string(&buffer, KAI_STRING("indentifier \""));
        kai__dynamic_buffer_append_string_max(&buffer, string, 24);
        kai__dynamic_buffer_append_string(&buffer, KAI_STRING("\" is already declared"));
        Kai_range range = kai__dynamic_buffer_next(&buffer);
        out_Error->memory = kai__dynamic_buffer_release(&buffer);
        out_Error->message = kai__range_to_string(range, out_Error->memory);
    }

    // Extra info
    {
        // Append to memory
        Kai_range info_range = kai__dynamic_buffer_push(&buffer, sizeof(Kai_Error));
        kai__dynamic_buffer_append_string(&buffer, KAI_STRING("see original definition of \""));
        kai__dynamic_buffer_append_string_max(&buffer, string, 24);
        kai__dynamic_buffer_append_string(&buffer, KAI_STRING("\""));
        Kai_range message_range = kai__dynamic_buffer_next(&buffer);
        Kai_Memory memory = kai__dynamic_buffer_release(&buffer);

        // Put everything together
        Kai_Error* info = kai__range_to_data(info_range, memory);
        *info = (Kai_Error) {
            .result = KAI_ERROR_INFO,
            .location = {
                .line = original_line_number,
                .string = original_name,
            },
            .message = kai__range_to_string(message_range, memory),
            .memory = memory,
        };
        out_Error->next = info;
    }

    return KAI_ERROR_SEMANTIC;
}

Kai_Result kai__error_not_declared(
    Kai_Error*      out_Error,
    Kai_Allocator*  allocator,
    Kai_string      string,
    Kai_u32         line_number)
{
    *out_Error = (Kai_Error) {
        .result = KAI_ERROR_SEMANTIC,
        .location = {
            .string = string,
            .line = line_number,
        },
    };
    Kai__Dynamic_Buffer buffer = {0};
    kai__dynamic_buffer_append_string(&buffer, KAI_STRING("indentifier \""));
    kai__dynamic_buffer_append_string_max(&buffer, string, 24);
    kai__dynamic_buffer_append_string(&buffer, KAI_STRING("\" not declared"));
    Kai_range range = kai__dynamic_buffer_next(&buffer);
    out_Error->memory = kai__dynamic_buffer_release(&buffer);
    out_Error->message = kai__range_to_string(range, out_Error->memory);
    return KAI_ERROR_SEMANTIC;
}

KAI_API (void) kai_destroy_program(Kai_Program Program)
{
    Kai_Allocator* allocator = &Program.allocator;
    kai__hash_table_destroy(Program.procedure_table);
    if (Program.code_size != 0)
        Program.allocator.jit(
            Program.allocator.user,
            Program.platform_machine_code,
            Program.code_size,
			KAI_MEMORY_FREE
        );
}

KAI_API (void*) kai_find_procedure(Kai_Program Program, Kai_string Name, Kai_Type Type)
{
    kai__unused(Type);
    Kai_uint* element_ptr = kai__hash_table_find(&Program.procedure_table, Name);
    if (element_ptr == NULL)
        return NULL;
    return (void*)*element_ptr;
}

#endif
#ifndef KAI_DONT_USE_MEMORY_API // KAI__SECTION_IMPLEMENTATION_MEMORY_API
#define KAI__REALLOC realloc
#define KAI__FREE    free

typedef struct {
    Kai_s64 total_allocated;
    //Kai_Memory_Tracker tracker;
} Kai__Memory_Internal;

#if defined(KAI__PLATFORM_LINUX) || defined(KAI__PLATFORM_APPLE)
#include <sys/mman.h> // -> mmap
#include <unistd.h>   // -> getpagesize

static void* kai__memory_jit(Kai_ptr user, void* ptr, Kai_u32 size, Kai_u32 op)
{
	Kai__Memory_Internal* internal = user;
	switch (op) {
	case KAI_MEMORY_ALLOCATE_WRITE_ONLY: {
        void* new_ptr = mmap(NULL, size, PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
        if (new_ptr == MAP_FAILED)
            return NULL;
        internal->total_allocated += size;
        return new_ptr;
	}
	case KAI_MEMORY_SET_EXECUTABLE: {
        kai__assert(mprotect(ptr, size, PROT_EXEC) == 0);
		return NULL;
	}
	case KAI_MEMORY_FREE: {
        kai__assert(munmap(ptr, size) == 0);
        internal->total_allocated -= size;
	}
	}
    return NULL;
}

#define kai__page_size() (Kai_u32)sysconf(_SC_PAGESIZE)

#elif defined(KAI__PLATFORM_WINDOWS)
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef uintptr_t           SIZE_T;
typedef struct _SYSTEM_INFO SYSTEM_INFO;
__declspec(dllimport) void* __stdcall VirtualAlloc(void* lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
__declspec(dllimport) BOOL __stdcall VirtualProtect(void* lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD* lpflOldProtect);
__declspec(dllimport) BOOL __stdcall VirtualFree(void* lpAddress, SIZE_T dwSize, DWORD dwFreeType);
__declspec(dllimport) void __stdcall GetSystemInfo(SYSTEM_INFO* lpSystemInfo);

static void* kai__memory_jit(Kai_ptr user, void* ptr, Kai_u32 size, Kai_u32 op)
{
	Kai__Memory_Internal* internal = user;
	switch (op) {
	case KAI_MEMORY_ALLOCATE_WRITE_ONLY: {
		void* result = VirtualAlloc(NULL, size, 0x1000|0x2000, 0x04);
		kai__assert(result != NULL);
		internal->total_allocated += size;
		return result;
	}
	case KAI_MEMORY_SET_EXECUTABLE: {
		DWORD old;
		kai__assert(VirtualProtect(ptr, size, 0x10, &old) != 0);
		return NULL;
	}
	case KAI_MEMORY_FREE: {
		kai__assert(VirtualFree(ptr, 0, 0x8000) != 0);
		internal->total_allocated -= size;
	}
	}
    return NULL;
}

static Kai_u32 kai__page_size(void)
{
    struct {
        DWORD dwOemId;
        DWORD dwPageSize;
        int _padding[10];
    } info;
    GetSystemInfo((struct _SYSTEM_INFO*)&info);
    return (Kai_u32)info.dwPageSize;
}

#else
#	error "[KAI] No memory allocator implemented for the current platform :("
#endif

static void* kai__memory_heap_allocate(void* user, void* old_ptr, Kai_u32 new_size, Kai_u32 old_size)
{
    void* ptr = NULL;
    if (new_size == 0) {
        KAI__FREE(old_ptr);
    } else {
        kai__assert(old_ptr != NULL || old_size == 0);
        ptr = KAI__REALLOC(old_ptr, new_size);
		kai__assert(ptr != NULL);
        if (ptr != NULL) {
            kai__memory_zero((Kai_u8*)ptr + old_size, new_size - old_size);
        }
    }
    Kai__Memory_Internal* internal = user;
    internal->total_allocated += new_size;
    internal->total_allocated -= old_size;
    return ptr;
}

KAI_API (Kai_Result) kai_memory_create(Kai_Allocator* Memory)
{
    kai__assert(Memory != NULL);
    
    Memory->jit           = kai__memory_jit;
    Memory->heap_allocate = kai__memory_heap_allocate;
    Memory->page_size     = kai__page_size();
    Memory->user          = KAI__REALLOC(NULL, sizeof(Kai__Memory_Internal));

	if (!Memory->user) {
        return KAI_MEMORY_ERROR_OUT_OF_MEMORY;
    }

    Kai__Memory_Internal* internal = Memory->user;
    internal->total_allocated = 0;
    return KAI_SUCCESS;
}
KAI_API (Kai_Result) kai_memory_destroy(Kai_Allocator* Memory)
{
    kai__assert(Memory != NULL);
    
    if (Memory->user == NULL)
        return KAI_ERROR_FATAL;
    
    Kai__Memory_Internal* internal = Memory->user;

    if (internal->total_allocated != 0)
        return KAI_MEMORY_ERROR_MEMORY_LEAK;
        
    KAI__FREE(Memory->user);
    *Memory = (Kai_Allocator) {0};
    return KAI_SUCCESS;
}
KAI_API (Kai_u64) kai_memory_usage(Kai_Allocator* Memory)
{
    Kai__Memory_Internal* internal = Memory->user;
    return (Kai_u64)internal->total_allocated;
}

#endif
#ifndef KAI_DONT_USE_WRITER_API // KAI__SECTION_IMPLEMENTATION_WRITER_API

char const* kai__term_debug_colors [KAI__COLOR_COUNT] = {
    [KAI_COLOR_PRIMARY]     = "\x1b[0;37m",
    [KAI_COLOR_SECONDARY]   = "\x1b[1;97m",
    [KAI_COLOR_IMPORTANT]   = "\x1b[1;91m",
    [KAI_COLOR_IMPORTANT_2] = "\x1b[1;94m",
    [KAI_COLOR_DECORATION]  = "\x1b[0;90m",
};

static void kai__file_writer_write_value(void* User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format Format)
{
    if (User == NULL) return;
    FILE* f = User;
    switch (Type) {
        case KAI_FILL: {
            kai__for_n (Format.min_count) {
                fputc(Format.fill_character, f);
            }
        } break;

        case KAI_F64: {
            fprintf(f, "%f", Value.f64);
        } break;

        case KAI_S32: {
            fprintf(f, "%i", Value.s32);
        } break;

        case KAI_U32: {
            fprintf(f, "%u", Value.u32);
        } break;

        case KAI_U64: {
            char base = 'u';
            if (Format.flags & KAI_WRITE_FLAG_HEXIDECIMAL)
                base = 'X';
            if (Format.min_count != 0) {
                if (Format.fill_character != 0) {
                    char fmt[7] = "%.*ll.";
                    fmt[1] = Format.fill_character;
                    fmt[5] = base;
                    fprintf(f, fmt, Format.min_count, Value.u64);
                }
                else {
                    char fmt[6] = "%*ll.";
                    fmt[4] = base;
                    fprintf(f, fmt, Format.min_count, Value.u64);
                }
                break;
            }
            char fmt[5] = "%ll.";
            fmt[3] = base;
            fprintf(f, fmt, Value.u64);
        } break;

        default: {
            fprintf(f, "[Writer] -- Case not defined for type %i\n", Type);
        } break;
    }
}
static void kai__file_writer_write_string(Kai_ptr User, Kai_string string)
{
    if (User == NULL) return;
    fwrite(string.data, 1, string.count, User);
}

static void kai__stdout_write_value(void* User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format Format)
{
    kai__unused(User);
    kai__file_writer_write_value(stdout, Type, Value, Format);
}
static void kai__stdout_write_string(Kai_ptr User, Kai_string string)
{
    kai__unused(User);
    fwrite(string.data, 1, string.count, stdout);
}
static void kai__stdout_writer_set_color(Kai_ptr User, Kai_Write_Color color)
{
    kai__unused(User);
    printf("%s", kai__term_debug_colors[color]);
}

#if defined(KAI__PLATFORM_WINDOWS)
__declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);
#endif

#if defined(KAI__PLATFORM_WINDOWS)
static FILE* kai__stdc_file_open(char const* path, char const* mode) {
    FILE* handle = NULL;
    fopen_s(&handle, path, mode); // this is somehow more safe? :/
    return (void*)handle;
}
#else
#    define kai__stdc_file_open fopen
#endif

KAI_API (Kai_Writer*) kai_writer_stdout(void)
{
#if defined(KAI__PLATFORM_WINDOWS)
    SetConsoleOutputCP(65001);
#endif
    setlocale(LC_CTYPE, ".UTF8");
    static Kai_Writer writer = {
        .write_string   = kai__stdout_write_string,
        .write_value    = kai__stdout_write_value,
        .set_color      = kai__stdout_writer_set_color,
        .user           = NULL,
    };
    return &writer;
}

KAI_API (void) kai_writer_file_open(Kai_Writer* out_Writer, const char* path)
{
    *out_Writer = (Kai_Writer) {
        .write_string   = kai__file_writer_write_string,
        .write_value    = kai__file_writer_write_value,
        .set_color      = NULL,
        .user           = kai__stdc_file_open(path, "wb"),
    };
}
KAI_API (void) kai_writer_file_close(Kai_Writer* writer)
{
    if (writer->user != NULL)
        fclose(writer->user);
}

#endif
#endif
#ifdef __cplusplus
}
#endif

#if defined(KAI__COMPILER_GNU) || defined(KAI__COMPILER_CLANG)
#	pragma GCC diagnostic pop
#elif defined(KAI__COMPILER_MSVC)
#	pragma warning(pop)
#endif

#endif // KAI__H

/*
    Copyright (C) 2025  lazergenixdev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
