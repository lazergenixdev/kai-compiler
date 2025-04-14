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
        KAI__SECTION_CORE_API_STRUCTS
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
	Kai_u16   in_count;
	Kai_u16   out_count;
	Kai_Type* input_output; // list of parameters types, then return types
} Kai_Type_Info_Procedure;

//! TODO: implement
typedef struct {
	Kai_u8   type;
	Kai_ptr  members;
} Kai_Type_Info_Struct;

#endif
#ifndef KAI__SECTION_CORE_API_STRUCTS

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

enum {
	KAI_SUCCESS = 0,

	KAI_ERROR_SYNTAX,
	KAI_ERROR_SEMANTIC,
	KAI_ERROR_TYPE_CAST,
	KAI_ERROR_TYPE_CHECK,
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
	struct Kai_Error* next;
} Kai_Error;

#endif
#ifndef KAI__SECTION_INTERNAL_STRUCTS

typedef struct Kai__Arena_Bucket {
    struct Kai__Arena_Bucket* prev;
} Kai__Arena_Bucket;

typedef struct {
    Kai__Arena_Bucket*   current_bucket;
    Kai_u32              current_allocated;
    Kai_u32              bucket_size;
    Kai_Allocator        allocator;
} Kai__Dynamic_Arena_Allocator;

//! TODO: small array optimization (store first element in `elements` pointer)
#define KAI__ARRAY(T) \
struct {              \
	Kai_u32 count;    \
	Kai_u32 capacity; \
	T*      elements; \
}

#define KAI__HASH_TABLE_OCCUPIED_BIT 0x8000000000000000

#define KAI__HASH_TABLE_SLOT(T) \
struct {                        \
    Kai_u64 hash;               \
    Kai_str key;                \
    T value;                    \
}

#define KAI__HASH_TABLE(T) \
struct {                   \
	Kai_u32 count;         \
	Kai_u32 capacity;      \
	struct {               \
        Kai_u64 hash;      \
    	Kai_str key;       \
    	T value;           \
	}* elements;           \
}

#endif
#ifndef KAI__SECTION_SYNTAX_TREE

typedef struct Kai_Expr_Base* Kai_Expr; // Expression Nodes
typedef struct Kai_Expr_Base* Kai_Stmt; // Statement Nodes

// Used to describe number literals
// Value = ( Whole + Frac / (10 ^ Frac_Denom) ) * 10 ^ Exp
typedef struct {
	Kai_u64 Whole_Part;
	Kai_u64 Frac_Part;
	Kai_s32 Exp_Part;
	Kai_u16 Frac_Denom;
} Kai_Number_Info;

typedef enum {
	KAI_EXPR_IDENTIFIER     = 0,
	KAI_EXPR_STRING         = 1,
	KAI_EXPR_NUMBER         = 2,
	KAI_EXPR_BINARY         = 3,
	KAI_EXPR_UNARY          = 4,
	KAI_EXPR_PROCEDURE_TYPE = 5,
	KAI_EXPR_PROCEDURE_CALL = 6,
	KAI_EXPR_PROCEDURE      = 7, // defines a procedure, e.g. "(a: int, b: int) -> int ret a + b;"
	KAI_STMT_RETURN         = 8,
	KAI_STMT_DECLARATION    = 9,
	KAI_STMT_ASSIGNMENT     = 10,
	KAI_STMT_COMPOUND       = 11,
	KAI_STMT_IF             = 12,
	KAI_STMT_FOR            = 13,
	KAI_STMT_DEFER          = 20,
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
	Kai_Number_Info info;
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
	KAI__HASH_TABLE(void*) procedure_table;
	Kai_Allocator allocator;
} Kai_Program;

#endif
#ifndef KAI__SECTION_CORE_API

KAI_API (Kai_bool)        kai_string_equals(Kai_str A, Kai_str B);
KAI_API (Kai_str)         kai_str_from_cstring(char const* String);
KAI_API (Kai_vector3_u32) kai_get_version(void);
KAI_API (Kai_str)         kai_get_version_string(void);

//! @note: Must call `kai_destroy_syntax_tree` on output syntax tree
KAI_API (Kai_Result) kai_create_syntax_tree(
	Kai_Syntax_Tree_Create_Info* Info,
	Kai_Syntax_Tree*             out_Syntax_Tree);

KAI_API (void) kai_destroy_syntax_tree(Kai_Syntax_Tree* Syntax_Tree);

//! @note: output program only non-null on success
KAI_API (Kai_Result) kai_create_program(Kai_Program_Create_Info* Info, Kai_Program* out_Program);
KAI_API (Kai_Result) kai_create_program_from_source(
	Kai_str        Source,
	Kai_Allocator* Allocator,
	Kai_Error*     out_Error,
	Kai_Program*   out_Program);

KAI_API (void)  kai_destroy_program(Kai_Program Program);
KAI_API (void*) kai_find_procedure(Kai_Program Program, char const* Name, char const* opt_Type);

#endif
#ifndef KAI__SECTION_INTERNAL_API

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

#define kai__hash kai__hash_djb2

#define kai__hash_table_create(Table) \
	kai__create_hash_table_stride(Table, sizeof((Table)->elements[0]), allocator)

#define kai__hash_table_destroy(Table) \
	kai__destroy_hash_table_stride(Table, sizeof((Table)->elements[0]), allocator)

#define kai__hash_table_find(Table, Key) \
	kai__hash_table_find_stride(&(Table), sizeof(Table.elements[0]), Key)

#define kai__hash_table_get(Table, Key, out_Value) \
    kai__hash_table_get_stride(&(Table), sizeof(Table.elements[0]), Key, out_Value, sizeof(*out_Value))

#define kai__hash_table_get_str(Table, Key, out_Value) \
    kai__hash_table_get_stride(&(Table), sizeof(Table.elements[0]), KAI_STRING(Key), out_Value, sizeof(*out_Value))

#define kai__hash_table_insert(Table, KEY, ...) \
	do { \
		Kai_u32 __index__ = kai__hash_table_insert_key_stride(&(Table), sizeof((Table).elements[0]), KEY); \
    	(Table).elements[__index__].value = (__VA_ARGS__); \
	} while (0)

static void kai__array_reserve_stride(void* Ptr_Array, Kai_u32 Size, Kai_Allocator* Ptr_Allocator, Kai_u32 Stride)
{
	KAI__ARRAY(int)* array = Ptr_Array;
	if (Size <= array->capacity) return;
	array->elements = Ptr_Allocator->heap_allocate(
		Ptr_Allocator->user,
		array->elements,
		Stride * Size,
		Stride * array->capacity
	);
	array->capacity = Size;
}

static void kai__array_grow_stride(void* Ptr_Array, Kai_u32 Min_Size, Kai_Allocator* Ptr_Allocator, Kai_u32 Stride)
{
	KAI__ARRAY(int)* array = Ptr_Array;
	if (Min_Size <= array->capacity) return;
	Kai_u32 new_capacity = (Min_Size * 3) / 2;
	array->elements = Ptr_Allocator->heap_allocate(
		Ptr_Allocator->user,
		array->elements,
		Stride * new_capacity,
		Stride * array->capacity
	);
	array->capacity = new_capacity;
}

static void kai__array_destroy_stride(void* Ptr_Array, Kai_Allocator* Ptr_Allocator, Kai_u32 Stride)
{
	KAI__ARRAY(int)* array = Ptr_Array;
	if (array->capacity <= 0) return;
	Ptr_Allocator->heap_allocate(
		Ptr_Allocator->user,
		array->elements,
		0,
		Stride * array->capacity
	);
	array->elements = 0;
	array->capacity = 0;
	array->count = 0;
}

// http://www.cse.yorku.ca/~oz/hash.html
// " this algorithm (k=33) was first reported by dan bernstein many years ago in comp.lang.c.
// " another version of this algorithm (now favored by bernstein) uses xor:
// " hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
// " (why it works better than many other constants, prime or not) has never been adequately explained. 
static Kai_u64 kai__hash_djb2(Kai_str In)
{
    Kai_u64 hash = 5381;
    int c;
    for (Kai_u32 i = 0; i < In.count; ++i)
        hash = ((hash << 5) + hash) + In.data[i]; /* hash * 33 + c */
    return hash;
}

static void* kai__create_hash_table_stride(void* Table, Kai_u32 Stride, Kai_Allocator* Ptr_Allocator)
{
    KAI__HASH_TABLE(int)* base = Table;
    base->count = 0;
	base->elements = Ptr_Allocator->heap_allocate(
		Ptr_Allocator->user,
		base->elements,
		Stride * 100,
		Stride * base->capacity
	);
    base->capacity = 100;
    return 0;
}

static void* kai__destroy_hash_table_stride(void* Table, Kai_u32 Stride, Kai_Allocator* Ptr_Allocator)
{
    KAI__HASH_TABLE(int)* base = Table;
	Ptr_Allocator->heap_allocate(
		Ptr_Allocator->user,
		base->elements,
		0,
		Stride * base->capacity
	);
    return 0;
}

static Kai_u32 kai__hash_table_insert_key_stride(void* raw_table, Kai_u64 Stride, Kai_str key)
{
    KAI__HASH_TABLE(int)* table = raw_table;
    Kai_u64 hash = KAI__HASH_TABLE_OCCUPIED_BIT | kai__hash(key);
    Kai_u64 start_index = hash % table->capacity;
    for (Kai_u64 i = start_index;; i = (i + 1) % table->capacity) {
        KAI__HASH_TABLE_SLOT(int)* element_header = (void*)((Kai_u8*)table->elements + Stride * i);

        //printf("hash[%i] -> %016llx\n", (int)i, element_header->hash);

        if ((element_header->hash >> 63) == 0) {
            element_header->hash = hash;
            element_header->key = key;
			table->count += 1;
            return i;
        }
    }
}

static void* kai__hash_table_find_stride(void* raw_table, Kai_u64 Stride, Kai_str key)
{
    KAI__HASH_TABLE(int)* table = raw_table;
    Kai_u64 hash = KAI__HASH_TABLE_OCCUPIED_BIT | kai__hash(key);
    Kai_u64 start_index = hash % table->capacity;
    for (Kai_u64 i = start_index;; i = (i + 1) % table->capacity) {
        KAI__HASH_TABLE_SLOT(char)* element_header = (void*)((Kai_u8*)table->elements + Stride * i);

        //printf("hash[%i] -> %016llx\n", (int)i, element_header->hash);

        if ((element_header->hash >> 63) == 0) {
            return 0;
        }
        if (element_header->hash == hash) {
            if (kai_string_equals(element_header->key, key)) {
                return &element_header->value;
            }
        }
    }
}

static Kai_bool kai__hash_table_get_stride(void* raw_table, Kai_u64 Stride, Kai_str key, void* out_Value, Kai_u32 value_size)
{
    void* ptr = kai__hash_table_find_stride(raw_table, Stride, key);
    if (ptr) {
		char* dst = out_Value;
		char* src = ptr;
		for (Kai_u32 i = 0; i < value_size; ++i)
			dst[i] = src[i];
	}
    return ptr != 0;
}

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
