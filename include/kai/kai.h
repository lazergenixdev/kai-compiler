/**
 *  < Kai >---< Scripting Language >
 * 
 * @author:       lazergenixdev@gmail.com
 * @development:  https://github.com/lazergenixdev/kai-compiler
 * @license:      GNU GPLv3 (see end of file)
 */
#ifndef KAI__H
#define KAI__H
#include <stdint.h>

/*
    CONVENTIONS:
  
    Types:           begin with "Kai_"
    Functions:       begin with "kai_"
    Enums/Macros:    begin with "KAI_"
    Internal API:    begin with "KAI__" or "kai__" or "Kai__"
    
    define  KAI_USE_MEMORY_API  to enable default memory allocator
    define  KAI_USE_DEBUG_API   to enable debug functionality (such as a function to print errors)
    define  KAI_USE_CPP_API     to enable C++ API (experimental)
  
    This header is divided into sections with `KAI__SECTION` for convenience
        KAI__SECTION_BUILTIN_TYPES
        KAI__SECTION_TYPE_INFO_STRUCTS
        KAI__SECTION_CORE_STRUCTS
        KAI__SECTION_SYNTAX_TREE
        KAI__SECTION_PROGRAM
        KAI__SECTION_CORE_API
 */
   
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
#if defined(_WIN32)
#	define KAI__PLATFORM_WINDOWS
#elif defined(__APPLE__)
#   define KAI__PLATFORM_APPLE
#elif defined(__linux__)
#   define KAI__PLATFORM_LINUX
#else
#	define KAI__PLATFORM_UNKNOWN
// TODO: do something better here
#   error "[KAI] Platform not recognized!"
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

#define KAI_API(RETURN_TYPE) extern RETURN_TYPE

#define KAI_EMPTY_STRING    KAI_STRUCT(Kai_str){0}
#define KAI_STRING(LITERAL) KAI_STRUCT(Kai_str){(Kai_u8*)(LITERAL), sizeof(LITERAL)-1}
#define KAI_BOOL(EXPR)      ((Kai_bool)((EXPR) ? KAI_TRUE : KAI_FALSE))

#ifdef __cplusplus
extern "C" {
#endif

#if defined(KAI__COMPILER_CLANG) || defined(KAI__COMPILER_GCC)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wc11-extensions"
#	pragma GCC diagnostic ignored "-Wunused-function"
#elif defined(KAI__COMPILER_MSVC)
#	pragma warning(push)
#	pragma warning(disable : 4201) // " nonstandard extension used: nameless struct/union
#endif

#ifndef KAI__SECTION_BUILTIN_TYPES

enum {
	KAI_FALSE = 0,
	KAI_TRUE  = 1,
};

#define KAI_X_PRIMITIVE_TYPES \
	X( int8_t   , s8  )       \
	X( int16_t  , s16 )       \
	X( int32_t  , s32 )       \
	X( int64_t  , s64 )       \
	X( uint8_t  , u8  )       \
	X( uint16_t , u16 )       \
	X( uint32_t , u32 )       \
	X( uint64_t , u64 )       \
	X( double   , f64 )       \
	X( float    , f32 )

#define X(TYPE, NAME) typedef TYPE Kai_##NAME;
	KAI_X_PRIMITIVE_TYPES
#undef X

typedef uint8_t   Kai_bool;
typedef void*     Kai_ptr;
typedef  intptr_t Kai_int;
typedef uintptr_t Kai_uint;

typedef struct Kai_Type_Info* Kai_Type;

#define KAI__VECTOR2_STRUCT(TYPE, NAME) \
	union {                             \
		struct { TYPE x, y; };          \
		struct { TYPE r, g; };          \
	}
#define KAI__VECTOR3_STRUCT(TYPE, NAME) \
	union {                             \
		Kai_vector2_##NAME xy;          \
		struct { TYPE x, y, z; };       \
		struct { TYPE r, g, b; };       \
	}
#define KAI__VECTOR4_STRUCT(TYPE, NAME) \
	union {                             \
		Kai_vector2_##NAME xy;          \
		Kai_vector3_##NAME xyz;         \
		struct { TYPE x, y, z, w; };    \
		struct { TYPE r, g, b, a; };    \
	}

//! TODO: Complete vector + matrix + ... types
//! NOTE: probably better idea to just use raw pointers in compiler, to avoid this macro mess
#define X(TYPE, NAME) \
	typedef KAI__VECTOR2_STRUCT(TYPE, NAME) Kai_vector2_##NAME; \
	typedef KAI__VECTOR3_STRUCT(TYPE, NAME) Kai_vector3_##NAME; \
	typedef KAI__VECTOR4_STRUCT(TYPE, NAME) Kai_vector4_##NAME; \
	typedef int Kai_matrix4x4_##NAME;
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
	KAI_TYPE_SLICE     = 5,
	KAI_TYPE_STRING    = 6,
	KAI_TYPE_STRUCT    = 7,
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
	Kai_Type*  member_types;
	Kai_str*   member_names;
} Kai_Type_Info_Struct;

#endif
#ifndef KAI__SECTION_CORE_STRUCTS

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

enum {
	KAI_SUCCESS = 0,

	KAI_ERROR_SYNTAX,
	KAI_ERROR_SEMANTIC,
	KAI_ERROR_INFO,

	// 'Meta' Errors
	KAI_ERROR_FATAL,    // means compiler bug probably
	KAI_ERROR_INTERNAL, // means compiler error unrelated to source code (e.g. out of memory)

	KAI_RESULT_COUNT,
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

#define KAI__ARRAY(T) \
struct {              \
	Kai_u32 count;    \
	Kai_u32 capacity; \
	T*      elements; \
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

#define KAI__MINIMUM_ARENA_BUCKET_SIZE 0x40000

typedef struct Kai__Arena_Bucket {
    struct Kai__Arena_Bucket* prev;
} Kai__Arena_Bucket;

typedef struct {
    Kai__Arena_Bucket*   current_bucket;
    Kai_u32              current_allocated;
    Kai_u32              bucket_size;
    Kai_Allocator        allocator;
} Kai__Dynamic_Arena_Allocator;

typedef struct {
    Kai_u32     type;
    Kai_u32     line_number;
    Kai_str     string;
    Kai_Number  number;
} Kai__Token;

typedef struct {
	Kai_u32 size;
	Kai_u32 capacity;
	Kai_u8* data;
	Kai_u32 offset;
} Kai__Dynamic_Buffer;

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
	KAI_STMT_RETURN         = 8,
	KAI_STMT_DECLARATION    = 9,
	KAI_STMT_ASSIGNMENT     = 10,
	KAI_STMT_COMPOUND       = 11,
	KAI_STMT_IF             = 12,
	KAI_STMT_FOR            = 13,
	KAI_STMT_DEFER          = 14,
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
#ifndef KAI__SECTION_PROGRAM

typedef struct {
	Kai_ptr address;
	Kai_str name;
	Kai_str signature;
} Kai_Native_Procedure;

typedef struct {
	Kai_Syntax_Tree*      trees;
	Kai_u32               tree_count;
	Kai_Allocator         allocator;
	Kai_Error*            error;
	Kai_Native_Procedure* native_procedures;
	Kai_u32               native_procedure_count;
} Kai_Program_Create_Info;

typedef struct {
	void* platform_machine_code;
	Kai_u32 code_size;
	//KAI__HASH_TABLE(void*) procedure_table;
	Kai_Allocator allocator;
} Kai_Program;

#endif
#ifndef KAI__SECTION_CORE_API

KAI_API (Kai_bool) kai_str_equals(Kai_str A, Kai_str B);
KAI_API (Kai_str) kai_str_from_cstring(char const* String);
KAI_API (Kai_vector3_u32) kai_get_version(void);
KAI_API (Kai_str) kai_get_version_string(void);
KAI_API (Kai_Result) kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* Info, Kai_Syntax_Tree* out_Syntax_Tree);
KAI_API (void) kai_destroy_syntax_tree(Kai_Syntax_Tree* Syntax_Tree);
KAI_API (Kai_Result) kai_create_program(Kai_Program_Create_Info* Info, Kai_Program* out_Program);
KAI_API (Kai_Result) kai_create_program_from_source(Kai_str Source, Kai_Allocator* Allocator, Kai_Error* out_Error, Kai_Program* out_Program);
KAI_API (void) kai_destroy_program(Kai_Program Program);
KAI_API (void*) kai_find_procedure(Kai_Program Program, Kai_str Name, Kai_Type opt_Type);
KAI_API (void) kai_destroy_error(Kai_Error* Error, Kai_Allocator* Allocator);

#endif
#ifndef KAI__SECTION_INTERNAL_API

// TODO: should this be here? should this exist? should I?
KAI_API (void) kai__fatal_error(char const* Desc, char const* Message, char const* File, int Line);

#define kai__assert(EXPR) \
	if (!(EXPR))          \
		kai__fatal_error("assertion failed", #EXPR, __FILE__, __LINE__)

		
static inline void kai__memcpy(void* Dst, void* Src, Kai_u32 Size)
{
	for (Kai_u32 i = 0; i < Size; ++i)
	{
		((Kai_u8*)Dst)[i] = ((Kai_u8*)Src)[i];
	}
}

//! @return The smallest number n such that n*Den >= Num
static inline Kai_u32 kai__ceil_div(Kai_u32 Num, Kai_u32 Den)
{
    return (Num + Den - 1) / Den;
}

static inline void kai__dynamic_arena_allocator_create(Kai__Dynamic_Arena_Allocator* Arena, Kai_Allocator* Allocator)
{
	kai__assert(Arena != 0);
	kai__assert(Allocator != 0);
    Arena->allocator = *Allocator;
    Arena->bucket_size = kai__ceil_div(KAI__MINIMUM_ARENA_BUCKET_SIZE, Allocator->page_size)
	                   * Allocator->page_size;
    Arena->current_allocated = sizeof(Kai__Arena_Bucket*);
    Arena->current_bucket = Allocator->allocate(
		Allocator->user,
		Arena->bucket_size,
		KAI_MEMORY_ACCESS_READ_WRITE
	);
}

static inline void kai__dynamic_arena_allocator_free_all(Kai__Dynamic_Arena_Allocator* Arena)
{
	kai__assert(Arena != 0);
    Kai__Arena_Bucket* bucket = Arena->current_bucket;
    while (bucket) {
        Kai__Arena_Bucket* prev = bucket->prev;
        Arena->allocator.free(Arena->allocator.user, bucket, Arena->bucket_size);
        bucket = prev;
    }
    Arena->current_bucket = 0;
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
	kai__assert(Arena != 0);

    if (Size > Arena->bucket_size)
        return 0;
    
    if (Arena->current_allocated + Size > Arena->bucket_size)
    {
        Kai__Arena_Bucket* new_bucket = Arena->allocator.allocate(
            Arena->allocator.user,
            Arena->bucket_size,
            KAI_MEMORY_ACCESS_READ_WRITE
        );
        if (new_bucket == 0)
            return 0; // Bubble down failure
        new_bucket->prev = Arena->current_bucket;
        Arena->current_bucket = new_bucket;
        Arena->current_allocated = sizeof(Kai__Arena_Bucket*);
    }

    Kai_u8* bytes = (Kai_u8*)Arena->current_bucket;
    void* ptr = bytes + Arena->current_allocated;
    Arena->current_allocated += Size;
    return ptr;
}

#define kai__bit_array_count(Bit_Count) \
	(((((Bit_Count) - 1) / 64) + 1) * sizeof(Kai_u64))

// Annoying to always have to pass allocator around,
//  so macros assume variable defined `allocator` exists
//  and is of type `Kai_Allocator*`

#define kai__array_reserve(Ptr_Array, Size) \
	kai__array_reserve_stride(Ptr_Array, Size, allocator, sizeof((Ptr_Array)->elements[0]))

#define kai__array_destroy(Ptr_Array) \
	kai__array_destroy_stride(Ptr_Array, allocator, sizeof((Ptr_Array)->elements[0]))

#define kai__array_append(Ptr_Array, ...) \
	kai__array_grow_stride(Ptr_Array, (Ptr_Array)->count + 1, allocator, sizeof((Ptr_Array)->elements[0])), \
	(Ptr_Array)->elements[(Ptr_Array)->count++] = (__VA_ARGS__)

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

static inline void kai__array_reserve_stride(void* Array, Kai_u32 Size, Kai_Allocator* Allocator, Kai_u32 Stride)
{
	KAI__ARRAY(void)* array = Array;
	
	if (Size <= array->capacity)
		return;

	array->elements = Allocator->heap_allocate(
		Allocator->user,
		array->elements,
		Stride * Size,
		Stride * array->capacity
	);
	array->capacity = Size;
}

static inline void kai__array_grow_stride(void* Array, Kai_u32 Min_Size, Kai_Allocator* Allocator, Kai_u32 Stride)
{
	KAI__ARRAY(void)* array = Array;
	
	if (Min_Size <= array->capacity)
		return;
	
	Kai_u32 new_capacity = (Min_Size * 3) / 2;
	array->elements = Allocator->heap_allocate(
		Allocator->user,
		array->elements,
		Stride * new_capacity,
		Stride * array->capacity
	);
	array->capacity = new_capacity;
}

static inline void kai__array_shrink_to_fit_stride(void* Array, Kai_Allocator* Allocator, Kai_u32 Stride)
{
	KAI__ARRAY(void)* array = Array;
	if (array->capacity != 0)
	{
		array->elements = Allocator->heap_allocate(
			Allocator->user,
			array->elements,
			Stride * array->count,
			Stride * array->capacity
		);
		array->capacity = array->count;
	}
}

static inline void kai__array_destroy_stride(void* Ptr_Array, Kai_Allocator* Ptr_Allocator, Kai_u32 Stride)
{
	KAI__ARRAY(int)* array = Ptr_Array;
	if (array->capacity != 0)
	{
		Ptr_Allocator->heap_allocate(
			Ptr_Allocator->user,
			array->elements,
			0,
			Stride * array->capacity
		);
	}
	array->elements = 0;
	array->capacity = 0;
	array->count    = 0;
}

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

static inline void kai__hash_table_grow(void* Table, Kai_Allocator* Allocator, Kai_u32 Elem_Size)
{
	KAI__HASH_TABLE(void)* table = Table;
	
	Kai_u32 new_capacity = (table->capacity == 0) ? 8 : table->capacity * 2;
	Kai_u32 occupied_size = kai__bit_array_count(new_capacity);
	Kai_u32 hashes_size   = new_capacity * sizeof(Kai_u64);
	Kai_u32 keys_size     = new_capacity * sizeof(Kai_str);
	Kai_u32 values_size   = new_capacity * Elem_Size;

	void* new_ptr = Allocator->heap_allocate(
		Allocator->user,
		0,
		occupied_size + hashes_size + keys_size + values_size,
		0
	);

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
				kai__memcpy(values + index * Elem_Size, src, Elem_Size);
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
		Allocator->heap_allocate(
			Allocator->user,
			table->occupied,
			0,
			kai__bit_array_count(table->capacity)
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
	kai__assert(Table     != 0);
	kai__assert(Allocator != 0);
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

	// Should be impossible to reach,
	// but make sure to crash the program if we reach here
	return 0xFFFFFFFF;
}

static inline void* kai__hash_table_find_stride(void* Table, Kai_str Key, Kai_u32 Elem_Size)
{
	kai__assert(Table     != 0);
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
			return 0; // Slot was empty
		}
		else if (table->hashes[index] == hash
		     &&  kai_str_equals(table->keys[index], Key))
		{
			return (Kai_u8*)table->values + Elem_Size * index;
		}

		index = (index + 1) & mask;
	}

	return 0;
}

static inline Kai_bool kai__hash_table_get_stride(void* Table, Kai_str Key, void* out_Value, Kai_u32 Value_Size, Kai_u32 Elem_Size)
{
	void* elem_ptr = kai__hash_table_find_stride(Table, Key, Elem_Size);
	if (elem_ptr == 0)
		return KAI_FALSE;
	kai__memcpy(out_Value, elem_ptr, Value_Size);
	return KAI_TRUE;
}

static inline void kai__hash_table_destroy_stride(void* Table, Kai_Allocator* Allocator, Kai_u32 Elem_Size)
{
	KAI__HASH_TABLE(void)* table = Table;
	if (table->capacity != 0)
	{
		Allocator->heap_allocate(
			Allocator->user,
			table->occupied,
			0,
			kai__bit_array_count(table->capacity)
			+ table->capacity * (sizeof(Kai_u64) + sizeof(Kai_str) + Elem_Size)
		);
	}
	table->count    = 0;
	table->capacity = 0;
	table->occupied = 0;
	table->hashes   = 0;
	table->keys     = 0;
	table->values   = 0;
}

#define kai__dynamic_buffer_append_string(Builder, String) \
	kai__dynamic_buffer_allocator_append(Builder, String.data, String.count, allocator)

#define kai__dynamic_buffer_append_string_max(Builder, String, Max_Count) \
	kai__dynamic_buffer_allocator_append_max(Builder, String, Max_Count, allocator)

static void kai__dynamic_buffer_allocator_append(Kai__Dynamic_Buffer* Buffer, void* Data, Kai_u32 Size, Kai_Allocator* Allocator)
{
	kai__array_grow_stride(Buffer, Buffer->size + Size, Allocator, 1);
	kai__memcpy(Buffer->data + Buffer->size, Data, Size);
	Buffer->size += Size;
}

static void kai__dynamic_buffer_allocator_append_max(Kai__Dynamic_Buffer* Buffer, Kai_str String, Kai_u32 Max_Count, Kai_Allocator* Allocator)
{
	kai__assert(Max_Count > 3);
	Kai_u32 size = (String.count > Max_Count) ? Max_Count : String.count;
	kai__array_grow_stride(Buffer, Buffer->size + size, Allocator, 1);
	if (String.count < Max_Count)
	{
		kai__memcpy(Buffer->data + Buffer->size, String.data, String.count);
		Buffer->size += String.count;
	}
	else
	{
		kai__memcpy(Buffer->data + Buffer->size, String.data, Max_Count - 3);
		kai__memcpy(Buffer->data + Buffer->size + Max_Count - 3, "...", 3);
		Buffer->size += Max_Count;
	}
}

#define kai__range_to_data(Range, Memory)   (void*)(((Kai_u8*)(Memory).data) + (Range).offset)
#define kai__range_to_string(Range, Memory) KAI_STRUCT(Kai_str) { .count = (Range).count, .data = kai__range_to_data(Range, Memory) }

static Kai_range kai__dynamic_buffer_next(Kai__Dynamic_Buffer* Buffer)
{
	Kai_range out = {
		.count = Buffer->size - Buffer->offset,
		.offset = Buffer->offset
	};
	Buffer->offset = Buffer->size;
	return out;
}

#define kai__dynamic_buffer_push(Buffer, Size) \
	kai__dynamic_buffer_allocator_push(Buffer, Size, allocator)

static Kai_range kai__dynamic_buffer_allocator_push(Kai__Dynamic_Buffer* Buffer, Kai_u32 Size, Kai_Allocator* Allocator)
{
	kai__array_grow_stride(Buffer, Buffer->size + Size, Allocator, 1);
	Kai_range out = { .count = Size, .offset = Buffer->offset };
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

//	Kai__String_Builder* builder;
//	kai__str_create(&builder);
//	kai__str_append(&builder, KAI_STRING("did not expect "));
//	error->message = kai__str_done(&builder);

static Kai_Result kai__error_internal(Kai_Error* out_Error, Kai_str Message)
{
	*out_Error = (Kai_Error) {
		.message = Message,
		.result = KAI_ERROR_INTERNAL,
	};
	return KAI_ERROR_INTERNAL;
}

/*void* kai__syntax_error(Kai_Error* out_Error, Kai_Allocator* allocator, Kai__Token* token, Kai_str where, Kai_str wanted)
{
    Kai_Error* e = out_Error;
    if (e->result != KAI_SUCCESS) return NULL;
    e->result          = KAI_ERROR_SYNTAX;
    e->location.string = token->string;
    e->location.line   = token->line_number;
    e->context         = wanted;
    e->message = (Kai_str){.count = 0, .data = (Kai_u8*)hack__delete_me};
    adjust_source_location(&e->location.string, token->type);
    str_insert_string(e->message, "unexpected ");
    insert_token_type_string(&e->message, token->type);
    e->message.data[e->message.count++] = ' ';
    str_insert_str(e->message, where);
    return NULL;
}*/

#endif
#ifdef KAI_USE_MEMORY_API

enum {
	KAI_MEMORY_DEBUG_OFF     = 0,
	KAI_MEMORY_DEBUG_VERBOSE = 1,
};

enum {
	KAI_MEMORY_ERROR_OUT_OF_MEMORY  = 1, //! @see kai_create_memory implementation
	KAI_MEMORY_ERROR_MEMORY_LEAK    = 2, //! @see kai_destroy_memory implementation
};	

KAI_API (Kai_Result) kai_create_memory(Kai_Allocator* out_Allocator);
KAI_API (Kai_Result) kai_destroy_memory(Kai_Allocator* Allocator);
KAI_API (void) kai_memory_set_debug(Kai_Allocator* Allocator, Kai_u32 Debug_Level);
KAI_API (Kai_u32) kai_memory_usage(Kai_Allocator* Allocator);

#endif
#ifdef KAI_USE_DEBUG_API

enum {
	KAI_DEBUG_COLOR_PRIMARY,
	KAI_DEBUG_COLOR_SECONDARY,
	KAI_DEBUG_COLOR_IMPORTANT,
	KAI_DEBUG_COLOR_IMPORTANT_2,
	KAI_DEBUG_COLOR_DECORATION,

	KAI_DEBUG_COLOR_COUNT,
};
typedef Kai_u32 Kai_Debug_Color;

typedef void Kai_P_Write_String   (Kai_ptr User, Kai_str String);
typedef void Kai_P_Write_C_String (Kai_ptr User, char const* C_String);
typedef void Kai_P_Write_Char     (Kai_ptr User, Kai_u8 Char);
typedef void Kai_P_Set_Color      (Kai_ptr User, Kai_Debug_Color Color);

typedef struct {
	Kai_P_Write_String*   write_string;
	Kai_P_Write_C_String* write_c_string;
	Kai_P_Write_Char*     write_char;
	Kai_P_Set_Color*      set_color;
	Kai_ptr               user;
} Kai_Debug_String_Writer;

KAI_API (Kai_Debug_String_Writer*) kai_debug_stdout_writer(void);
KAI_API (void) kai_debug_open_file_writer(Kai_Debug_String_Writer* out_Writer, const char* Path);
KAI_API (void) kai_debug_close_file_writer(Kai_Debug_String_Writer* Writer);
KAI_API (void) kai_debug_write_syntax_tree(Kai_Debug_String_Writer* Writer, Kai_Syntax_Tree* Tree);
KAI_API (void) kai_debug_write_error(Kai_Debug_String_Writer* Writer, Kai_Error* Error);
KAI_API (void) kai_debug_write_type(Kai_Debug_String_Writer* Writer, Kai_Type Type);
KAI_API (void) kai_debug_write_expression(Kai_Debug_String_Writer* Writer, Kai_Expr Expr);

#endif

#if defined(KAI__COMPILER_CLANG) || defined(KAI__COMPILER_GCC)
#	pragma GCC diagnostic pop
#elif defined(KAI__COMPILER_MSVC)
#	pragma warning(pop)
#endif

#ifdef __cplusplus
}
#endif

#ifdef KAI_USE_CPP_API
#include <string>
#include <fstream>

namespace Kai {
    enum Result {
        Success = KAI_SUCCESS,
    };

    struct String : public Kai_str {
        String(const char* s): Kai_str(kai_str_from_c(s)) {}
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
