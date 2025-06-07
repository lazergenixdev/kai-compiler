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

    define  KAI_USE_MEMORY_API  to enable default memory allocator
    define  KAI_USE_WRITER_API  to enable stdout and file writer
    define  KAI_USE_CPP_API     to enable C++ API (experimental)

	NOTE: When not using these APIs, no C standard libraries are used!

--- CONVENTIONS -----------------------------------------------------------------------------------------

    Types:           begin with "Kai_"
    Functions:       begin with "kai_"
    Enums/Macros:    begin with "KAI_"
    Internal API:    begin with "KAI__" or "kai__" or "Kai__"

    This header is divided into sections with `KAI__SECTION` for convenience
        KAI__SECTION_BUILTIN_TYPES
        KAI__SECTION_TYPE_INFO_STRUCTS
        KAI__SECTION_CORE_STRUCTS
        KAI__SECTION_SYNTAX_TREE
        KAI__SECTION_PROGRAM
        KAI__SECTION_CORE_API

---------------------------------------------------------------------------------------------------------

	CODE STYLE:

	I have one very simple rule, "always" choose formatting that increases visibility

	This means:
		- Strict following of 'CONVENTIONS' above
			- Note: Macros that act like functions are allowed to look like functions
		- No complicated macros (anything that makes the code NOT look like C)
		- Align similar lines together

*/
#ifndef KAI__H
#define KAI__H
#include <stdint.h> // --> uint32, uint64, ...
#include <stddef.h> // --> NULL
   
#define KAI_VERSION_MAJOR 0
#define KAI_VERSION_MINOR 1
#define KAI_VERSION_PATCH 0
#define KAI__MAKE_VERSION_STRING(X,Y,Z) #X "." #Y "." #Z
#define KAI__MAKE_VERSION_STRING2(X,Y,Z) KAI__MAKE_VERSION_STRING(X,Y,Z) // macros suck
#define KAI_VERSION_STRING \
    KAI__MAKE_VERSION_STRING2(KAI_VERSION_MAJOR,KAI_VERSION_MINOR,KAI_VERSION_PATCH)

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
#	define KAI__MACHINE_WASM // God bless your soul 🙏
#elif defined(__x86_64__) || defined(_M_X64)
#	define KAI__MACHINE_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#	define KAI__MACHINE_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
#	define KAI__MACHINE_ARM64
#else
#	error "[KAI] Architecture not supported!"
#endif

#if defined(KAI__PLATFORM_WASM)
#	define KAI_API(RETURN_TYPE) __attribute__((__visibility__("default"))) extern RETURN_TYPE
#else
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

#if defined(KAI__COMPILER_GNU) || defined(KAI__COMPILER_CLANG)
#	pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmultichar" // ? this is a feature, why warning??
#elif defined(KAI__COMPILER_MSVC)
#	pragma warning(push)
//#	pragma warning(disable: 4127)
#endif


#define KAI_BOOL(EXPR)      ((Kai_bool)((EXPR) ? KAI_TRUE : KAI_FALSE))

#if defined(KAI__COMPILER_MSVC)
#   define KAI_CONSTANT_STRING(S) {(Kai_u8*)(S), sizeof(S)-1}
#else
#   define KAI_CONSTANT_STRING(S) KAI_STRING(S)
#endif
#define KAI_STRING(LITERAL) KAI_STRUCT(Kai_str){(Kai_u8*)(LITERAL), sizeof(LITERAL)-1}
#define KAI_EMPTY_STRING    KAI_STRUCT(Kai_str){0}

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
} Kai_str;

typedef struct {
	Kai_u32 count;
	Kai_u32 offset;
} Kai_range;

typedef struct {
	Kai_ptr data;
	Kai_u32 count;
} Kai_slice;

#endif
#ifndef KAI__SECTION_TYPE_INFO_STRUCTS

enum {
	KAI_TYPE_TYPE      = 0, // No Type_Info struct for this.
	KAI_TYPE_INTEGER   = 1,
	KAI_TYPE_FLOAT     = 2,
	KAI_TYPE_POINTER   = 3,
	KAI_TYPE_PROCEDURE = 4,
	KAI_TYPE_STRUCT    = 5,
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
	Kai_u32    member_count;
	Kai_Type*  member_types;
	Kai_str*   member_names;
} Kai_Type_Info_Struct;

typedef struct {
	Kai_str  name;
	Kai_Type type;
} Kai_Named_Type;

// If there is a better solution to this, let me know
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

// TODO: move to implementation
static Kai_Named_Type kai__builtin_types [] = {
    { KAI_CONSTANT_STRING("Type"), (Kai_Type)&kai__type_info_type },
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

	// Special type for String_Writer to
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

typedef struct {
	Kai_u32 flags;
	Kai_u32 min_count; // default is 0 (no minimum)
	Kai_u32 max_count; // default is 0 (no maximum)
	Kai_u8  fill_character;
} Kai_Write_Format;

typedef void Kai_P_Write_String (void* User, Kai_str String);
typedef void Kai_P_Write_Value  (void* User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format Format);
typedef void Kai_P_Set_Color    (void* User, Kai_Write_Color Color);

typedef struct {
	Kai_P_Write_String * write_string;
	Kai_P_Write_Value  * write_value;
	Kai_P_Set_Color    * set_color;
	void               * user;
} Kai_String_Writer;

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Base Memory Allocator -------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

enum {
	KAI_MEMORY_ACCESS_READ    = 1 << 0,
	KAI_MEMORY_ACCESS_WRITE   = 1 << 1,
	KAI_MEMORY_ACCESS_EXECUTE = 1 << 2,
	KAI_MEMORY_ACCESS_READ_WRITE = KAI_MEMORY_ACCESS_READ | KAI_MEMORY_ACCESS_WRITE,
};

typedef void* Kai_P_Memory_Allocate      (void* User, Kai_u32 Num_Bytes, Kai_u32 access);
typedef void  Kai_P_Memory_Free          (void* User, void* Ptr, Kai_u32 Num_Bytes);
typedef void* Kai_P_Memory_Heap_Allocate (void* User, void* Ptr, Kai_u32 New_Size, Kai_u32 Old_Size);
typedef void  Kai_P_Memory_Set_Access    (void* User, void* Ptr, Kai_u32 Num_Bytes, Kai_u32 access);

typedef struct {
	Kai_P_Memory_Allocate*      allocate;
	Kai_P_Memory_Free*          free;
	Kai_P_Memory_Set_Access*    set_access;
	Kai_P_Memory_Heap_Allocate* heap_allocate;
	void*                       user;
	Kai_u32                     page_size;
} Kai_Allocator;

typedef struct {
	void*   data;
	Kai_u32 size;
} Kai_Memory;

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Errors ----------------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

enum {
	KAI_SUCCESS = 0,
	KAI_ERROR_SYNTAX,
	KAI_ERROR_SEMANTIC,
	KAI_ERROR_INFO,
	KAI_ERROR_FATAL,    // means compiler bug probably
	KAI_ERROR_INTERNAL, // means compiler error unrelated to source code (e.g. out of memory)
	KAI__RESULT_COUNT,
};
typedef Kai_u32 Kai_Result;

typedef struct {
	Kai_str file_name;
	Kai_str string;
	Kai_u8* source; // source code for this file
	Kai_u32 line;
} Kai_Location;

typedef struct Kai_Error {
	Kai_Result        result;
	Kai_Location      location;
	Kai_str           message;
	Kai_str           context;
	Kai_Memory        memory;
	struct Kai_Error* next;
} Kai_Error;

// Used to describe number literals
// Value = ( Whole + Frac / (10 ^ Frac_Denom) ) * 10 ^ Exp
typedef struct {
	Kai_u64 Whole_Part;
	Kai_u64 Frac_Part;
	Kai_s32 Exp_Part;
	Kai_u16 Frac_Denom;
} Kai_Number;

#endif
#ifndef KAI__SECTION_INTERNAL_STRUCTS

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Data Structures -------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

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
		T*      elements;     \
		T       storage[N];   \
	};                        \
}

#define KAI__HASH_TABLE(T) \
struct {                   \
	Kai_u32  count;        \
	Kai_u32  capacity;     \
	Kai_u64* occupied;     \
	Kai_u64* hashes;       \
	Kai_str* keys;         \
	T*       values;       \
}

typedef struct {
	Kai_u32  size;
	Kai_u32  capacity;
	Kai_u8*  data;
	Kai_u32  offset;
} Kai__Dynamic_Buffer;

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Memory Allocators -----------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define KAI__MINIMUM_ARENA_BUCKET_SIZE 0x40000

typedef struct Kai__Arena_Bucket {
    struct Kai__Arena_Bucket* prev;
} Kai__Arena_Bucket;

typedef struct {
    Kai__Arena_Bucket*  current_bucket;
    Kai_u32             current_allocated;
    Kai_u32             bucket_size;
    Kai_Allocator       allocator;
} Kai__Dynamic_Arena_Allocator;


#if 0

typedef KAI__ARRAY(Kai_u8) Kai__String_Builder;

static inline void __string_builder_example()
{
	Kai__String_Builder builder;
	kai__string_builder_reserve(&builder, 100);
	kai__string_builder_append(&builder, "not handled ");
	kai__string_builder_append_u64(&builder, expr->id);
	kai__string_builder_release(&builder);
}
#endif

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Tokens ----------------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

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
    Kai_str     string;
    Kai_Number  number;
} Kai__Token;

typedef struct {
    Kai__Token  current_token;
    Kai__Token  peeked_token;
    Kai_str     source;
    Kai_u32     cursor;
    Kai_u32     line_number;
    Kai_bool    peeking;
} Kai__Tokenizer;

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Parser ----------------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

typedef struct {
    Kai__Tokenizer                tokenizer;
    Kai__Dynamic_Arena_Allocator  arena;
    Kai_Error                     error;
} Kai__Parser;

#endif
#ifndef KAI__SECTION_SYNTAX_TREE

typedef struct Kai_Expr_Base* Kai_Expr; // Expression Nodes
typedef struct Kai_Expr_Base* Kai_Stmt; // Statement Nodes

typedef enum {
	KAI_EXPR_IDENTIFIER     = 0,
	KAI_EXPR_STRING         = 1,
	KAI_EXPR_NUMBER         = 2,
	KAI_EXPR_BINARY         = 3,
	KAI_EXPR_UNARY          = 4,
	KAI_EXPR_PROCEDURE_TYPE = 5,
	KAI_EXPR_PROCEDURE_CALL = 6,
	KAI_EXPR_PROCEDURE      = 7,
	KAI_EXPR_CODE           = 8,
	KAI_EXPR_STRUCT         = 9,
	KAI_STMT_RETURN         = 10,
	KAI_STMT_DECLARATION    = 11,
	KAI_STMT_ASSIGNMENT     = 12,
	KAI_STMT_IF             = 13,
	KAI_STMT_FOR            = 14,
	KAI_STMT_DEFER          = 15,

	KAI_STMT_COMPOUND       = 16, // TODO: remove?
} Kai_Node_ID;

enum {
	KAI_DECL_FLAG_CONST = 1 << 0,
	KAI_DECL_FLAG_USING = 1 << 1,
};

#define KAI_BASE_MEMBERS \
	Kai_u8  id;          \
	Kai_u8  flags;       \
	Kai_str source_code; \
	Kai_str name;        \
	Kai_Expr next;       \
	Kai_u32 line_number
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
	Kai_Stmt body;
	Kai_Stmt else_body;
} Kai_Stmt_If;

typedef struct {
	KAI_BASE_MEMBERS;
	Kai_Stmt body;
	Kai_Expr from;
	Kai_Expr to; // optional (interates through `from` if this is null)
	Kai_str  iterator_name;
} Kai_Stmt_For;

typedef struct {
    Kai_Stmt_Compound            root;
    Kai_str                      source_filename; // TODO: rename
	Kai__Dynamic_Arena_Allocator allocator;
} Kai_Syntax_Tree;

typedef struct {
    Kai_str        source_code; // input
    Kai_Allocator  allocator;   // input
    Kai_Error*     error;       // output [optional]
} Kai_Syntax_Tree_Create_Info;

#endif
#ifndef KAI__SECTION_BYTECODE

// Bytecode Operations (BOP)
enum {
    KAI_BOP_NOP           = 0,  //  nop
    KAI_BOP_LOAD_CONSTANT = 1,  //  %0 <- load_constant.u32 8
    KAI_BOP_ADD           = 2,  //  %2 <- add.u32 %0, %1
    KAI_BOP_SUB           = 3,  //  %7 <- sub.u32 %0, %1
    KAI_BOP_MUL           = 4,  //  %8 <- mul.u32 %0, %1
    KAI_BOP_DIV           = 5,  //  %9 <- div.u32 %0, %1
    KAI_BOP_COMPARE       = 6,  //  %3 <- compare.gt.s64 %0, 0
    KAI_BOP_BRANCH        = 7,  //  branch %5 {0x43}
    KAI_BOP_JUMP          = 8,  //  jump 0x43
    KAI_BOP_CALL          = 9,  //  %4, %5 <- call {0x34} (%1, %2)
    KAI_BOP_RETURN        = 10, //  ret %1, %2
    KAI_BOP_NATIVE_CALL   = 11, //  %4 <- native_call {0x100f3430} (%1, %2)
    KAI_BOP_LOAD          = 12, //  %5 <- u64 [%1 + 0x24]
    KAI_BOP_STORE         = 13, //  u32 [%2 + 0x8] <- %5
    KAI_BOP_STACK_ALLOC   = 14, //  %6 <- stack_alloc 48
    KAI_BOP_STACK_FREE    = 15, //  stack_free 48
    KAI_BOP_CHECK_ADDRESS = 99, //  check_address.u32 (%1 + 0x8)
};

enum {
    KAI_CMP_LT = 0,
    KAI_CMP_GE = 1,
    KAI_CMP_GT = 2,
    KAI_CMP_LE = 3,
    KAI_CMP_EQ = 4,
    KAI_CMP_NE = 5,
};

enum {
    KAI_BC_ERROR_MEMORY   = 1, // bcs_insert_* ; bcs_
    KAI_BC_ERROR_BRANCH   = 2, // bcs_set_branch
	KAI_BC_ERROR_OVERFLOW = 3, // bytecode instruction went outside of buffer
};

enum {
    KAI_INTERP_FLAGS_DONE       = 1 << 0, // returned from call stack
    KAI_INTERP_FLAGS_OVERFLOW   = 1 << 1, // the next bytecode instruction went outside of buffer
    KAI_INTERP_FLAGS_INCOMPLETE = 1 << 2, // a bytecode instruction being executed was not able to be fully read
    KAI_INTERP_FLAGS_INVALID    = 1 << 3, // instruction was invalid (e.g. %0xFFFFFFF <- 69)
};

typedef Kai_u32 Kai_Reg;

typedef struct {
    char const * name; // metadata
    void const * address;
    Kai_u8       input_count;
} Kai_Native_Procedure;

// TODO: replace with Kai_Byte_Array
typedef struct {
    Kai_u8*        data;
    Kai_u32        count;
    Kai_u32        capacity;
    Kai_Allocator* allocator;
} Kai_BC_Stream;

typedef KAI__ARRAY(Kai_u8) Kai__Byte_Array;

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
    Kai_u32                   max_return_values_count;
    
    // Native Functions
    Kai_Native_Procedure    * native_procedures;
    Kai_u32                   native_procedure_count;

    // Stack Memory
    Kai_u8                  * stack;
    Kai_u32                   stack_size;
    Kai_u32                   max_stack_size;

    void                    * scratch_buffer; // at least 255 x 4 Bytes
} Kai_Interpreter;

typedef struct {
    Kai_u32 max_register_count;
    Kai_u32 max_call_stack_count;
    Kai_u32 max_return_value_count;
    Kai_u32 max_stack_size;
    void*   memory;
} Kai_Interpreter_Setup;

typedef struct {
	void                    * data;
    Kai_u32                   count;
	Kai_range                 range;
    Kai_u8                    ret_count;
    Kai_u8                    arg_count;
    Kai_Native_Procedure    * natives;
    Kai_u32                   native_count;
    Kai_u32                 * branch_hints;
    Kai_u32                   branch_count;
} Kai_Bytecode;

#endif
#ifndef KAI__SECTION_PROGRAM

typedef struct {
	Kai_Syntax_Tree*      trees;
	Kai_u32               tree_count;
	Kai_Allocator         allocator;
	Kai_Error*            error;
	Kai_Native_Procedure* native_procedures;
	Kai_u32               native_procedure_count;
} Kai_Program_Create_Info;

typedef struct {
	void*                  platform_machine_code;
	Kai_u32                code_size;
	KAI__HASH_TABLE(void*) procedure_table;
	Kai_Allocator          allocator;
} Kai_Program;

typedef struct {
	Kai_u32 flags; // (LOCAL_VARIABLE, TYPE)
    Kai_u32 value; // stored index value
} Kai__DG_Node_Index;

#define KAI__DG_NODE_EVALUATED      (1 << 0)
#define KAI__DG_NODE_TYPE           (1 << 0)
#define KAI__DG_NODE_LOCAL_VARIABLE (1 << 1)
#define KAI__SCOPE_GLOBAL_INDEX     0
#define KAI__SCOPE_NO_PARENT        0xFFFFFFFF

typedef union {
    Kai_Value value;
    Kai_Type  type;
	Kai_u32   procedure_location;
} Kai__DG_Value;

typedef KAI__ARRAY(Kai__DG_Node_Index)      Kai__DG_Dependency_Array;
typedef KAI__HASH_TABLE(Kai__DG_Node_Index) Kai__DG_Identifier_Map;

typedef struct {
	Kai_Type                 type;  // evaluated type
	Kai__DG_Value            value; // evaluated value
	Kai__DG_Dependency_Array value_dependencies, type_dependencies;
    Kai_u32                  value_flags,        type_flags;  // (NODE_EVALUATED)
	Kai_str                  name;
	Kai_Expr                 expr;
	Kai_u32                  line_number;
	Kai_u32                  scope_index;
} Kai__DG_Node;

typedef struct {
    Kai__DG_Identifier_Map identifiers;
    Kai_u32                parent;
    Kai_bool               is_proc_scope;
} Kai__DG_Scope;

typedef struct {
	Kai_Error*                error;
    Kai_Allocator             allocator;
	KAI__ARRAY(Kai__DG_Scope) scopes;
	KAI__ARRAY(Kai__DG_Node)  nodes;
	int*                      compilation_order;
} Kai__Dependency_Graph;

typedef struct {
	Kai_u32  location;
	Kai_Type type;
} Kai__Bytecode_Procedure;

typedef KAI__HASH_TABLE(Kai__Bytecode_Procedure) Kai__Bytecode_Procedure_Table;

typedef struct {
	Kai_BC_Stream                 stream; // Global bytecode stream
	Kai_Interpreter               interp; // <3
	KAI__ARRAY(Kai_u32)           branch_hints;
	Kai__Bytecode_Procedure_Table procedure_table;
} Kai__Bytecode;

typedef struct {
	Kai_Error*        error;
	Kai_Syntax_Tree*  trees;
	Kai_u32           tree_count;
	Kai_Allocator     allocator;
} Kai__Dependency_Graph_Create_Info;

typedef struct {
	Kai_Error*              error;
	Kai__Dependency_Graph*  dependency_graph;
} Kai__Bytecode_Create_Info;

typedef struct {
	Kai_str name;
	Kai_Reg reg;
} Kai__Bytecode_Register;

typedef struct {
	Kai_Error*                   error;
	Kai__Dependency_Graph*       dependency_graph;
	Kai__Bytecode*               bytecode;
	Kai__Dynamic_Arena_Allocator arena;
	KAI__ARRAY(Kai__Bytecode_Register) registers;
} Kai__Bytecode_Generation_Context;

typedef struct {
	Kai_Error*     error;
	Kai_Allocator  allocator;
    Kai__Bytecode* bytecode;
} Kai__Program_Create_Info;

#endif
#ifndef KAI__SECTION_CORE_API

KAI_API (Kai_bool) kai_str_equals       (Kai_str A, Kai_str B);
KAI_API (Kai_str)  kai_str_from_cstring (char const* String);

KAI_API (Kai_vector3_u32) kai_version        (void);
KAI_API (Kai_str)         kai_version_string (void);

/// @param out_Syntax_Tree Must be freed using @ref(kai_destroy_syntax_tree)
KAI_API (Kai_Result) kai_create_syntax_tree  (Kai_Syntax_Tree_Create_Info* Info, Kai_Syntax_Tree* out_Syntax_Tree);
KAI_API (void)       kai_destroy_syntax_tree (Kai_Syntax_Tree* Syntax_Tree);

KAI_API (Kai_Result) kai_create_program             (Kai_Program_Create_Info* Info, Kai_Program* out_Program);
KAI_API (Kai_Result) kai_create_program_from_source (Kai_str Source, Kai_Allocator* Allocator, Kai_Error* out_Error, Kai_Program* out_Program);
KAI_API (void)       kai_destroy_program            (Kai_Program Program);

//! @note With WASM backend you cannot get any function pointers
//!       (because WASM is complete garbage and doesn't support it)
KAI_API (void*) kai_find_procedure (Kai_Program Program, Kai_str Name, Kai_Type opt_Type);
KAI_API (void)  kai_destroy_error  (Kai_Error* Error, Kai_Allocator* Allocator);

KAI_API (void) kai_write_syntax_tree (Kai_String_Writer* Writer, Kai_Syntax_Tree* Tree);
KAI_API (void) kai_write_error       (Kai_String_Writer* Writer, Kai_Error* Error);
KAI_API (void) kai_write_type        (Kai_String_Writer* Writer, Kai_Type Type);
KAI_API (void) kai_write_expression  (Kai_String_Writer* Writer, Kai_Expr Expr);

KAI_API (Kai_Result) kai_bc_insert_load_constant   (Kai_BC_Stream* Stream, Kai_u8 type, Kai_Reg reg_dst, Kai_Value value);
KAI_API (Kai_Result) kai_bc_insert_math            (Kai_BC_Stream* Stream, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2);
KAI_API (Kai_Result) kai_bc_insert_math_value      (Kai_BC_Stream* Stream, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value);
KAI_API (Kai_Result) kai_bc_insert_compare         (Kai_BC_Stream* Stream, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2);
KAI_API (Kai_Result) kai_bc_insert_compare_value   (Kai_BC_Stream* Stream, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value);
KAI_API (Kai_Result) kai_bc_insert_branch_location (Kai_BC_Stream* Stream, Kai_u32 location, Kai_Reg reg_src);
KAI_API (Kai_Result) kai_bc_insert_branch          (Kai_BC_Stream* Stream, Kai_u32* branch, Kai_Reg reg_src);
KAI_API (Kai_Result) kai_bc_insert_jump_location   (Kai_BC_Stream* Stream, Kai_u32 location);
KAI_API (Kai_Result) kai_bc_insert_jump            (Kai_BC_Stream* Stream, Kai_u32* branch);
KAI_API (Kai_Result) kai_bc_insert_call            (Kai_BC_Stream* Stream, Kai_u32* branch, Kai_u8 ret_count, Kai_Reg* reg_ret, Kai_u8 arg_count, Kai_Reg* reg_arg);
KAI_API (Kai_Result) kai_bc_insert_return          (Kai_BC_Stream* Stream, Kai_u8 count, Kai_Reg* regs);
KAI_API (Kai_Result) kai_bc_insert_native_call     (Kai_BC_Stream* Stream, Kai_u8 use_dst, Kai_Reg reg_dst, Kai_Native_Procedure* proc, Kai_Reg* reg_src);
KAI_API (Kai_Result) kai_bc_insert_load            (Kai_BC_Stream* Stream, Kai_Reg reg_dst, Kai_u8 type, Kai_Reg reg_addr, Kai_u32 offset);
KAI_API (Kai_Result) kai_bc_insert_store           (Kai_BC_Stream* Stream, Kai_Reg reg_src, Kai_u8 type, Kai_Reg reg_addr, Kai_u32 offset);
KAI_API (Kai_Result) kai_bc_insert_check_address   (Kai_BC_Stream* Stream, Kai_u8 type, Kai_Reg reg_addr, Kai_u32 offset);
KAI_API (Kai_Result) kai_bc_insert_stack_alloc     (Kai_BC_Stream* Stream, Kai_Reg reg_dst, Kai_u32 size);
KAI_API (Kai_Result) kai_bc_insert_stack_free      (Kai_BC_Stream* Stream, Kai_u32 size);
KAI_API (Kai_Result) kai_bc_set_branch             (Kai_BC_Stream* Stream, Kai_u32 branch, Kai_u32 location);
KAI_API (Kai_Result) kai_bc_reserve                (Kai_BC_Stream* Stream, Kai_u32 byte_count);

#define kai_interp_run(Interpreter, Max_Step_Count) \
	for (int __iteration__ = 0; kai_interp_step(Interpreter) && ++__iteration__ < Max_Step_Count;)

/// @brief Execute one bytecode instruction
/// @return 1 if done executing or error, 0 otherwise
int kai_interp_step(Kai_Interpreter* interp);

KAI_API (Kai_u32)    kai_interp_required_memory_size (Kai_Interpreter_Setup* info);
KAI_API (Kai_Result) kai_interp_create               (Kai_Interpreter* Interp, Kai_Interpreter_Setup* info);
KAI_API (void)       kai_interp_reset                (Kai_Interpreter* Interp, Kai_u32 location);
KAI_API (void)       kai_interp_set_input            (Kai_Interpreter* Interp, Kai_u32 index, Kai_Value value);
KAI_API (void)       kai_interp_push_output          (Kai_Interpreter* Interp, Kai_Reg reg);

// TODO: rename -> kai_interp_load_from_array(Kai_Interpreter* Interp, Kai__Byte_Array* array)
KAI_API (void) kai_interp_load_from_stream (Kai_Interpreter* Interp, Kai_BC_Stream* stream);
KAI_API (void) kai_interp_load_from_memory (Kai_Interpreter* Interp, void* code, Kai_u32 size);

KAI_API (Kai_Result) kai_bytecode_to_string (Kai_Bytecode* Bytecode, Kai_String_Writer* Writer);
KAI_API (Kai_Result) kai_bytecode_to_c      (Kai_Bytecode* Bytecode, Kai_String_Writer* Writer);

#endif
#ifndef KAI__SECTION_INTERNAL_API

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Convienence -----------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__unused(VAR) (void)VAR

#define kai__count(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

#define kai__for_n(N) for (Kai_int i = 0; i < (Kai_int)(N); ++i)

#define kai__allocate(Old, New_Size, Old_Size) \
	allocator->heap_allocate(allocator->user, Old, New_Size, Old_Size)

#define kai__free(Ptr,Size) \
	allocator->heap_allocate(allocator->user, Ptr, 0, Size)

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- String Writer ---------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__write(C_String) \
	writer->write_string(writer->user, KAI_STRING(C_String))

#define kai__write_string(String) \
	writer->write_string(writer->user, String)

#define kai__write_char(Char) \
	writer->write_string(writer->user, (Kai_str){.data = (Kai_u8[]){Char}, .count = 1})

#define kai__set_color(Color) \
	if (writer->set_color != NULL) writer->set_color(writer->user, Color)

#define kai__write_fill(Char, Count)                                        \
	writer->write_value(writer->user, KAI_FILL, (Kai_Value){},              \
		(Kai_Write_Format){.fill_character = (Kai_u8)Char, .min_count = Count})

#define kai__write_u32(Value) \
	writer->write_value(writer->user, KAI_U32, (Kai_Value){.u32 = Value}, (Kai_Write_Format){})

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- (Fatal) Error Handling + Debug Assertions -----------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#if defined(KAI__DISABLE_ALL_CHECKS)

#define kai__assert(EXPR)          (void)0
#define kai__unreachable()         (void)0
#define kai__check_allocation(PTR) (void)0

#else

KAI_API (void) kai__fatal_error(char const* Desc, char const* Message, char const* File, int Line);

#define KAI__FATAL_ERROR(Desc, Message) \
	kai__fatal_error(Desc, Message, __FILE__, __LINE__);

#define kai__assert(EXPR) \
	if (!(EXPR))          \
		KAI__FATAL_ERROR("Assertion Failed", #EXPR)
		
#define kai__unreachable() \
	KAI__FATAL_ERROR("Assertion Failed", "Unreachable was reached! D:")

// TODO: never check allocations -> move checks to memory allocator API
#define kai__check_allocation(PTR) \
	if ((PTR) == NULL)             \
		KAI__FATAL_ERROR("Allocation Error", #PTR " was null")

#endif
		
// 
// --- Basic Memory Operations -----------------------------------------------
// 

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

static inline void kai__string_copy(Kai_u8* Dst, char const* restrict Src, Kai_u32 Dst_Size)
{
	for (Kai_u32 i = 0; i < Dst_Size && Src[i] != '\0'; ++i)
	{
		((Kai_u8*)Dst)[i] = ((Kai_u8*)Src)[i];
	}
}

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Basic Math Functions --------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

static inline Kai_u32 kai__ceil_div(Kai_u32 Num, Kai_u32 Den)
{
    return (Num + Den - 1) / Den;
}

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Dynamic Arena Allocator -----------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

static inline void kai__dynamic_arena_allocator_create(
	Kai__Dynamic_Arena_Allocator* Arena,
	Kai_Allocator*                Allocator)
{
	kai__assert(Arena != NULL);
	kai__assert(Allocator != NULL);

    Arena->allocator = *Allocator;
    Arena->bucket_size = kai__ceil_div(KAI__MINIMUM_ARENA_BUCKET_SIZE, Allocator->page_size)
	                   * Allocator->page_size;
    Arena->current_allocated = sizeof(Kai__Arena_Bucket*);
    Arena->current_bucket = Allocator->allocate(
		Allocator->user,
		Arena->bucket_size,
		KAI_MEMORY_ACCESS_READ_WRITE
	);
	kai__check_allocation(Arena->current_bucket);
}

static inline void kai__dynamic_arena_allocator_free_all(Kai__Dynamic_Arena_Allocator* Arena)
{
	kai__assert(Arena != NULL);
    Kai__Arena_Bucket* bucket = Arena->current_bucket;

    while (bucket)
	{
        Kai__Arena_Bucket* prev = bucket->prev;
        Arena->allocator.free(Arena->allocator.user, bucket, Arena->bucket_size);
        bucket = prev;
    }
    Arena->current_bucket = NULL;
    Arena->current_allocated = 0;
}

static inline void kai__dynamic_arena_allocator_destroy(Kai__Dynamic_Arena_Allocator* Arena)
{
    kai__dynamic_arena_allocator_free_all(Arena);
    Arena->bucket_size = 0;
    Arena->allocator = (Kai_Allocator) {0};
}

static inline void* kai__arena_allocate(Kai__Dynamic_Arena_Allocator* Arena, Kai_u32 Size)
{    
	kai__assert(Arena != NULL);

    if (Size > Arena->bucket_size)
        KAI__FATAL_ERROR("Arena Allocator", "Object size greater than bucket size (incorrect usage)");
    
    if (Arena->current_allocated + Size > Arena->bucket_size)
    {
        Kai__Arena_Bucket* new_bucket = Arena->allocator.allocate(
            Arena->allocator.user,
            Arena->bucket_size,
            KAI_MEMORY_ACCESS_READ_WRITE
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

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Array API -------------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__array_iterate(Array, Var) \
	for (Kai_u32 Var = 0; Var < (Array).count; ++Var)

#define kai__array_reserve(Ptr_Array, Size) \
	kai__array_reserve_stride(Ptr_Array, Size, allocator, sizeof((Ptr_Array)->elements[0]))

#define kai__array_destroy(Ptr_Array) \
	kai__array_destroy_stride(Ptr_Array, allocator, sizeof((Ptr_Array)->elements[0]))

#define kai__array_append(Ptr_Array, ...) \
	kai__array_grow_stride(Ptr_Array, (Ptr_Array)->count + 1, allocator, sizeof((Ptr_Array)->elements[0])), \
	(Ptr_Array)->elements[(Ptr_Array)->count++] = (__VA_ARGS__)

static inline void kai__array_reserve_stride(void* Array, Kai_u32 Size, Kai_Allocator* allocator, Kai_u32 Stride)
{
	KAI__ARRAY(void)* array = Array;
	
	if (Size <= array->capacity)
		return;

	array->elements = kai__allocate(array->elements, Stride * Size, Stride * array->capacity);
	array->capacity = Size;
}

static inline void kai__array_grow_stride(void* Array, Kai_u32 Min_Size, Kai_Allocator* allocator, Kai_u32 Stride)
{
	KAI__ARRAY(void)* array = Array;
	
	if (Min_Size <= array->capacity)
		return;
	
	Kai_u32 new_capacity = (Min_Size * 3) / 2;
	array->elements = kai__allocate(array->elements, Stride * new_capacity, Stride * array->capacity);
	array->capacity = new_capacity;
	kai__check_allocation(array->elements);
}

static inline void kai__array_shrink_to_fit_stride(void* Array, Kai_Allocator* allocator, Kai_u32 Stride)
{
	KAI__ARRAY(void)* array = Array;

	if (array->capacity != 0)
	{
		array->elements = kai__allocate(array->elements, Stride * array->count, Stride * array->capacity);
		array->capacity = array->count;
	}
}

static inline void kai__array_destroy_stride(void* Ptr_Array, Kai_Allocator* allocator, Kai_u32 Stride)
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

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Hash Table API --------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define kai__bit_array_size(Bit_Count) \
	(((((Bit_Count) - 1) / 64) + 1) * sizeof(Kai_u64))

#define kai__hash_table_destroy(Table) \
	kai__hash_table_destroy_stride(&(Table), allocator, sizeof((Table).values[0]))

#define kai__hash_table_find(Table, Key) \
	kai__hash_table_find_stride(&(Table), Key, sizeof((Table).values[0]))

#define kai__hash_table_remove_index(Table, Index) \
	(Table).occupied[(Index) / 64] &=~ ((Kai_u64)1 << (Index % 64))

#define kai__hash_table_get(Table, Key, out_Value) \
    kai__hash_table_get_stride(&(Table), Key, out_Value, sizeof(*out_Value), sizeof((Table).values[0]))

#define kai__hash_table_get_str(Table, Key, out_Value) \
    kai__hash_table_get_stride(&(Table), KAI_STRING(Key), out_Value, sizeof(*out_Value), sizeof((Table).values[0]))
	
#define kai__hash_table_iterate(Table, Iter_Var)                               \
	for (Kai_u32 Iter_Var = 0; Iter_Var < (Table).capacity; ++Iter_Var)        \
		if ((Table).occupied[Iter_Var / 64] & ((Kai_u64)1 << (Iter_Var % 64)))

#define kai__hash_table_emplace(Table, Key, ...)                  \
	do {                                                          \
		Kai_u32 __index__ = kai__hash_table_emplace_key_stride(   \
			&(Table), Key, allocator, sizeof((Table).values[0])); \
		(Table).values[__index__] = (__VA_ARGS__);                \
	} while (0)

static inline Kai_u64 kai__str_hash(Kai_str In)
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

static inline void kai__hash_table_grow(void* Table, Kai_Allocator* allocator, Kai_u32 Elem_Size)
{
	KAI__HASH_TABLE(void)* table = Table;
	
	// Calculate total space required
	Kai_u32 new_capacity  = (table->capacity == 0) ? 8 : table->capacity * 2;
	Kai_u32 occupied_size = kai__bit_array_size(new_capacity);
	Kai_u32 hashes_size   = new_capacity * sizeof(Kai_u64);
	Kai_u32 keys_size     = new_capacity * sizeof(Kai_str);
	Kai_u32 values_size   = new_capacity * Elem_Size;

	void* new_ptr = kai__allocate(NULL, occupied_size + hashes_size + keys_size + values_size, 0);

	// Distribute allocated memory
	Kai_u64* occupied = new_ptr;
	Kai_u64* hashes   = (Kai_u64*)((Kai_u8*)occupied + occupied_size);
	Kai_str* keys     = (Kai_str*)((Kai_u8*)hashes + hashes_size);
	Kai_u8*  values   = (Kai_u8*)((Kai_u8*)keys + keys_size);

	Kai_u32 count = 0;

	// Rehash all elements
	kai__hash_table_iterate(*table, i)
	{
		Kai_u64 hash = kai__str_hash(table->keys[i]);
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
		kai__free(
			table->occupied,
			kai__bit_array_size(table->capacity)
			+ table->capacity * (sizeof(Kai_u64) + sizeof(Kai_str) + Elem_Size)
		);
	}

	table->capacity = new_capacity;
	table->occupied = occupied;
	table->hashes   = hashes;
	table->keys     = keys;
	table->values   = values;
}

static inline Kai_u32 kai__hash_table_emplace_key_stride(void* Table, Kai_str Key, Kai_Allocator* Allocator, Kai_u32 Elem_Size)
{
	kai__assert(Table     != NULL);
	kai__assert(Allocator != NULL);
	kai__assert(Elem_Size != 0);
	
	KAI__HASH_TABLE(void)* table = Table;

	// Check load factor and grow hash table
	if (4 * table->count >= 3 * table->capacity)
	{
		kai__hash_table_grow(Table, Allocator, Elem_Size);
	}

	Kai_u64 hash = kai__str_hash(Key);
	Kai_u32 mask = table->capacity - 1;
	Kai_u32 index = (Kai_u32)hash & mask;

	for (Kai_u32 i = 0; i < table->capacity; ++i) {
		Kai_u64 byte = table->occupied[index / 64];
		Kai_u64 bit  = (Kai_u64)1 << (index % 64);

		// Do an insertion
		if ((byte & bit) == 0)
		{
			table->occupied[index / 64] |= bit;
			table->hashes[index] = hash;
			table->keys[index] = Key;
			table->count += 1;
			return index;
		}

		// Emplace an existing item
		if (table->hashes[index] == hash
		&&  kai_str_equals(table->keys[index], Key))
		{
			return index;
		}

		index = (index + 1) & mask;
	}

	kai__unreachable();
	return 0xFFFFFFFF;
}

static inline void* kai__hash_table_find_stride(void* Table, Kai_str Key, Kai_u32 Elem_Size)
{
	kai__assert(Table     != NULL);
	kai__assert(Elem_Size != 0);
	
	KAI__HASH_TABLE(void)* table = Table;

	Kai_u64 hash = kai__str_hash(Key);
	Kai_u32 mask = table->capacity - 1;
	Kai_u32 index = (Kai_u32)hash & mask;

	for (Kai_u32 i = 0; i < table->capacity; ++i) {
		Kai_u64 block = table->occupied[index / 64];
		Kai_u64 bit   = (Kai_u64)1 << (index % 64);

		if ((block & bit) == 0)
		{
			return NULL; // Slot was empty
		}
		else if (table->hashes[index] == hash
		     &&  kai_str_equals(table->keys[index], Key))
		{
			return (Kai_u8*)table->values + Elem_Size * index;
		}

		index = (index + 1) & mask;
	}

	return NULL;
}

static inline Kai_bool kai__hash_table_get_stride(void* Table, Kai_str Key, void* out_Value, Kai_u32 Value_Size, Kai_u32 Elem_Size)
{
	void* elem_ptr = kai__hash_table_find_stride(Table, Key, Elem_Size);

	if (elem_ptr == NULL)
		return KAI_FALSE;
		
	kai__memory_copy(out_Value, elem_ptr, Value_Size);
	return KAI_TRUE;
}

static inline void kai__hash_table_destroy_stride(void* Table, Kai_Allocator* allocator, Kai_u32 Elem_Size)
{
	KAI__HASH_TABLE(void)* table = Table;

	if (table->capacity != 0)
	{
		kai__free(table->occupied,
			kai__bit_array_size(table->capacity)
			+ table->capacity * (sizeof(Kai_u64) + sizeof(Kai_str) + Elem_Size)
		);
	}
	table->count    = 0;
	table->capacity = 0;
	table->occupied = NULL;
	table->hashes   = NULL;
	table->keys     = NULL;
	table->values   = NULL;
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
	(KAI_STRUCT(Kai_str) { .count = (Range).count, .data = kai__range_to_data(Range, Memory) })

static inline void kai__dynamic_buffer_append_a(Kai__Dynamic_Buffer* Buffer, void* Data, Kai_u32 Size, Kai_Allocator* Allocator)
{
	kai__array_grow_stride(Buffer, Buffer->size + Size, Allocator, 1);
	kai__memory_copy(Buffer->data + Buffer->size, Data, Size);
	Buffer->size += Size;
}

static inline void kai__dynamic_buffer_append_max_a(Kai__Dynamic_Buffer* Buffer, Kai_str String, Kai_u32 Max_Count, Kai_Allocator* Allocator)
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

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Parser API ------------------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// out_String->count: Maximum number of characters
// out_String->count must be >= 3
static inline void kai__token_type_string(Kai_u32 Type, Kai_str* out_String)
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
			kai__string_copy(out_String->data, "end of file", out_String->count);
		break;

		case KAI__TOKEN_IDENTIFIER:
			kai__string_copy(out_String->data, "identifier", out_String->count);
		break;

        case KAI__TOKEN_DIRECTIVE:
			kai__string_copy(out_String->data, "directive", out_String->count);
		break;

        case KAI__TOKEN_STRING:
			kai__string_copy(out_String->data, "string", out_String->count);
		break;

        case KAI__TOKEN_NUMBER:
			kai__string_copy(out_String->data, "number", out_String->count);
		break;

#define X(SYMBOL) \
        case SYMBOL: \
			kai__string_copy(out_String->data, #SYMBOL, out_String->count); \
		break;
        KAI__X_TOKEN_SYMBOLS
#undef X

#define X(NAME,ID) \
		case ID: \
			kai__string_copy(out_String->data, "'" #NAME "'", out_String->count); \
		break;
        KAI__X_TOKEN_KEYWORDS
#undef X
    }
}

extern Kai__Token* kai__next_token(Kai__Tokenizer* context);
extern Kai__Token* kai__peek_token(Kai__Tokenizer* context);

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// --- Error Creation API ----------------------------------------------------
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

static inline Kai_Result kai__error_internal(Kai_Error* out_Error, Kai_str Message)
{
	*out_Error = (Kai_Error) {
		.message = Message,
		.result = KAI_ERROR_INTERNAL,
	};
	return KAI_ERROR_INTERNAL;
}

static inline void kai__error_unexpected(Kai__Parser* parser, Kai__Token* token, Kai_str where, Kai_str wanted)
{
    Kai_Allocator* allocator = &parser->arena.allocator;

    if (parser->error.result != KAI_SUCCESS)
        return;

    Kai__Dynamic_Buffer buffer = {0};
    kai__dynamic_buffer_append_string(&buffer, KAI_STRING("unexpected "));
    Kai_u8 temp [32] = {};
    Kai_str type = { .data = temp, .count = sizeof(temp) };
	kai__token_type_string(token->type, &type);
    kai__dynamic_buffer_append_string(&buffer, type);
    kai__dynamic_buffer_append_string(&buffer, KAI_STRING(" "));
    kai__dynamic_buffer_append_string(&buffer, where);
    Kai_range message_range = kai__dynamic_buffer_next(&buffer);
    Kai_Memory memory = kai__dynamic_buffer_release(&buffer);

    parser->error = (Kai_Error) {
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
		parser->error.location.string.data  -= 1; // strings must begin with "
        parser->error.location.string.count += 2;
    }
    else if (token->type == KAI__TOKEN_DIRECTIVE)
	{
        parser->error.location.string.data  -= 1;
        parser->error.location.string.count += 1;
    }
}

#endif
#ifdef KAI_USE_MEMORY_API

enum {
	KAI_MEMORY_ERROR_OUT_OF_MEMORY  = 1, //! @see kai_memory_create implementation
	KAI_MEMORY_ERROR_MEMORY_LEAK    = 2, //! @see kai_memory_destroy implementation
};

KAI_API (Kai_Result) kai_memory_create  (Kai_Allocator* out_Allocator);
KAI_API (Kai_Result) kai_memory_destroy (Kai_Allocator* Allocator);
KAI_API (Kai_u64)    kai_memory_usage   (Kai_Allocator* Allocator);

#endif
#ifdef KAI_USE_DEBUG_API

KAI_API (Kai_String_Writer*) kai_debug_stdout_writer(void);
KAI_API (void) kai_debug_open_file_writer(Kai_String_Writer* out_Writer, const char* Path);
KAI_API (void) kai_debug_close_file_writer(Kai_String_Writer* Writer);

#endif
#ifdef KAI_USE_CPP_API
#include <string>
#include <fstream>

namespace Kai {
    enum Result {
        Success = KAI_SUCCESS,
    };

    struct String : public Kai_str {
        String(const char* s): Kai_str(kai_str_from_cstring(s)) {}
        String(std::string s) {
			data = (Kai_u8*)s.data();
			count = (Kai_u32)s.size();
		}
		std::string string() {
			return std::string((char*)data, count);
		}
    };
	
#ifdef KAI_USE_MEMORY_API
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
#ifdef KAI_USE_DEBUG_API
		void print() {
			kai_debug_write_error(kai_debug_stdout_writer(), this);
		}
#endif
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

#ifdef KAI_IMPLEMENTATION

#define X(TYPE, NAME, ...) ;TYPE NAME = __VA_ARGS__;
	KAI__X_TYPE_INFO_DEFINITIONS
#undef X

KAI_API (Kai_vector3_u32) kai_version(void)
{
	return (Kai_vector3_u32) {
		.x = KAI_VERSION_MAJOR,
		.y = KAI_VERSION_MINOR,
		.z = KAI_VERSION_PATCH,
	};
}

KAI_API (Kai_str) kai_version_string(void)
{
	return KAI_STRING("Kai Compiler v" KAI_VERSION_STRING);
}

KAI_API (Kai_bool) kai_str_equals(Kai_str A, Kai_str B)
{
	if (A.count != B.count)
		return KAI_FALSE;
	for (Kai_int i = 0; i < A.count; ++i) {
		if (A.data[i] != B.data[i])
			return KAI_FALSE;
	}
	return KAI_TRUE;
}

KAI_API (Kai_str) kai_str_from_cstring(char const* String)
{
	Kai_u32 len = 0;
	while (String[len] != '\0') ++len;
	return (Kai_str) {.data = (Kai_u8*)String, .count = len};
}

KAI_API (void) kai_destroy_error(Kai_Error* Error, Kai_Allocator* allocator)
{
	while (Error) {
		Kai_Error* next = Error->next;
		if (Error->memory.size)
			kai__free(Error->memory.data, Error->memory.size);
		Error = next;
	}
}

static Kai_str const kai__result_string_map[KAI__RESULT_COUNT] = {
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
	while (line > 0) {
		if (*source++ == '\n') --line;
	}
	return (Kai_u8*)source;
}


// TODO: Audit code
static void kai__write_source_code(Kai_String_Writer* writer, Kai_u8* src, Kai_u8** end)
{
	while (*src != 0 && *src != '\n') {
		if (*src == '\t')
			kai__write_char(' ');
		else
			kai__write_char(*src);
		++src;
	}
	*end = src;
}

// TODO: Audit code
static uint32_t kai__utf8_decode(Kai_u8 **s) {
	const unsigned char *p = (const unsigned char *)*s;
	uint32_t cp = 0;
	size_t len = 0;

	if (p[0] < 0x80) {
		cp = p[0];
		len = 1;
	} else if ((p[0] & 0xE0) == 0xC0) {
		cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
		len = 2;
	} else if ((p[0] & 0xF0) == 0xE0) {
		cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
		len = 3;
	} else if ((p[0] & 0xF8) == 0xF0) {
		cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
			 ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
		len = 4;
	} else {
		// Invalid byte, skip
		cp = 0xFFFD;
		len = 1;
	}

	*s += len;
	return cp;
}

// TODO: Audit code
static int kai__unicode_char_width(Kai_String_Writer* writer, uint32_t cp, Kai_u8 first, Kai_u8 ch) {
	int count = 0;
	if (cp == 0) return 0;                   // NULL
	if (cp < 0x20 || (cp >= 0x7f && cp < 0xa0)) return 0; // Control
	if (cp >= 0x300 && cp <= 0x36F) return 0; // Combining marks
	if (
		(cp >= 0x1100 && cp <= 0x115F) ||
		(cp >= 0x2329 && cp <= 0x232A) ||
		(cp >= 0x2E80 && cp <= 0xA4CF) ||
		(cp >= 0xAC00 && cp <= 0xD7A3) ||
		(cp >= 0xF900 && cp <= 0xFAFF) ||
		(cp >= 0xFE10 && cp <= 0xFE19) ||
		(cp >= 0xFE30 && cp <= 0xFE6F) ||
		(cp >= 0xFF00 && cp <= 0xFF60) ||
		(cp >= 0xFFE0 && cp <= 0xFFE6) ||
		(cp >= 0x1F300 && cp <= 0x1FAFF)
	) {
		kai__write_fill(first, 1);
		first = ch;
		count += 1;
	}

	kai__write_fill(first, 1);
	first = ch;
	return count + 1;
}

// TODO: Audit code
static void kai__write_source_code_fill(Kai_String_Writer* writer, Kai_u8* src, Kai_u8* end, Kai_u8 first, Kai_u8 ch)
{
	while (src < end && *src != 0 && *src != '\n') {
		uint32_t cp = kai__utf8_decode(&src);
		kai__unicode_char_width(writer, cp, first, ch);
		first = ch;
	}
}

KAI_API (void) kai_write_error(Kai_String_Writer* writer, Kai_Error* error)
{
    for (; error != NULL; error = error->next) {
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
    	Kai_u8* end = begin;

        kai__set_color(KAI_COLOR_PRIMARY);
        kai__write_source_code(writer, begin, &end);
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

KAI_API (void) kai_write_type(Kai_String_Writer* writer, Kai_Type Type)
{
	void* void_Type = Type;
	if (Type == NULL) {
		kai__write("[null]");
		return;
	}
	switch (Type->type) {
		default:                 { kai__write("[Unknown]"); } break;
		case KAI_TYPE_TYPE:      { kai__write("Type");      } break;
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
		case KAI_TYPE_POINTER:   { kai__write("Pointer");   } break;
		case KAI_TYPE_PROCEDURE: {
			Kai_Type_Info_Procedure* info = void_Type;
			kai__write("(");
			for (int i = 0;;) {
				kai_write_type(writer, info->sub_types[i]);
				if (++i < info->in_count)
				{
					kai__write(", ");
				}
				else break;
			}
			kai__write(") -> (");
			for (int i = 0;;) {
				kai_write_type(writer, info->sub_types[info->in_count+i]);
				if (++i < info->out_count)
				{
					kai__write(", ");
				}
				else break;
			}
			kai__write(")");
		} break;
		case KAI_TYPE_STRUCT:    { kai__write("Struct");    } break;
	}
}

static Kai_str kai__tree_branches[4] = {
    KAI_CONSTANT_STRING(KAI_UTF8("\u2503   ")),
    KAI_CONSTANT_STRING(KAI_UTF8("\u2523\u2501\u2501 ")),
    KAI_CONSTANT_STRING(KAI_UTF8("    ")),
    KAI_CONSTANT_STRING(KAI_UTF8("\u2517\u2501\u2501 ")),
};

static Kai_str kai__binary_operator_name(Kai_u32 op)
{
	switch (op) {
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

static Kai_str kai__unary_operator_name(Kai_u32 op)
{
	switch (op) {
		case '-':  return KAI_STRING("negate");
		case '*':  return KAI_STRING("pointer to");
		case '[':  return KAI_STRING("array");
		default:   return KAI_EMPTY_STRING;
	}
}

typedef struct {
	Kai_String_Writer * writer;
	Kai_u64             stack[256]; // 64 * 256 max depth
	Kai_u32             stack_count;
	char const*         prefix;
} Kai__Tree_Traversal_Context;

#define kai__push_traversal_stack(B)                  \
    do { Kai_u32 i = Context->stack_count++;          \
         kai__assert(i < sizeof(Context->stack)*8);   \
         Kai_u64 mask = 1llu << (i%64);               \
         if (B) Context->stack[i/64] |= mask;         \
         else   Context->stack[i/64] &=~ mask;        \
    } while (0)

#define kai__pop_traversal_stack()                \
    do { kai__assert(Context->stack_count != 0);  \
		 Context->stack_count -= 1;               \
	} while (0)

#define kai__explore(EXPR, IS_LAST) kai__traverse_tree(Context, EXPR, IS_LAST, KAI_TRUE)
static void kai__traverse_tree(Kai__Tree_Traversal_Context* Context, Kai_Expr Expr, Kai_u8 is_last, Kai_bool push) {
    Kai_String_Writer* writer = Context->writer;

    if (push) kai__push_traversal_stack(is_last);

    kai__set_color(KAI_COLOR_DECORATION);
    Kai_int last = Context->stack_count - 1;
    kai__for_n (Context->stack_count) {
    	Kai_u32 k = (((Context->stack[i/64] >> i%64) & 1) << 1) | (i == last);
        kai__write_string(kai__tree_branches[k]);
    }

    if (Context->prefix) {
        kai__set_color(KAI_COLOR_IMPORTANT_2);
        kai__write(Context->prefix);
        kai__write_char(' ');
        Context->prefix = NULL;
    }
    if (Expr == NULL) {
        kai__set_color(KAI_COLOR_IMPORTANT_2);
        kai__write("null\n");
        kai__set_color(KAI_COLOR_PRIMARY);
        kai__pop_traversal_stack();
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
                Context->prefix = "in ";
                kai__explore(current, i == end);
                current = current->next;
            }

            kai__for_n (node->out_count) {
                Context->prefix = "out";
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

            Context->prefix = "proc";
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
                Context->prefix = "in ";
                kai__explore(current, 0);
                current = current->next;
            }

            kai__for_n (node->out_count) {
                Context->prefix = "out";
                kai__explore(current, 0);
                current = current->next;
            }

            kai__explore(node->body, 1);
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
                Context->prefix = "type";
                kai__explore(node->type, !has_expr);
            }

        	if (has_expr)
				kai__explore(node->expr, 1);
        }
        break; case KAI_STMT_ASSIGNMENT: {
            Kai_Stmt_Assignment* node = void_Expr;
            kai__set_color(KAI_COLOR_SECONDARY);
            kai__write("assignment\n");

            Context->prefix = "left ";
            kai__explore(node->left, 0);
            Context->prefix = "right";
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

            Context->prefix = "expr";
            kai__explore(node->expr, 0);
            kai__explore(node->body, node->else_body == NULL);
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

            Context->prefix = "from";
            kai__explore(node->from, 0);

            Context->prefix = "to";
            kai__explore(node->to, 0);

            kai__explore(node->body, 1);
        }
    }

    if (push) kai__pop_traversal_stack();
}

KAI_API (void) kai_write_syntax_tree(Kai_String_Writer* writer, Kai_Syntax_Tree* tree)
{
	Kai__Tree_Traversal_Context context = {
		.writer = writer,
	};
	kai__write("Top Level\n");
	kai__traverse_tree(&context, (Kai_Stmt)&tree->root, KAI_TRUE, KAI_TRUE);
}

KAI_API (void) kai_write_expression(Kai_String_Writer* writer, Kai_Expr expr)
{
	Kai__Tree_Traversal_Context context = {
		.writer = writer,
	};
	kai__traverse_tree(&context, expr, KAI_TRUE, KAI_FALSE);
}

//! @TODO: Handle parsing of values during compilation

static inline void parse_number_bin(Kai__Tokenizer* context, Kai_u64* n) {
    for(; context->cursor < context->source.count; ++context->cursor) {
        Kai_u8 ch = context->source.data[context->cursor];

        if( ch < '0' ) break;
        if( ch > '1' ) {
            if( ch == '_' ) continue;
            else break;
        }

        *n = *n << 1;
        *n += (Kai_u64)ch - '0';
    }
}

static inline void parse_number_dec(Kai__Tokenizer* context, Kai_u64* n) {
    for(; context->cursor < context->source.count; ++context->cursor) {
        Kai_u8 ch = context->source.data[context->cursor];

        if( ch < '0' ) break;
        if( ch > '9' ) {
            if( ch == '_' ) continue;
            else break;
        }

        *n = *n * 10;
        *n += (Kai_u64)ch - '0';
    }
}

static inline void parse_number_hex(Kai__Tokenizer* context, Kai_u64* n) {
    // TODO: just use switch
    enum {
        X = 16, /* BREAK */ S = 17, /* SKIP */
        A = 0xA, B = 0xB, C = 0xC, D = 0xD, E = 0xE, F = 0xF,
    };
    Kai_u8 lookup_table[56] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, X, X, X, X, X, X,
        X, A, B, C, D, E, F, X, X, X, X, X, X, X, X, X,
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, S,
        X, A, B, C, D, E, F, X,
    };

    for(; context->cursor < context->source.count; ++context->cursor) {
        Kai_u8 ch = context->source.data[context->cursor];

        if( ch < '0' || ch > 'f' ) break;

        Kai_u8 what = lookup_table[ch - '0'];

        if( what == S ) continue;
        if( what == X ) break;

        *n = *n << 4;
        *n += what;
    }
}

#define KAI__CHECK(A,B)                               \
    if (context->source.data[context->cursor] == A) { \
        t->type = B;                                  \
        ++context->cursor;                            \
        ++t->string.count;                            \
        return;                                       \
    }

static inline void parse_multi_token(Kai__Tokenizer* context, Kai__Token* t, Kai_u8 current) {
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
    default: (void)0;
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

Kai_str keyword_map[] = {
#define X(NAME, ID) KAI_CONSTANT_STRING(#NAME),
    KAI__X_TOKEN_KEYWORDS
#undef X
};

static Kai_u8 kai__keyword_map[16] = {
    4, 3, 3, 3, 7, 10, 6, 6, 1, 11, 9, 8, 2, 0, 0, 5,
};
static inline int kai__hash_keyword(Kai_str s) {
    if (s.count < 2) return 6;
    return (s.count&3) | ((s.data[0]&2) << 2) | ((s.data[1]&2) << 1);
}

//! @TODO: String Escape \"
//! @TODO: Add "false" and "true" keywords ?
static inline Kai__Token generate_token(Kai__Tokenizer* context)
{
    Kai__Token token = (Kai__Token) {
        .type = KAI__TOKEN_END,
        .string = {.count = 1},
        .line_number = context->line_number,
    };

    while (context->cursor < context->source.count) {
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
            if (kai_str_equals(keyword_map[keyword_index], token.string)) {
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
            token.number.Whole_Part = ch - '0';

            // Integer
            if (token.number.Whole_Part == 0 && context->cursor < context->source.count) {
                if (context->source.data[context->cursor] == 'b') {
                    ++context->cursor;
                    parse_number_bin(context, &token.number.Whole_Part);
                    token.string.count = (Kai_u32)((Kai_uint)(context->source.data + context->cursor) - (Kai_uint)token.string.data);
                    return token;
                }
                if (context->source.data[context->cursor] == 'x') {
                    ++context->cursor;
                    parse_number_hex(context, &token.number.Whole_Part);
                    token.string.count = (Kai_u32)((Kai_uint)(context->source.data + context->cursor) - (Kai_uint)token.string.data);
                    return token;
                }
            }

            parse_number_dec(context, &token.number.Whole_Part);

        parse_fraction:
            // Fractional Part
            if (context->cursor < context->source.count && context->source.data[context->cursor] == '.' && !kai__next_character_equals('.')) {
                ++context->cursor;
                Kai_int start = context->cursor;
                parse_number_dec(context, &token.number.Frac_Part);
                token.number.Frac_Denom = (Kai_u16)(context->cursor - start);
            }

            // Parse Exponential Part
            if (context->cursor < context->source.count && (context->source.data[context->cursor] == 'e' || context->source.data[context->cursor] == 'E')) {
                Kai_u64 n = 0;
                Kai_s32 factor = 1;

                ++context->cursor;
                if (context->cursor < context->source.count) {
                    if (context->source.data[context->cursor] == '-') { ++context->cursor; factor = -1; }
                    if (context->source.data[context->cursor] == '+') ++context->cursor; // skip
                }

                parse_number_dec(context, &n);

                if( n > INT32_MAX ) token.number.Exp_Part = factor * INT32_MAX;
                else                token.number.Exp_Part = factor * (Kai_s32)n;
            }

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

Kai__Token* kai__next_token(Kai__Tokenizer* context) {
    if (!context->peeking) {
        context->current_token = generate_token(context);
        return &context->current_token;
    }
    context->peeking = KAI_FALSE;
    context->current_token = context->peeked_token;
    return &context->current_token;
}

Kai__Token* kai__peek_token(Kai__Tokenizer* context) {
    if (context->peeking) return &context->peeked_token;
    context->peeking = KAI_TRUE;
    context->peeked_token = generate_token(context);
    return &context->peeked_token;
}

typedef struct {
  Kai_u32 type;
  Kai_u32 class;
  Kai_s32 prec;
} Kai__Operator;

// TODO: refactor!!!
Kai_Expr kai__parse_expression(Kai__Parser* Parser, Kai_Expr Expr, Kai_u32 Precedence);
Kai_Expr kai__parse_type(Kai__Parser* Parser);
Kai_Stmt kai__parse_statement(Kai__Parser* Parser);
Kai_Stmt kai__parse_procedure(Kai__Parser* Parser);
Kai_Stmt kai__parse_declaration(Kai__Parser* Parser);
Kai_Stmt kai__parse_assignment(Kai__Parser* Parser);
Kai_bool kai__is_procedure_next(Kai__Parser* Parser);
Kai__Operator kai__token_type_to_operator(Kai_u32 Type);
#define kai__allocate_node(Type)

// Optimization Idea:
// 2 pass parser,
//   first pass to check for errors and calcuate memory required.
//   then allocate memory required.
//   second pass to store syntax tree.
// (also enables smaller syntax tree size, everything in
//    one memory allocation, so could use 32 bit pointers)
// Would make parser slower, but could
//   save space/be more cache friendly for later stages.
// .. But this would require a lot of duplicate code,
//     so not going to implement.
// NOTE: how would that even fit in with tree rewriting??

// Im somewhat of a scientist myself, what can I say?
#define _ID_Expr_Identifier KAI_EXPR_IDENTIFIER
#define _ID_Expr_String KAI_EXPR_STRING
#define _ID_Expr_Number KAI_EXPR_NUMBER
#define _ID_Expr_Binary KAI_EXPR_BINARY
#define _ID_Expr_Unary KAI_EXPR_UNARY
#define _ID_Expr_Procedure_Type KAI_EXPR_PROCEDURE_TYPE
#define _ID_Expr_Procedure_Call KAI_EXPR_PROCEDURE_CALL
#define _ID_Expr_Procedure KAI_EXPR_PROCEDURE
#define _ID_Expr_Code KAI_EXPR_CODE
#define _ID_Expr_Struct KAI_EXPR_STRUCT
#define _ID_Stmt_Return KAI_STMT_RETURN
#define _ID_Stmt_Declaration KAI_STMT_DECLARATION
#define _ID_Stmt_Assignment KAI_STMT_ASSIGNMENT
#define _ID_Stmt_Compound KAI_STMT_COMPOUND
#define _ID_Stmt_If KAI_STMT_IF
#define _ID_Stmt_For KAI_STMT_FOR
#define _ID_Stmt_Defer KAI_STMT_DEFER

#define p_alloc_node(TYPE) \
  (Kai_##TYPE*)alloc_type(parser, sizeof(Kai_##TYPE), _ID_##TYPE)

extern inline Kai_Expr alloc_type(Kai__Parser* p, Kai_u32 size, Kai_u8 id) {
  Kai_Expr expr = kai__arena_allocate(&p->arena, size);
  expr->id = id;
  return expr;
}

#define p_error_unexpected_s(WHERE, CONTEXT)                      \
  kai__error_unexpected(parser, &parser->tokenizer.current_token, \
                        KAI_STRING(WHERE), KAI_STRING(CONTEXT)),  \
      NULL

#define p_expect(OK, WHERE, CONTEXT) \
  if (!(OK))                         \
  return p_error_unexpected_s(WHERE, CONTEXT)

#define p_next() kai__next_token(&parser->tokenizer)
#define p_peek() kai__peek_token(&parser->tokenizer)
#define _parse_expr(PREC) parse_expression(parser, PREC)
#define _parse_stmt(TOP) parse_statement(parser, TOP)
#define _parse_proc() parse_procedure(parser)
#define _parse_type() parse_type(parser)

Kai_Expr parse_type(Kai__Parser* parser);
Kai_Expr parse_expression(Kai__Parser* parser, int precedence);
Kai_Expr parse_procedure(Kai__Parser* parser);
Kai_Stmt parse_statement(Kai__Parser* parser, Kai_bool is_top_level);

enum {
  OP_BINARY,
  OP_INDEX,
  OP_PROCEDURE_CALL,
};

typedef struct {
  Kai_u32 op;
  int prec;
  int type;
} Op_Info;

#define DEFAULT_PREC -6942069
#define CAST_PREC 0x0900
#define UNARY_PREC 0x1000

Op_Info operator_of_token_type(Kai_u32 t) {
  switch (t) {
    case '&&':
      return (Op_Info){t, 0x0010, OP_BINARY};
    case '||':
      return (Op_Info){t, 0x0010, OP_BINARY};
    case '==':
      return (Op_Info){t, 0x0040, OP_BINARY};
    case '<=':
      return (Op_Info){t, 0x0040, OP_BINARY};
    case '>=':
      return (Op_Info){t, 0x0040, OP_BINARY};
    case '<':
      return (Op_Info){t, 0x0040, OP_BINARY};
    case '>':
      return (Op_Info){t, 0x0040, OP_BINARY};
    case '->':
      return (Op_Info){t, CAST_PREC, OP_BINARY};
    case '+':
      return (Op_Info){t, 0x0100, OP_BINARY};
    case '-':
      return (Op_Info){t, 0x0100, OP_BINARY};
    case '*':
      return (Op_Info){t, 0x0200, OP_BINARY};
    case '/':
      return (Op_Info){t, 0x0200, OP_BINARY};
    case '[':
      return (Op_Info){t, 0xBEEF, OP_INDEX};
    case '(':
      return (Op_Info){t, 0xBEEF, OP_PROCEDURE_CALL};
    case '.':
      return (Op_Info){t, 0xFFFF, OP_BINARY};  // member access
    default:
      return (Op_Info){t, 0};
  }
}

Kai_bool is_procedurep_next(Kai__Parser* parser) {
  Kai__Token* p = p_peek();
  if (p->type == ')')
    return KAI_TRUE;
  Kai__Tokenizer state = parser->tokenizer;
  Kai__Token* cur = p_next();
  Kai_bool found = KAI_FALSE;

  // go until we hit ')' or END, searching for ':'
  while (cur->type != KAI__TOKEN_END && cur->type != ')') {
    if (cur->type == ':') {
      found = KAI_TRUE;
      break;
    }
    p_next();
  }
  parser->tokenizer = state;
  return found;
}

// TODO: handle more types than just function pls (different from parse
// expression?)
Kai_Expr parse_type(Kai__Parser* parser) {
  Kai__Token* t = &parser->tokenizer.current_token;
  if (t->type == '[') {
    p_next();  // get token after

    p_expect(t->type == ']', "[todo]", "array");

    p_next();
    Kai_Expr type = _parse_type();
    p_expect(type, "[todo]", "type");

    Kai_Expr_Unary* unary = p_alloc_node(Expr_Unary);
    unary->expr = type;
    unary->op = '[';
    return (Kai_Expr)unary;
  }

  if (t->type != '(')
    return _parse_expr(DEFAULT_PREC);

  p_next();  // get next after parenthesis

  Kai_u8 in_count = 0;
  Kai_u8 out_count = 0;
  Kai_Expr current = NULL;
  Kai_Expr head = NULL;

  // Parse Procedure input types
  if (t->type != ')')
    for (;;) {
      Kai_Expr type = _parse_type();
      if (!type)
        return NULL;

      if (!head)
        head = type;
      else
        current->next = type;
      current = type;

      p_expect(in_count != 255, "in procedure type",
               "too many inputs to procedure");
      in_count += 1;

      p_next();  // get ',' token or ')'

      if (t->type == ')')
        break;
      if (t->type == ',')
        p_next();
      else
        return p_error_unexpected_s("in procedure type",
                                    "',' or ')' expected here");
    }

  Kai__Token* peek = p_peek();  // see if we have any returns

  ///////////////////////////////////////////////////////////////////////
  // "This code is broken as hell."
  if (peek->type == '->') {
    p_next();  // eat '->'
    p_next();  // get token after

    Kai_bool enclosed_return = KAI_FALSE;
    if (t->type == '(') {
      enclosed_return = KAI_TRUE;
      p_next();
    }

    for (;;) {
      Kai_Expr type = _parse_type();
      if (!type)
        return NULL;

      if (!head)
        head = type;
      else
        current->next = type;
      current = type;

      p_expect(out_count != 255, "in procedure type",
               "too many inputs to procedure");
      out_count += 1;

      p_peek();  // get ',' token or something else

      if (peek->type == ',') {
        p_next();  // eat ','
        p_next();  // get token after
      } else
        break;
    }

    if (enclosed_return) {
      if (peek->type != ')') {
        p_next();
        return p_error_unexpected_s("in procedure type",
                                    "should be ')' after return types");
      } else
        p_next();
    }
  }
  ///////////////////////////////////////////////////////////////////////

  Kai_Expr_Procedure_Type* proc = p_alloc_node(Expr_Procedure_Type);
  proc->in_out_expr = head;
  proc->in_count = in_count;
  proc->out_count = out_count;
  return (Kai_Expr)proc;
}

Kai_Expr parse_expression(Kai__Parser* parser, int prec) {
  Kai_Expr left = NULL;
  Kai__Token* t = &parser->tokenizer.current_token;

  switch (t->type) {
    ////////////////////////////////////////////////////////////
    // Handle Parenthesis
    case '(': {
      p_next();
      left = _parse_expr(DEFAULT_PREC);
      p_expect(left, "in expression", "should be an expression here");
      p_next();
      p_expect(t->type == ')', "in expression",
               "an operator or ')' in expression");
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Unary Operators
    case '-':
    case '+':
    case '*':
    case '/': {
      Kai_u32 op = t->type;
      p_next();
      left = _parse_expr(UNARY_PREC);
      p_expect(left, "in unary expression", "should be an expression here");

      Kai_Expr_Unary* unary = p_alloc_node(Expr_Unary);
      unary->expr = left;
      unary->op = op;
      left = (Kai_Expr)unary;
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Explicit Casting "cast(int) x"
    case KAI__TOKEN_cast: {
      p_next();
      p_expect(t->type == '(', "in expression", "'(' after cast keyword");
      p_next();
      Kai_Expr type = _parse_type();
      p_expect(type, "in expression", "type");
      p_next();
      p_expect(t->type == ')', "in expression",
               "')' after Type in cast expression");
      p_next();
      Kai_Expr expr = _parse_expr(CAST_PREC);
      p_expect(expr, "in expression", "expression after cast");
      Kai_Expr_Binary* binary = p_alloc_node(Expr_Binary);
      binary->op = '->';
      binary->left = expr;
      binary->right = type;
      left = (Kai_Expr)binary;
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Directives
    case KAI__TOKEN_DIRECTIVE: {
      if (kai_str_equals(KAI_STRING("type"), t->string)) {
        p_next();
        left = _parse_type();
        p_expect(left, "in expression", "type");
      } else if (kai_str_equals(KAI_STRING("string"), t->string)) {
        // p_next();
        // tok_start_raw_string(&parser->tokenizer);
        // p_next();
        // tok_end_raw_string(&parser->tokenizer);
        // Kai_Expr_String* str = p_alloc_node(Expr_String);
        // str->line_number = t->line_number;
        // str->source_code = t->string;
        // left = (Kai_Expr) str;
        return p_error_unexpected_s("", "not implemented");
      } else if (kai_str_equals(KAI_STRING("Julie"), t->string)) {
        Kai_Expr_String* str = p_alloc_node(Expr_String);
        str->line_number = t->line_number;
        str->source_code = KAI_STRING("<3");
        left = (Kai_Expr)str;
      } else
        return NULL;
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Struct
    case KAI__TOKEN_struct: {
      p_next();
      left = _parse_stmt(KAI_FALSE);
      p_expect(left, "todo", "todo");
      return left;
    } break;
    ////////////////////////////////////////////////////////////
    // Handle Single-Token Expressions
    case KAI__TOKEN_IDENTIFIER: {
      Kai_Expr_Identifier* ident = p_alloc_node(Expr_Identifier);
      ident->line_number = t->line_number;
      ident->source_code = t->string;
      left = (Kai_Expr)ident;
    } break;
    case KAI__TOKEN_NUMBER: {
      Kai_Expr_Number* num = p_alloc_node(Expr_Number);
      num->value = t->number;
      num->line_number = t->line_number;
      num->source_code = t->string;
      left = (Kai_Expr)num;
    } break;
    case KAI__TOKEN_STRING: {
      Kai_Expr_String* str = p_alloc_node(Expr_String);
      str->line_number = t->line_number;
      str->source_code = t->string;
      left = (Kai_Expr)str;
    } break;
    default:
      return NULL;
  }

loop_back:
  (void)0;
  Kai__Token* p = p_peek();
  Op_Info op_info = operator_of_token_type(p->type);

  // handle precedence by returning early
  if (!op_info.prec || op_info.prec <= prec)
    return left;

  p_next();
  p_next();

  switch (op_info.type) {
    case OP_BINARY: {
      Kai_Expr right = _parse_expr(op_info.prec);
      p_expect(right, "in binary expression",
               "should be an expression after binary operator");
      Kai_Expr_Binary* binary = p_alloc_node(Expr_Binary);
      binary->op = op_info.op;
      binary->left = left;
      binary->right = right;
      left = (Kai_Expr)binary;
    } break;

    case OP_INDEX: {
      Kai_Expr right = _parse_expr(DEFAULT_PREC);
      p_expect(right, "in index operation", "should be an expression here");
      p_next();
      p_expect(t->type == ']', "in index operation", "expected ']' here");
      Kai_Expr_Binary* binary = p_alloc_node(Expr_Binary);
      binary->op = op_info.op;
      binary->left = left;
      binary->right = right;
      left = (Kai_Expr)binary;
    } break;

    case OP_PROCEDURE_CALL: {
      Kai_Expr head = NULL;
      Kai_Expr current = NULL;
      Kai_u8 arg_count = 0;
      if (t->type != ')')
        for (;;) {
          Kai_Expr expr = _parse_expr(DEFAULT_PREC);
          p_expect(expr, "[todo]", "an expression");

          if (!head)
            head = expr;
          else
            current->next = expr;
          current = expr;

          p_expect(arg_count != 255, "in procedure call",
                   "too many inputs to procedure");
          arg_count += 1;

          p_next();
          if (t->type == ')')
            break;
          if (t->type == ',')
            p_next();
          else
            return p_error_unexpected_s("in procedure call",
                                        "',' or ')' expected here");
        }
      Kai_Expr_Procedure_Call* call = p_alloc_node(Expr_Procedure_Call);
      call->proc = left;
      call->arg_head = head;
      call->arg_count = arg_count;
      left = (Kai_Expr)call;
    } break;

    default:
      kai__unreachable();
  }
  goto loop_back;
}

Kai_Expr parse_procedure(Kai__Parser* parser) {
  Kai__Token* t = &parser->tokenizer.current_token;
  // sanity check
  p_expect(t->type == '(', "", "this is likely a compiler bug, sorry :c");
  p_next();

  Kai_u8 in_count = 0;
  Kai_u8 out_count = 0;
  Kai_Expr head = NULL;
  Kai_Expr current = NULL;

  if (t->type != ')') {
  parse_parameter:
    (void)0;
    Kai_u8 flags = 0;
    if (t->type == KAI__TOKEN_using) {
      flags |= 1;  // KAI_PARAMETER_FLAG_USING;
      p_next();
    }
    p_expect(t->type == KAI__TOKEN_IDENTIFIER, "in procedure input",
             "should be an identifier");
    Kai_str name = t->string;
    p_next();
    p_expect(t->type == ':', "in procedure input", "wanted a ':' here");
    p_next();
    Kai_Expr type = _parse_type();
    p_expect(type, "in procedure input", "should be type");

    type->name = name;
    type->flags = flags;

    if (!head)
      head = type;
    else
      current->next = type;
    current = type;

    p_expect(in_count != 255, "in procedure call",
             "too many inputs to procedure");
    in_count += 1;

    p_next();

    switch (t->type) {
      case ')':
        break;
      case ',':
        p_next();
        goto parse_parameter;
      default:
        return p_error_unexpected_s("in procedure input",
                                    "wanted ')' or ',' here");
    }
  }
  p_next();

  // return value
  if (t->type == '->') {
    p_next();
    Kai_Expr type = _parse_type();
    p_expect(type, "in procedure return type", "should be type");

    if (!head)
      head = type;
    else
      current->next = type;
    current = type;

    p_expect(out_count != 255, "in procedure call",
             "too many inputs to procedure");
    out_count += 1;

    p_next();
  }
  Kai_Expr_Procedure* proc = p_alloc_node(Expr_Procedure);
  proc->in_out_expr = head;
  proc->in_count = in_count;
  proc->out_count = out_count;

  Kai_Stmt body = NULL;
  if (t->type == KAI__TOKEN_DIRECTIVE &&
      kai_str_equals(KAI_STRING("native"), t->string)) {
    p_next();
    p_expect(t->type == ';', "???", "???");
  } else {
    body = _parse_stmt(KAI_FALSE);
    if (!body)
      return NULL;
  }
  proc->body = body;
  return (Kai_Expr)proc;
}

// TODO: seperate parser for top level statements??
Kai_Expr parse_statement(Kai__Parser* parser, Kai_bool is_top_level) {
  Kai__Token* t = &parser->tokenizer.current_token;
  switch (t->type) {
    case KAI__TOKEN_END:
      if (is_top_level)
        return NULL;
    default: {
    parse_expression_statement:
      if (is_top_level) {
        return p_error_unexpected_s("in top level",
                                    "should be a top level statement");
      }
      Kai_Expr expr = _parse_expr(DEFAULT_PREC);
      p_expect(expr, "in statement", "should be an expression or statement");
      p_next();
      if (t->type == '=') {
        p_next();
        Kai_Expr right = _parse_expr(DEFAULT_PREC);
        p_expect(right, "in assignment statement", "should be an expression");
        Kai_Stmt_Assignment* assignment = p_alloc_node(Stmt_Assignment);
        assignment->left = expr;
        assignment->expr = right;
        expr = (Kai_Expr)assignment;
        p_next();
      }
      p_expect(t->type == ';', "in expression statement",
               "should be ';' after expression");
      return expr;
    }

    case '{': {
      if (is_top_level)
        return p_error_unexpected_s("in top level statement",
                                    "a top level statement");

      p_next();

      Kai_Stmt head = NULL;
      Kai_Stmt current = NULL;

      // parse statements until we get a '}'
      while (t->type != '}') {
        Kai_Stmt statement = _parse_stmt(KAI_FALSE);
        if (!statement)
          return NULL;

        if (!head)
          head = statement;
        else
          current->next = statement;
        current = statement;

        p_next();  // eat ';' (get token after)
      }

      // TODO: is this OK?
      // No need for compound if there is only one statement
      // if (statement_array.count == 1) {
      //    p_tarray_destroy(statement_array);
      //    return *(Kai_Stmt*)((Kai_u8*)parser->memory.temperary +
      //    statement_array.offset);
      //}

      Kai_Stmt_Compound* compound = p_alloc_node(Stmt_Compound);
      compound->head = head;
      return (Kai_Stmt)compound;
    }

    case KAI__TOKEN_ret: {
      if (is_top_level)
        return p_error_unexpected_s("in top level statement",
                                    "a top level statement");
      p_next();

      //! @TODO: fix this [ret statement]
      if (t->type == ';') {
        Kai_Stmt_Return* ret = p_alloc_node(Stmt_Return);
        ret->expr = NULL;
        return (Kai_Stmt)ret;
      }

      Kai_Expr expr = _parse_expr(DEFAULT_PREC);
      p_expect(expr, "in return statement", "should be an expression");
      p_next();
      p_expect(t->type == ';', "after statement",
               "there should be a ';' before this");
      Kai_Stmt_Return* ret = p_alloc_node(Stmt_Return);
      ret->expr = expr;
      return (Kai_Stmt)ret;
    }

    case KAI__TOKEN_if: {
      if (is_top_level)
        return p_error_unexpected_s("is top level statement",
                                    "should be within a procedure");
      p_next();
      Kai_Expr expr = _parse_expr(DEFAULT_PREC);
      p_expect(expr, "in if statement", "should be an expression here");
      p_next();
      Kai_Stmt body = _parse_stmt(KAI_FALSE);
      if (!body)
        return NULL;

      // parse "else"
      Kai__Token* p = p_peek();
      Kai_Stmt else_body = NULL;
      if (p->type == KAI__TOKEN_else) {
        p_next();
        p_next();
        else_body = _parse_stmt(KAI_FALSE);
        if (!else_body)
          return NULL;
      }
      Kai_Stmt_If* if_statement = p_alloc_node(Stmt_If);
      if_statement->expr = expr;
      if_statement->body = body;
      if_statement->else_body = else_body;
      return (Kai_Stmt)if_statement;
    }

    case KAI__TOKEN_for: {
      if (is_top_level)
        return p_error_unexpected_s("in top level statement",
                                    "should be within a procedure");
      p_next();
      p_expect(t->type == KAI__TOKEN_IDENTIFIER, "in for statement",
               "should be the name of the iterator");
      Kai_str iterator_name = t->string;
      p_next();
      p_expect(t->type == ':', "in for statement", "should be ':' here");
      p_next();
      Kai_Expr from = _parse_expr(DEFAULT_PREC);
      p_expect(from, "in for statement", "should be an expression here");
      Kai_Expr to = NULL;
      p_next();
      if (t->type == '..') {
        p_next();
        to = _parse_expr(DEFAULT_PREC);
        p_expect(to, "in for statement", "should be an expression here");
        p_next();
      }
      Kai_Stmt body = _parse_stmt(KAI_FALSE);
      if (!body)
        return NULL;

      Kai_Stmt_For* for_statement = p_alloc_node(Stmt_For);
      for_statement->body = body;
      for_statement->from = from;
      for_statement->to = to;
      for_statement->iterator_name = iterator_name;
      for_statement->flags = 0;
      return (Kai_Stmt)for_statement;
    }

    case KAI__TOKEN_IDENTIFIER: {
      Kai__Token* p = p_peek();
      // just an expression?
      if (p->type != ':')
        goto parse_expression_statement;

      Kai_str name = t->string;
      Kai_u32 line_number = t->line_number;
      p_next();  // current = peeked
      p_next();  // see what is after current

      Kai_u8 flags = 0;
      Kai_Expr type = _parse_type();
      if (type)
        p_next();

      Kai_Expr expr = NULL;

      switch (t->type) {
        case ':':
          flags |= KAI_DECL_FLAG_CONST;
          break;
        case '=':
          break;
        case ';':
          goto done_declaration;
        default:
          return p_error_unexpected_s("in declaration",
                                      "should be '=', ':', or ';'");
      }
      p_next();

      Kai_bool require_semicolon = KAI_FALSE;
      if (t->type == '(' && is_procedurep_next(parser)) {
        expr = _parse_proc();
        if (!expr)
          return NULL;
      } else {
        require_semicolon = KAI_TRUE;
        if (t->type == KAI__TOKEN_struct)
          require_semicolon = KAI_FALSE;
        expr = _parse_expr(DEFAULT_PREC);
        p_expect(expr, "in declaration", "should be an expression here");
      }
      if (require_semicolon) {
        p_next();
        p_expect(t->type == ';', "after statement",
                 "there should be a ';' before this");
      }

    done_declaration: {
      Kai_Stmt_Declaration* decl = p_alloc_node(Stmt_Declaration);
      decl->expr = expr;
      decl->type = type;
      decl->name = name;
      decl->flags = flags;
      decl->line_number = line_number;
      return (Kai_Stmt)decl;
    }
    }
  }
}

// TODO: better API for linked lists ??
#define KAI__LINKED_LIST(TYPE) \
  struct {                     \
    TYPE head;                 \
    TYPE current;              \
  }
#define kai__linked_list_append(List, Ptr) \
  if (List.head == NULL)                   \
    List.head = Ptr;                       \
  else                                     \
    List.current->next = Ptr;              \
  List.current = Ptr

Kai_Result kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* info,
                                  Kai_Syntax_Tree* tree) {
  Kai__Parser p = {
      .tokenizer =
          {
              .source = info->source_code,
              .cursor = 0,
              .line_number = 1,
          },
  };
  KAI__LINKED_LIST(Kai_Stmt) statements = {0};
  kai__dynamic_arena_allocator_create(&p.arena, &info->allocator);

  Kai__Token* token = kai__next_token(&p.tokenizer);
  while (token->type != KAI__TOKEN_END) {
    Kai_Stmt statement = parse_statement(&p, KAI_TRUE);
    if (statement == NULL)
      break;
    kai__linked_list_append(statements, statement);
    kai__next_token(&p.tokenizer);
  }

  tree->allocator = p.arena;
  tree->root.id = KAI_STMT_COMPOUND;
  tree->root.head = statements.head;

  // TODO: parser holds error pointer
  if (p.error.result != KAI_SUCCESS) {
    if (info->error) {
      *info->error = p.error;
      info->error->location.file_name = tree->source_filename;
      info->error->location.source = info->source_code.data;
    }
    return p.error.result;
  }
  return KAI_SUCCESS;
}

void kai_destroy_syntax_tree(Kai_Syntax_Tree* tree) {
  kai__dynamic_arena_allocator_destroy(&tree->allocator);
  *tree = (Kai_Syntax_Tree){0};
}

// TODO: remove these + rename bcs__
#define __debug_log(...) (void)0
#define bcs__for(N) for (int i = 0; i < (int)N; ++i)
#define __type_to_size(T) ((T) >> 4)

extern char const* bc__op_to_name [100];

uint32_t bcs__grow_function(uint32_t x)
{
    return x + x / 2;
}

// TODO: no reason why this should be specific to Bytecode Arrays
Kai_Result bcs__ensure_space(Kai_BC_Stream* stream, uint32_t added_count)
{
    uint32_t required_capacity = stream->count + added_count;
    
    if (stream->capacity >= required_capacity)
        return KAI_SUCCESS;
    
    uint32_t new_capacity = bcs__grow_function(required_capacity);

    Kai_Allocator* allocator = stream->allocator;
    void* new_data = kai__allocate(stream->data, new_capacity, stream->capacity);

    if (new_data == NULL)
        return KAI_BC_ERROR_MEMORY;

    stream->data = new_data;
    stream->capacity = new_capacity;
    return KAI_SUCCESS;
}

#define bcs__make_space(BYTES_REQUIRED)            \
    if (bcs__ensure_space(stream, BYTES_REQUIRED)) \
        return KAI_BC_ERROR_MEMORY

#define bcs__push_u8(stream, value) stream->data[stream->count++] = value;
#define bcs__push_u32(stream, value) *(Kai_u32*)(stream->data + stream->count) = value, stream->count += 4
#define bcs__push_address(stream, value) *(uintptr_t*)(stream->data + stream->count) = (uintptr_t)value, stream->count += sizeof(uintptr_t)

static void bcs__push_value(Kai_BC_Stream* stream, Kai_u8 type, Kai_Value value)
{
    uint32_t size = __type_to_size(type);
    kai__memory_copy(stream->data + stream->count, &value, size);
    stream->count += size;
}

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


// ---------------------------------------------------------------------
//  LOAD_CONSTANT
// >> [DST]:R [TYPE]:8 [VALUE]:$
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_load_constant(Kai_BC_Stream* stream, Kai_u8 type, Kai_Reg reg_dst, Kai_Value value)
{
    bcs__make_space(2 + sizeof(Kai_Reg) + __type_to_size(type));
    bcs__push_u8(stream, KAI_BOP_LOAD_CONSTANT);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_value(stream, type, value);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  ADD, SUB, MUL, DIV, ... with register
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [LEFT]:R [RIGHT]:R
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_math(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 3);
    bcs__push_u8(stream, operation);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_src1);
    bcs__push_u32(stream, reg_src2);
    __debug_log("math %%%i <- %%%i, %%%i\n", reg_dst, reg_src1, reg_src2);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  ADD, SUB, MUL, DIV, ... with value 
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [LEFT]:R [VALUE]:$
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_math_value(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 2 + __type_to_size(type));
    bcs__push_u8(stream, operation);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, 0x80 | type);
    bcs__push_u32(stream, reg_src1);
    bcs__push_value(stream, type, value);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  COMPARE
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [COMPARISON]:8 [LEFT]:R [RIGHT]:R
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_compare(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2)
{
    bcs__make_space(3 + sizeof(Kai_Reg) * 3);
    bcs__push_u8(stream, KAI_BOP_COMPARE);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u8(stream, comparison);
    bcs__push_u32(stream, reg_src1);
    bcs__push_u32(stream, reg_src2);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  COMPARE
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [COMPARISON]:8 [LEFT]:R [VALUE]:$
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_compare_value(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value)
{
    bcs__make_space(3 + sizeof(Kai_Reg) * 2 + __type_to_size(type));
    bcs__push_u8(stream, KAI_BOP_COMPARE);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, 0x80 | type);
    bcs__push_u8(stream, comparison);
    bcs__push_u32(stream, reg_src1);
    bcs__push_value(stream, type, value);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  BRANCH
// >> [LOCATION]:32 [SRC]:R
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_branch_location(Kai_BC_Stream* stream, uint32_t location, Kai_Reg reg_src)
{
    bcs__make_space(1 + sizeof(Kai_Reg) * 2);
    bcs__push_u8(stream, KAI_BOP_BRANCH);
    bcs__push_u32(stream, location);
    bcs__push_u32(stream, reg_src);
    return KAI_SUCCESS;
}
Kai_Result kai_bc_insert_branch(Kai_BC_Stream* stream, uint32_t* branch, Kai_Reg reg_src)
{
    bcs__make_space(1 + sizeof(Kai_Reg) * 2);
    bcs__push_u8(stream, KAI_BOP_BRANCH);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    bcs__push_u32(stream, reg_src);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  JUMP
// >> [LOCATION]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_jump_location(Kai_BC_Stream* stream, uint32_t location)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_JUMP);
    bcs__push_u32(stream, location);
    return KAI_SUCCESS;
}
Kai_Result kai_bc_insert_jump(Kai_BC_Stream* stream, uint32_t* branch)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_JUMP);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  CALL
// >> [LOCATION]:32 [RET_COUNT]:8 [ARG_COUNT]:8 ([DST]:R ...) ([SRC]:R ...)
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_call(Kai_BC_Stream* stream, uint32_t* branch, Kai_u8 ret_count, Kai_Reg* reg_ret, Kai_u8 arg_count, Kai_Reg* reg_arg)
{
    bcs__make_space(3 + sizeof(uint32_t) + sizeof(Kai_Reg) * (ret_count + arg_count));
    bcs__push_u8(stream, KAI_BOP_CALL);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    bcs__push_u8(stream, ret_count);
    bcs__push_u8(stream, arg_count);
    bcs__for (ret_count) bcs__push_u32(stream, reg_ret[i]);
    bcs__for (arg_count) bcs__push_u32(stream, reg_arg[i]);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  RETURN
// >> [COUNT]:8 ([SRC]:R ...)
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_return(Kai_BC_Stream* stream, Kai_u8 count, Kai_Reg* regs)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * count);
    bcs__push_u8(stream, KAI_BOP_RETURN);
    bcs__push_u8(stream, count);
    bcs__for (count) bcs__push_u32(stream, regs[i]);
    __debug_log("ret %%%i\n", regs[0]);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  NATIVE_CALL
// >> [ADDRESS]:S [USE_DST]:8 ([DST]:R) [COUNT]:8 ([SRC]:R ...)
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_native_call(Kai_BC_Stream* stream, Kai_u8 use_dst, Kai_Reg reg_dst, Kai_Native_Procedure* proc, Kai_Reg* reg_src)
{
    bcs__make_space(3 + sizeof(uintptr_t) + (use_dst? sizeof(Kai_Reg):0) + sizeof(Kai_Reg) * proc->input_count);
    bcs__push_u8(stream, KAI_BOP_NATIVE_CALL);
    bcs__push_address(stream, proc->address);
    bcs__push_u8(stream, use_dst? 1:0);
    if (use_dst) bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, proc->input_count);
    bcs__for (proc->input_count) bcs__push_u32(stream, reg_src[i]);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  LOAD, STORE
// >> [DST_OR_SRC]:R [TYPE]:8 [ADDR]:R [OFFSET]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_load(Kai_BC_Stream* stream, Kai_Reg reg_dst, Kai_u8 type, Kai_Reg reg_addr, uint32_t offset)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 2 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_LOAD);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_addr);
    bcs__push_u32(stream, offset);
    return KAI_SUCCESS;
}
Kai_Result kai_bc_insert_store(Kai_BC_Stream* stream, Kai_Reg reg_src, Kai_u8 type, Kai_Reg reg_addr, uint32_t offset)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 2 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_STORE);
    bcs__push_u32(stream, reg_src);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_addr);
    bcs__push_u32(stream, offset);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  STACK_ALLOC (type of R = u32)
// >> [DST]:R [SIZE]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_stack_alloc(Kai_BC_Stream* stream, Kai_Reg reg_dst, uint32_t size)
{
    bcs__make_space(1 + sizeof(Kai_Reg) + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_STACK_ALLOC);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u32(stream, size);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  STACK_FREE
// >> [SIZE]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_stack_free(Kai_BC_Stream* stream, uint32_t size)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_STACK_FREE);
    bcs__push_u32(stream, size);
    return KAI_SUCCESS;
}

Kai_Result kai_bc_set_branch(Kai_BC_Stream* stream, uint32_t branch, uint32_t location)
{
    if (branch >= stream->count) return KAI_BC_ERROR_BRANCH;
    *(uint32_t*)(stream->data + branch) = location;
    return KAI_SUCCESS;
}

Kai_str __math_op_to_string(Kai_u8 op)
{
    switch (op)
    {
        case KAI_BOP_ADD: return KAI_STRING("+");
        case KAI_BOP_MUL: return KAI_STRING("*");
        case KAI_BOP_DIV: return KAI_STRING("/");
        case KAI_BOP_SUB: return KAI_STRING("-");
        default:          return KAI_STRING("?");
    }
}

Kai_Result kai_bytecode_to_c(Kai_Bytecode* bytecode, Kai_String_Writer* writer)
{
    //! TODO: use type info
    //! TODO: find all branch locations
    //! TODO: use procedure name and argument count

//    #define bcc__write(...) \
//    { \
//        int length = snprintf(temp_buffer, sizeof(temp_buffer), __VA_ARGS__); \
//        writer->write_string(writer->user, (Kai_str) {.data = (Kai_u8*)temp_buffer, .count = length}); \
//    } (void)0

    #define bcc__indent() kai__write("    ")
    #define bcc__branch_check() \
    bcs__for (bytecode->branch_count) { \
        if (cursor == bytecode->branch_hints[i]) { \
            kai__write("__loc_"); \
            kai__write_u32(i); \
            kai__write(":\n"); \
            break; \
        } \
    }    

    Kai_u8 const* data = bytecode->data;
    uint32_t cursor = bytecode->range.offset;

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

    while (cursor < bytecode->count) {
        bcc__branch_check();

        Kai_u8 operation = data[cursor++];

        //if (operation < 100) {
        //    bc__debug_log("decode %s", bc__op_to_name[operation]);
        //}

        switch (operation)
        {
        case KAI_BOP_LOAD_CONSTANT: {
            Kai_Reg dst;
            dst = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            Kai_u8 type;
            type = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_Value value = {0};
            value = *(Kai_Value*)(data + cursor);
            cursor += __type_to_size(type);

            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = ");
            kai__write_u32(value.s32);
            kai__write(";\n");
        } break;

        case KAI_BOP_ADD:
        case KAI_BOP_SUB:
        case KAI_BOP_MUL:
        case KAI_BOP_DIV:
        {
            Kai_Reg dst;
            dst = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            Kai_u8 type;
            type = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_u8 is_value = type & 0x80;
            type &= 0x7F;

            Kai_Reg a;
            a = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = __");
			kai__write_u32(a);
			kai__write(" ");
			kai__write_string(__math_op_to_string(operation));
			kai__write(" ");
            if (is_value) {
				Kai_Value value;
                Kai_u8 size = __type_to_size(type);
                Kai_u8* u8_out = (Kai_u8*)&value;
                for (int i = 0; i < size; ++i)
				u8_out[i] = data[cursor + i];
                cursor += size;
				kai__write_u32(value.s32);
            } else {
                Kai_Reg b = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
				kai__write("__");
				kai__write_u32(b);
            }
			kai__write(";\n");

        } break;

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
            bcs__for (bytecode->native_count) {
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

            bcs__for (input_count) {
                //Kai_Reg reg;
                //reg = *(Kai_Reg*)(data + cursor);
                //cursor += sizeof(Kai_Reg);

				kai__unreachable();
                //bcc__write("__%d", reg);

                if (i != input_count - 1) {
					kai__unreachable();
					//bcc__write(", ");
                }
            }
			
			kai__unreachable();
            //bcc__write(");\n");
        } break;

        case KAI_BOP_COMPARE: {
            Kai_Reg dst;
            dst = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            Kai_u8 type;
            type = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_u8 is_value = type & 0x80;
            type &= 0x7F;

            Kai_u8 comp;
            comp = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_Reg a;
            a = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            char const* tab[] = { "<", ">=", ">", "<=", "==", "!=" };

            bcc__indent();
            kai__write("int32_t __");
            kai__write_u32(dst);
            kai__write(" = __");
			kai__write_u32(a);
			kai__write(" ");
			kai__write_string(kai_str_from_cstring(tab[comp]));
			kai__write(" ");
            if (is_value) {
				Kai_Value value;
                Kai_u8 size = __type_to_size(type);
                Kai_u8* u8_out = (Kai_u8*)&value;
                for (int i = 0; i < size; ++i)
				u8_out[i] = data[cursor + i];
                cursor += size;
				kai__write_u32(value.s32);
            } else {
				Kai_Reg b = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
				kai__write("__");
				kai__write_u32(b);
            }
			kai__write(";\n");
        } break;

        case KAI_BOP_BRANCH: {
            uint32_t location = *(uint32_t*)(data + cursor);
            cursor += sizeof(uint32_t);

            Kai_Reg src = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            bcc__indent();
            bcs__for (bytecode->branch_count) {
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
            cursor += sizeof(uint32_t);
            bcc__indent();
            kai__write("goto __endif;\n");
            kai__write("__else:\n");
        } break;

        case KAI_BOP_CALL: {
            //uint32_t location = *(uint32_t*)(data + cursor);
            cursor += sizeof(uint32_t);

            Kai_u8 ret_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_u8 arg_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);
            
            bcc__indent();
            kai__assert(ret_count <= 1);
            bcs__for (ret_count) {
                Kai_Reg dst = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                kai__write("int32_t __");
                kai__write_u32(dst);
                kai__write(" = ");
            }
            kai__write("func(");
            bcs__for (arg_count) {
				Kai_Reg src = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                kai__write("__");
                kai__write_u32(src);
                if (i != arg_count - 1)
                  kai__write(", ");
            }
            kai__write(");\n");
        } break;

        case KAI_BOP_RETURN: {
            Kai_u8 ret_count;
            ret_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_Reg a;
            bcc__indent();
            bcs__for (ret_count) {
                a = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                kai__write("return __");
                kai__write_u32(a);
            	kai__write(";\n");
            }
        } break;
        
        default: {
            kai__write("BYTECODE ERROR\n}");
			//kai__unreachable();
            //bc__debug_log("invalid bytecode instruction! (pc=%x op=%d)", cursor - 1, operation);
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
    if (arg_count-- < 0) goto call; asm ( "mov x0, %0" :: "r" (args[0]) : "x0" );
    if (arg_count-- < 0) goto call; asm ( "mov x1, %0" :: "r" (args[1]) : "x1" );
    if (arg_count-- < 0) goto call; asm ( "mov x2, %0" :: "r" (args[2]) : "x2" );
    if (arg_count-- < 0) goto call; asm ( "mov x3, %0" :: "r" (args[3]) : "x3" );
    if (arg_count-- < 0) goto call; asm ( "mov x4, %0" :: "r" (args[4]) : "x4" );
    if (arg_count-- < 0) goto call; asm ( "mov x5, %0" :: "r" (args[5]) : "x5" );
    if (arg_count-- < 0) goto call; asm ( "mov x6, %0" :: "r" (args[6]) : "x6" );
    if (arg_count-- < 0) goto call; asm ( "mov x7, %0" :: "r" (args[7]) : "x7" );

    // TODO: push onto stack with >8 arguments
    if (arg_count > 0) printf("ERROR: Too many arguments!");

call:
    asm ( "blr %0"      :: "r" (address) : "x30" );
    asm ( "mov %0, x0"  : "=r" (result) );
#else
    kai__unused(address);
    kai__unused(args);
    kai__unused(arg_count);
    kai__unreachable();
//#   error "Dynamic call not implemented for this architecture!"
#endif
    return result;
}

Kai_Value bci__compute_math(uint8_t type, uint8_t op, Kai_Value a, Kai_Value b) {
#   define BCI__MATH_OPERATION(NAME,A,B)                    \
    switch (op) {                                           \
        case KAI_BOP_ADD: return (Kai_Value) {.NAME = A + B};  \
        case KAI_BOP_SUB: return (Kai_Value) {.NAME = A - B};  \
        case KAI_BOP_MUL: return (Kai_Value) {.NAME = A * B};  \
        case KAI_BOP_DIV: return (Kai_Value) {.NAME = A / B};  \
    }
    switch (type) {
#       define X(TYPE, ITEM, NAME) case KAI_##NAME: BCI__MATH_OPERATION(ITEM, a.ITEM, b.ITEM)
        KAI_X_PRIMITIVE_TYPES
#       undef X
    }
    return (Kai_Value) {0};
#   undef BCI__MATH_OPERATION
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

Kai_u32 kai_interp_required_memory_size(Kai_Interpreter_Setup* info) {
    return info->max_register_count     * sizeof(Kai_Value)
        +  info->max_call_stack_count   * sizeof(Kai_BC_Procedure_Frame)
        +  info->max_return_value_count * sizeof(Kai_Reg)
        +  256                          * sizeof(Kai_Reg);
}

Kai_Result kai_interp_create(Kai_Interpreter* interp, Kai_Interpreter_Setup* info)
{
    if (info->memory == NULL)
        return KAI_BC_ERROR_MEMORY;

    uint8_t* base = info->memory;
    interp->registers = (Kai_Value*)base;
    base += info->max_register_count * sizeof(Kai_Value);
    interp->call_stack = (Kai_BC_Procedure_Frame*)base;
    base += info->max_call_stack_count * sizeof(Kai_BC_Procedure_Frame);
    interp->return_registers = (Kai_Reg*)base;
    base += info->max_return_value_count * sizeof(Kai_Reg);
    interp->scratch_buffer = (uint32_t*)base;

    interp->bytecode = 0;
    interp->count = 0;
    interp->flags = 0;
    interp->pc = 0;
    interp->call_stack_count = 0;
    interp->return_registers_count = 0;
    interp->max_register_count = info->max_register_count;
    interp->max_call_stack_count = info->max_call_stack_count;
    interp->max_return_values_count = info->max_return_value_count;
    return 0;
}

void kai_interp_load_from_stream(Kai_Interpreter* interp, Kai_BC_Stream* stream)
{
    interp->bytecode = stream->data;
    interp->count    = stream->count;
}

void kai_interp_load_from_memory(Kai_Interpreter* interp, void* code, uint32_t size)
{	
    interp->bytecode = code;
    interp->count    = size;
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


#define BC_X_INSTRUCTIONS    \
    X(KAI_BOP_NOP          ) \
    X(KAI_BOP_LOAD_CONSTANT) \
    X(KAI_BOP_ADD          ) \
    X(KAI_BOP_SUB          ) \
    X(KAI_BOP_MUL          ) \
    X(KAI_BOP_DIV          ) \
    X(KAI_BOP_COMPARE      ) \
    X(KAI_BOP_BRANCH       ) \
    X(KAI_BOP_JUMP         ) \
    X(KAI_BOP_CALL         ) \
    X(KAI_BOP_RETURN       ) \
    X(KAI_BOP_NATIVE_CALL  ) \
    X(KAI_BOP_LOAD         ) \
    X(KAI_BOP_STORE        ) \
    X(KAI_BOP_STACK_ALLOC  ) \
    X(KAI_BOP_STACK_FREE   ) \
    X(KAI_BOP_CHECK_ADDRESS)

char const* bc__op_to_name[] = {   
#define X(NAME) [NAME] = #NAME,
    BC_X_INSTRUCTIONS
#undef X
};

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

    if (operation < 100) {
        //bc__debug_log("ex %s", bc__op_to_name[operation]);
    }

    switch (operation)
    {
    case KAI_BOP_LOAD_CONSTANT: {
        Kai_Reg dst;
        uint8_t type;
        Kai_Value value = {0};

        if (bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_value(interp, type, &value)) return 0;
        if (bci__write_register(interp, dst, value)) return 0;
    } break;

    case KAI_BOP_ADD:
    case KAI_BOP_SUB:
    case KAI_BOP_MUL:
    case KAI_BOP_DIV:
    {
        Kai_Reg dst;
        uint8_t type;

        if (bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &type)) return 0;

        uint8_t is_value = type & 0x80;
        type &= 0x7F;

        Kai_Value va, vb;
        {
            Kai_Reg a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }

        if (is_value) {
            if (bci__next_value(interp, type, &vb)) return 0;
        } else {
            Kai_Reg b;
            if (bci__next_reg(interp, &b)) return 0;
            if (bci__read_register(interp, b, &vb)) return 0;
        }

        Kai_Value result = bci__compute_math(type, operation, va, vb);
        if (bci__write_register(interp, dst, result)) return 0;
    } break;

    case KAI_BOP_COMPARE: {
        Kai_Reg dst;
        uint8_t type;
        uint8_t comp;
		
        if (bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_u8(interp, &comp)) return 0;
		
        uint8_t is_value = type & 0x80;
        type &= 0x7F;
		
        Kai_Value va, vb;
        {
			uint32_t a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }
		
        if (is_value) {
			if (bci__next_value(interp, type, &vb)) return 0;
        } else {
			uint32_t b;
            if (bci__next_reg(interp, &b)) return 0;
            if (bci__read_register(interp, b, &vb)) return 0;
        }

        
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

        if (bci__next_u32(interp, &location)) return 0;
        if (bci__next_u8(interp, &ret_count)) return 0;
        if (bci__next_u8(interp, &arg_count)) return 0;

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

        if (bci__next_address(interp, &address)) return 0;
        if (bci__next_u8(interp, &use_dst)) return 0;
        if (use_dst && bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &input_count)) return 0;

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
        // reg dst
        // u8  type
        // reg addr
        // u32 offset
        kai__unreachable();
        //bc__debug_log("KAI_BOP_LOAD not implemented!");
    } break;

    case KAI_BOP_STORE: {
        // reg src
        // u8  type
        // reg addr
        // u32 offset
        kai__unreachable();
    } break;

    case KAI_BOP_STACK_ALLOC: {
        // reg dst
        // u32 size
        kai__unreachable();
    } break;

    case KAI_BOP_STACK_FREE: {
        // u32 size
        kai__unreachable();
    } break;
    
    default: {
        kai__unreachable();
        //bc__debug_log("invalid bytecode instruction! (pc=%x op=%d)", interp->pc - 1, operation);
        interp->flags |= KAI_INTERP_FLAGS_INCOMPLETE;
        return 0;
    } break;
    }

    return 1;
}

// TODO: 😭
//#define DEBUG_DEPENDENCY_GRAPH
//#define DEBUG_COMPILATION_ORDER
#define DEBUG_CODE_GENERATION

Kai_Result kai__create_dependency_graph(
	Kai__Dependency_Graph_Create_Info* Info,
	Kai__Dependency_Graph*             out_Graph);

Kai_Result kai__determine_compilation_order(Kai__Dependency_Graph* Graph, Kai_Error* out_Error);
Kai_Result kai__generate_bytecode(Kai__Bytecode_Create_Info* Info, Kai__Bytecode* out_Bytecode);
Kai_Result kai__create_program(Kai__Program_Create_Info* Info, Kai_Program* out_Program);

void kai__destroy_dependency_graph(Kai__Dependency_Graph* Graph);
void kai__destroy_bytecode(Kai__Bytecode* Bytecode);

Kai_Result kai__dg_insert_value_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr,
    Kai_bool                  In_Procedure);

Kai_Result kai__dg_insert_type_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr);

Kai_Result kai_create_program(Kai_Program_Create_Info* info, Kai_Program* program)
{
	(void)(program);
	return kai__error_internal(info->error, KAI_STRING("kai_create_program not implemented"));
}

#define kai__check(RESULT) if (RESULT != KAI_SUCCESS) goto cleanup;
Kai_Result kai_create_program_from_source(
	Kai_str        Source,
	Kai_Allocator* Allocator,
	Kai_Error*     out_Error,
	Kai_Program*   out_Program)
{
	Kai_Result            result           = KAI_SUCCESS;
	Kai_Syntax_Tree       syntax_tree      = {0};
	Kai__Dependency_Graph dependency_graph = {0};
	Kai__Bytecode         bytecode         = {0};

	{
		Kai_Syntax_Tree_Create_Info info = {
			.source_code = Source,
			.error = out_Error,
			.allocator = *Allocator,
		};
		result = kai_create_syntax_tree(&info, &syntax_tree);
	}
	kai__check(result);
	
	{
		Kai__Dependency_Graph_Create_Info info = {
			.trees = &syntax_tree,
			.tree_count = 1,
			.allocator = *Allocator,
			.error = out_Error,
		};
		result = kai__create_dependency_graph(&info, &dependency_graph);
	}
	kai__check(result);

	{
		result = kai__determine_compilation_order(&dependency_graph, out_Error);
	}
	kai__check(result);

	{
		Kai__Bytecode_Create_Info info = {
			.error = out_Error,
			.dependency_graph = &dependency_graph,
		};
		result = kai__generate_bytecode(&info, &bytecode);
	}
	kai__check(result);

	{
		Kai__Program_Create_Info info = {
			.bytecode = &bytecode,
			.error = out_Error,
			.allocator = *Allocator,
		};
		result = kai__create_program(&info, out_Program);
	}
	kai__check(result);

cleanup:
	kai__destroy_bytecode(&bytecode);
	kai__destroy_dependency_graph(&dependency_graph);
	kai_destroy_syntax_tree(&syntax_tree);
	return result;
}
#undef kai__check

Kai__DG_Node_Index* kai__recursive_scope_find(
	Kai__Dependency_Graph* Graph,
	Kai_u32                Scope_Index,
	Kai_str                Identifier)
{
	do {
		Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;
		Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, Identifier);
		if (node_index != NULL) {
			return node_index;
		}
		Scope_Index = scope->parent;
	} while (Scope_Index != KAI__SCOPE_NO_PARENT);
	return NULL;
}

Kai_Result kai__error_redefinition(
	Kai_Error*     out_Error,
	Kai_Allocator* allocator,
	Kai_str        string,
	Kai_u32        line_number,
	Kai_str        original_name,
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
	Kai_str         string,
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

void add_dependency(
	Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
	Kai__DG_Node_Index        Reference)
{
	Kai_Allocator* allocator = &Graph->allocator;
	for (Kai_int i = 0; i < out_Dependency_Array->count; ++i) {
		Kai__DG_Node_Index node_index = out_Dependency_Array->elements[i];
		if (node_index.flags == Reference.flags
		&&  node_index.value == Reference.value)
			return;
	}
	kai__array_append(out_Dependency_Array, Reference);
}

Kai_Result kai__dg_create_nodes_from_statement(
	Kai__Dependency_Graph* Graph,
	Kai_Stmt               Stmt,
	Kai_u32                Scope_Index,
	Kai_bool               In_Procedure,
	Kai_bool               From_Procedure)
{
	Kai_Result result = KAI_SUCCESS;
	Kai_Allocator* allocator = &Graph->allocator;
	void* base = Stmt;

    switch (Stmt->id)
	{
	case KAI_EXPR_PROCEDURE: {
		Kai_Expr_Procedure* node = base;

		Kai_u32 new_scope_index = Graph->scopes.count;
		kai__array_append(&Graph->scopes, (Kai__DG_Scope) {
			.parent = Scope_Index,
			.is_proc_scope = KAI_TRUE,
		});
		//kai__hash_table_create(&Graph->scopes.elements[new_scope_index].identifiers);

		node->_scope = new_scope_index;
		if (node->body) {
			return kai__dg_create_nodes_from_statement(Graph, node->body, new_scope_index, KAI_TRUE, KAI_TRUE);
		}
		else {
			KAI__FATAL_ERROR("Compile Error", "Native Functions not implemented!");
		//	printf("\x1b[93mWarning\x1b[0m: Native functions not implemented yet!\n");
		}
	} break;

    case KAI_STMT_IF: {
        Kai_Stmt_If* node = base;

		result = kai__dg_create_nodes_from_statement(Graph, node->body, Scope_Index, In_Procedure, KAI_FALSE);
		if (result != KAI_SUCCESS)
			return result;

		if (node->else_body) {
			result = kai__dg_create_nodes_from_statement(Graph, node->else_body, Scope_Index, In_Procedure, KAI_FALSE);
			if (result != KAI_SUCCESS)
				return result;
		}
    } break;

    case KAI_STMT_FOR: {
        Kai_Stmt_For* node = base;

		return kai__dg_create_nodes_from_statement(Graph, node->body, Scope_Index, In_Procedure, KAI_FALSE);
    } break;

    case KAI_STMT_DECLARATION: {
		Kai_Stmt_Declaration* node = base;

		if (In_Procedure && !(node->flags & KAI_DECL_FLAG_CONST))
			return result;

		Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;

		// Does this declaration already exist for this Scope?
		Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, node->name);
		if (node_index != NULL) {
			Kai__DG_Node* existing = &Graph->nodes.elements[node_index->value];
			return kai__error_redefinition(
				Graph->error,
				allocator,
				node->name,
				node->line_number,
				existing->name,
				existing->line_number
			);
		}

		{
			Kai__DG_Node_Index index = {
				.value = Graph->nodes.count
			};

			Kai__DG_Node dg_node = {
				.name = node->name,
				.expr = node->expr,
				.line_number = node->line_number,
				.scope_index = Scope_Index,			
			};

			kai__array_append(&dg_node.value_dependencies, (Kai__DG_Node_Index) {
				.flags = KAI__DG_NODE_TYPE,
				.value = index.value,
			});

			kai__array_append(&Graph->nodes, dg_node);
			kai__hash_table_emplace(scope->identifiers, node->name, index);
		}

		if (node->expr) {
			return kai__dg_create_nodes_from_statement(
				Graph,
				node->expr,
				Scope_Index,
				In_Procedure,
				KAI_FALSE
			);
		}
	} break;

	case KAI_STMT_COMPOUND: {
		Kai_Stmt_Compound* node = base;
		Kai_u32 new_scope_index = Scope_Index;

		if (!From_Procedure) {
			new_scope_index = Graph->scopes.count;
			kai__array_append(&Graph->scopes, (Kai__DG_Scope) {
				.parent = Scope_Index,
				.is_proc_scope = KAI_TRUE,
			});
			//kai__hash_table_create(&Graph->scopes.elements[new_scope_index].identifiers);
		}

		node->_scope = new_scope_index;

		Kai_Stmt current = node->head;
		while (current) {
			result = kai__dg_create_nodes_from_statement(Graph, current, new_scope_index, In_Procedure, KAI_FALSE);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
		}
	} break;
	}
	return result;
}

void kai__print_scope(Kai__DG_Scope* Scope)
{
#if 0
	kai__hash_table_iterate(Scope->identifiers, i)
	{
		Kai__DG_Node_Index node_index = Scope->identifiers.values[i];
		if (node_index.flags & KAI__DG_NODE_LOCAL_VARIABLE)
		putchar('L');
		Kai_str name = Scope->identifiers.keys[i];
		printf("\"%.*s\"+%i ", name.count, name.data, node_index.value);
	}
#else
		kai__unused(Scope);
#endif	
}

Kai__DG_Node_Index* kai__dg_resolve_dependency_node(
	Kai__Dependency_Graph* Graph,
	Kai_str                Name,
	Kai_u32                Scope_Index,
	Kai_bool               In_Procedure)
{
    Kai_bool allow_locals = KAI_TRUE;
    while (1) {
        Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;
		Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, Name);

        if (node_index != NULL) {
            Kai_bool is_local = KAI_BOOL(node_index->flags & KAI__DG_NODE_LOCAL_VARIABLE);
            if (In_Procedure && is_local) {
                if (allow_locals) {
					return node_index;
                }
            }
            else {
				return node_index;
            }
		}

        if (Scope_Index == KAI__SCOPE_GLOBAL_INDEX)
            return NULL;
        
        // Do not allow Local variables from higher procedure scopes
        if (scope->is_proc_scope)
            allow_locals = KAI_FALSE;

        Scope_Index = scope->parent;
    }
}

void kai__remove_local_variables(Kai__DG_Scope* Scope)
{
	kai__hash_table_iterate(Scope->identifiers, i)
	{
		Kai__DG_Node_Index node_index = Scope->identifiers.values[i];
		if (!(node_index.flags & KAI__DG_NODE_LOCAL_VARIABLE))
			continue;
		kai__hash_table_remove_index(Scope->identifiers, i);
	}
}

Kai_Result kai__dg_insert_value_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr,
    Kai_bool                  In_Procedure)
{
	Kai_Result result = KAI_SUCCESS;
	Kai_Allocator* allocator = &Graph->allocator;
	void* base = Expr;
    if (Expr == NULL) { kai__assert(0 && "null expression\n"); return 0; }

    switch (Expr->id)
    {
    default: {
        kai__assert(0 && "undefined expr (id = %d)\n");
	} break;

    case KAI_EXPR_IDENTIFIER: {
		Kai__DG_Node_Index* node_index = kai__dg_resolve_dependency_node(
			Graph,
			Expr->source_code,
			Scope_Index,
			In_Procedure
		);

		if (node_index == NULL) {
			return kai__error_not_declared(Graph->error, allocator, Expr->source_code, Expr->line_number);
		}

        // DO NOT depend on local variables
        if (In_Procedure && (node_index->flags & KAI__DG_NODE_LOCAL_VARIABLE))
            break;
		
		add_dependency(Graph, out_Dependency_Array, *node_index);
    } break;

    case KAI_EXPR_NUMBER: {} break;
    case KAI_EXPR_STRING: {} break;

    case KAI_EXPR_UNARY: {
        Kai_Expr_Unary* node = base;
		return kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->expr,
			In_Procedure
		);
    } break;

    case KAI_EXPR_BINARY: {
        Kai_Expr_Binary* node = base;
		result = kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->left,
			In_Procedure
		);
		if (result != KAI_SUCCESS)
            return result;

		return kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->right,
			In_Procedure
		);
    } break;

    case KAI_EXPR_PROCEDURE_CALL: {
        Kai_Expr_Procedure_Call* node = base;

		// Procedure calls in procedures are fine, we only need to know the type to generate the bytecode
		// TODO: possible bug for nested procedures
        if (In_Procedure) {
			result = kai__dg_insert_type_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->proc
			);
        }
        else {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->proc,
				In_Procedure
			);
        }
		if (result != KAI_SUCCESS)
			return result;

		Kai_Expr current = node->arg_head;
        while (current != NULL) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				current,
				In_Procedure
			);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
        }
    } break;

    case KAI_EXPR_PROCEDURE_TYPE: {
        //Kai_Expr_Procedure_Type* proc = base;
		//panic_with_message("procedure type\n");
    } break;

    case KAI_EXPR_PROCEDURE: {
        Kai_Expr_Procedure* node = base;
		Kai__DG_Scope* local_scope = Graph->scopes.elements + node->_scope;

		// Insert procedure input names to local scope
		Kai_Expr current = node->in_out_expr;
        for (int i = 0; i < (int)node->in_count; ++i) {
			Kai__DG_Node_Index* node_index = kai__hash_table_find(local_scope->identifiers, current->name);
			
			if (node_index != NULL) {
				Kai__DG_Node* original = &Graph->nodes.elements[node_index->value];
				return kai__error_redefinition(
					Graph->error,
					allocator,
					current->name,
					current->line_number,
					original->name,
					original->line_number
				);
			}

			kai__hash_table_emplace(local_scope->identifiers, current->name, (Kai__DG_Node_Index) {
				.flags = KAI__DG_NODE_LOCAL_VARIABLE,
			});

            current = current->next;
        }

		if (node->body) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				node->_scope,
				node->body,
				KAI_TRUE
			);
		}

		kai__remove_local_variables(local_scope);
    } break;

    break; case KAI_STMT_DECLARATION: {
        if (In_Procedure) {
            Kai_Stmt_Declaration* node = base;
            // Already has a dependency node
            if (node->flags & KAI_DECL_FLAG_CONST)
				return KAI_SUCCESS;
            Kai__DG_Scope* scope = Graph->scopes.elements + Scope_Index;
            // Insert into local scope (if not already defined)
			Kai__DG_Node_Index* node_index = kai__hash_table_find(scope->identifiers, node->name);
            if (node_index != NULL && node_index->flags != KAI__DG_NODE_LOCAL_VARIABLE) {
				Kai__DG_Node* original = &Graph->nodes.elements[node_index->value];
                return kai__error_redefinition(
					Graph->error,
					allocator,
					node->name,
					node->line_number,
					original->name,
					original->line_number
				);
			}

			kai__hash_table_emplace(scope->identifiers,
				node->name,
				(Kai__DG_Node_Index) {
					.flags = KAI__DG_NODE_LOCAL_VARIABLE,
				}
			);

            // Look into it's definition to find possible dependencies
            return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->expr,
				KAI_TRUE
			);
        }
        else panic_with_message("invalid declaration\n");
    }

    break; case KAI_STMT_ASSIGNMENT: {
        if (In_Procedure) {
            Kai_Stmt_Assignment* node = base;
			return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->expr,
				KAI_TRUE
			);
        }
        else panic_with_message("invalid assignment\n");
    }

    case KAI_STMT_RETURN: {
		Kai_Stmt_Return* node = base;
        if (In_Procedure) {
			return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->expr,
				KAI_TRUE
			);
        }
		else panic_with_message("invalid return\n");
    } break;

    break; case KAI_STMT_IF: {
		Kai_Stmt_If* node = base;
		result = kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->body,
			In_Procedure
		);
		if (result != KAI_SUCCESS)
			return result;
		if (node->else_body != NULL) {
			return kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				node->else_body,
				In_Procedure
			);
		}
    }

    break; case KAI_STMT_COMPOUND: {
        if (In_Procedure) {
            Kai_Stmt_Compound* node = base;
			Kai_Expr current = node->head;
            while (current != NULL) {
				result = kai__dg_insert_value_dependencies(
					Graph,
					out_Dependency_Array,
					node->_scope,
					current,
					KAI_TRUE
				);
				if (result != KAI_SUCCESS)
					return result;
				current = current->next;
            }
            Kai__DG_Scope* scope = Graph->scopes.elements + node->_scope;
            kai__remove_local_variables(scope);
        }
        else panic_with_message("invalid compound statement\n");
    }

    break; case KAI_STMT_FOR: {
        Kai_Stmt_For* node = base;
		return kai__dg_insert_value_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->body,
			In_Procedure
		);
    }

    break;
    }

	return result;
}

Kai_Result kai__dg_insert_type_dependencies(
    Kai__Dependency_Graph*    Graph,
	Kai__DG_Dependency_Array* out_Dependency_Array,
    Kai_u32                   Scope_Index,
    Kai_Expr                  Expr)
{
	Kai_Result result = KAI_SUCCESS;
	void* base = Expr;
    if (Expr == NULL) { panic_with_message("null expression\n"); return KAI_FALSE; }

    switch (Expr->id)
    {
    default:
    break; case KAI_EXPR_IDENTIFIER: {
		// TODO: why true?
		Kai__DG_Node_Index* node_index = kai__dg_resolve_dependency_node(
			Graph,
			Expr->source_code,
			Scope_Index,
			KAI_TRUE
		);

        if (node_index == NULL)
            return kai__error_not_declared(Graph->error, &Graph->allocator, Expr->source_code, Expr->line_number);

        // DO NOT depend on local variables
        if (node_index->flags & KAI__DG_NODE_LOCAL_VARIABLE)
            return result;

		add_dependency(Graph, out_Dependency_Array, (Kai__DG_Node_Index) {
			.flags = KAI__DG_NODE_TYPE,
			.value = node_index->value,
		});
    }

    break; case KAI_EXPR_UNARY: {
        Kai_Expr_Unary* node = base;
        return kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->expr
		);
    }

    break; case KAI_EXPR_BINARY: {
        Kai_Expr_Binary* node = base;
        result = kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->left
		);
		if (result != KAI_SUCCESS)
			return result;
        return kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->right
		);
    }

    break; case KAI_EXPR_PROCEDURE_CALL: {
        Kai_Expr_Procedure_Call* node = base;

        result = kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->proc
		);
		if (result != KAI_SUCCESS)
			return result;

		Kai_Expr current = node->arg_head;
		while (current != NULL) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				current,
				KAI_FALSE // TODO: possible bug here ??
			);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
        }
    }

    break; case KAI_EXPR_PROCEDURE_TYPE: {
        //panic_with_message("procedure type\n");
    }

    break; case KAI_EXPR_PROCEDURE: {
        Kai_Expr_Procedure* node = base;
		Kai_Expr current = node->in_out_expr;
        while (current != NULL) {
			result = kai__dg_insert_value_dependencies(
				Graph,
				out_Dependency_Array,
				Scope_Index,
				current,
				KAI_FALSE // TODO: possible bug here ??
			);
			if (result != KAI_SUCCESS)
				return result;
			current = current->next;
        }
    }

    break; case KAI_STMT_DECLARATION: {
		// What do we do here exactly?
        panic_with_message("declaration\n");
    }

    break; case KAI_STMT_RETURN: {
        Kai_Stmt_Return* node = base;
        return kai__dg_insert_type_dependencies(
			Graph,
			out_Dependency_Array,
			Scope_Index,
			node->expr
		);
    }

    break; case KAI_STMT_COMPOUND: {
        panic_with_message("compound\n");
    }

    break;
    }
	
    return result;
}

Kai_Result kai__create_dependency_graph(
	Kai__Dependency_Graph_Create_Info* Info,
	Kai__Dependency_Graph*             out_Graph)
{
	Kai_Result result = KAI_SUCCESS;
	Kai_Allocator* allocator = &Info->allocator;
	out_Graph->compilation_order = NULL;
	out_Graph->allocator = Info->allocator;
	out_Graph->error = Info->error;

	// Initialize Global Scope
	kai__array_append(&out_Graph->scopes, (Kai__DG_Scope) {
		.is_proc_scope = KAI_FALSE,
		.parent = KAI__SCOPE_NO_PARENT,
	});
	Kai__DG_Scope* global = &out_Graph->scopes.elements[KAI__SCOPE_GLOBAL_INDEX];
	//kai__hash_table_create(&global->identifiers);

	// Insert builtin types
	for (int i = 0, next = 0; i < kai__count(kai__builtin_types); ++i)
	{
		kai__array_append(&out_Graph->nodes, (Kai__DG_Node) {
			.type        = &kai__type_info_type,
			.value       = { .type = kai__builtin_types[i].type },
			.value_flags = KAI__DG_NODE_EVALUATED,
			.type_flags  = KAI__DG_NODE_EVALUATED,
			.name        = kai__builtin_types[i].name,
			.scope_index = KAI__SCOPE_GLOBAL_INDEX,
		});
		kai__hash_table_emplace(global->identifiers,
			kai__builtin_types[i].name,
			(Kai__DG_Node_Index) {
				.value = next++
			}
		);
	}

	// Insert nodes from syntax tree
	for (Kai_u32 i = 0; i < Info->tree_count; ++i) {
		Kai_Syntax_Tree* tree = Info->trees + i;
		result = kai__dg_create_nodes_from_statement(
			out_Graph,
			(Kai_Stmt)&tree->root,
			KAI__SCOPE_GLOBAL_INDEX,
			KAI_FALSE,
			KAI_FALSE
		);
		if (result != KAI_SUCCESS)
			return result;
	}

	// Insert dependencies for each node
	for (Kai_u32 i = 0; i < out_Graph->nodes.count; ++i) {
		Kai__DG_Node* node = out_Graph->nodes.elements + i;

		if (!(node->value_flags&KAI__DG_NODE_EVALUATED)) {
			result = kai__dg_insert_value_dependencies(
				out_Graph,
				&node->value_dependencies,
				node->scope_index,
				node->expr,
				KAI_FALSE
			);
            if (result != KAI_SUCCESS)
				return result;
		}

		if (!(node->type_flags&KAI__DG_NODE_EVALUATED)) {
			result = kai__dg_insert_type_dependencies(
				out_Graph,
				&node->type_dependencies,
				node->scope_index,
				node->expr
			);
            if (result != KAI_SUCCESS)
				return result;
		}
	}

#if defined(DEBUG_DEPENDENCY_GRAPH)
	for (Kai_u32 i = 0; i < out_Graph->nodes.count; ++i) {
		Kai__DG_Node* node = out_Graph->nodes.elements+i;

		printf("Node \"\x1b[94m%.*s\x1b[0m\"", node->name.count, node->name.data);
		if (node->type_flags & KAI__DG_NODE_EVALUATED) {
			printf(" type: ");
			kai_write_type(kai_debug_stdout_writer(), node->type);
		}
		if (node->value_flags & KAI__DG_NODE_EVALUATED) {
			printf(" value: ");
			if (node->type->type == KAI_TYPE_TYPE) {
				kai_write_type(kai_debug_stdout_writer(), node->value.type);
			}
		}
		putchar('\n');

		if (node->value_dependencies.count) {
			printf("    V deps: ");
			for (Kai_u32 d = 0; d < node->value_dependencies.count; ++d) {
				Kai__DG_Node_Index node_index = node->value_dependencies.elements[d];
				Kai_str name = out_Graph->nodes.elements[node_index.value].name;
				if (node_index.flags & KAI__DG_NODE_TYPE) {
					printf("T(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
				else {
					printf("V(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
			}
			putchar('\n');
		}

		if (node->type_dependencies.count) {
			printf("    T deps: ");
			for (Kai_u32 d = 0; d < node->type_dependencies.count; ++d) {
				Kai__DG_Node_Index node_index = node->type_dependencies.elements[d];
				Kai_str name = out_Graph->nodes.elements[node_index.value].name;
				if (node_index.flags & KAI__DG_NODE_TYPE) {
					printf("T(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
				else {
					printf("V(\x1b[94m%.*s\x1b[0m) ", name.count, name.data);
				}
			}
			putchar('\n');
		}
	}
#endif // DEBUG_DEPENDENCY_GRAPH

	return result;
}

// TODO: bad name, rename
Kai_u32 kai__node_index_to_index(Kai__DG_Node_Index node_index)
{
	Kai_bool is_type = KAI_BOOL(node_index.flags & KAI__DG_NODE_TYPE);
	return node_index.value << 1 | is_type;
}

typedef struct {
	Kai__Dependency_Graph* graph;
    Kai_u32*       post;
    Kai_u32*       prev;
    Kai_bool*      visited;
    Kai_u32        next;
} Kai__DFS_Context;

void dfs_explore(Kai__DFS_Context* dfs, Kai__DG_Node_Index node_index)
{
	Kai_u32 index = kai__node_index_to_index(node_index);
	dfs->visited[index] = KAI_TRUE;

	Kai__Dependency_Graph* dg = dfs->graph;
	Kai__DG_Node* node = &dg->nodes.elements[node_index.value];

	Kai__DG_Dependency_Array* deps;
	if (node_index.flags & KAI__DG_NODE_TYPE) {
		deps = &node->type_dependencies;
	} else {
		deps = &node->value_dependencies;
	}

	for (Kai_u32 d = 0; d < deps->count; ++d) {
		Kai__DG_Node_Index dep = deps->elements[d];
		Kai_u32 d_index = kai__node_index_to_index(dep);

		if (!dfs->visited[d_index]) {
			dfs->prev[d_index] = index;
			dfs_explore(dfs, dep);
		}
	}

	dfs->post[index] = dfs->next++;
}

Kai_Result kai__determine_compilation_order(Kai__Dependency_Graph* Graph, Kai_Error* out_Error)
{
	(void)out_Error; // TODO: remove
	// TODO: only one allocation necessary
	Kai_Allocator* allocator = &Graph->allocator;
	Kai__DFS_Context dfs = { .graph = Graph, .next = 0 };
	dfs.post    = kai__allocate(NULL, Graph->nodes.count * 2 * sizeof(Kai_u32) , 0);
	dfs.prev    = kai__allocate(NULL, Graph->nodes.count * 2 * sizeof(Kai_u32) , 0);
	dfs.visited = kai__allocate(NULL, Graph->nodes.count * 2 * sizeof(Kai_bool), 0);

	kai__memory_fill(dfs.prev, 0xFF, Graph->nodes.count * 2* sizeof(Kai_u32));

	// Do not waste time on builtins
	//iterate (builtin_types) {
	//	dfs.visited[i] = dfs.visited[i + context.dependency_graph.nodes.count] = KAI_TRUE;
	//}

	// Perform DFS traversal
	kai__array_iterate (Graph->nodes, i) {
		Kai__DG_Node_Index node_index = {.value = (Kai_u32)i};
		Kai_u32 v = kai__node_index_to_index(node_index);
		if (!dfs.visited[v]) dfs_explore(&dfs, node_index);

		node_index.flags = KAI__DG_NODE_TYPE;
		Kai_u32 t = kai__node_index_to_index(node_index);
		if (!dfs.visited[t]) dfs_explore(&dfs, node_index);
	}

	// Get compilation order  TODO: fix this O(N^2) algorithm
	Graph->compilation_order = kai__allocate(NULL, Graph->nodes.count * 2 * sizeof(Kai_u32), 0);
	{
		Kai_u32 next = 0;
		Kai_u32 count = 0;
		kai__for_n (Graph->nodes.count * 2) {
			cont:
			if (dfs.post[i] == next) {
				Graph->compilation_order[count++] = (int)i;
				if (count >= Graph->nodes.count * 2)
					break;
				++next;
				i = 0;
				goto cont;
			}
		}

#if defined(DEBUG_COMPILATION_ORDER)
		char temp[256];
		Kai_Debug_String_Writer* writer = kai_debug_stdout_writer();
		kai__write("Compilation Order:\n");
		for_ (i, Graph->nodes.count * 2) {
			Kai_u32 v = Graph->compilation_order[i];
			Kai_bool is_type = v & 1;
			Kai_u32 index = v >> 1;
			if (is_type) {
				kai__write_char('T');
			} else {
				kai__write_char('V');
			}
			Kai__DG_Node* node = &Graph->nodes.elements[index];
			//kai__write_format("(%.*s,%i) ", node->name.count, node->name.data, dfs.post[v]);
		}
		kai__write_char('\n');
#endif
	}

	// TODO: Check for any circular dependencies
    //for_n (context->dependency_graph.nodes.count * 2) {
	//	_write_format("prev(%i) = %i\n", (int)i, dfs.prev[i]);
	//}

	// Check for any circular dependencies
    /*for_ (u, Graph->nodes.count * 2)
    {
		Node_Ref* dependencies = context->dependency_graph.dependencies.data;
		Node* node;
		Kai_range deps;

		if (u < context->dependency_graph.nodes.count) {
			node = context->dependency_graph.nodes.data + u;
			deps = node->value_dependencies;
		} else {
			node = context->dependency_graph.nodes.data + u - context->dependency_graph.nodes.count;
			deps = node->type_dependencies;
		}
		
        for_n (deps.count) {
			Node_Ref v = dependencies[deps.offset + i];
			
			if (v & TYPE_BIT) {
				v &=~ TYPE_BIT;
				v += context->dependency_graph.nodes.count;
			}

            if (dfs.post[u] < dfs.post[v]) {
				memset(dfs.visited, 0, context->dependency_graph.nodes.count * 2 * sizeof(Kai_bool));
                dfs_explore(&dfs, u);

                // loop through prev pointers to get to u, and add errors in reverse order

                void* start = c_error_circular_dependency(node_at(u));

				// TODO: Fix error messages here
                Kai_Error* last = &context->error;
				// write_format("index = %i\n", (int)u);
                //for (u32 i = dfs.prev[u];; i = dfs.prev[i]) {
				//	 write_format("index = %i\n", (int)i);
                //    last->next = (Kai_Error*)start;
                //    start = c_error_dependency_info(start, node_at(i));
                //    last = last->next;
                //    if (i == (~1) || i == u) break;
                //}
				(void)start;
				(void)last;
				goto error;
            }
        }
    }*/

	kai__free(dfs.post,    Graph->nodes.count * 2 * sizeof(Kai_u32));
	kai__free(dfs.prev,    Graph->nodes.count * 2 * sizeof(Kai_u32));
	kai__free(dfs.visited, Graph->nodes.count * 2 * sizeof(Kai_bool));
	return KAI_SUCCESS;
}

Kai__DG_Node* kai__dg_find_node(Kai__Dependency_Graph* graph, Kai_str name)
{
	kai__array_iterate (graph->nodes, i) {
		if (kai_str_equals(graph->nodes.elements[i].name, name))
			return &graph->nodes.elements[i];
	}
	return NULL;
}

Kai__DG_Value kai__value_from_expression(Kai__Bytecode_Generation_Context* Context, Kai_Expr Expr, Kai_Type* Type);

Kai_Type kai__type_from_expression(Kai__Bytecode_Generation_Context* Context, Kai_Expr Expr)
{
	void* void_Expr = Expr;
	switch (Expr->id) {
		case KAI_EXPR_IDENTIFIER: {
			Kai__DG_Node* d = kai__dg_find_node(Context->dependency_graph, Expr->source_code);
			kai__assert(d != NULL);
			return d->type;
		} break;

		case KAI_EXPR_PROCEDURE: {
			Kai_Expr_Procedure* node = void_Expr;
			Kai_Type_Info_Procedure* type_info = kai__arena_allocate(&Context->arena, sizeof(Kai_Type_Info_Procedure));
			type_info->type = KAI_TYPE_PROCEDURE;
			type_info->in_count = node->in_count;
			type_info->out_count = node->out_count;
			type_info->sub_types = kai__arena_allocate(&Context->arena, sizeof(Kai_Type) * (node->in_count + node->out_count));
			Kai_Expr current = node->in_out_expr;
			int i = 0;
			while (current != NULL)
			{
				Kai_Type type = NULL;
				Kai__DG_Value value = kai__value_from_expression(Context, current, &type);
				if (type != NULL && type->type == KAI_TYPE_TYPE)
					type_info->sub_types[i++] = value.type;
				else
					panic_with_message("Type was not a type!");
				current = current->next;
			}
			return (Kai_Type)type_info;
		}
		case KAI_EXPR_PROCEDURE_CALL: {
			Kai_Expr_Procedure_Call* node = void_Expr;
			// Find type of the procedure we are calling
			if (node->proc->id != KAI_EXPR_IDENTIFIER)
				panic_with_message("procedure calls only implemented for identifiers");

			Kai_Expr_Identifier* proc = (Kai_Expr_Identifier*)node->proc;
			Kai__DG_Node* dg_node = kai__dg_find_node(Context->dependency_graph, proc->source_code);

			if (dg_node == NULL)
				panic_with_message("could not resolve node");

			Kai_Type type = dg_node->type;
			if (type->type != KAI_TYPE_PROCEDURE)
				panic_with_message("calling something that does not have procedure type!");

			Kai_Type_Info_Procedure* proc_type = (Kai_Type_Info_Procedure*)type;

			if (proc_type->out_count == 0)
				panic_with_message("procedure does not have a return");

			return proc_type->sub_types[proc_type->in_count];
		} break;

		default: {
			panic_with_message("not handled %i", Expr->id);
		}
	}

	return NULL;
}

Kai_Result kai__bytecode_generate_type(Kai__Bytecode_Generation_Context* Context, Kai__DG_Node* dg_node)
{
	if (dg_node->type_flags & KAI__DG_NODE_EVALUATED)
		return KAI_SUCCESS;

	dg_node->type = kai__type_from_expression(Context, dg_node->expr);

	return KAI_SUCCESS;
}

Kai_bool kai__bytecode_find_register(Kai__Bytecode_Generation_Context* Context, Kai_str Name, Kai_Reg* out)
{
	kai__array_iterate (Context->registers, i) {
		Kai__Bytecode_Register br = Context->registers.elements[Context->registers.count-1-i];
		if (kai_str_equals(br.name, Name)) {
			*out = br.reg;
			return KAI_TRUE;
		}
	}
	return KAI_FALSE;
}

Kai_Reg kai__bytecode_allocate_register(Kai__Bytecode_Generation_Context* Context)
{
	Kai_Allocator* allocator = &Context->dependency_graph->allocator;
	Kai_Reg reg = Context->registers.count;
	kai__array_append(&Context->registers, (Kai__Bytecode_Register){
		.reg = reg,
	});
	return reg;
}

Kai_Result kai__bytecode_emit_expression(
	Kai__Bytecode_Generation_Context* Context,
	Kai_Expr Expr,
	Kai_u32* output_register)
{
	void* void_Expr = Expr;
	switch (Expr->id) {
	case KAI_EXPR_NUMBER: {
		// TODO: Only Parse number HERE, generate error if number can not be represented by the type
		Kai_Expr_Number* node = void_Expr;
		Kai_Reg dst = kai__bytecode_allocate_register(Context);
		Kai_Value value;
		value.s32 = (Kai_s32)node->value.Whole_Part;
		kai_bc_insert_load_constant(&Context->bytecode->stream, KAI_S32, dst, value);
		*output_register = dst;
	} break;

	case KAI_EXPR_IDENTIFIER: {
		Kai_Expr_Identifier* node = void_Expr;
		kai__bytecode_find_register(Context, node->source_code, output_register);
	} break;

	case KAI_EXPR_BINARY: {
		Kai_Expr_Binary* node = void_Expr;
		Kai_Reg left, right;
		kai__bytecode_emit_expression(Context, node->left, &left);
		kai__bytecode_emit_expression(Context, node->right, &right);
		Kai_Reg dst = kai__bytecode_allocate_register(Context);
		Kai_u8 op = 0;
		Kai_u8 cmp = 0;
		switch (node->op)
		{
			default: panic_with_message("not implemented");
			case '+': op = KAI_BOP_ADD; goto insert_math;
			case '-': op = KAI_BOP_SUB; goto insert_math;
			case '*': op = KAI_BOP_MUL; goto insert_math;
			case '/': op = KAI_BOP_DIV; goto insert_math;

			insert_math: {
				kai_bc_insert_math(&Context->bytecode->stream, KAI_S32, op, dst, left, right);
			} break;

			case '>':  cmp = KAI_CMP_LE; goto insert_compare;
			case '<':  cmp = KAI_CMP_GE; goto insert_compare;
			case '>=': cmp = KAI_CMP_LT; goto insert_compare;
			case '<=': cmp = KAI_CMP_GT; goto insert_compare;
			case '==': cmp = KAI_CMP_NE; goto insert_compare;
			case '!=': cmp = KAI_CMP_EQ; goto insert_compare;

			insert_compare: {
				kai_bc_insert_compare(&Context->bytecode->stream, KAI_S32, cmp, dst, left, right);
			} break;
		}
		*output_register = dst;
	} break;

	case KAI_EXPR_PROCEDURE_CALL: {
		Kai_Expr_Procedure_Call* node = void_Expr;
		Kai_Expr current = node->arg_head;
		Kai_Reg inputs[8];
		int i = 0;
		while (current != NULL)
		{
			kai__bytecode_emit_expression(Context, current, inputs + i++);
			current = current->next;
		}
		Kai_u32 branch;
		Kai_Reg dst = kai__bytecode_allocate_register(Context);
		kai_bc_insert_call(&Context->bytecode->stream, &branch, 1, (Kai_Reg[]){dst}, node->arg_count, inputs);
		*output_register = dst;
	} break;

	default: {
		Kai_String_Writer* writer = &debug_writer;
		kai__set_color(KAI_COLOR_IMPORTANT);
		kai__write("[emit_expression] skipping Expr(");
		kai__write_u32(Expr->id);
		kai__write(")\n");
		kai__set_color(KAI_COLOR_PRIMARY);
	} break;
	}

	return KAI_SUCCESS;
}

Kai_Result kai__bytecode_emit_statement(
	Kai__Bytecode_Generation_Context* Context,
	Kai_Expr Expr)
{
	void* void_Expr = Expr;
	switch (Expr->id) {
		case KAI_STMT_COMPOUND: {
			Kai_Stmt_Compound* node = void_Expr;
			Kai_Expr current = node->head;
			while (current != NULL)
			{
				kai__bytecode_emit_statement(Context, current);
				current = current->next;
			}
		} break;

		case KAI_STMT_RETURN: {
			Kai_Stmt_Return* node = void_Expr;
			Kai_Reg result;
			kai__bytecode_emit_expression(Context, node->expr, &result);
			kai_bc_insert_return(&Context->bytecode->stream, 1, &result);
		} break;

		case KAI_STMT_IF: {
			Kai_Stmt_If* node = void_Expr;
			Kai_Reg expr;
			kai__bytecode_emit_expression(Context, node->expr, &expr);
			Kai_u32 branch = 0;
			kai_bc_insert_branch(&Context->bytecode->stream, &branch, expr);
			kai__bytecode_emit_statement(Context, node->body);
			Kai_u32 location = Context->bytecode->stream.count;
			kai_bc_set_branch(&Context->bytecode->stream, branch, location);
			Kai_Allocator* allocator = &Context->dependency_graph->allocator;
			kai__array_append(&Context->bytecode->branch_hints, location);
			// TODO: else body
		} break;

		default: {
			Kai_String_Writer* writer = &debug_writer;
			kai__set_color(KAI_COLOR_IMPORTANT);
			kai__write("[emit_statement] skipping Stmt(");
			kai__write_u32(Expr->id);
			kai__write(")\n");
			kai__set_color(KAI_COLOR_PRIMARY);
		} break;
	}

	return KAI_SUCCESS;
}

Kai_Result kai__bytecode_emit_procedure(
	Kai__Bytecode_Generation_Context* Context,
	Kai_Expr_Procedure* Procedure,
	Kai_u32* out_Location)
{
	*out_Location = Context->bytecode->stream.count;

	Kai_Expr current = Procedure->body;
	while (current != NULL)
	{
		kai__bytecode_emit_statement(Context, current);
		current = current->next;
	}

	return KAI_SUCCESS;
}

#if 0
// TODO: Now we can give our compiler options like:
typedef struct {
	Kai_u32 Interpreter_Max_Step_Count;
	Kai_u32 Interpreter_Max_Call_Depth;
} Kai_Compile_Options;

#define KAI_DEFAULT_COMPILE_OPTIONS            \
	(Kai_Compile_Options) {                    \
		.Interpreter_Max_Call_Depth = 1024,    \
		.Interpreter_Max_Step_Count = 1000000, \
	}

void __example() {
	Kai_Compile_Options options = KAI_DEFAULT_COMPILE_OPTIONS;
	Kai_Program_Create_Info info = {0};
	Kai_Program program;
	kai_create_program(&info, &program);
}
#endif

Kai__DG_Value kai__value_from_expression(Kai__Bytecode_Generation_Context* Context, Kai_Expr Expr, Kai_Type* type)
{
	Kai_Allocator* allocator = &Context->dependency_graph->allocator;
	void* void_Expr = Expr;
	switch (Expr->id)
	{
		case KAI_EXPR_NUMBER: {
			// TODO: Only Parse number HERE, generate error if number can not be represented by the type
			Kai_Expr_Number* node = void_Expr;
			return (Kai__DG_Value) {.value = {.u64 = node->value.Whole_Part}};
		} break;

		case KAI_EXPR_PROCEDURE: {
			Kai_Expr_Procedure* procedure = void_Expr;

			if (procedure->body == NULL)
			{
				Kai_String_Writer* writer = &debug_writer;
				kai__write("Skip native procedure...\n");
				return (Kai__DG_Value) {};
			}

			// Add all input registers
            Context->registers.count = 0;
			Kai_Expr current = procedure->in_out_expr;
			for (int i = 0; i < procedure->in_count; i++)
			{
				kai__array_append(&Context->registers, (Kai__Bytecode_Register) {
					.reg = i,
					.name = current->name,
				});
				current = current->next;
			}

			Kai_u32 location = 0;
			Kai_Result result = kai__bytecode_emit_procedure(Context, procedure, &location);

#ifdef DEBUG_CODE_GENERATION
			Kai_Bytecode bytecode = {
				.data = Context->bytecode->stream.data,
				.count = Context->bytecode->stream.count,
				.range = {.offset = location},
				.ret_count = 1,
				.arg_count = procedure->in_count,
				.natives = NULL,
				.native_count = 0,
				.branch_hints = Context->bytecode->branch_hints.elements,
				.branch_count = Context->bytecode->branch_hints.count,
			};
			Kai_String_Writer* writer = &debug_writer;
			kai_bytecode_to_c(&bytecode, &debug_writer);
			kai__write_char('\n');
#endif

			if (result != KAI_SUCCESS)
				panic_with_message("something else went wrong...");

			return (Kai__DG_Value) {
				.procedure_location = location,
			};
		} break;

		case KAI_EXPR_IDENTIFIER: {
			Kai_Expr_Identifier* expr = void_Expr;
			Kai__DG_Node* other_node = kai__dg_find_node(Context->dependency_graph, expr->source_code);

			if (other_node == NULL)
			{
				panic_with_message("something went wrong");
				//return kai__error_internal(Context->error, KAI_STRING("I did not think this through..."));
			}

			if (type != NULL)
				*type = other_node->type;
			return other_node->value;
		} break;

		case KAI_EXPR_PROCEDURE_CALL: {
			// TODO: check that procedure type matches call input
			Kai_Expr_Procedure_Call* call = void_Expr;

			if (call->proc->id != KAI_EXPR_IDENTIFIER)
				panic_with_message("procedure must be identifier");

			Kai_Expr_Identifier* proc = (Kai_Expr_Identifier*)call->proc;
			Kai__DG_Node* proc_node = kai__dg_find_node(Context->dependency_graph, proc->source_code);

			// Generate procedure input
			// Note: have to put this here since interpreter must be free
			//       for recursive calls to use
			Kai_Value values[8] = {0};
			{
				Kai_Expr current = call->arg_head;
				for (Kai_u32 i = 0; i < call->arg_count; i++)
				{
					values[i] = kai__value_from_expression(Context, current, NULL).value;
					current = current->next;
				}
			}

			Kai_Interpreter* interp = &Context->bytecode->interp;
			kai_interp_load_from_stream(interp, &Context->bytecode->stream);
			kai_interp_reset(interp, proc_node->value.procedure_location);

			for (Kai_u32 i = 0; i < call->arg_count; i++)
			{
				kai_interp_set_input(interp, i, values[i]);
			}

			// TODO: don't assume only one return value
			kai_interp_push_output(interp, 0);

			int max_step_count = 65536;
			kai_interp_run(interp, max_step_count);

			if (interp->flags != KAI_INTERP_FLAGS_DONE) {
				panic_with_message("failed to interpret bytecode...");
			}

			return (Kai__DG_Value) { .value = interp->registers[0] };
		} break;

		default: {
			panic_with_message("kai__bytecode_generate_value ? %i", Expr->id);
		}
	}

	return (Kai__DG_Value) {.value = {.u64 = 0xDEADBEEFDEADBEEF}};
}

Kai_Result kai__bytecode_generate_value(Kai__Bytecode_Generation_Context* Context, Kai__DG_Node* node)
{
	if (node->value_flags & KAI__DG_NODE_EVALUATED)
		return KAI_SUCCESS;

	node->value = kai__value_from_expression(Context, node->expr, NULL);
	return KAI_SUCCESS;
}

Kai_Result kai__generate_bytecode(Kai__Bytecode_Create_Info* Info, Kai__Bytecode* out_Bytecode)
{
	Kai_Result result = KAI_SUCCESS;
	Kai__Dependency_Graph* graph = Info->dependency_graph;
	Kai_Allocator* allocator = &graph->allocator;
	int* order = graph->compilation_order;

	Kai__Bytecode_Generation_Context context = {
		.error = Info->error,
		.dependency_graph = Info->dependency_graph,
		.bytecode = out_Bytecode,
	};

	Kai_u32 _size = 0;
	// Initialization
	{
		Kai_Interpreter_Setup interp_info = {
			.max_register_count     = 4096,
			.max_call_stack_count   = 1024,
			.max_return_value_count = 1024,
		};
		_size = kai_interp_required_memory_size(&interp_info);
		interp_info.memory = kai__allocate(NULL, _size, 0);
		kai_interp_create(&context.bytecode->interp, &interp_info);
		context.bytecode->stream.allocator = allocator;
	}

	kai__dynamic_arena_allocator_create(&context.arena, &Info->dependency_graph->allocator);

	for (Kai_u32 i = 0; i < graph->nodes.count * 2; ++i)
	{
		int index = order[i];
		Kai__DG_Node_Index node_index = {
			.flags = (index & 1 ? KAI__DG_NODE_TYPE : 0),
			.value = index >> 1,
		};

		Kai__DG_Node* node = &graph->nodes.elements[node_index.value];
		
        #if 1
		if (node_index.flags & KAI__DG_NODE_TYPE)
        {
			if (node->type_flags & KAI__DG_NODE_EVALUATED)
				continue;
        }
        else
        {
			if (node->value_flags & KAI__DG_NODE_EVALUATED)
				continue;
        }
        #endif

#ifdef DEBUG_CODE_GENERATION
		Kai_String_Writer* writer = &debug_writer;
		kai__set_color(KAI_COLOR_IMPORTANT_2);
		kai__write("Generating ");
		kai__write_char((node_index.flags & KAI__DG_NODE_TYPE? 'T':'V'));
		kai__write_char('(');
		kai__write_string(node->name);
		kai__write_char(')');
		kai__write_char('\n');
		kai__set_color(KAI_COLOR_PRIMARY);
#endif

		if (node_index.flags & KAI__DG_NODE_TYPE)
		{
			result = kai__bytecode_generate_type(&context, node);

#ifdef DEBUG_CODE_GENERATION
			(&debug_writer)->write_string(0, KAI_STRING("--> Value "));
			kai_write_type((&debug_writer), node->type);
			(&debug_writer)->write_string(0, KAI_STRING("\n"));
#endif
		}
		else {
			result = kai__bytecode_generate_value(&context, node);

#ifdef DEBUG_CODE_GENERATION
			// TODO: write a function that does EXACTLY THIS!
			switch (node->type->type)
			{
				case KAI_TYPE_TYPE: {
					(&debug_writer)->write_string(0, KAI_STRING("--> Value "));
					kai_write_type((&debug_writer), node->value.type);
					(&debug_writer)->write_string(0, KAI_STRING("\n"));
				} break;
				case KAI_TYPE_INTEGER: {
					kai__write("--> Value ");
					kai__write_u32(node->value.value.u32);
					kai__write_char('\n');
				} break;
				case KAI_TYPE_FLOAT: {
					kai__write("--> Value ");
					kai__write_u32(node->value.value.u32);
					kai__write_char('\n');
				} break;
				case KAI_TYPE_PROCEDURE: {
					kai__write("--> Procedure ");
					kai__write_u32(node->value.procedure_location);
					kai__write_char('\n');
				} break;
				default: {
					panic_with_message("Excuse me, what type IS THIS?! %i", node->type->type);
				}
			}
#endif
		}

		if (result != KAI_SUCCESS)
			return result;
	}

	kai__array_destroy(&context.registers);
	kai__free(context.bytecode->interp.registers, _size);
	kai__dynamic_arena_allocator_destroy(&context.arena);
	return result;
}

extern inline uint32_t kai__arm64_add(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b0001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_sub(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b1001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_subs(uint32_t imm12, uint32_t Rn, uint8_t sf) { return (sf << 31) | (0b11100010 << 23) | (imm12 << 10) | (Rn << 5) | 0b11111; }
extern inline uint32_t kai__arm64_movz(uint32_t Rd, uint32_t imm16, uint8_t sf) { return (sf << 31) | (0b10100101 << 23) | (imm16 << 5) | Rd; }
extern inline uint32_t kai__arm64_bl(int32_t imm26) { return (0b100101 << 26) | (imm26 & 0x3FFFFFF); }
extern inline uint32_t kai__arm64_ret() { return 0xd65f03c0; }

Kai_Result kai__create_program(Kai__Program_Create_Info* Info, Kai_Program* out_Program)
{
	Kai_Allocator* allocator = &Info->allocator;

	//uint32_t code [] = {
	//	kai__arm64_sub(0, 1, 2, 1),
	//	kai__arm64_add(2, 1, 0, 1),
	//	kai__arm64_subs(69, 5, 0),
	//	kai__arm64_movz(4, 42069, 1),
	//	kai__arm64_bl(-16),
	//	kai__arm64_ret(),
	//};
//
	//for (int i = 0; i < sizeof(code)/sizeof(uint32_t); ++i)
	//{
	//	union {
	//		uint8_t byte[4];
	//		uint32_t u32;
	//	} temp = { .u32 = code[i] }, out;
	//	out.byte[0] = temp.byte[3];
	//	out.byte[1] = temp.byte[2];
	//	out.byte[2] = temp.byte[1];
	//	out.byte[3] = temp.byte[0];
	//	printf("%x\n", out.u32);
	//}

#define kai__bytecode_decode_add(Position, Dst_Var, Src1_Var, Src2_Var) \
    uint32_t Dst_Var = *(uint32_t*)(stream->data + Position);           \
    Position += sizeof(uint32_t);                                       \
    uint32_t Src1_Var = *(uint32_t*)(stream->data + Position);          \
    Position += sizeof(uint32_t);                                       \
    uint32_t Src2_Var = *(uint32_t*)(stream->data + Position);          \
    Position += sizeof(uint32_t);

	//KAI__ARRAY(uint32_t) arm64_machine_code = {0};

	// Generate machine code
	//Bc_Stream* stream = &Info->bytecode->stream;
	//
	//for (Kai_u32 i = 0; i < stream->count; ++i) \
	//	switch (stream->data[i])
	//	{
	//		case BC_OP_ADD: {
	//			kai__bytecode_decode_add(i, dst, src1, src2);
	//			uint32_t instr = kai__arm64_add(dst, src1, src2, 0);
	//			kai__array_append(&arm64_machine_code, instr);
	//		} break;
	//	}

	// Allocate memory to store machine code
	Kai_Program program = {0};
	program.allocator = *allocator;
	program.platform_machine_code = allocator->allocate(allocator->user, allocator->page_size, KAI_MEMORY_ACCESS_WRITE);
	program.code_size = allocator->page_size;

	//kai__memory_copy(
	//	program.platform_machine_code,
	//	arm64_machine_code.elements,
	//	arm64_machine_code.count * sizeof(uint32_t)
	//);


	kai__memory_copy(program.platform_machine_code,
                         "\xB8\x45\x00\x00\x00\xC3", 7);
	//uint32_t arm_instructions[] = {
	//	kai__arm64_movz(0, 0xA, 0),
	//	kai__arm64_ret(),
	//};
	//kai__memory_copy(program.platform_machine_code,
    //                     arm_instructions, sizeof(arm_instructions));

	// Set memory as executable
	allocator->set_access(allocator->user, program.platform_machine_code, program.code_size, KAI_MEMORY_ACCESS_EXECUTE);

	kai__hash_table_emplace(program.procedure_table, KAI_STRING("main"), program.platform_machine_code);

	*out_Program = program;

	//Info->error->result = KAI_ERROR_FATAL;
	//Info->error->message = KAI_STRING("todo");
	//return KAI_ERROR_FATAL;
	return KAI_SUCCESS;
}

void kai__destroy_dependency_graph(Kai__Dependency_Graph* Graph)
{
	Kai_Allocator* allocator = &Graph->allocator;
	kai__free(Graph->compilation_order, Graph->nodes.count * 2 * sizeof(Kai_u32));
	kai__array_iterate (Graph->nodes, i) {
		Kai__DG_Node* node = &Graph->nodes.elements[i];
		kai__array_destroy(&node->value_dependencies);
		kai__array_destroy(&node->type_dependencies);
	}
	kai__array_destroy(&Graph->nodes);
	kai__array_iterate (Graph->scopes, i) {
		Kai__DG_Scope* scope = &Graph->scopes.elements[i];
		kai__hash_table_destroy(scope->identifiers);
	}
	kai__array_destroy(&Graph->scopes);
}

void kai__destroy_bytecode(Kai__Bytecode* Bytecode)
{
	if (Bytecode->stream.capacity) {
		Kai_Allocator* allocator = Bytecode->stream.allocator;
		kai__free(Bytecode->stream.data, Bytecode->stream.capacity);
		kai__array_destroy(&Bytecode->branch_hints);
	}
}

void kai_destroy_program(Kai_Program Program)
{
	Kai_Allocator* allocator = &Program.allocator;
	kai__hash_table_destroy(Program.procedure_table);
	if (Program.code_size != 0)
		Program.allocator.free(
			Program.allocator.user,
			Program.platform_machine_code,
			Program.code_size
		);
}

void* kai_find_procedure(Kai_Program Program, Kai_str Name, Kai_Type Type)
{
	(void)Type;
	return *(void**)kai__hash_table_find(Program.procedure_table, Name);
}

#if 0

#define c_error_circular_dependency(REF) \
	error_circular_dependency(context, REF)

void* error_circular_dependency(Compiler_Context* context, Node_Ref ref) {
#define err context->error
	Node* node = context->dependency_graph.nodes.data + (ref & ~TYPE_BIT);

    err.result = KAI_ERROR_SEMANTIC;
    err.location.string = node->name;
    err.location.line = node->line_number;
    err.message.count = 0;
	err.message.data = (u8*)context->memory.temperary; // uses temperary memory

    str_insert_string(err.message, ref & TYPE_BIT? "type" : "value");
    str_insert_string(err.message, " of \"");
    str_insert_str   (err.message, node->name);
    str_insert_string(err.message, "\" cannot depend on itself");

    return (u8*)err.message.data + err.message.count;
#undef err
}

#define c_error_dependency_info(START, REF) \
	error_dependency_info(context, START, REF)

void* error_dependency_info(Compiler_Context* context, void* start, Node_Ref ref) {
#define err context->error
	_write_format("got node %i\n", ref & ~TYPE_BIT);
	Node* node = context->dependency_graph.nodes.data + (ref & ~TYPE_BIT);

    err.result = KAI_ERROR_INFO;
    err.location.string = node->name;
    err.location.line = node->line_number;
    err.message.count = 0;
	err.message.data = (u8*)context->memory.temperary; // uses temperary memory
    err.context = KAI_EMPTY_STRING;
    err.next = NULL;

    str_insert_string(err.message, "see ");
    str_insert_string(err.message, ref & TYPE_BIT? "type" : "value");
    str_insert_string(err.message, " of \"");
    str_insert_str   (err.message, node->name);
    str_insert_string(err.message, "\"");

    return (u8*)err.message.data + err.message.count;
#undef err
}

#define scopes       context->dependency_graph.scopes
#define dependencies context->dependency_graph.dependencies
#define nodes        context->dependency_graph.nodes

Kai_Type type_of_expression(Compiler_Context* context, Kai_Expr expr, u32 scope_index) {
	switch (expr->id)
	{
    default: {
		panic_with_message("undefined expr [type_of_expression] (id = %d)\n", expr->id);
	}
	
	break; case KAI_EXPR_IDENTIFIER: {
		// locate this value in the scope
		u32 node_index = recursive_scope_find(context, scope_index, expr->source_code);

		// confirm we found something
		if (node_index == NONE) {
			_write("ERROR: did not find identifier [type_of_expression]");
			return NULL;
		}

		Node* node = nodes.data + node_index;

		// confirm that this value is evaluated
		if (!(node->type_flags & NODE_EVALUATED)) {
			_write("ERROR: node is not evaluated [type_of_expression]");
			return NULL;
		}

		_write("got a type!!\n");

		kai_write_type(writer, node->type);

		// set the value of this node??
		return node->type;
	}

	}

	return NULL;
}

Kai_bool compile_type (Compiler_Context* context, u32 index) {
	Node* node = nodes.data + index;
	Kai_Expr expr = node->expr;

	_write_format("Compiling : %i, expr = %i\n", index, expr->id);

	switch (expr->id)
	{
    default: {
		panic_with_message("undefined expr [compile_type] (id = %d)\n", expr->id);
	}
	
	break; case KAI_EXPR_IDENTIFIER: {
		Kai_Type type = type_of_expression(context, expr, node->scope);

		if (type == NULL) return KAI_FALSE;

		node->type = type;
	}

	}
	return KAI_TRUE;
}
#endif

#ifdef KAI_USE_MEMORY_API

typedef struct {
	Kai_s64 total_allocated;
	//Kai_Memory_Tracker tracker;
} Kai__Memory_Internal;

#if defined(KAI__PLATFORM_WASM)
extern void* __wasm_allocate(Kai_u32 size);
extern void* __wasm_free(void* ptr);

// TODO: what the F is this?
void* memset(void* dst, int val, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		((char*)dst)[i] = (char)val;
	}
	return dst;
}
void* memcpy(void* dst, void* src, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		((char*)dst)[i] = ((char*)src)[i];
	}
	return dst;
}

static void* kai__memory_allocate(Kai_ptr user, Kai_u32 size, Kai_u32 access)
{
    //__wasm_console_log("kai__memory_allocate", size);
    return __wasm_allocate(size);
}

static void kai__memory_free(Kai_ptr user, Kai_ptr ptr, Kai_u32 size)
{
    //__wasm_console_log("kai__memory_free", size);
    __wasm_free(ptr);
}

static void kai__memory_set_access(Kai_ptr user, Kai_ptr ptr, Kai_u32 size, Kai_u32 access)
{
    //__wasm_console_log("kai__memory_set_access", size);
}

static Kai_u32 kai__page_size()
{
    return 64 * 1024; // 64 KiB
}

#elif defined(KAI__PLATFORM_LINUX) || defined(KAI__PLATFORM_APPLE)
#include <sys/mman.h> // -> mmap
#include <unistd.h>   // -> getpagesize

static void* kai__memory_allocate(Kai_ptr user, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ)?    PROT_READ  : 0;
    flags |= (access & KAI_MEMORY_ACCESS_WRITE)?   PROT_WRITE : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)? PROT_EXEC  : 0;
    void* ptr = mmap(NULL, size, flags, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
    Kai__Memory_Internal* internal = user;
    internal->total_allocated += size;
    return (ptr == MAP_FAILED) ? NULL : ptr;
}

static void kai__memory_free(Kai_ptr user, Kai_ptr ptr, Kai_u32 size)
{
    munmap(ptr, size);
    Kai__Memory_Internal* internal = user;
    internal->total_allocated -= size;
}

static void kai__memory_set_access(Kai_ptr user, Kai_ptr ptr, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ)?    PROT_READ  : 0;
    flags |= (access & KAI_MEMORY_ACCESS_WRITE)?   PROT_WRITE : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)? PROT_EXEC  : 0;
    mprotect(ptr, size, flags);
}

#define kai__page_size() sysconf(_SC_PAGESIZE)

#elif defined(KAI__PLATFORM_WINDOWS)
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef int            BOOL;
typedef uintptr_t      SIZE_T;
__declspec(dllimport) void* __stdcall VirtualAlloc(void* lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
__declspec(dllimport) BOOL __stdcall VirtualProtect(void* lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD* lpflOldProtect);
__declspec(dllimport) BOOL __stdcall VirtualFree(void* lpAddress, SIZE_T dwSize, DWORD dwFreeType);
__declspec(dllimport) void __stdcall GetSystemInfo(struct _SYSTEM_INFO* lpSystemInfo);

static void* kai__memory_allocate(Kai_ptr user, Kai_u32 size, Kai_u32 access)
{
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ_WRITE)? 0x04 : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)?    0x10 : 0;
    void* ptr = VirtualAlloc(NULL, size, 0x1000|0x2000, flags);
    if (!ptr) return NULL;
    Kai__Memory_Internal* internal = user;
	internal->total_allocated += size;
    return ptr;
}

static void kai__memory_free(Kai_ptr user, Kai_ptr ptr, Kai_u32 size)
{
    kai__assert(VirtualFree(ptr, 0, 0x8000) != 0);
    Kai__Memory_Internal* internal = user;
    internal->total_allocated -= size;
}

static void kai__memory_set_access(Kai_ptr user, Kai_ptr ptr, Kai_u32 size, Kai_u32 access)
{
	kai__unused(user);
    int flags = 0;
    flags |= (access & KAI_MEMORY_ACCESS_READ_WRITE) ? 0x04 : 0;
    flags |= (access & KAI_MEMORY_ACCESS_EXECUTE)    ? 0x10 : 0;
	DWORD old;
    kai__assert(VirtualProtect(ptr, size, flags, &old) != 0);
}

static Kai_u32 kai__page_size()
{
	struct {
		DWORD dwOemId;
		DWORD dwPageSize;
		int _padding[10];
	} info;
	GetSystemInfo((struct _SYSTEM_INFO*)&info);
	return (Kai_u32)info.dwPageSize;
}

#endif

#if defined(KAI__PLATFORM_WASM)
static Kai_u8* scratch = 0;
static Kai_u32 offset = 0;
// TODO: write my own heap allocator 💀
static void* kai__memory_heap_allocate(void* user, void* old_ptr, Kai_u32 new_size, Kai_u32 old_size)
{
    Kai_u32 ptr = offset;
    offset += new_size;
    for (int i = 0; i < old_size; ++i) {
        scratch[ptr + i] = ((Kai_u8*)old_ptr)[i];
    }
    //__wasm_console_log("heap allocated", new_size);
    return scratch + ptr;
}
#else
#include "stdlib.h" // -> malloc, free

static void* kai__memory_heap_allocate(void* user, void* old_ptr, Kai_u32 new_size, Kai_u32 old_size)
{
    void* ptr = NULL;
    if (new_size == 0) {
        free(old_ptr);
    } else {
        kai__assert(old_ptr != NULL || old_size == 0);
        ptr = realloc(old_ptr, new_size);
        if (ptr != NULL && old_ptr == NULL) {
            kai__memory_zero(ptr, new_size);
        }
    }
    Kai__Memory_Internal* internal = user;
    internal->total_allocated += new_size;
    internal->total_allocated -= old_size;
    return ptr;
}
#endif

KAI_API (Kai_Result) kai_memory_create(Kai_Allocator* Memory)
{
	kai__assert(Memory != NULL);
	
    Memory->allocate      = kai__memory_allocate;
    Memory->free          = kai__memory_free;
    Memory->heap_allocate = kai__memory_heap_allocate;
    Memory->set_access    = kai__memory_set_access;
    Memory->page_size     = kai__page_size();
#if defined(KAI__PLATFORM_WASM)
	scratch = __wasm_allocate(kai__page_size() * 2);
	offset = 0;
	Memory->user          = 0;
#else
	Memory->user          = malloc(sizeof(Kai__Memory_Internal));
#endif
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
		
#if !defined(KAI__PLATFORM_WASM)
	free(Memory->user);
#endif
    *Memory = (Kai_Allocator) {0};
	return KAI_SUCCESS;
}

KAI_API (Kai_u64) kai_memory_usage(Kai_Allocator* Memory)
{
    Kai__Memory_Internal* internal = Memory->user;
    return (Kai_u64)internal->total_allocated;
}

#endif
#ifdef KAI_USE_DEBUG_API
#include <stdio.h>
#include <locale.h>

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

        case KAI_U32: {
            fprintf(f, "%u", Value.u32);
        } break;

        default: {
            fprintf(f, "[StdOut Writer] -- Case not defined for type %i\n", Type);
        } break;
    }
}

static void kai__file_writer_write_string(Kai_ptr User, Kai_str string)
{
	if (User == NULL) return;
	fwrite(string.data, 1, string.count, User);
}

static void kai__file_writer_set_color(Kai_ptr user, Kai_Write_Color color)
{
    kai__unused(user);
    kai__unused(color);
}

static void kai__stdout_write_value(void* User, Kai_u32 Type, Kai_Value Value, Kai_Write_Format Format)
{
	kai__unused(User);
	kai__file_writer_write_value(stdout, Type, Value, Format);
}

static void kai__stdout_write_string(Kai_ptr User, Kai_str string)
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

KAI_API (Kai_String_Writer*) kai_debug_stdout_writer(void)
{
#if defined(KAI__PLATFORM_WINDOWS)
	SetConsoleOutputCP(65001);
#endif
	setlocale(LC_CTYPE, ".UTF8");
	static Kai_String_Writer writer = {
		.write_string   = kai__stdout_write_string,
		.write_value    = kai__stdout_write_value,
		.set_color      = kai__stdout_writer_set_color,
		.user           = NULL,
	};
	writer.user = stdout;
	return &writer;
}

KAI_API (void) kai_debug_open_file_writer(Kai_String_Writer* writer, const char* path)
{
    *writer = (Kai_String_Writer) {
        .write_string   = kai__file_writer_write_string,
    	.write_value    = kai__file_writer_write_value,
        .set_color      = kai__file_writer_set_color,
        .user           = kai__stdc_file_open(path, "wb"),
    };
}

KAI_API (void) kai_debug_close_file_writer(Kai_String_Writer* writer)
{
    if (writer->user != NULL)
        fclose(writer->user);
}

#endif

// TODO: move to kai_dev
#if !defined(KAI__PLATFORM_WASM)
#include <stdio.h>
#include <stdlib.h>
static char const* kai__file(char const* cs)
{
	Kai_str s = kai_str_from_cstring(cs);
	Kai_u32 i = s.count - 1;
	while (i > 0)
	{
		if (cs[i] == '/' || cs[i] == '\\')
		{
			return cs + i + 1;
		}
		i -= 1;
	}
	return cs;
}
void kai__fatal_error(
	char const* Desc,
	char const* Message,
	char const* File,
	int         Line)
{
	printf("\x1b[91m%s\x1b[0m: %s\nin \x1b[92m%s:%i\x1b[0m",
		Desc, Message, kai__file(File), Line
	);
	exit(1);
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
