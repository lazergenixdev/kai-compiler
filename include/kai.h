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

#endif
#ifndef KAI__SECTION_CORE_STRUCTS

enum {
    //        SIZE       ID
    KAI_U8  = (1 << 4) | 0,
    KAI_U16 = (2 << 4) | 1,
    KAI_U32 = (4 << 4) | 2,
    KAI_U64 = (8 << 4) | 3,
    KAI_S8  = (1 << 4) | 4,
    KAI_S16 = (2 << 4) | 5,
    KAI_S32 = (4 << 4) | 6,
    KAI_S64 = (8 << 4) | 7,
    KAI_F32 = (4 << 4) | 8,
    KAI_F64 = (8 << 4) | 9,

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

#endif
#ifndef KAI__SECTION_CORE_API

KAI_API (Kai_bool) kai_str_equals       (Kai_str A, Kai_str B);
KAI_API (Kai_str)  kai_str_from_cstring (char const* String);

KAI_API (Kai_vector3_u32) kai_version        (void);
KAI_API (Kai_str)         kai_version_string (void);

/// @param out_Syntax_Tree Must be freed using @ref(kai_destroy_syntax_tree)
KAI_API (Kai_Result) kai_create_syntax_tree (Kai_Syntax_Tree_Create_Info* Info, Kai_Syntax_Tree* out_Syntax_Tree);

KAI_API (void)       kai_destroy_syntax_tree        (Kai_Syntax_Tree* Syntax_Tree);
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

#define kai__allocate(Old, New_Size, Old_Size) \
	allocator->heap_allocate(allocator->user, Old, New_Size, Old_Size)

#define kai__free(Ptr,Size) \
	allocator->heap_allocate(allocator->user, Ptr, 0, Size)

#define kai__unused(VAR) (void)VAR

#define kai__for_n(N) for (Kai_int i = 0; i < (Kai_int)(N); ++i)

#define kai__count(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

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
void kai__token_type_string(Kai_u32 Type, Kai_str* out_String);

Kai__Token* kai__next_token(Kai__Tokenizer* context);
Kai__Token* kai__peek_token(Kai__Tokenizer* context);

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

KAI_API (Kai_Result) kai_memory_create   (Kai_Allocator* out_Allocator);
KAI_API (Kai_Result) kai_memory_destroy  (Kai_Allocator* Allocator);
KAI_API (Kai_u64)    kai_memory_usage    (Kai_Allocator* Allocator);

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

#ifdef KAI_USE_MEMORY_API
// TODO
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
	kai__file_writer_write_value(stdout, Type, Value, Format);
}

static void kai__stdout_write_string(Kai_ptr User, Kai_str string)
{
	fwrite(string.data, 1, string.count, stdout);
}

static void kai__stdout_writer_set_color(Kai_ptr User, Kai_Write_Color color)
{
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
