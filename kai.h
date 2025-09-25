/**
 *  < Kai >---< Scripting Language >
 *
 * @author:       lazergenixdev@gmail.com
 * @development:  https://github.com/lazergenixdev/kai-compiler
 * @license:      GNU GPLv3 (see end of file)
 */
#ifndef KAI__H
#define KAI__H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> // --> uint32, uint64, ...
#include <stddef.h> // --> NULL
#ifndef KAI_DONT_USE_WRITER_API
#include <stdio.h>
#include <locale.h>
#endif
#ifndef KAI_DONT_USE_MEMORY_API
#include <stdlib.h>
#endif

#define KAI_BUILD_DATE 20250925022612 // YMD HMS (UTC)
#define KAI_VERSION_MAJOR 0
#define KAI_VERSION_MINOR 1
#define KAI_VERSION_PATCH 0
#define KAI_VERSION_STRING "0.1.0 alpha"

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

#if defined(__WASM__)
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

#if defined(__WASM__)
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
    (printf("[\x1b[92mkai.h:%i\x1b[0m] \x1b[91m%s\x1b[0m: %s\n", __LINE__, DESC, MESSAGE), exit(1))
#endif

#ifndef kai_unreachable
#define kai_unreachable() kai_fatal_error("Assertion Failed", "Unreachable was reached! D:")
#endif

#ifndef kai__todo
#define kai__todo(...)                                       \
do { char __message__[1024] = {0};                           \
    int __length__ = snprintf(__message__, sizeof(__message__), __VA_ARGS__); \
    snprintf(__message__ + __length__, sizeof(__message__) - __length__, " (%s)", __FUNCTION__); \
    kai_fatal_error("TODO", __message__);                    \
} while (0)
#endif

#define KAI_BOOL(EXPR) ((Kai_bool)((EXPR) ? KAI_TRUE : KAI_FALSE))
#define KAI_STRING(LITERAL) KAI_STRUCT(Kai_string){.count = (Kai_u32)(sizeof(LITERAL)-1), .data = (Kai_u8*)(LITERAL)}
#define KAI_CONST_STRING(LITERAL) {.count = (Kai_u32)(sizeof(LITERAL)-1), .data = (Kai_u8*)(LITERAL)}
#define KAI_SLICE(TYPE) struct { Kai_u32 count; TYPE* data; }
#define KAI_DYNAMIC_ARRAY(T) struct { Kai_u32 count; Kai_u32 capacity; T* data; }
#define KAI_HASH_TABLE(T) struct { Kai_u32 count; Kai_u32 capacity; Kai_u64* occupied; Kai_u64* hashes; Kai_string* keys; T* values; }
#define KAI_LINKED_LIST(T) struct { T *head, *last; }
#define KAI_FILL 0x800

#define kai_number_is_integer(N) ((N).n == 0 || ((N).d == 1 && (N).e >= 0))
#define kai__write_u32(Value) writer->write_value(writer->user, KAI_U32, (Kai_Value){.u32 = Value}, (Kai_Write_Format){0})
#define kai__write_s32(Value) writer->write_value(writer->user, KAI_S32, (Kai_Value){.s32 = Value}, (Kai_Write_Format){0})
#define kai__write_u64(Value) writer->write_value(writer->user, KAI_U64, (Kai_Value){.u64 = Value}, (Kai_Write_Format){0})
#define kai__write_s64(Value) writer->write_value(writer->user, KAI_S64, (Kai_Value){.s64 = Value}, (Kai_Write_Format){0})
#define kai__write_f64(Value) writer->write_value(writer->user, KAI_F64, (Kai_Value){.f64 = Value}, (Kai_Write_Format){0})
#define kai__write(C_String) writer->write_string(writer->user, KAI_STRING(C_String))
#define kai__write_string(...) writer->write_string(writer->user, __VA_ARGS__)
#define kai__set_color(Color) if (writer->set_color != NULL) writer->set_color(writer->user, Color)
#define kai__write_fill(Char,Count) writer->write_value(writer->user, KAI_FILL, (Kai_Value){0}, (Kai_Write_Format){.min_count = Count, .fill_character = (Kai_u8)Char})
#define kai__next_character_equals(C) ( (context->cursor+1) < context->source.count && C == context->source.data[context->cursor+1] )
#define kai__allocate(Old,New_Size,Old_Size) allocator->heap_allocate(allocator->user, Old, New_Size, Old_Size)
#define kai__free(Ptr,Size) allocator->heap_allocate(allocator->user, Ptr, 0, Size)
#define kai_array_destroy(ARRAY) (allocator->heap_allocate(allocator->user, (ARRAY)->data, 0, (ARRAY)->capacity * sizeof((ARRAY)->data[0])), (ARRAY)->count = 0, (ARRAY)->capacity = 0, (ARRAY)->data = NULL)
#define kai_array_reserve(ARRAY,NEW_CAPACITY) kai_raw_array_reserve((Kai_Raw_Dynamic_Array*)(ARRAY), NEW_CAPACITY, allocator, sizeof((ARRAY)->data[0]))
#define kai_array_resize(ARRAY,NEW_SIZE) kai_raw_array_resize((Kai_Raw_Dynamic_Array*)(ARRAY), NEW_SIZE, allocator, sizeof((ARRAY)->data[0]))
#define kai_array_grow(ARRAY,COUNT) kai_raw_array_grow((Kai_Raw_Dynamic_Array*)(ARRAY), COUNT, allocator, sizeof((ARRAY)->data[0]))
#define kai_array_push(ARRAY,...) (kai_raw_array_grow((Kai_Raw_Dynamic_Array*)(ARRAY), 1, allocator, sizeof((ARRAY)->data[0])), (ARRAY)->data[(ARRAY)->count++] = (__VA_ARGS__))
#define kai_array_pop(ARRAY) (ARRAY)->data[--(ARRAY)->count]
#define kai_array_last(ARRAY) (ARRAY)->data[(ARRAY)->count - 1]
#define kai_array_insert(ARRAY) TODO
#define kai_array_insert_n(ARRAY) TODO
#define kai_array_remove(ARRAY) TODO
#define kai_array_remove_n(ARRAY) TODO
#define kai_array_remove_swap(ARRAY) TODO
#define kai_table_set(T,KEY,...) do{ Kai_u32 _; kai_raw_hash_table_emplace_key((Kai_Raw_Hash_Table*)(T), KEY, &_, allocator, sizeof (T)->values[0]); (T)->values[_] = (__VA_ARGS__); }while(0)
#define kai_table_find(T,KEY) kai_raw_hash_table_find((Kai_Raw_Hash_Table*)(T), KEY)

#ifndef KAI_API
#define KAI_API(RETURN) extern RETURN
#endif

#define KAI_INTERNAL inline static

typedef  uint8_t Kai_u8;
typedef   int8_t Kai_s8;
typedef uint16_t Kai_u16;
typedef  int16_t Kai_s16;
typedef uint32_t Kai_u32;
typedef  int32_t Kai_s32;
typedef uint64_t Kai_u64;
typedef  int64_t Kai_s64;
typedef    float Kai_f32;
typedef   double Kai_f64;
typedef struct { Kai_u8  x, y; } Kai_vector2_u8;
typedef struct { Kai_s8  x, y; } Kai_vector2_s8;
typedef struct { Kai_u16 x, y; } Kai_vector2_u16;
typedef struct { Kai_s16 x, y; } Kai_vector2_s16;
typedef struct { Kai_u32 x, y; } Kai_vector2_u32;
typedef struct { Kai_s32 x, y; } Kai_vector2_s32;
typedef struct { Kai_u64 x, y; } Kai_vector2_u64;
typedef struct { Kai_s64 x, y; } Kai_vector2_s64;
typedef struct { Kai_f32 x, y; } Kai_vector2_f32;
typedef struct { Kai_f64 x, y; } Kai_vector2_f64;
typedef struct { Kai_u8  x, y, z; } Kai_vector3_u8;
typedef struct { Kai_s8  x, y, z; } Kai_vector3_s8;
typedef struct { Kai_u16 x, y, z; } Kai_vector3_u16;
typedef struct { Kai_s16 x, y, z; } Kai_vector3_s16;
typedef struct { Kai_u32 x, y, z; } Kai_vector3_u32;
typedef struct { Kai_s32 x, y, z; } Kai_vector3_s32;
typedef struct { Kai_u64 x, y, z; } Kai_vector3_u64;
typedef struct { Kai_s64 x, y, z; } Kai_vector3_s64;
typedef struct { Kai_f32 x, y, z; } Kai_vector3_f32;
typedef struct { Kai_f64 x, y, z; } Kai_vector3_f64;
typedef struct { Kai_u8  x, y, z, w; } Kai_vector4_u8;
typedef struct { Kai_s8  x, y, z, w; } Kai_vector4_s8;
typedef struct { Kai_u16 x, y, z, w; } Kai_vector4_u16;
typedef struct { Kai_s16 x, y, z, w; } Kai_vector4_s16;
typedef struct { Kai_u32 x, y, z, w; } Kai_vector4_u32;
typedef struct { Kai_s32 x, y, z, w; } Kai_vector4_s32;
typedef struct { Kai_u64 x, y, z, w; } Kai_vector4_u64;
typedef struct { Kai_s64 x, y, z, w; } Kai_vector4_s64;
typedef struct { Kai_f32 x, y, z, w; } Kai_vector4_f32;
typedef struct { Kai_f64 x, y, z, w; } Kai_vector4_f64;

typedef Kai_u8 Kai_bool;
enum {
    KAI_FALSE = 0,
    KAI_TRUE = 1,
};
typedef  intptr_t Kai_int;
typedef uintptr_t Kai_uint;
typedef struct { Kai_uint count; Kai_u8* data; } Kai_string;
typedef const char* Kai_cstring;

typedef Kai_u32 Kai_Primitive_Type;
typedef struct Kai_Range Kai_Range;
typedef struct Kai_Memory Kai_Memory;
typedef Kai_u32 Kai_Result;
typedef struct Kai_Source Kai_Source;
typedef struct Kai_Location Kai_Location;
typedef struct Kai_Error Kai_Error;
typedef Kai_u32 Kai_Memory_Command;
typedef struct Kai_Allocator Kai_Allocator;
typedef struct Kai_Raw_Dynamic_Array Kai_Raw_Dynamic_Array;
typedef struct Kai_Raw_Hash_Table Kai_Raw_Hash_Table;
typedef struct Kai_Hash_Table_Size Kai_Hash_Table_Size;
typedef struct Kai_Context Kai_Context;
typedef Kai_u8 Kai_Type_Id;
typedef struct Kai_Type_Info Kai_Type_Info;
typedef struct Kai_Type_Info_Integer Kai_Type_Info_Integer;
typedef struct Kai_Type_Info_Float Kai_Type_Info_Float;
typedef struct Kai_Type_Info_Pointer Kai_Type_Info_Pointer;
typedef struct Kai_Type_Info_Procedure Kai_Type_Info_Procedure;
typedef struct Kai_Type_Info_Array Kai_Type_Info_Array;
typedef struct Kai_Struct_Field Kai_Struct_Field;
typedef struct Kai_Type_Info_Struct Kai_Type_Info_Struct;
typedef struct Kai_Number Kai_Number;
typedef union Kai_Value Kai_Value;
typedef Kai_u32 Kai_Write_Color;
typedef Kai_u32 Kai_Write_Flags;
typedef struct Kai_Write_Format Kai_Write_Format;
typedef struct Kai_Writer Kai_Writer;
typedef struct Kai_Fixed_Allocator Kai_Fixed_Allocator;
typedef struct Kai_Arena_Bucket Kai_Arena_Bucket;
typedef struct Kai_Arena_Allocator Kai_Arena_Allocator;
typedef struct Kai_Buffer Kai_Buffer;
typedef struct Kai__Tree_Traversal_Context Kai__Tree_Traversal_Context;

typedef Kai_u8 Kai_Expr_Id;
typedef Kai_u8 Kai_Special_Kind;
typedef Kai_u8 Kai_Control_Kind;
typedef Kai_u8 Kai_Expr_Flags;
typedef struct Kai_Tag Kai_Tag;
typedef struct Kai_Expr Kai_Expr;
typedef struct Kai_Expr_String Kai_Expr_String;
typedef struct Kai_Expr_Number Kai_Expr_Number;
typedef struct Kai_Expr_Literal Kai_Expr_Literal;
typedef struct Kai_Expr_Unary Kai_Expr_Unary;
typedef struct Kai_Expr_Binary Kai_Expr_Binary;
typedef struct Kai_Expr_Procedure_Call Kai_Expr_Procedure_Call;
typedef struct Kai_Expr_Procedure_Type Kai_Expr_Procedure_Type;
typedef struct Kai_Expr_Procedure Kai_Expr_Procedure;
typedef struct Kai_Expr_Struct Kai_Expr_Struct;
typedef struct Kai_Expr_Enum Kai_Expr_Enum;
typedef struct Kai_Expr_Array Kai_Expr_Array;
typedef struct Kai_Expr_Special Kai_Expr_Special;
typedef struct Kai_Stmt_Return Kai_Stmt_Return;
typedef struct Kai_Stmt_Declaration Kai_Stmt_Declaration;
typedef struct Kai_Stmt_Assignment Kai_Stmt_Assignment;
typedef struct Kai_Stmt_Compound Kai_Stmt_Compound;
typedef struct Kai_Stmt_If Kai_Stmt_If;
typedef struct Kai_Stmt_While Kai_Stmt_While;
typedef struct Kai_Stmt_For Kai_Stmt_For;
typedef struct Kai_Stmt_Control Kai_Stmt_Control;
typedef struct Kai_Syntax_Tree Kai_Syntax_Tree;
typedef struct Kai_Syntax_Tree_Create_Info Kai_Syntax_Tree_Create_Info;
typedef Kai_u32 Kai_Token_Id;
typedef struct Kai_Token Kai_Token;
typedef struct Kai_Tokenizer Kai_Tokenizer;
typedef struct Kai_Parser Kai_Parser;
typedef struct Kai__Operator Kai__Operator;
typedef Kai_u32 Kai__Operator_Type;

typedef Kai_u32 Kai_Compile_Flags;
typedef struct Kai_Compile_Options Kai_Compile_Options;
typedef struct Kai_Native_Procedure Kai_Native_Procedure;
typedef struct Kai_Import Kai_Import;
typedef struct Kai_Program_Create_Info Kai_Program_Create_Info;
typedef struct Kai_Variable Kai_Variable;
typedef struct Kai_Program Kai_Program;
typedef Kai_u32 Kai_Node_Flags;
typedef struct Kai_Node_Reference Kai_Node_Reference;
typedef struct Kai_Node Kai_Node;
typedef struct Kai_Local_Node Kai_Local_Node;
typedef struct Kai_Scope Kai_Scope;
typedef struct Kai_Compiler_Context Kai_Compiler_Context;
typedef struct Kai__DFS_Context Kai__DFS_Context;

typedef void* Kai_P_Memory_Heap_Allocate(void* user, void* ptr, Kai_u32 new_size, Kai_u32 old_size);
typedef void* Kai_P_Memory_Platform_Allocate(void* user, void* ptr, Kai_u32 size, Kai_Memory_Command op);
typedef Kai_Type_Info* Kai_Type;
typedef void Kai_P_Write_String(void* user, Kai_string str);
typedef void Kai_P_Write_Value(void* user, Kai_u32 type, Kai_Value value, Kai_Write_Format format);
typedef void Kai_P_Set_Color(void* user, Kai_Write_Color color);

typedef Kai_Expr Kai_Stmt;
typedef KAI_LINKED_LIST(Kai_Expr) Kai_Expr_List;
typedef KAI_LINKED_LIST(Kai_Stmt) Kai_Stmt_List;
#define KAI_TOP_PRECEDENCE 1
#define KAI_PRECEDENCE_MASK 65535

typedef KAI_SLICE(Kai_Type) Kai_Type_Slice;
typedef KAI_SLICE(Kai_Struct_Field) Kai_Struct_Field_Slice;
typedef KAI_DYNAMIC_ARRAY(Kai_u8) Kai_u8_DynArray;
typedef KAI_SLICE(Kai_Source) Kai_Source_Slice;
typedef KAI_SLICE(Kai_Import) Kai_Import_Slice;
typedef KAI_SLICE(Kai_u8) Kai_u8_Slice;
typedef KAI_SLICE(Kai_Syntax_Tree) Kai_Syntax_Tree_Slice;
typedef KAI_HASH_TABLE(Kai_u32) Kai_u32_HashTable;
typedef KAI_HASH_TABLE(Kai_Variable) Kai_Variable_HashTable;
typedef KAI_HASH_TABLE(Kai_Type) Kai_Type_HashTable;
typedef KAI_DYNAMIC_ARRAY(Kai_Node_Reference) Kai_Node_Reference_DynArray;
typedef KAI_HASH_TABLE(Kai_Node_Reference) Kai_Node_Reference_HashTable;
typedef KAI_DYNAMIC_ARRAY(Kai_Scope) Kai_Scope_DynArray;
typedef KAI_DYNAMIC_ARRAY(Kai_Node) Kai_Node_DynArray;
typedef KAI_DYNAMIC_ARRAY(Kai_Local_Node) Kai_Local_Node_DynArray;
typedef KAI_SLICE(Kai_u32) Kai_u32_Slice;

// Type: Kai_Primitive_Type
enum {
    KAI_U8 = 0|1<<4,
    KAI_U16 = 1|2<<4,
    KAI_U32 = 2|4<<4,
    KAI_U64 = 3|8<<4,
    KAI_S8 = 4|1<<4,
    KAI_S16 = 5|2<<4,
    KAI_S32 = 6|4<<4,
    KAI_S64 = 7|8<<4,
    KAI_F32 = 8|4<<4,
    KAI_F64 = 9|8<<4,
    KAI_TYPE = 10|sizeof(void*)<<4,
};

struct Kai_Range {
    Kai_u32 start;
    Kai_u32 count;
};

struct Kai_Memory {
    Kai_u32 size;
    void* data;
};

// Type: Kai_Result
enum {
    KAI_SUCCESS = 0,
    KAI_ERROR_MEMORY = 1,
    KAI_ERROR_SYNTAX = 2,
    KAI_ERROR_SEMANTIC = 3,
    KAI_ERROR_INFO = 4,
    KAI_ERROR_FATAL = 5,
    KAI_ERROR_INTERNAL = 6,
    KAI_RESULT_COUNT = 7,
};

struct Kai_Source {
    Kai_string name;
    Kai_string contents;
};

struct Kai_Location {
    Kai_Source source;
    Kai_string string;
    Kai_u32 line;
};

struct Kai_Error {
    Kai_Result result;
    Kai_Location location;
    Kai_string message;
    Kai_string context;
    Kai_Memory memory;
    Kai_Error* next;
};

// Type: Kai_Memory_Command
enum {
    KAI_MEMORY_COMMAND_ALLOCATE_WRITE_ONLY = 0,
    KAI_MEMORY_COMMAND_SET_EXECUTABLE = 1,
    KAI_MEMORY_COMMAND_FREE = 2,
};

struct Kai_Allocator {
    Kai_P_Memory_Heap_Allocate* heap_allocate;
    Kai_P_Memory_Platform_Allocate* platform_allocate;
    void* user;
    Kai_u32 page_size;
};

struct Kai_Raw_Dynamic_Array {
    Kai_u32 count;
    Kai_u32 capacity;
    void* data;
};

struct Kai_Raw_Hash_Table {
    Kai_u32 count;
    Kai_u32 capacity;
    Kai_u64* occupied;
    Kai_u64* hashes;
    Kai_string* keys;
    void* values;
};

struct Kai_Hash_Table_Size {
    Kai_u32 occupied;
    Kai_u32 hashes;
    Kai_u32 keys;
    Kai_u32 values;
    Kai_u32 total;
};

struct Kai_Context {
    Kai_Allocator allocator;
    void* user;
};

// Type: Kai_Type_Id
enum {
    KAI_TYPE_ID_TYPE = 0,
    KAI_TYPE_ID_VOID = 1,
    KAI_TYPE_ID_BOOLEAN = 2,
    KAI_TYPE_ID_INTEGER = 3,
    KAI_TYPE_ID_FLOAT = 4,
    KAI_TYPE_ID_POINTER = 5,
    KAI_TYPE_ID_PROCEDURE = 6,
    KAI_TYPE_ID_ARRAY = 7,
    KAI_TYPE_ID_STRUCT = 8,
    KAI_TYPE_ID_STRING = 9,
    KAI_TYPE_ID_NUMBER = 10,
};

struct Kai_Type_Info {
    Kai_u8 id;
};

struct Kai_Type_Info_Integer {
    Kai_u8 id;
    Kai_bool is_signed;
    Kai_u8 bits;
};

struct Kai_Type_Info_Float {
    Kai_u8 id;
    Kai_u8 bits;
};

struct Kai_Type_Info_Pointer {
    Kai_u8 id;
    Kai_Type sub_type;
};

struct Kai_Type_Info_Procedure {
    Kai_u8 id;
    Kai_Type_Slice inputs;
    Kai_Type_Slice outputs;
};

struct Kai_Type_Info_Array {
    Kai_u8 id;
    Kai_u32 rows;
    Kai_u32 cols;
    Kai_Type sub_type;
};

struct Kai_Struct_Field {
    Kai_string name;
    Kai_u32 offset;
    Kai_Type type;
};

struct Kai_Type_Info_Struct {
    Kai_u8 id;
    Kai_u32 size;
    Kai_Struct_Field_Slice fields;
};

struct Kai_Number {
    Kai_u64 n;
    Kai_u64 d;
    Kai_s32 e;
    Kai_u32 is_neg;
};

union Kai_Value {
    Kai_u8 u8;
    Kai_u16 u16;
    Kai_u32 u32;
    Kai_u64 u64;
    Kai_s8 s8;
    Kai_s16 s16;
    Kai_s32 s32;
    Kai_s64 s64;
    Kai_f32 f32;
    Kai_f64 f64;
    void* ptr;
    Kai_Type_Info* type;
    void* procedure;
    Kai_Number number;
    Kai_string string;
    Kai_u8 inline_struct[24];
};

// Type: Kai_Write_Color
enum {
    KAI_WRITE_COLOR_PRIMARY = 0,
    KAI_WRITE_COLOR_SECONDARY = 1,
    KAI_WRITE_COLOR_IMPORTANT = 2,
    KAI_WRITE_COLOR_IMPORTANT_2 = 3,
    KAI_WRITE_COLOR_DECORATION = 4,
    KAI_WRITE_COLOR_TYPE = 5,
    KAI_WRITE_COLOR_COUNT = 6,
};

// Type: Kai_Write_Flags
enum {
    KAI_WRITE_FLAGS_NONE = 0,
    KAI_WRITE_FLAGS_HEXIDECIMAL = 1<<0,
};

struct Kai_Write_Format {
    Kai_u32 flags;
    Kai_u32 min_count;
    Kai_u32 max_count;
    Kai_u8 fill_character;
};

struct Kai_Writer {
    Kai_P_Write_String* write_string;
    Kai_P_Write_Value* write_value;
    Kai_P_Set_Color* set_color;
    void* user;
};

struct Kai_Fixed_Allocator {
    void* data;
    Kai_u32 offset;
    Kai_u32 size;
};

struct Kai_Arena_Bucket {
    Kai_Arena_Bucket* prev;
};

struct Kai_Arena_Allocator {
    Kai_Arena_Bucket* current_bucket;
    Kai_u32 current_allocated;
    Kai_u32 bucket_size;
    Kai_Allocator base;
};

struct Kai_Buffer {
    Kai_u8_DynArray array;
    Kai_u32 offset;
    Kai_Allocator allocator;
};

struct Kai__Tree_Traversal_Context {
    Kai_Writer* writer;
    Kai_u64 stack[256];
    Kai_u32 stack_count;
    Kai_string prefix;
};

// Type: Kai_Expr_Id
enum {
    KAI_EXPR_IDENTIFIER = 0,
    KAI_EXPR_STRING = 1,
    KAI_EXPR_NUMBER = 2,
    KAI_EXPR_LITERAL = 3,
    KAI_EXPR_UNARY = 4,
    KAI_EXPR_BINARY = 5,
    KAI_EXPR_PROCEDURE_TYPE = 6,
    KAI_EXPR_PROCEDURE_CALL = 7,
    KAI_EXPR_PROCEDURE = 8,
    KAI_EXPR_CODE = 9,
    KAI_EXPR_IMPORT = 10,
    KAI_EXPR_STRUCT = 11,
    KAI_EXPR_ENUM = 12,
    KAI_EXPR_ARRAY = 13,
    KAI_EXPR_SPECIAL = 14,
    KAI_STMT_RETURN = 15,
    KAI_STMT_DECLARATION = 16,
    KAI_STMT_ASSIGNMENT = 17,
    KAI_STMT_IF = 18,
    KAI_STMT_WHILE = 19,
    KAI_STMT_FOR = 20,
    KAI_STMT_CONTROL = 21,
    KAI_STMT_COMPOUND = 22,
};

// Type: Kai_Special_Kind
enum {
    KAI_SPECIAL_EVAL_TYPE = 0,
    KAI_SPECIAL_EVAL_SIZE = 1,
    KAI_SPECIAL_TYPE = 2,
    KAI_SPECIAL_NUMBER = 3,
    KAI_SPECIAL_CODE = 4,
};

// Type: Kai_Control_Kind
enum {
    KAI_CONTROL_CASE = 0,
    KAI_CONTROL_BREAK = 1,
    KAI_CONTROL_CONTINUE = 2,
    KAI_CONTROL_THROUGH = 3,
    KAI_CONTROL_DEFER = 4,
};

// Type: Kai_Expr_Flags
enum {
    KAI_FLAG_DECL_CONST = 1<<0,
    KAI_FLAG_DECL_USING = 1<<1,
    KAI_FLAG_EXPR_USING = 1<<1,
    KAI_FLAG_PROC_USING = 1<<1,
    KAI_FLAG_DECL_EXPORT = 1<<2,
    KAI_FLAG_STRUCT_UNION = 1<<2,
    KAI_FLAG_IF_CASE = 1<<2,
    KAI_FLAG_FOR_LESS_THAN = 1<<2,
    KAI_FLAG_ARRAY_DYNAMIC = 1<<2,
    KAI_FLAG_DECL_HOST_IMPORT = 1<<3,
};

struct Kai_Tag {
    Kai_string name;
    Kai_Expr* expr;
    Kai_Tag* next;
};

struct Kai_Expr {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
};

struct Kai_Expr_String {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_string value;
};

struct Kai_Expr_Number {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Number value;
};

struct Kai_Expr_Literal {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* head;
    Kai_u32 count;
};

struct Kai_Expr_Unary {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* expr;
    Kai_u32 op;
};

struct Kai_Expr_Binary {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* left;
    Kai_Expr* right;
    Kai_u32 op;
};

struct Kai_Expr_Procedure_Call {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* proc;
    Kai_Expr* arg_head;
    Kai_u8 arg_count;
};

struct Kai_Expr_Procedure_Type {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* in_out_expr;
    Kai_u8 in_count;
    Kai_u8 out_count;
};

struct Kai_Expr_Procedure {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* in_out_expr;
    Kai_Stmt* body;
    Kai_u8 in_count;
    Kai_u8 out_count;
};

struct Kai_Expr_Struct {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_u32 field_count;
    Kai_Stmt* head;
};

struct Kai_Expr_Enum {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* type;
    Kai_u32 field_count;
    Kai_Stmt* head;
};

struct Kai_Expr_Array {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* rows;
    Kai_Expr* cols;
    Kai_Expr* expr;
};

struct Kai_Expr_Special {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_u8 kind;
};

struct Kai_Stmt_Return {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* expr;
};

struct Kai_Stmt_Declaration {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* expr;
    Kai_Expr* type;
};

struct Kai_Stmt_Assignment {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_u32 op;
    Kai_Expr* left;
    Kai_Expr* expr;
};

struct Kai_Stmt_Compound {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Stmt* head;
};

struct Kai_Stmt_If {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* expr;
    Kai_Stmt* then_body;
    Kai_Stmt* else_body;
};

struct Kai_Stmt_While {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Expr* expr;
    Kai_Stmt* body;
};

struct Kai_Stmt_For {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_Stmt* body;
    Kai_Expr* from;
    Kai_Expr* to;
    Kai_string iterator_name;
};

struct Kai_Stmt_Control {
    Kai_Expr_Id id;
    Kai_Expr_Flags flags;
    Kai_string source_code;
    Kai_string name;
    Kai_Expr* next;
    Kai_Tag* tag;
    Kai_Type_Info* this_type;
    Kai_u32 line_number;
    Kai_u8 kind;
    Kai_Expr* expr;
};

struct Kai_Syntax_Tree {
    Kai_Stmt_Compound root;
    Kai_Source source;
    Kai_Arena_Allocator allocator;
};

struct Kai_Syntax_Tree_Create_Info {
    Kai_Source source;
    Kai_Allocator allocator;
    Kai_Error* error;
};

// Type: Kai_Token_Id
enum {
    KAI_TOKEN_END = 0,
    KAI_TOKEN_IDENTIFIER = 1,
    KAI_TOKEN_NUMBER = 2,
    KAI_TOKEN_DIRECTIVE = 3,
    KAI_TOKEN_TAG = 4,
    KAI_TOKEN_STRING = 5,
    KAI_TOKEN_break = 128,
    KAI_TOKEN_case = 129,
    KAI_TOKEN_cast = 130,
    KAI_TOKEN_continue = 131,
    KAI_TOKEN_defer = 132,
    KAI_TOKEN_else = 133,
    KAI_TOKEN_enum = 134,
    KAI_TOKEN_for = 135,
    KAI_TOKEN_if = 136,
    KAI_TOKEN_loop = 137,
    KAI_TOKEN_ret = 138,
    KAI_TOKEN_struct = 139,
    KAI_TOKEN_union = 140,
    KAI_TOKEN_using = 141,
    KAI_TOKEN_while = 142,
};

struct Kai_Token {
    Kai_Token_Id id;
    Kai_u32 line_number;
    Kai_string string;
    struct { Kai_string string; Kai_Number number; } value;
};

struct Kai_Tokenizer {
    Kai_Token current_token;
    Kai_Token peeked_token;
    Kai_string source;
    Kai_u32 cursor;
    Kai_u32 line_number;
    Kai_bool peeking;
    Kai_Fixed_Allocator string_arena;
};

struct Kai_Parser {
    Kai_Tokenizer tokenizer;
    Kai_Arena_Allocator arena;
    Kai_Error* error;
};

struct Kai__Operator {
    Kai_u32 prec;
    Kai_u32 type;
};

// Type: Kai__Operator_Type
enum {
    KAI__OPERATOR_TYPE_BINARY = 0,
    KAI__OPERATOR_TYPE_INDEX = 1,
    KAI__OPERATOR_TYPE_PROCEDURE_CALL = 2,
};

// Type: Kai_Compile_Flags
enum {
    KAI_COMPILE_NO_CODE_GEN = 1,
    KAI_COMPILE_ALLOW_UNDEFINED = 2,
    KAI_COMPILE_DEBUG = 32768,
};

struct Kai_Compile_Options {
    Kai_u32 interpreter_max_step_count;
    Kai_u32 interpreter_max_call_depth;
    Kai_Compile_Flags flags;
};

struct Kai_Native_Procedure {
    Kai_u8* name;
    void* address;
    Kai_string typestring;
};

struct Kai_Import {
    Kai_string name;
    Kai_string type;
    Kai_Value value;
};

struct Kai_Program_Create_Info {
    Kai_Source_Slice sources;
    Kai_Import_Slice imports;
    Kai_Allocator allocator;
    Kai_Error* error;
    Kai_Compile_Options options;
};

struct Kai_Variable {
    Kai_u32 location;
    Kai_Type type;
};

struct Kai_Program {
    Kai_u8_DynArray data;
    union { Kai_u8_Slice machine; Kai_Syntax_Tree_Slice trees; } code;
    Kai_u32_HashTable procedure_table;
    Kai_Variable_HashTable variable_table;
    Kai_Type_HashTable type_table;
    Kai_Allocator allocator;
};

// Type: Kai_Node_Flags
enum {
    KAI_NODE_TYPE = 1,
    KAI_NODE_TYPE_EVALUATED = 2,
    KAI_NODE_VALUE_EVALUATED = 4,
    KAI_NODE_EVALUATED = 6,
    KAI_NODE_LOCAL = 8,
    KAI_NODE_EXPORT = 16,
    KAI_NODE_IMPORT = 32,
    KAI_NODE_NOT_FOUND = 128,
};

struct Kai_Node_Reference {
    Kai_Node_Flags flags;
    Kai_u32 index;
};

struct Kai_Node {
    Kai_Type_Info* type;
    Kai_Value value;
    Kai_Location location;
    Kai_Expr* expr;
    Kai_Expr* type_expr;
    Kai_Node_Reference_DynArray value_dependencies;
    Kai_Node_Reference_DynArray type_dependencies;
    Kai_Node_Flags flags;
};

struct Kai_Local_Node {
    Kai_Type_Info* type;
    Kai_Location location;
};

struct Kai_Scope {
    Kai_Node_Reference_HashTable identifiers;
    Kai_bool is_proc_scope;
};

struct Kai_Compiler_Context {
    Kai_Error* error;
    Kai_Allocator allocator;
    Kai_Program* program;
    Kai_Compile_Options options;
    Kai_Scope_DynArray scopes;
    Kai_Node_DynArray nodes;
    Kai_Local_Node_DynArray local_nodes;
    Kai_Import_Slice imports;
    Kai_u32_Slice compilation_order;
    Kai_Arena_Allocator type_allocator;
    Kai_Arena_Allocator temp_allocator;
    Kai_Source current_source;
    Kai_Node_Reference current_node;
    Kai_Type_Info* number_type;
    Kai_Type_Info* type_type;
    Kai_Type_Info* bool_type;
    Kai_Writer debug_writer;
};

struct Kai__DFS_Context {
    Kai_Compiler_Context* context;
    Kai_u32* post;
    Kai_u32* prev;
    Kai_bool* visited;
    Kai_u32 next;
};

KAI_API(Kai_string) kai_version_string(void);
KAI_API(Kai_vector3_u32) kai_version(void);
KAI_API(Kai_bool) kai_string_equals(Kai_string left, Kai_string right);
KAI_API(Kai_string) kai_string_from_c(Kai_cstring s);
KAI_API(Kai_string) kai_string_copy_from_c(Kai_string dst, Kai_cstring src);
KAI_API(Kai_string) kai_merge_strings(Kai_string a, Kai_string b);
KAI_API(Kai_u64) kai_string_hash(Kai_string s);
KAI_API(Kai_u64) kai_string_hash_next(Kai_u64 hash, Kai_string s);
KAI_API(void) kai_raw_array_reserve(Kai_Raw_Dynamic_Array* array, Kai_u32 new_capacity, Kai_Allocator* allocator, Kai_u32 elem_size);
KAI_API(void) kai_raw_array_resize(Kai_Raw_Dynamic_Array* array, Kai_u32 new_size, Kai_Allocator* allocator, Kai_u32 elem_size);
KAI_API(void) kai_raw_array_grow(Kai_Raw_Dynamic_Array* array, Kai_u32 count, Kai_Allocator* allocator, Kai_u32 elem_size);
KAI_API(Kai_Hash_Table_Size) kai_raw_hash_table_size(Kai_u32 capacity, Kai_u32 elem_size);
KAI_API(void) kai_raw_hash_table_grow(Kai_Raw_Hash_Table* table, Kai_Allocator* allocator, Kai_u32 elem_size);
KAI_API(Kai_bool) kai_raw_hash_table_emplace_key(Kai_Raw_Hash_Table* table, Kai_string key, Kai_u32* out_index, Kai_Allocator* allocator, Kai_u32 elem_size);
KAI_API(Kai_int) kai_raw_hash_table_find(Kai_Raw_Hash_Table* table, Kai_string key);
KAI_API(Kai_u64) kai_number_to_u64(Kai_Number number);
KAI_API(Kai_f64) kai_number_to_f64(Kai_Number number);
KAI_API(Kai_Number) kai_number_normalize(Kai_Number number);
KAI_API(Kai_Number) kai_number_neg(Kai_Number a);
KAI_API(Kai_Number) kai_number_abs(Kai_Number a);
KAI_API(Kai_Number) kai_number_inv(Kai_Number a);
KAI_API(Kai_Number) kai_number_add(Kai_Number a, Kai_Number b);
KAI_API(Kai_Number) kai_number_sub(Kai_Number a, Kai_Number b);
KAI_API(Kai_Number) kai_number_mul(Kai_Number a, Kai_Number b);
KAI_API(Kai_Number) kai_number_div(Kai_Number a, Kai_Number b);
KAI_API(Kai_Number) kai_number_pow_int(Kai_Number a, Kai_u32 exp);
KAI_API(Kai_Number) kai_number_add_same_exp(Kai_Number a, Kai_Number b);
KAI_API(void) kai_number_match_exponents(Kai_Number* a, Kai_Number* b);
KAI_API(Kai_Number) kai_number_parse_whole(Kai_string source, Kai_u32* offset, Kai_u32 base);
KAI_API(Kai_Number) kai_number_parse_decimal(Kai_string source, Kai_u32* offset);
KAI_API(Kai_Number) kai_number_parse_exponent(Kai_string source, Kai_u32* offset);
KAI_API(void*) kai_fixed_allocate(Kai_Fixed_Allocator* arena, Kai_u32 size);
KAI_API(void) kai_arena_create(Kai_Arena_Allocator* arena, Kai_Allocator* base);
KAI_API(void) kai_arena_destroy(Kai_Arena_Allocator* arena);
KAI_API(void) kai_arena_free_all(Kai_Arena_Allocator* arena);
KAI_API(void*) kai_arena_allocate(Kai_Arena_Allocator* arena, Kai_u32 size);
KAI_API(void) kai_write_error(Kai_Writer* writer, Kai_Error* error);
KAI_API(void) kai_write_type(Kai_Writer* writer, Kai_Type_Info* type);
KAI_API(void) kai_write_value(Kai_Writer* writer, void* data, Kai_Type_Info* type);
KAI_API(void) kai_write_expression(Kai_Writer* writer, Kai_Expr* expr, Kai_u32 depth);
KAI_API(void) kai_write_syntax_tree(Kai_Writer* writer, Kai_Syntax_Tree* tree);
KAI_API(void) kai_write_number(Kai_Writer* writer, Kai_Number number);
KAI_API(void) kai_destroy_error(Kai_Error* error, Kai_Allocator* allocator);

KAI_API(void) kai_write_token(Kai_Writer* writer, Kai_Token token);
KAI_API(Kai_string) kai_token_string(Kai_Token_Id id, Kai_string dst);
KAI_API(Kai_Token) kai_tokenizer_generate(Kai_Tokenizer* context);
KAI_API(Kai_Token*) kai_tokenizer_next(Kai_Tokenizer* context);
KAI_API(Kai_Token*) kai_tokenizer_peek(Kai_Tokenizer* context);
KAI_API(Kai_Expr*) kai_parse_procedure_call_arguments(Kai_Parser* parser, Kai_u32* arg_count);
KAI_API(Kai_Expr*) kai_parse_tag_to_expr(Kai_Parser* parser, Kai_Expr* expr);
KAI_API(Kai_Expr*) kai_parse_expression(Kai_Parser* parser, Kai_u32 flags);
KAI_API(Kai_Expr*) kai_parse_type_expression(Kai_Parser* parser);
KAI_API(Kai_Expr*) kai_parse_procedure(Kai_Parser* parser);
KAI_API(Kai_Stmt*) kai_parse_declaration(Kai_Parser* parser);
KAI_API(Kai_Stmt*) kai_parse_statement(Kai_Parser* parser);
KAI_API(Kai_Result) kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* info, Kai_Syntax_Tree* out_tree);
KAI_API(void) kai_destroy_syntax_tree(Kai_Syntax_Tree* tree);

KAI_API(void) kai_add_dependency(Kai_Compiler_Context* context, Kai_Node_Reference ref);
KAI_API(Kai_Result) kai_create_program(Kai_Program_Create_Info* info, Kai_Program* out_program);
KAI_API(void) kai_destroy_program(Kai_Program* program);
KAI_API(void*) kai_find_variable(Kai_Program* program, Kai_string name, Kai_Type* out_type);
KAI_API(void*) kai_find_procedure(Kai_Program* program, Kai_string name, Kai_string type);

#ifndef KAI_DONT_USE_WRITER_API
KAI_API(Kai_Writer) kai_writer_stdout(void);
KAI_API(Kai_Writer) kai_writer_file_open(Kai_cstring path);
KAI_API(void) kai_writer_file_close(Kai_Writer* writer);
#endif

#ifndef KAI_DONT_USE_MEMORY_API
typedef Kai_u32 Kai_Memory_Error;
typedef struct Kai_Memory_Metadata Kai_Memory_Metadata;

// Type: Kai_Memory_Error
enum {
    KAI_MEMORY_ERROR_OUT_OF_MEMORY = 1,
    KAI_MEMORY_ERROR_MEMORY_LEAK = 2,
};

struct Kai_Memory_Metadata {
    Kai_u64 total_allocated;
};

KAI_API(Kai_Result) kai_memory_create(Kai_Allocator* out_allocator);
KAI_API(Kai_Result) kai_memory_destroy(Kai_Allocator* allocator);
KAI_API(Kai_u64) kai_memory_usage(Kai_Allocator* allocator);
#endif

#ifdef KAI_IMPLEMENTATION

#define kai__explore(Expr,IS_LAST) (kai__tree_traversal_push(context, IS_LAST),kai__write_tree(context, Expr),kai__tree_traversal_pop(context))
#define kai__linked_list_append(L,P) ((L).head == NULL? ((L).head = (P)):((L).last->next = (P)), (L).last = (P))
#define kai__unexpected(Where, Context) kai__error_unexpected(parser, &parser->tokenizer.current_token, KAI_STRING(Where), KAI_STRING(Context))
#define kai__expect(EXPR, Where, Context) if (!(EXPR)) return kai__unexpected(Where, Context)
#define kai__next_token() kai_tokenizer_next(&parser->tokenizer)
#define kai__peek_token() kai_tokenizer_peek(&parser->tokenizer)
#define kai__dump_memory(ARRAY) do { for (Kai_u32 i = 0; i < (ARRAY).count * sizeof((ARRAY).data[0]); ++i) printf("%02x ", ((Kai_u8*)((ARRAY).data))[i]); printf("\n"); } while (0)
#define KAI__W ((1<<1)|1)
#define KAI__T ((2<<1)|1)
#define KAI__D ((3<<1)|1)
#define KAI__G ((4<<1)|1)
#define KAI__K ((1<<1)|0)
#define KAI__N ((2<<1)|0)
#define KAI__C ((5<<1)|1)
#define KAI__S ((7<<1)|1)
#define KAI__Z ((8<<1)|1)
#define KAI__PREC_CAST 2304
#define KAI__PREC_UNARY 4096

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
#if defined(__WASM__) && !defined(KAI_NO_INTRINSIC_128)
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

KAI_INTERNAL Kai_string kai__range_to_string(Kai_Range range, Kai_Memory memory);
KAI_INTERNAL Kai_u64 kai__ceil_div(Kai_u64 num, Kai_u64 den);
KAI_INTERNAL Kai_u64 kai__ceil_div_fast(Kai_u64 num, Kai_u32 exp);
KAI_INTERNAL Kai_u32 kai__max_u32(Kai_u32 a, Kai_u32 b);
KAI_INTERNAL Kai_u32 kai__min_u32(Kai_u32 a, Kai_u32 b);
KAI_INTERNAL Kai_u64 kai__gcd(Kai_u64 a, Kai_u64 b);
KAI_INTERNAL Kai_f64 kai__ldexp(Kai_f64 x, Kai_int n);
KAI_INTERNAL Kai_u64 kai__mul_with_shift(Kai_u64 a, Kai_u64 b, Kai_s32* exp, Kai_bool sub);
KAI_INTERNAL Kai_u64 kai__add_with_shift(Kai_u64 a, Kai_u64 b, Kai_s32* exp, Kai_bool sub);
KAI_INTERNAL void kai__memory_copy(void* dst, void* src, Kai_u32 size);
KAI_INTERNAL void kai__memory_zero(void* dst, Kai_u32 size);
KAI_INTERNAL void kai__memory_fill(void* dst, Kai_u8 byte, Kai_u32 size);
KAI_INTERNAL void kai__buffer_append_string(Kai_Buffer* buffer, Kai_string s);
KAI_INTERNAL Kai_Range kai__buffer_end(Kai_Buffer* buffer);
KAI_INTERNAL Kai_Range kai__buffer_push(Kai_Buffer* buffer, Kai_u32 size);
KAI_INTERNAL Kai_Memory kai__buffer_done(Kai_Buffer* buffer);
KAI_INTERNAL Kai_u32 kai__base10_digit_count(Kai_u32 x);
KAI_INTERNAL Kai_u8* kai__advance_to_line(Kai_u8* source, Kai_u32 line);
KAI_INTERNAL void kai__write_source_code(Kai_Writer* writer, Kai_u8* src);
KAI_INTERNAL Kai_u32 kai__utf8_decode(Kai_u8** s);
KAI_INTERNAL Kai_u32 kai__unicode_char_width(Kai_Writer* writer, Kai_u32 cp, Kai_u8 first, Kai_u8 ch);
KAI_INTERNAL void kai__write_source_code_fill(Kai_Writer* writer, Kai_u8* src, Kai_u8* end, Kai_u8 first, Kai_u8 ch);
KAI_INTERNAL void kai__tree_traversal_push(Kai__Tree_Traversal_Context* context, Kai_bool is_last);
KAI_INTERNAL void kai__tree_traversal_pop(Kai__Tree_Traversal_Context* context);
KAI_INTERNAL void kai__write_expr_id_with_name(Kai_Writer* writer, Kai_string id, Kai_Expr* expr);
KAI_INTERNAL void kai__write_unary_operator_name(Kai_Writer* writer, Kai_u32 op);
KAI_INTERNAL void kai__write_binary_operator_name(Kai_Writer* writer, Kai_u32 op);
KAI_INTERNAL void kai__write_assignment_operator_name(Kai_Writer* writer, Kai_u32 op);
KAI_INTERNAL void kai__write_tree_branches(Kai__Tree_Traversal_Context* context);
KAI_INTERNAL void kai__write_tree(Kai__Tree_Traversal_Context* context, Kai_Expr* expr);
KAI_INTERNAL Kai_u32 kai__hash_keyword(Kai_string s);
KAI_INTERNAL Kai_Number kai__parse_fractional_part(Kai_string source, Kai_u32* offset, Kai_Number start);
KAI_INTERNAL Kai_bool kai__make_multi_token(Kai_Tokenizer* context, Kai_Token* t, Kai_u8 current);
KAI_INTERNAL void kai__tokenizer_advance_to_identifier_end(Kai_Tokenizer* context);
KAI_INTERNAL Kai_Expr* kai__error_unexpected(Kai_Parser* parser, Kai_Token* token, Kai_string where, Kai_string wanted);
KAI_INTERNAL Kai__Operator kai__operator_info(Kai_u32 op);
KAI_INTERNAL Kai_Expr* kai__parser_create_identifier(Kai_Parser* parser, Kai_Token token);
KAI_INTERNAL Kai_Expr* kai__parser_create_string(Kai_Parser* parser, Kai_Token token);
KAI_INTERNAL Kai_Expr* kai__parser_create_number(Kai_Parser* parser, Kai_Token token);
KAI_INTERNAL Kai_Expr* kai__parser_create_literal(Kai_Parser* parser, Kai_Token token, Kai_Expr* head, Kai_u32 count);
KAI_INTERNAL Kai_Expr* kai__parser_create_unary(Kai_Parser* parser, Kai_Token op_token, Kai_Expr* expr);
KAI_INTERNAL Kai_Expr* kai__parser_create_binary(Kai_Parser* parser, Kai_Expr* left, Kai_Expr* right, Kai_u32 op);
KAI_INTERNAL Kai_Expr* kai__parser_create_array(Kai_Parser* parser, Kai_Token op_token, Kai_Expr* expr, Kai_Expr* rows, Kai_Expr* cols, Kai_u8 flags);
KAI_INTERNAL Kai_Expr* kai__parser_create_special(Kai_Parser* parser, Kai_Token token, Kai_u8 kind);
KAI_INTERNAL Kai_Expr* kai__parser_create_procedure_type(Kai_Parser* parser, Kai_Expr* in_out, Kai_u8 in_count, Kai_u8 out_count);
KAI_INTERNAL Kai_Expr* kai__parser_create_procedure_call(Kai_Parser* parser, Kai_Expr* proc, Kai_Expr* args, Kai_u8 arg_count);
KAI_INTERNAL Kai_Expr* kai__parser_create_procedure(Kai_Parser* parser, Kai_Token token, Kai_Expr* in_out, Kai_Stmt* body, Kai_u8 in_count, Kai_u8 out_count);
KAI_INTERNAL Kai_Expr* kai__parser_create_import(Kai_Parser* parser, Kai_Token token, Kai_Token import);
KAI_INTERNAL Kai_Expr* kai__parser_create_struct(Kai_Parser* parser, Kai_Token token, Kai_u32 field_count, Kai_Stmt* body);
KAI_INTERNAL Kai_Expr* kai__parser_create_enum(Kai_Parser* parser, Kai_Token token, Kai_Expr* type, Kai_u32 field_count, Kai_Stmt* body);
KAI_INTERNAL Kai_Expr* kai__parser_create_return(Kai_Parser* parser, Kai_Token ret_token, Kai_Expr* expr);
KAI_INTERNAL Kai_Expr* kai__parser_create_declaration(Kai_Parser* parser, Kai_string name, Kai_Expr* type, Kai_Expr* expr, Kai_u8 flags, Kai_u32 line_number);
KAI_INTERNAL Kai_Expr* kai__parser_create_assignment(Kai_Parser* parser, Kai_u32 op, Kai_Expr* left, Kai_Expr* expr);
KAI_INTERNAL Kai_Expr* kai__parser_create_if(Kai_Parser* parser, Kai_Token if_token, Kai_u8 flags, Kai_Expr* expr, Kai_Stmt* then_body, Kai_Stmt* else_body);
KAI_INTERNAL Kai_Expr* kai__parser_create_while(Kai_Parser* parser, Kai_Token while_token, Kai_Expr* expr, Kai_Stmt* body);
KAI_INTERNAL Kai_Expr* kai__parser_create_for(Kai_Parser* parser, Kai_Token for_token, Kai_string name, Kai_Expr* from, Kai_Expr* to, Kai_Stmt* body, Kai_u8 flags);
KAI_INTERNAL Kai_Expr* kai__parser_create_control(Kai_Parser* parser, Kai_Token token, Kai_u8 kind, Kai_Expr* expr);
KAI_INTERNAL Kai_Expr* kai__parser_create_compound(Kai_Parser* parser, Kai_Token token, Kai_Stmt* body);
KAI_INTERNAL Kai_Tag* kai__parser_create_tag(Kai_Parser* parser, Kai_Token token, Kai_Expr* expr);
KAI_INTERNAL Kai_bool kai__is_procedure_next(Kai_Parser* parser);
KAI_INTERNAL Kai_bool kai__create_syntax_trees(Kai_Compiler_Context* context, Kai_Source_Slice sources);
KAI_INTERNAL Kai_bool kai__inside_procedure_scope(Kai_Compiler_Context* context);
KAI_INTERNAL Kai_bool kai__error_redefinition(Kai_Compiler_Context* context, Kai_Location location, Kai_u32 original);
KAI_INTERNAL Kai_bool kai__error_not_declared(Kai_Compiler_Context* context, Kai_Location location);
KAI_INTERNAL Kai_bool kai__create_nodes(Kai_Compiler_Context* context, Kai_Expr* expr);
KAI_INTERNAL Kai_Node_Reference kai__lookup_node(Kai_Compiler_Context* context, Kai_string name);
KAI_INTERNAL Kai_bool kai__insert_value_dependencies(Kai_Compiler_Context* context, Kai_Expr* expr);
KAI_INTERNAL Kai_bool kai__insert_type_dependencies(Kai_Compiler_Context* context, Kai_Expr* expr);
KAI_INTERNAL Kai_bool kai__generate_dependency_builtin_types(Kai_Compiler_Context* context);
KAI_INTERNAL Kai_bool kai__generate_dependency_graph(Kai_Compiler_Context* context);
KAI_INTERNAL void kai__explore_dependencies(Kai__DFS_Context* dfs, Kai_Node_Reference ref);
KAI_INTERNAL Kai_bool kai__generate_compilation_order(Kai_Compiler_Context* context);
KAI_INTERNAL Kai_bool kai__error_fatal(Kai_Compiler_Context* context, Kai_string message);
KAI_INTERNAL Kai_bool kai__value_to_number(Kai_Value value, Kai_Type_Info* type, Kai_Number* out_number);
KAI_INTERNAL Kai_Value kai__evaluate_binary_operation(Kai_u32 op, Kai_Type_Info* type, Kai_Value a, Kai_Value b);
KAI_INTERNAL Kai_bool kai__type_check(Kai_Compiler_Context* context, Kai_Expr* expr, Kai_Type_Info** out_or_expected);
KAI_INTERNAL Kai_bool kai__value_of_expression(Kai_Compiler_Context* context, Kai_Expr* expr, Kai_Value* out_value, Kai_Type* out_type);
KAI_INTERNAL Kai_Type kai__type_of_expression(Kai_Compiler_Context* context, Kai_Expr* expr);
KAI_INTERNAL Kai_bool kai__compile_node_value(Kai_Compiler_Context* context, Kai_Node* node);
KAI_INTERNAL Kai_Import* kai__find_import(Kai_Compiler_Context* context, Kai_string name);
KAI_INTERNAL Kai_Expr* kai__expression_from_string(Kai_Compiler_Context* context, Kai_string s);
KAI_INTERNAL Kai_bool kai__compile_node_type(Kai_Compiler_Context* context, Kai_Node* node);
KAI_INTERNAL Kai_u32 kai__type_size(Kai_Type_Info* type);
KAI_INTERNAL void kai__copy_value(Kai_u8* out, Kai_Type_Info* type, Kai_Value value);
KAI_INTERNAL Kai_u32 kai__push_value(Kai_Compiler_Context* context, Kai_Type_Info* type, Kai_Value value);
KAI_INTERNAL Kai_bool kai__compile_all_nodes(Kai_Compiler_Context* context);
KAI_INTERNAL Kai_bool kai__generate_compiler_ir(Kai_Compiler_Context* context);
KAI_INTERNAL void kai__file_writer_write_value(void* user, Kai_u32 type, Kai_Value value, Kai_Write_Format format);
KAI_INTERNAL void kai__file_writer_write_string(void* user, Kai_string s);
KAI_INTERNAL void kai__stdout_writer_write_value(void* user, Kai_u32 type, Kai_Value value, Kai_Write_Format format);
KAI_INTERNAL void kai__stdout_writer_write_string(void* user, Kai_string s);
KAI_INTERNAL void kai__stdout_writer_set_color(void* user, Kai_Write_Color color);
KAI_INTERNAL void* kai__memory_heap_allocate(void* user, void* old_ptr, Kai_u32 new_size, Kai_u32 old_size);

KAI_API(Kai_string) kai_version_string(void)
{
    return KAI_STRING(KAI_VERSION_STRING);
}

KAI_API(Kai_vector3_u32) kai_version(void)
{
    Kai_vector3_u32 v = {0};
    v.x = KAI_VERSION_MAJOR;
    v.y = KAI_VERSION_MINOR;
    v.z = KAI_VERSION_PATCH;
    return v;
}

KAI_INTERNAL Kai_string kai__range_to_string(Kai_Range range, Kai_Memory memory)
{
    return ((Kai_string){.count = range.count, .data = (Kai_u8*)(memory.data)+range.start});
}

KAI_INTERNAL Kai_u64 kai__ceil_div(Kai_u64 num, Kai_u64 den)
{
    return ((num+den)-1)/den;
}

KAI_INTERNAL Kai_u64 kai__ceil_div_fast(Kai_u64 num, Kai_u32 exp)
{
    return ((num+(((Kai_u64)(1))<<exp))-1)>>exp;
}

KAI_INTERNAL Kai_u32 kai__max_u32(Kai_u32 a, Kai_u32 b)
{
    if (a<b)
        return b;
    return a;
}

KAI_INTERNAL Kai_u32 kai__min_u32(Kai_u32 a, Kai_u32 b)
{
    if (a<b)
        return a;
    return b;
}

KAI_INTERNAL Kai_u64 kai__gcd(Kai_u64 a, Kai_u64 b)
{
    Kai_u64 r = 1;
    while (r!=0)
    {
        r = a%b;
        a = b;
        b = r;
    }
    return a;
}

KAI_INTERNAL Kai_f64 kai__ldexp(Kai_f64 x, Kai_int n)
{
    union { Kai_f64 f; Kai_u64 i; } u = {0};
    Kai_f64 y = x;
    if (n>1023)
    {
        y *= 0x1p1023;
        n -= 1023;
        if (n>1023)
        {
            y *= 0x1p1023;
            n -= 1023;
            if (n>1023)
                n = 1023;
        }
    }
    else
    if (n<-1022)
    {
        y *= 0x1p-1022*0x1p53;
        n += 1022-53;
        if (n<-1022)
        {
            y *= 0x1p-1022*0x1p53;
            n += 1022-53;
            if (n<-1022)
                n = -1022;
        }
    }
    u.i = ((Kai_u64)(1023+n))<<52;
    x = y*u.f;
    return x;
}

KAI_INTERNAL Kai_u64 kai__mul_with_shift(Kai_u64 a, Kai_u64 b, Kai_s32* exp, Kai_bool sub)
{
    Kai_u128 full = kai_intrinsics_u128_multiply(a, b);
    Kai_u64 hi = kai_intrinsics_u128_high(full);
    Kai_u64 lo = kai_intrinsics_u128_low(full);
    if (hi==0)
    {
        return lo;
    }
    Kai_s32 shift = 64-(Kai_s32)(kai_intrinsics_clz64(hi));
    if (sub)
    {
        *exp -= shift;
    }
    else
    {
        *exp += shift;
    }
    return hi<<(64-shift)|lo>>shift;
}

KAI_INTERNAL Kai_u64 kai__add_with_shift(Kai_u64 a, Kai_u64 b, Kai_s32* exp, Kai_bool sub)
{
    Kai_u64 sum = a+b;
    Kai_u32 carry = sum<a;
    if (!carry)
        return sum;
    if (sub)
    {
        *exp -= 1;
    }
    else
    {
        *exp += 1;
    }
    return ((Kai_u64)(1))<<63|sum>>1;
}

KAI_INTERNAL void kai__memory_copy(void* dst, void* src, Kai_u32 size)
{
    for (Kai_u32 i = 0; i < size; ++i)
    {
        ((Kai_u8*)(dst))[i] = ((Kai_u8*)(src))[i];
    }
}

KAI_INTERNAL void kai__memory_zero(void* dst, Kai_u32 size)
{
    for (Kai_u32 i = 0; i < size; ++i)
    {
        ((Kai_u8*)(dst))[i] = 0;
    }
}

KAI_INTERNAL void kai__memory_fill(void* dst, Kai_u8 byte, Kai_u32 size)
{
    for (Kai_u32 i = 0; i < size; ++i)
    {
        ((Kai_u8*)(dst))[i] = byte;
    }
}

KAI_API(Kai_bool) kai_string_equals(Kai_string left, Kai_string right)
{
    if (left.count!=right.count)
        return KAI_FALSE;
    for (Kai_u32 i = 0; i < left.count; ++i)
    {
        if ((left.data)[i]!=(right.data)[i])
            return KAI_FALSE;
    }
    return KAI_TRUE;
}

KAI_API(Kai_string) kai_string_from_c(Kai_cstring s)
{
    Kai_u32 count = {0};
    while (s[count]!=0)
        count += 1;
    return ((Kai_string){.count = count, .data = (Kai_u8*)(s)});
}

KAI_API(Kai_string) kai_string_copy_from_c(Kai_string dst, Kai_cstring src)
{
    Kai_u32 i = {0};
    while (i<dst.count&&src[i]!=0)
    {
        (dst.data)[i] = src[i];
        i += 1;
    }
    return ((Kai_string){.count = i, .data = dst.data});
}

KAI_API(Kai_string) kai_merge_strings(Kai_string a, Kai_string b)
{
    Kai_string left = {0};
    Kai_string right = {0};
    if (a.data<b.data)
    {
        left = a;
        right = b;
    }
    else
    {
        left = b;
        right = a;
    }
    return ((Kai_string){.count = (Kai_u32)((right.data+(Kai_uint)(right.count))-left.data), .data = left.data});
}

KAI_API(Kai_u64) kai_string_hash(Kai_string s)
{
    Kai_u64 hash = 5381;
    for (Kai_u32 i = 0; i < s.count; ++i)
        hash = ((hash<<5)+hash)+(s.data)[i];
    return hash;
}

KAI_API(Kai_u64) kai_string_hash_next(Kai_u64 hash, Kai_string s)
{
    for (Kai_u32 i = 0; i < s.count; ++i)
        hash = ((hash<<5)+hash)+(s.data)[i];
    return hash;
}

KAI_API(void) kai_raw_array_reserve(Kai_Raw_Dynamic_Array* array, Kai_u32 new_capacity, Kai_Allocator* allocator, Kai_u32 elem_size)
{
    if (new_capacity<=array->capacity)
        return;
    array->data = kai__allocate(array->data, new_capacity*elem_size, array->capacity*elem_size);
    array->capacity = new_capacity;
}

KAI_API(void) kai_raw_array_resize(Kai_Raw_Dynamic_Array* array, Kai_u32 new_size, Kai_Allocator* allocator, Kai_u32 elem_size)
{
    kai_raw_array_reserve(array, new_size, allocator, elem_size);
    array->count = new_size;
}

KAI_API(void) kai_raw_array_grow(Kai_Raw_Dynamic_Array* array, Kai_u32 count, Kai_Allocator* allocator, Kai_u32 elem_size)
{
    if (array->count+count<=array->capacity)
        return;
    Kai_u32 n = array->count+count;
    Kai_u32 new_capacity = kai__max_u32(n+(n>>1), 8);
    kai_raw_array_reserve(array, new_capacity, allocator, elem_size);
}

KAI_API(Kai_Hash_Table_Size) kai_raw_hash_table_size(Kai_u32 capacity, Kai_u32 elem_size)
{
    Kai_Hash_Table_Size r = {0};
    r.occupied = (Kai_u32)(kai__ceil_div_fast(capacity, 6))*sizeof(Kai_u64);
    r.hashes = capacity*sizeof(Kai_u64);
    r.keys = capacity*sizeof(Kai_string);
    r.values = (capacity+1)*elem_size;
    r.total = ((r.occupied+r.hashes)+r.keys)+r.values;
    return r;
}

KAI_API(void) kai_raw_hash_table_grow(Kai_Raw_Hash_Table* table, Kai_Allocator* allocator, Kai_u32 elem_size)
{
    Kai_u32 new_capacity = kai__max_u32(8, table->capacity*2);
    Kai_Hash_Table_Size new_size = kai_raw_hash_table_size(new_capacity, elem_size);
    void* new_ptr = kai__allocate(NULL, new_size.total, 0);
    Kai_u64* occupied = (Kai_u64*)(new_ptr);
    Kai_u64* hashes = (Kai_u64*)((Kai_u8*)(occupied)+new_size.occupied);
    Kai_string* keys = (Kai_string*)((Kai_u8*)(hashes)+new_size.hashes);
    Kai_u8* values = (Kai_u8*)((Kai_u8*)(keys)+new_size.keys)+elem_size;
    Kai_u32 count = 0;
    for (Kai_u32 i = 0; i < table->capacity; ++i)
        if ((table->occupied)[i/64]&((Kai_u64)(1))<<(i%64))
        {
            Kai_u64 hash = kai_string_hash((table->keys)[i]);
            Kai_u32 mask = new_capacity-1;
            Kai_u32 index = (Kai_u32)(hash)&mask;
            for (Kai_u32 j = 0; j < new_capacity; ++j)
            {
                Kai_u64 block = occupied[index/64];
                Kai_u32 bit = ((Kai_u64)(1))<<(index%64);
                if ((block&bit)==0)
                {
                    occupied[index/64] |= bit;
                    hashes[index] = hash;
                    keys[index] = (table->keys)[i];
                    Kai_u8* src = (Kai_u8*)(table->values)+i*elem_size;
                    kai__memory_copy(values+index*elem_size, src, elem_size);
                    count += 1;
                    break;
                }
                index = index+1&mask;
            }
        }
    kai_assert(count==table->count);
    if (table->capacity!=0)
    {
        kai__free(table->occupied, kai_raw_hash_table_size(table->capacity, elem_size).total);
    }
    table->capacity = new_capacity;
    table->occupied = occupied;
    table->hashes = hashes;
    table->keys = keys;
    table->values = values;
}

KAI_API(Kai_bool) kai_raw_hash_table_emplace_key(Kai_Raw_Hash_Table* table, Kai_string key, Kai_u32* out_index, Kai_Allocator* allocator, Kai_u32 elem_size)
{
    if (4*table->count>=3*table->capacity)
        kai_raw_hash_table_grow(table, allocator, elem_size);
    Kai_u64 hash = kai_string_hash(key);
    Kai_u32 mask = table->capacity-1;
    Kai_u32 index = (Kai_u32)(hash)&mask;
    for (Kai_u32 i = 0; i < table->capacity; ++i)
    {
        Kai_u64 byte = (table->occupied)[index/64];
        Kai_u64 bit = ((Kai_u64)(1))<<(index%64);
        if ((byte&bit)==0)
        {
            (table->occupied)[index/64] |= bit;
            (table->hashes)[index] = hash;
            (table->keys)[index] = key;
            table->count += 1;
            *out_index = index;
            return KAI_TRUE;
        }
        if ((table->hashes)[index]==hash&&kai_string_equals((table->keys)[index], key))
        {
            *out_index = index;
            return KAI_FALSE;
        }
        index = index+1&mask;
    }
    *out_index = 4294967295;
    return KAI_FALSE;
}

KAI_API(Kai_int) kai_raw_hash_table_find(Kai_Raw_Hash_Table* table, Kai_string key)
{
    Kai_u64 hash = kai_string_hash(key);
    Kai_u32 mask = table->capacity-1;
    Kai_u32 index = (Kai_u32)(hash)&mask;
    for (Kai_u32 i = 0; i < table->capacity; ++i)
    {
        Kai_u64 block = (table->occupied)[index/64];
        Kai_u64 bit = ((Kai_u64)(1))<<(index%64);
        if ((block&bit)==0)
        {
            return -1;
        }
        else
        if ((table->hashes)[index]==hash&&kai_string_equals((table->keys)[index], key))
        {
            return (Kai_int)(index);
        }
        index = index+1&mask;
    }
    return -1;
}

KAI_API(Kai_u64) kai_number_to_u64(Kai_Number number)
{
    return (number.n)<<(number.e);
}

KAI_API(Kai_f64) kai_number_to_f64(Kai_Number number)
{
    if (number.n==0)
        return 0;
    Kai_f64 val = (Kai_f64)(number.n)/(Kai_f64)(number.d);
    if (number.is_neg)
    {
        val = -val;
    }
    return kai__ldexp(val, number.e);
}

KAI_API(Kai_Number) kai_number_normalize(Kai_Number number)
{
    Kai_s32 ns = kai_intrinsics_ctz64(number.n);
    Kai_s32 ds = kai_intrinsics_ctz64(number.d);
    Kai_s32 ex = number.e+(ns-ds);
    Kai_u64 nu = (number.n)>>ns;
    Kai_u64 de = (number.d)>>ds;
    Kai_u64 cd = kai__gcd(nu, de);
    return ((Kai_Number){.n = nu/cd, .d = de/cd, .e = ex, .is_neg = number.is_neg});
}

KAI_API(Kai_Number) kai_number_neg(Kai_Number a)
{
    return ((Kai_Number){.n = a.n, .d = a.d, .e = a.e, .is_neg = a.is_neg^1});
}

KAI_API(Kai_Number) kai_number_abs(Kai_Number a)
{
    return ((Kai_Number){.n = a.n, .d = a.d, .e = a.e, .is_neg = 0});
}

KAI_API(Kai_Number) kai_number_inv(Kai_Number a)
{
    return ((Kai_Number){.n = a.d, .d = a.n, .e = -a.e, .is_neg = a.is_neg});
}

KAI_API(Kai_Number) kai_number_add(Kai_Number a, Kai_Number b)
{
    if (a.n==0)
        return b;
    if (b.n==0)
        return a;
    if (a.e==b.e)
        return kai_number_add_same_exp(a, b);
    kai_number_match_exponents(&a, &b);
    return kai_number_add_same_exp(a, b);
}

KAI_API(Kai_Number) kai_number_sub(Kai_Number a, Kai_Number b)
{
    return kai_number_add(a, kai_number_neg(b));
}

KAI_API(Kai_Number) kai_number_mul(Kai_Number a, Kai_Number b)
{
    if (a.n==0||b.n==0)
        return ((Kai_Number){0});
    Kai_s32 ex = a.e+b.e;
    Kai_u64 d0 = kai__gcd(a.n, b.d);
    Kai_u64 d1 = kai__gcd(b.n, a.d);
    Kai_u64 nu = kai__mul_with_shift(a.n/d0, b.n/d1, &ex, 0);
    Kai_u64 de = kai__mul_with_shift(a.d/d1, b.d/d0, &ex, 1);
    return kai_number_normalize(((Kai_Number){.n = nu, .d = de, .e = ex, .is_neg = a.is_neg^b.is_neg}));
}

KAI_API(Kai_Number) kai_number_div(Kai_Number a, Kai_Number b)
{
    return kai_number_mul(a, kai_number_inv(b));
}

KAI_API(Kai_Number) kai_number_pow_int(Kai_Number a, Kai_u32 exp)
{
    Kai_Number r = a;
    Kai_s32 i = 30-(Kai_s32)(kai_intrinsics_clz32(exp));
    if (i>=0)
    {
        Kai_u32 bit = ((Kai_u32)(1))<<i;
        while (bit!=0)
        {
            r = kai_number_mul(r, r);
            if (exp&bit)
                r = kai_number_mul(r, a);
            bit = bit>>1;
        }
    }
    return r;
}

KAI_API(Kai_Number) kai_number_add_same_exp(Kai_Number a, Kai_Number b)
{
    Kai_s32 t0e = 0;
    Kai_s32 t1e = 0;
    Kai_u64 t0 = kai__mul_with_shift(a.n, b.d, &t0e, 0);
    Kai_u64 t1 = kai__mul_with_shift(b.n, a.d, &t1e, 0);
    Kai_s32 ex = a.e;
    if (t0e!=t1e)
    {
        if (t0e<t1e)
        {
            t0 = t0>>(t1e-t0e);
            ex += t1e;
        }
        else
        {
            t1 = t1>>(t0e-t1e);
            ex += t0e;
        }
    }
    Kai_u64 de = kai__mul_with_shift(a.d, b.d, &ex, 1);
    Kai_u64 nu = 0;
    Kai_u32 is_neg = a.is_neg&b.is_neg;
    if (a.is_neg&&!b.is_neg)
    {
        if (t0>t1)
        {
            nu = t0-t1;
            is_neg = 1;
        }
        else
        {
            nu = t1-t0;
            is_neg = 0;
        }
    }
    else
    if (b.is_neg&&!a.is_neg)
    {
        if (t0>t1)
        {
            nu = t0-t1;
            is_neg = 0;
        }
        else
        {
            nu = t1-t0;
            is_neg = 1;
        }
    }
    else
    {
        nu = kai__add_with_shift(t0, t1, &ex, 0);
    }
    return kai_number_normalize(((Kai_Number){.n = nu, .d = de, .e = ex, .is_neg = is_neg}));
}

KAI_API(void) kai_number_match_exponents(Kai_Number* a, Kai_Number* b)
{
    if (a->e<b->e)
    {
        while (a->e!=b->e&&(a->d)>>63==0)
        {
            a->d = (a->d)<<1;
            a->e += 1;
        }
        while (a->e!=b->e&&(b->n)>>63==0)
        {
            b->n = (b->n)<<1;
            b->e -= 1;
        }
        Kai_s64 diff = b->e-a->e;
        a->n = (a->n)>>((Kai_u32)(diff));
        a->e = b->e;
    }
    else
    {
        while (a->e!=b->e&&(b->d)>>63==0)
        {
            b->d = (b->d)<<1;
            b->e += 1;
        }
        while (a->e!=b->e&&(a->n)>>63==0)
        {
            a->n = (a->n)<<1;
            a->e -= 1;
        }
        Kai_s64 diff = a->e-b->e;
        b->n = (b->n)>>((Kai_u32)(diff));
        b->e = a->e;
    }
}

KAI_API(Kai_Number) kai_number_parse_whole(Kai_string source, Kai_u32* offset, Kai_u32 base)
{
    Kai_Number result = {0};
    Kai_Number base_number = kai_number_normalize(((Kai_Number){.n = base, .d = 1}));
    Kai_u32 i = *offset;
    while (i<source.count)
    {
        Kai_u64 ch = (source.data)[i];
        Kai_u64 dg = 0;
        if (ch>=48&&ch<=57)
            dg = ch-48;
        else
        if (ch>=65&&ch<=70)
            dg = (ch-65)+10;
        else
        if (ch>=97&&ch<=102)
            dg = (ch-97)+10;
        else
        if (ch==95)
        {
            i += 1;
            continue;
        }
        else
            break;
        if (dg>=base)
            break;
        result = kai_number_mul(result, base_number);
        result = kai_number_add(result, ((Kai_Number){.n = dg, .d = 1}));
        i += 1;
    }
    *offset = i;
    return result;
}

KAI_API(Kai_Number) kai_number_parse_decimal(Kai_string source, Kai_u32* offset)
{
    Kai_Number result = {0};
    Kai_Number power = ((Kai_Number){1, 5, -1, 0});
    Kai_u32 i = *offset;
    while (i<source.count)
    {
        Kai_u32 d = (Kai_u32)((source.data)[i]-48);
        if (d>9)
            break;
        Kai_Number h = kai_number_mul(power, ((Kai_Number){d, 1, 0, 0}));
        result = kai_number_add(result, h);
        power = kai_number_mul(power, ((Kai_Number){1, 5, -1, 0}));
        i += 1;
    }
    *offset = i;
    return result;
}

static Kai_u64 kai__powers_of_five[10] = {
    1, 5, 25, 125, 625, 3125, 15625, 78125, 390625, 1953125
};

KAI_API(Kai_Number) kai_number_parse_exponent(Kai_string source, Kai_u32* offset)
{
    Kai_Number result = ((Kai_Number){1, 1, 0, 0});
    Kai_u32 i = *offset;
    while (i<source.count)
    {
        Kai_s32 d = (source.data)[i]-48;
        if (d>9)
            break;
        result = kai_number_pow_int(result, 10);
        result = kai_number_mul(result, ((Kai_Number){kai__powers_of_five[d], 1, d, 0}));
        i += 1;
    }
    *offset = i;
    return result;
}

KAI_API(void*) kai_fixed_allocate(Kai_Fixed_Allocator* arena, Kai_u32 size)
{
    kai_assert(arena!=NULL);
    kai_assert(arena->offset+size<=arena->size);
    void* ptr = (Kai_u8*)(arena->data)+arena->offset;
    arena->offset += size;
    return ptr;
}

KAI_API(void) kai_arena_create(Kai_Arena_Allocator* arena, Kai_Allocator* base)
{
    kai_assert(arena!=NULL);
    kai_assert(base!=NULL);
    arena->base = *base;
    arena->bucket_size = (Kai_u32)(kai__ceil_div(65536, base->page_size))*base->page_size;
    arena->current_allocated = sizeof(Kai_Arena_Bucket*);
    arena->current_bucket = (Kai_Arena_Bucket*)(base->heap_allocate(base->user, NULL, arena->bucket_size, 0));
}

KAI_API(void) kai_arena_destroy(Kai_Arena_Allocator* arena)
{
    kai_arena_free_all(arena);
    arena->bucket_size = 0;
    arena->base = ((Kai_Allocator){});
}

KAI_API(void) kai_arena_free_all(Kai_Arena_Allocator* arena)
{
    kai_assert(arena!=NULL);
    Kai_Arena_Bucket* bucket = arena->current_bucket;
    while (bucket)
    {
        Kai_Arena_Bucket* prev = bucket->prev;
        (arena->base).heap_allocate((arena->base).user, bucket, 0, arena->bucket_size);
        bucket = prev;
    }
    arena->current_bucket = NULL;
    arena->current_allocated = 0;
}

KAI_API(void*) kai_arena_allocate(Kai_Arena_Allocator* arena, Kai_u32 size)
{
    kai_assert(arena!=NULL);
    if (size>arena->bucket_size)
        kai_fatal_error("Arena Allocator", "Object size greater than bucket size (incorrect usage)");
    if (arena->current_allocated+size>arena->bucket_size)
    {
        Kai_Arena_Bucket* new_bucket = (Kai_Arena_Bucket*)((arena->base).heap_allocate((arena->base).user, NULL, arena->bucket_size, 0));
        if (new_bucket==NULL)
            return NULL;
        new_bucket->prev = arena->current_bucket;
        arena->current_bucket = new_bucket;
        arena->current_allocated = sizeof(Kai_Arena_Bucket*);
    }
    Kai_u8* bytes = (Kai_u8*)(arena->current_bucket);
    void* ptr = bytes+arena->current_allocated;
    arena->current_allocated += size;
    return ptr;
}

KAI_INTERNAL void kai__buffer_append_string(Kai_Buffer* buffer, Kai_string s)
{
    Kai_Allocator* allocator = &buffer->allocator;
    kai_array_grow(&buffer->array, s.count);
    kai__memory_copy((buffer->array).data+(buffer->array).count, s.data, s.count);
    (buffer->array).count += s.count;
}

KAI_INTERNAL Kai_Range kai__buffer_end(Kai_Buffer* buffer)
{
    Kai_Range out = ((Kai_Range){.start = buffer->offset, .count = (buffer->array).count-buffer->offset});
    buffer->offset = (buffer->array).count;
    return out;
}

KAI_INTERNAL Kai_Range kai__buffer_push(Kai_Buffer* buffer, Kai_u32 size)
{
    Kai_Allocator* allocator = &buffer->allocator;
    kai_array_grow(&buffer->array, size);
    (buffer->array).count += size;
    return kai__buffer_end(buffer);
}

KAI_INTERNAL Kai_Memory kai__buffer_done(Kai_Buffer* buffer)
{
    Kai_Memory memory = ((Kai_Memory){.size = (buffer->array).capacity, .data = (buffer->array).data});
    *buffer = ((Kai_Buffer){.allocator = buffer->allocator});
    return memory;
}

KAI_INTERNAL Kai_u32 kai__base10_digit_count(Kai_u32 x)
{
    if (x<10)
        return 1;
    if (x<100)
        return 2;
    if (x<1000)
        return 3;
    if (x<10000)
        return 4;
    if (x<100000)
        return 5;
    if (x<1000000)
        return 6;
    if (x<10000000)
        return 7;
    if (x<100000000)
        return 8;
    if (x<1000000000)
        return 9;
    return 0;
}

KAI_INTERNAL Kai_u8* kai__advance_to_line(Kai_u8* source, Kai_u32 line)
{
    line -= 1;
    while (line>0)
    {
        if (*source==10)
            line -= 1;
        source += 1;
    }
    return source;
}

KAI_INTERNAL void kai__write_source_code(Kai_Writer* writer, Kai_u8* src)
{
    while (*src!=0&&*src!=10)
    {
        if (*src==9)
            kai__write(" ");
        else
            kai__write_string(((Kai_string){.count = 1, .data = src}));
        src += 1;
    }
}

KAI_INTERNAL Kai_u32 kai__utf8_decode(Kai_u8** s)
{
    Kai_u8* p = *s;
    Kai_u32 cp = 0;
    Kai_u32 len = 0;
    if (p[0]<128)
    {
        cp = p[0];
        len = 1;
    }
    else
    if ((p[0]&224)==192)
    {
        cp = (p[0]&31)<<6|(p[1]&63);
        len = 2;
    }
    else
    if ((p[0]&240)==224)
    {
        cp = ((p[0]&15)<<12|(p[1]&63)<<6)|(p[2]&63);
        len = 3;
    }
    else
    if ((p[0]&248)==240)
    {
        cp = (((p[0]&7)<<18|(p[1]&63)<<12)|(p[2]&63)<<6)|(p[3]&63);
        len = 4;
    }
    else
    {
        cp = 65533;
        len = 1;
    }
    *s += len;
    return cp;
}

KAI_INTERNAL Kai_u32 kai__unicode_char_width(Kai_Writer* writer, Kai_u32 cp, Kai_u8 first, Kai_u8 ch)
{
    Kai_u32 count = 0;
    if (((cp==0||(cp<32&&cp!=9))||(cp>=127&&cp<160))||(cp>=768&&cp<=879))
        return 0;
    if ((((((((((cp>=4352&&cp<=4447)||(cp>=9001&&cp<=9002))||(cp>=11904&&cp<=42191))||(cp>=44032&&cp<=55203))||(cp>=63744&&cp<=64255))||(cp>=65040&&cp<=65049))||(cp>=65072&&cp<=65135))||(cp>=65280&&cp<=65376))||(cp>=65504&&cp<=65510))||(cp>=127744&&cp<=129791))
    {
        kai__write_fill(first, 1);
        first = ch;
        count += 1;
    }
    kai__write_fill(first, 1);
    first = ch;
    return count+1;
}

KAI_INTERNAL void kai__write_source_code_fill(Kai_Writer* writer, Kai_u8* src, Kai_u8* end, Kai_u8 first, Kai_u8 ch)
{
    while ((src<end&&*src!=0)&&*src!=10)
    {
        Kai_u32 cp = kai__utf8_decode(&src);
        kai__unicode_char_width(writer, cp, first, ch);
        first = ch;
    }
}

static Kai_string kai_result_string_map[7] = {
    KAI_CONST_STRING("Success"), KAI_CONST_STRING("Memory Error"), KAI_CONST_STRING("Syntax Error"), 
    KAI_CONST_STRING("Semantic Error"), KAI_CONST_STRING("Info"), KAI_CONST_STRING("Fatal Error"), 
    KAI_CONST_STRING("Internal Error")
};

KAI_API(void) kai_write_error(Kai_Writer* writer, Kai_Error* error)
{
    while (error!=NULL)
    {
        if (error->result==KAI_SUCCESS)
        {
            kai__write("[Success]\n");
            return;
        }
        if (error->result>=KAI_RESULT_COUNT)
        {
            kai__write("[Invalid result value]\n");
            return;
        }
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT_2);
        if ((((error->location).source).name).count!=0)
            kai__write_string(((error->location).source).name);
        else
            kai__write("[...]");
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write(" --> ");
        if (error->result!=KAI_ERROR_INFO)
        {
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
        }
        else
        {
            kai__set_color(KAI_WRITE_COLOR_SECONDARY);
        }
        kai__write_string(kai_result_string_map[error->result]);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write(": ");
        kai__write_string(error->message);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write("\n");
        if (error->result==KAI_ERROR_FATAL||error->result==KAI_ERROR_INTERNAL)
        {
            error = error->next;
            continue;
        }
        kai__set_color(KAI_WRITE_COLOR_DECORATION);
        Kai_u32 digits = kai__base10_digit_count((error->location).line);
        kai__write_fill(32, digits);
        kai__write("  |\n");
        kai__write(" ");
        kai__write_u32((error->location).line);
        kai__write(" | ");
        Kai_u8* begin = kai__advance_to_line((((error->location).source).contents).data, (error->location).line);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write_source_code(writer, begin);
        kai__write("\n");
        kai__set_color(KAI_WRITE_COLOR_DECORATION);
        kai__write_fill(32, digits);
        kai__write("  | ");
        kai__write_source_code_fill(writer, begin, ((error->location).string).data, 32, 32);
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
        kai__write_source_code_fill(writer, ((error->location).string).data, ((error->location).string).data+((error->location).string).count, 94, 126);
        kai__write(" ");
        kai__write_string(error->context);
        kai__write("\n");
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        error = error->next;
    }
}

KAI_API(void) kai_write_type(Kai_Writer* writer, Kai_Type_Info* type)
{
    if (type==NULL)
    {
        kai__write("null");
        return;
    }
    switch (type->id)
    {
        break; default:
        {
            kai__write("id = ");
            kai__write_u32(type->id);
        }
        break; case KAI_TYPE_ID_VOID:
        kai__write("void");
        break; case KAI_TYPE_ID_TYPE:
        kai__write("#Type");
        break; case KAI_TYPE_ID_NUMBER:
        kai__write("#Number");
        break; case KAI_TYPE_ID_BOOLEAN:
        kai__write("bool");
        break; case KAI_TYPE_ID_INTEGER:
        {
            Kai_Type_Info_Integer* info = (Kai_Type_Info_Integer*)(type);
            if (info->is_signed)
                kai__write("s");
            else
                kai__write("u");
            kai__write_u32(info->bits);
        }
        break; case KAI_TYPE_ID_FLOAT:
        {
            Kai_Type_Info_Float* info = (Kai_Type_Info_Float*)(type);
            kai__write("f");
            kai__write_u32(info->bits);
        }
        break; case KAI_TYPE_ID_POINTER:
        {
            Kai_Type_Info_Pointer* info = (Kai_Type_Info_Pointer*)(type);
            kai__write("*");
            kai_write_type(writer, info->sub_type);
        }
        break; case KAI_TYPE_ID_PROCEDURE:
        {
            Kai_Type_Info_Procedure* info = (Kai_Type_Info_Procedure*)(type);
            kai__write("(");
            for (Kai_u32 i = 0; i < (info->inputs).count; ++i)
            {
                kai_write_type(writer, ((info->inputs).data)[i]);
                if (i<(info->inputs).count-1)
                    kai__write(", ");
            }
            kai__write(") -> (");
            for (Kai_u32 i = 0; i < (info->outputs).count; ++i)
            {
                kai_write_type(writer, ((info->outputs).data)[i]);
                if (i<(info->outputs).count-1)
                    kai__write(", ");
            }
            kai__write(")");
        }
        break; case KAI_TYPE_ID_STRING:
        kai__write("string");
        break; case KAI_TYPE_ID_STRUCT:
        {
            Kai_Type_Info_Struct* info = (Kai_Type_Info_Struct*)(type);
            kai__write("struct {");
            for (Kai_u32 i = 0; i < (info->fields).count; ++i)
            {
                Kai_Struct_Field field = ((info->fields).data)[i];
                kai__write_string(field.name);
                kai__write(":");
                kai_write_type(writer, field.type);
                if (i!=(info->fields).count-1)
                    kai__write("; ");
            }
            kai__write("}");
        }
    }
}

KAI_API(void) kai_write_value(Kai_Writer* writer, void* data, Kai_Type_Info* type)
{
    switch (type->id)
    {
        break; case KAI_TYPE_ID_TYPE:
        {
            Kai_Type* t = (Kai_Type*)(data);
            kai__set_color(KAI_WRITE_COLOR_TYPE);
            kai_write_type(writer, *t);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        }
        break; case KAI_TYPE_ID_VOID:
        kai__write("nothing");
        break; case KAI_TYPE_ID_BOOLEAN:
        {
            Kai_bool value = *((Kai_bool*)(data));
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            if (value)
                kai__write("true");
            else
                kai__write("false");
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        }
        break; case KAI_TYPE_ID_INTEGER:
        {
            union { Kai_u64 u; Kai_s64 s; } value = {0};
            Kai_Type_Info_Integer* i = (Kai_Type_Info_Integer*)(type);
            switch (i->bits)
            {
                break; case 8:
                value.u = *((Kai_u8*)(data));
                break; case 16:
                value.u = *((Kai_u16*)(data));
                break; case 32:
                value.u = *((Kai_u32*)(data));
                break; case 64:
                value.u = *((Kai_u64*)(data));
            }
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            if (i->is_signed)
                kai__write_s64(value.s);
            else
                kai__write_u64(value.u);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        }
        break; case KAI_TYPE_ID_FLOAT:
        {
            Kai_Type_Info_Float* f = (Kai_Type_Info_Float*)(type);
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            switch (f->bits)
            {
                break; case 32:
                kai__write_f64((Kai_f64)(*((Kai_f32*)(data))));
                break; case 64:
                kai__write_f64(*((Kai_f64*)(data)));
            }
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        }
        break; case KAI_TYPE_ID_POINTER:
        case KAI_TYPE_ID_PROCEDURE:
        {
            Kai_uint value = *((Kai_uint*)(data));
            Kai_Write_Format fmt = ((Kai_Write_Format){.fill_character = 48, .min_count = 2*sizeof(Kai_uint), .flags = KAI_WRITE_FLAGS_HEXIDECIMAL});
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai__write("0x");
            writer->write_value(writer->user, KAI_U64, ((Kai_Value){.u64 = (Kai_u64)(value)}), fmt);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        }
        break; case KAI_TYPE_ID_ARRAY:
        {
            kai__write("[todo array]");
        }
        break; case KAI_TYPE_ID_STRING:
        {
            Kai_string value = *((Kai_string*)(data));
            kai__write("\"");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai__write_string(value);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write("\"");
        }
        break; case KAI_TYPE_ID_STRUCT:
        {
            Kai_Type_Info_Struct* s = (Kai_Type_Info_Struct*)(type);
            kai__write("{");
            for (Kai_u32 i = 0; i < (s->fields).count; ++i)
            {
                Kai_Struct_Field field = ((s->fields).data)[i];
                kai__write_string(field.name);
                kai__write(" = ");
                kai_write_value(writer, data+field.offset, field.type);
                if (i!=(s->fields).count-1)
                {
                    kai__write(", ");
                }
            }
            kai__write("}");
        }
        break; case KAI_TYPE_ID_NUMBER:
        default:
        {
            kai__write("[todo]");
        }
    }
}

KAI_INTERNAL void kai__tree_traversal_push(Kai__Tree_Traversal_Context* context, Kai_bool is_last)
{
    if (is_last)
        (context->stack)[context->stack_count/64] |= ((Kai_u64)(1))<<(context->stack_count%64);
    else
        (context->stack)[context->stack_count/64] &= ~(((Kai_u64)(1))<<(context->stack_count%64));
    context->stack_count += 1;
}

KAI_INTERNAL void kai__tree_traversal_pop(Kai__Tree_Traversal_Context* context)
{
    context->stack_count -= 1;
}

static Kai_string kai__tree_branches[4] = {
    KAI_CONST_STRING("\u2503   "), KAI_CONST_STRING("\u2523\u2501\u2501 "), KAI_CONST_STRING("    "), 
    KAI_CONST_STRING("\u2517\u2501\u2501 ")
};

KAI_INTERNAL void kai__write_expr_id_with_name(Kai_Writer* writer, Kai_string id, Kai_Expr* expr)
{
    kai__set_color(KAI_WRITE_COLOR_SECONDARY);
    kai__write_string(id);
    kai__set_color(KAI_WRITE_COLOR_PRIMARY);
    if ((expr->name).count!=0)
    {
        kai__write(" (name = \"");
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
        kai__write_string(expr->name);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write("\")");
    }
    if (expr->this_type!=NULL)
    {
        kai__write(" [");
        kai__set_color(KAI_WRITE_COLOR_TYPE);
        kai_write_type(writer, expr->this_type);
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        kai__write("]");
    }
}

KAI_INTERNAL void kai__write_unary_operator_name(Kai_Writer* writer, Kai_u32 op)
{
    switch (op)
    {
        break; case 45:
        kai__write("negate");
        break; case 42:
        kai__write("pointer to");
        break; case 91:
        kai__write("dereference");
        break; case 46:
        kai__write("inference");
        break; case 33:
        kai__write("not");
        break; case 126:
        kai__write("bitwise not");
        break; case 15917:
        kai__write("auto cast");
        break; default:
        kai__write("[unknown]");
    }
}

KAI_INTERNAL void kai__write_binary_operator_name(Kai_Writer* writer, Kai_u32 op)
{
    switch (op)
    {
        break; case 15917:
        kai__write("cast");
        break; case 9766:
        kai__write("and");
        break; case 31868:
        kai__write("or");
        break; case 15677:
        kai__write("equals");
        break; case 15649:
        kai__write("not equals");
        break; case 15676:
        kai__write("less or equal");
        break; case 15678:
        kai__write("greater or equal");
        break; case 15420:
        kai__write("shift left");
        break; case 15934:
        kai__write("shift right");
        break; case 60:
        kai__write("less than");
        break; case 62:
        kai__write("greater than");
        break; case 43:
        kai__write("add");
        break; case 45:
        kai__write("subtract");
        break; case 42:
        kai__write("multiply");
        break; case 47:
        kai__write("divide");
        break; case 37:
        kai__write("mod");
        break; case 94:
        kai__write("xor");
        break; case 38:
        kai__write("bitwise and");
        break; case 124:
        kai__write("bitwise or");
        break; case 46:
        kai__write("member access");
        break; case 91:
        kai__write("index");
        break; default:
        kai__write("[unknown]");
    }
}

KAI_INTERNAL void kai__write_assignment_operator_name(Kai_Writer* writer, Kai_u32 op)
{
    switch (op)
    {
        break; case 15659:
        kai__write("+=");
        break; case 15661:
        kai__write("-=");
        break; case 15658:
        kai__write("*=");
        break; case 15663:
        kai__write("/=");
        break; default:
        kai__write("[unknown]");
    }
}

KAI_INTERNAL void kai__write_tree_branches(Kai__Tree_Traversal_Context* context)
{
    Kai_Writer* writer = context->writer;
    kai__set_color(KAI_WRITE_COLOR_DECORATION);
    Kai_u32 last = context->stack_count-1;
    for (Kai_u32 i = 0; i < context->stack_count; ++i)
    {
        Kai_u64 bit = ((context->stack)[i/64])>>(i%64);
        Kai_u32 a = bit&1;
        Kai_u32 b = i==last;
        kai__write_string(kai__tree_branches[a<<1|b]);
    }
    if ((context->prefix).count>0)
    {
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT_2);
        kai__write_string(context->prefix);
        kai__write(" ");
        context->prefix = ((Kai_string){0});
    }
}

KAI_INTERNAL void kai__write_tree(Kai__Tree_Traversal_Context* context, Kai_Expr* expr)
{
    Kai_Writer* writer = context->writer;
    kai__write_tree_branches(context);
    if (expr==NULL)
    {
        kai__set_color(KAI_WRITE_COLOR_IMPORTANT_2);
        kai__write("null\n");
        kai__set_color(KAI_WRITE_COLOR_PRIMARY);
        return;
    }
    Kai_bool has_tag = expr->tag!=NULL;
    switch (expr->id)
    {
        break; case KAI_EXPR_IDENTIFIER:
        {
            kai__write_expr_id_with_name(writer, KAI_STRING("identifier"), expr);
            kai__write(" \"");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai__write_string(expr->source_code);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write("\"\n");
        }
        break; case KAI_EXPR_STRING:
        {
            Kai_Expr_String* s = (Kai_Expr_String*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("string"), expr);
            kai__write(" ");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai__write_string(s->source_code);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write("\n");
        }
        break; case KAI_EXPR_NUMBER:
        {
            Kai_Expr_Number* n = (Kai_Expr_Number*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("number"), expr);
            kai__write(" ");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai_write_number(writer, n->value);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write("\n");
        }
        break; case KAI_EXPR_LITERAL:
        {
            Kai_Expr_Literal* l = (Kai_Expr_Literal*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("literal"), expr);
            kai__write("\n");
            Kai_Stmt* current = l->head;
            while (current)
            {
                kai__explore(current, current->next==NULL);
                current = current->next;
            }
        }
        break; case KAI_EXPR_UNARY:
        {
            Kai_Expr_Unary* u = (Kai_Expr_Unary*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("unary"), expr);
            kai__write(" (op = ");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai__write_unary_operator_name(writer, u->op);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write(")\n");
            kai__explore(u->expr, KAI_TRUE);
        }
        break; case KAI_EXPR_BINARY:
        {
            Kai_Expr_Binary* b = (Kai_Expr_Binary*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("binary"), expr);
            kai__write(" (op = ");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai__write_binary_operator_name(writer, b->op);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write(")\n");
            kai__explore(b->left, b->right==NULL);
            if (b->right!=NULL)
                kai__explore(b->right, KAI_TRUE);
        }
        break; case KAI_EXPR_ARRAY:
        {
            Kai_Expr_Array* a = (Kai_Expr_Array*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("array"), expr);
            if (a->flags&KAI_FLAG_ARRAY_DYNAMIC)
                kai__write(" DYNAMIC");
            kai__write("\n");
            if (a->rows!=NULL)
            {
                context->prefix = KAI_STRING("rows");
                kai__explore(a->rows, KAI_FALSE);
                if (a->cols!=NULL)
                {
                    context->prefix = KAI_STRING("cols");
                    kai__explore(a->cols, KAI_FALSE);
                }
            }
            kai__explore(a->expr, KAI_TRUE);
        }
        break; case KAI_EXPR_SPECIAL:
        {
            Kai_Expr_Special* s = (Kai_Expr_Special*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("special"), expr);
            kai__write(" \"");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            switch (s->kind)
            {
                break; case KAI_SPECIAL_EVAL_TYPE:
                kai__write("type");
                break; case KAI_SPECIAL_EVAL_SIZE:
                kai__write("size");
                break; case KAI_SPECIAL_TYPE:
                kai__write("Type");
                break; case KAI_SPECIAL_NUMBER:
                kai__write("Number");
                break; case KAI_SPECIAL_CODE:
                kai__write("Code");
            }
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write("\"\n");
        }
        break; case KAI_EXPR_PROCEDURE_TYPE:
        {
            Kai_Expr_Procedure_Type* p = (Kai_Expr_Procedure_Type*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("procedure type"), expr);
            kai__write("\n");
            Kai_u32 end = (p->in_count+p->out_count)-1;
            Kai_Expr* current = p->in_out_expr;
            if (p->in_count)
                for (Kai_u32 i = 0; i < p->in_count; ++i)
                {
                    context->prefix = KAI_STRING("in ");
                    kai__explore(current, i==end);
                    current = current->next;
                }
            if (p->out_count)
                for (Kai_u32 i = 0; i < p->out_count; ++i)
                {
                    context->prefix = KAI_STRING("out");
                    Kai_u32 idx = i+p->in_count;
                    kai__explore(current, idx==end);
                    current = current->next;
                }
        }
        break; case KAI_EXPR_PROCEDURE_CALL:
        {
            Kai_Expr_Procedure_Call* p = (Kai_Expr_Procedure_Call*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("procedure call"), expr);
            kai__write("\n");
            context->prefix = KAI_STRING("proc");
            kai__explore(p->proc, p->arg_count==0);
            Kai_Expr* current = p->arg_head;
            while (current!=NULL)
            {
                kai__explore(current, current->next==NULL);
                current = current->next;
            }
        }
        break; case KAI_EXPR_PROCEDURE:
        {
            Kai_Expr_Procedure* p = (Kai_Expr_Procedure*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("procedure"), expr);
            kai__write("\n");
            Kai_Expr* current = p->in_out_expr;
            if (p->in_count)
                for (Kai_u32 i = 0; i < p->in_count; ++i)
                {
                    context->prefix = KAI_STRING("in ");
                    kai__explore(current, KAI_FALSE);
                    current = current->next;
                }
            if (p->out_count)
                for (Kai_u32 i = 0; i < p->out_count; ++i)
                {
                    context->prefix = KAI_STRING("out");
                    kai__explore(current, KAI_FALSE);
                    current = current->next;
                }
            kai__explore(p->body, KAI_TRUE);
        }
        break; case KAI_EXPR_IMPORT:
        {
            kai__write_expr_id_with_name(writer, KAI_STRING("import"), expr);
            kai__write("\n");
        }
        break; case KAI_EXPR_STRUCT:
        {
            Kai_Expr_Struct* s = (Kai_Expr_Struct*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("struct"), expr);
            kai__write("\n");
            Kai_Stmt* current = s->head;
            while (current)
            {
                kai__explore(current, current->next==NULL);
                current = current->next;
            }
        }
        break; case KAI_EXPR_ENUM:
        {
            Kai_Expr_Enum* e = (Kai_Expr_Enum*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("enum"), expr);
            kai__write("\n");
            Kai_Stmt* current = e->head;
            while (current)
            {
                kai__explore(current, current->next==NULL);
                current = current->next;
            }
        }
        break; case KAI_STMT_RETURN:
        {
            Kai_Stmt_Return* r = (Kai_Stmt_Return*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("return"), expr);
            kai__write("\n");
            kai__explore(r->expr, KAI_TRUE);
        }
        break; case KAI_STMT_DECLARATION:
        {
            Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("declaration"), expr);
            if (d->flags&KAI_FLAG_DECL_CONST)
                kai__write(" const");
            if (d->flags&KAI_FLAG_DECL_EXPORT)
            {
                kai__set_color(KAI_WRITE_COLOR_IMPORTANT_2);
                kai__write(" export");
                kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            }
            if (d->flags&KAI_FLAG_DECL_HOST_IMPORT)
            {
                kai__set_color(KAI_WRITE_COLOR_IMPORTANT_2);
                kai__write(" host_import");
                kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            }
            kai__write("\n");
            Kai_bool has_expr = d->expr!=NULL;
            if (d->type)
            {
                context->prefix = KAI_STRING("type");
                kai__explore(d->type, !has_expr);
            }
            if (has_expr)
                kai__explore(d->expr, !has_tag);
        }
        break; case KAI_STMT_ASSIGNMENT:
        {
            Kai_Stmt_Assignment* a = (Kai_Stmt_Assignment*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("assignment"), expr);
            if (a->op!=61)
            {
                kai__write(" (op = ");
                kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
                kai__write_assignment_operator_name(writer, a->op);
                kai__set_color(KAI_WRITE_COLOR_PRIMARY);
                kai__write(")");
            }
            kai__write("\n");
            context->prefix = KAI_STRING("left ");
            kai__explore(a->left, KAI_FALSE);
            context->prefix = KAI_STRING("right");
            kai__explore(a->expr, KAI_TRUE);
        }
        break; case KAI_STMT_COMPOUND:
        {
            Kai_Stmt_Compound* c = (Kai_Stmt_Compound*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("compound statement"), expr);
            kai__write("\n");
            Kai_Stmt* current = c->head;
            while (current)
            {
                kai__explore(current, current->next==NULL);
                current = current->next;
            }
        }
        break; case KAI_STMT_IF:
        {
            Kai_Stmt_If* i = (Kai_Stmt_If*)(expr);
            Kai_string name = KAI_STRING("if");
            if (i->flags&KAI_FLAG_IF_CASE)
                name = KAI_STRING("if-case");
            kai__write_expr_id_with_name(writer, name, expr);
            kai__write("\n");
            context->prefix = KAI_STRING("expr");
            kai__explore(i->expr, KAI_FALSE);
            kai__explore(i->then_body, i->else_body==NULL);
            if (i->else_body!=NULL)
            {
                kai__explore(i->else_body, KAI_TRUE);
            }
        }
        break; case KAI_STMT_FOR:
        {
            Kai_Stmt_For* f = (Kai_Stmt_For*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("for"), expr);
            kai__write(" (iterator name = ");
            kai__set_color(KAI_WRITE_COLOR_IMPORTANT);
            kai__write_string(f->iterator_name);
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write(")\n");
            context->prefix = KAI_STRING("from");
            kai__explore(f->from, 0);
            context->prefix = KAI_STRING("to");
            kai__explore(f->to, 0);
            kai__explore(f->body, 1);
        }
        break; case KAI_STMT_WHILE:
        {
            Kai_Stmt_While* w = (Kai_Stmt_While*)(expr);
            kai__write_expr_id_with_name(writer, KAI_STRING("while"), expr);
            kai__write("\n");
            kai__explore(w->expr, KAI_FALSE);
            kai__explore(w->body, KAI_TRUE);
        }
        break; case KAI_STMT_CONTROL:
        {
            kai__write_expr_id_with_name(writer, KAI_STRING("control"), expr);
            kai__write(" TODO\n");
        }
        break; default:
        {
            kai__set_color(KAI_WRITE_COLOR_SECONDARY);
            kai__write("unknown");
            kai__set_color(KAI_WRITE_COLOR_PRIMARY);
            kai__write(" (id = ");
            kai__write_u32(expr->id);
            kai__write(")\n");
        }
    }
    if (has_tag)
    {
        kai__tree_traversal_push(context, KAI_TRUE);
        context->prefix = KAI_STRING("tag");
        kai__write_tree_branches(context);
        kai__set_color(KAI_WRITE_COLOR_SECONDARY);
        kai__write_string((expr->tag)->name);
        kai__write("\n");
        Kai_Expr* current = (expr->tag)->expr;
        while (current)
        {
            kai__explore(current, current->next==NULL);
            current = current->next;
        }
        kai__tree_traversal_pop(context);
    }
}

KAI_API(void) kai_write_expression(Kai_Writer* writer, Kai_Expr* expr, Kai_u32 depth)
{
    Kai__Tree_Traversal_Context context = {0};
    context.writer = writer;
    (context.stack)[0] = 1;
    context.stack_count = depth;
    kai__write_tree(&context, expr);
}

KAI_API(void) kai_write_syntax_tree(Kai_Writer* writer, Kai_Syntax_Tree* tree)
{
    (void)(writer);
    (void)(tree);
}

KAI_API(void) kai_write_number(Kai_Writer* writer, Kai_Number number)
{
    if (!kai_number_is_integer(number))
    {
        kai__write_f64(kai_number_to_f64(number));
        return;
    }
    if (number.is_neg)
    {
        kai__write("-");
    }
    if ((Kai_u32)(number.e)<=kai_intrinsics_clz64(number.n))
    {
        kai__write_u64(kai_number_to_u64(number));
        return;
    }
    kai__write_u64(number.n);
    kai__write(" * 2^");
    kai__write_s32(number.e);
}

KAI_API(void) kai_destroy_error(Kai_Error* error, Kai_Allocator* allocator)
{
    while (error)
    {
        Kai_Error* next = error->next;
        if ((error->memory).size>0)
            kai__free((error->memory).data, (error->memory).size);
        error = next;
    }
}

static Kai_u8 kai__keyword_map[32] = {
    8, 10, 1, 14, 4, 8, 8, 8, 13, 8, 12, 8, 0, 9, 8, 8, 3, 2, 8, 6, 5, 11, 7, 8, 8, 8, 
    8, 8, 8, 8, 8, 8
};

static Kai_string kai__keywords[15] = {
    KAI_CONST_STRING("break"), KAI_CONST_STRING("case"), KAI_CONST_STRING("cast"), KAI_CONST_STRING("continue"), 
    KAI_CONST_STRING("defer"), KAI_CONST_STRING("else"), KAI_CONST_STRING("enum"), KAI_CONST_STRING("for"), 
    KAI_CONST_STRING("if"), KAI_CONST_STRING("loop"), KAI_CONST_STRING("ret"), KAI_CONST_STRING("struct"), 
    KAI_CONST_STRING("union"), KAI_CONST_STRING("using"), KAI_CONST_STRING("while")
};

KAI_INTERNAL Kai_u32 kai__hash_keyword(Kai_string s)
{
    Kai_u32 h = 0;
    Kai_u32 i = 0;
    while (i<8&&i<s.count)
    {
        h = (((h<<4)+h)+(s.data)[i])+97;
        i += 1;
    }
    return h%27;
}

KAI_API(void) kai_write_token(Kai_Writer* writer, Kai_Token token)
{
    Kai_bool symbol = KAI_FALSE;
    switch (token.id)
    {
        break; case KAI_TOKEN_IDENTIFIER:
        kai__write("Ident");
        break; case KAI_TOKEN_STRING:
        kai__write("String");
        break; case KAI_TOKEN_NUMBER:
        kai__write("Number");
        break; case KAI_TOKEN_DIRECTIVE:
        kai__write("Dir");
        break; case KAI_TOKEN_TAG:
        kai__write("Tag");
        break; default:
        symbol = KAI_TRUE;
    }
    if (!symbol)
        kai__write("(");
    if (token.id>256)
    {
        Kai_u32 id = token.id;
        Kai_u8 ch = {0};
        Kai_string str = ((Kai_string){.count = 1, .data = &ch});
        while (id!=0)
        {
            ch = id&255;
            kai__write_string(str);
            id = id>>8;
        }
    }
    else
        kai__write_string(token.string);
    if (!symbol)
        kai__write(")");
}

KAI_API(Kai_string) kai_token_string(Kai_Token_Id id, Kai_string dst)
{
    kai_assert(dst.count>=12);
    switch (id)
    {
        break; case KAI_TOKEN_END:
        return kai_string_copy_from_c(dst, "end of file");
        break; case KAI_TOKEN_IDENTIFIER:
        return kai_string_copy_from_c(dst, "identifier");
        break; case KAI_TOKEN_NUMBER:
        return kai_string_copy_from_c(dst, "number");
        break; case KAI_TOKEN_DIRECTIVE:
        return kai_string_copy_from_c(dst, "directive");
        break; case KAI_TOKEN_TAG:
        return kai_string_copy_from_c(dst, "tag");
        break; case KAI_TOKEN_STRING:
        return kai_string_copy_from_c(dst, "string");
        break; case KAI_TOKEN_break:
        return kai_string_copy_from_c(dst, "'break'");
        break; case KAI_TOKEN_case:
        return kai_string_copy_from_c(dst, "'case'");
        break; case KAI_TOKEN_cast:
        return kai_string_copy_from_c(dst, "'cast'");
        break; case KAI_TOKEN_continue:
        return kai_string_copy_from_c(dst, "'continue'");
        break; case KAI_TOKEN_defer:
        return kai_string_copy_from_c(dst, "'defer'");
        break; case KAI_TOKEN_else:
        return kai_string_copy_from_c(dst, "'else'");
        break; case KAI_TOKEN_enum:
        return kai_string_copy_from_c(dst, "'enum'");
        break; case KAI_TOKEN_for:
        return kai_string_copy_from_c(dst, "'for'");
        break; case KAI_TOKEN_if:
        return kai_string_copy_from_c(dst, "'if'");
        break; case KAI_TOKEN_loop:
        return kai_string_copy_from_c(dst, "'loop'");
        break; case KAI_TOKEN_ret:
        return kai_string_copy_from_c(dst, "'ret'");
        break; case KAI_TOKEN_struct:
        return kai_string_copy_from_c(dst, "'struct'");
        break; case KAI_TOKEN_union:
        return kai_string_copy_from_c(dst, "'union'");
        break; case KAI_TOKEN_using:
        return kai_string_copy_from_c(dst, "'using'");
        break; case KAI_TOKEN_while:
        return kai_string_copy_from_c(dst, "'while'");
        break; default:
        (dst.data)[0] = 39;
        Kai_u32 i = 0;
        while (i<=3)
        {
            Kai_u8 ch = (Kai_u8)(id>>(i*8));
            if (ch==0)
                break;
            (dst.data)[i+1] = ch;
            i += 1;
        }
        (dst.data)[i+1] = 39;
        return ((Kai_string){.count = i+2, .data = dst.data});
    }
}

KAI_INTERNAL Kai_Number kai__parse_fractional_part(Kai_string source, Kai_u32* offset, Kai_Number start)
{
    if ((*offset<source.count&&(source.data)[*offset]==46)&&!(*offset+1<source.count&&(source.data)[*offset+1]==46))
    {
        *offset += 1;
        Kai_Number decimal = kai_number_parse_decimal(source, offset);
        start = kai_number_add(start, decimal);
    }
    if (*offset<source.count&&((source.data)[*offset]==101||(source.data)[*offset]==69))
    {
        Kai_bool is_neg = KAI_FALSE;
        *offset += 1;
        if (*offset<source.count)
        {
            if ((source.data)[*offset]==45)
            {
                *offset += 1;
                is_neg = KAI_TRUE;
            }
            if ((source.data)[*offset]==43)
            {
                *offset += 1;
            }
        }
        Kai_Number exponent = kai_number_parse_exponent(source, offset);
        if (is_neg)
            exponent = kai_number_inv(exponent);
        start = kai_number_mul(start, exponent);
    }
    return start;
}

KAI_INTERNAL Kai_bool kai__make_multi_token(Kai_Tokenizer* context, Kai_Token* t, Kai_u8 current)
{
    if (context->cursor>=(context->source).count)
        return KAI_FALSE;
    switch (current)
    {
        break; case 38:
        {
            if (((context->source).data)[context->cursor]==38)
            {
                t->id = 9766;
                return KAI_TRUE;
            }
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15654;
                return KAI_TRUE;
            }
        }
        break; case 124:
        {
            if (((context->source).data)[context->cursor]==124)
            {
                t->id = 31868;
                return KAI_TRUE;
            }
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15740;
                return KAI_TRUE;
            }
        }
        break; case 61:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15677;
                return KAI_TRUE;
            }
            if (((context->source).data)[context->cursor]==62)
            {
                t->id = 15933;
                return KAI_TRUE;
            }
        }
        break; case 62:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15678;
                return KAI_TRUE;
            }
            if (((context->source).data)[context->cursor]==62)
            {
                t->id = 15934;
                return KAI_TRUE;
            }
        }
        break; case 60:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15676;
                return KAI_TRUE;
            }
            if (((context->source).data)[context->cursor]==60)
            {
                t->id = 15420;
                return KAI_TRUE;
            }
        }
        break; case 33:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15649;
                return KAI_TRUE;
            }
        }
        break; case 43:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15659;
                return KAI_TRUE;
            }
        }
        break; case 45:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15661;
                return KAI_TRUE;
            }
            if (((context->source).data)[context->cursor]==62)
            {
                t->id = 15917;
                return KAI_TRUE;
            }
            if ((context->cursor+1<(context->source).count&&((context->source).data)[context->cursor]==45)&&((context->source).data)[context->cursor+1]==45)
            {
                t->id = 2960685;
                context->cursor += 1;
                (t->string).count += 1;
                return KAI_TRUE;
            }
        }
        break; case 42:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15658;
                return KAI_TRUE;
            }
        }
        break; case 47:
        {
            if (((context->source).data)[context->cursor]==61)
            {
                t->id = 15663;
                return KAI_TRUE;
            }
        }
    }
    return KAI_FALSE;
}

static Kai_u8 kai__token_lookup_table[128] = {
    KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, 
    KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, 
    KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, KAI__W, 
    KAI__T, KAI__S, KAI__D, KAI__T, KAI__T, KAI__T, KAI__T, KAI__T, KAI__T, KAI__T, KAI__T, 
    KAI__T, KAI__T, KAI__Z, KAI__C, KAI__N, KAI__N, KAI__N, KAI__N, KAI__N, KAI__N, KAI__N, 
    KAI__N, KAI__N, KAI__N, KAI__T, KAI__T, KAI__T, KAI__T, KAI__T, KAI__T, KAI__G, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KAI__T, 
    KAI__T, KAI__T, KAI__T, 0, KAI__T, 0, KAI__K, KAI__K, KAI__K, KAI__K, KAI__K, 0, 
    0, KAI__K, 0, 0, KAI__K, 0, 0, 0, 0, 0, KAI__K, KAI__K, 0, KAI__K, 0, KAI__K, 0, 
    0, 0, KAI__T, KAI__T, KAI__T, KAI__T, KAI__W
};

KAI_INTERNAL void kai__tokenizer_advance_to_identifier_end(Kai_Tokenizer* context)
{
    while (context->cursor<(context->source).count)
    {
        Kai_u8 c = ((context->source).data)[context->cursor];
        if (c<128&&kai__token_lookup_table[c]&1)
            break;
        context->cursor += 1;
    }
}

KAI_API(Kai_Token) kai_tokenizer_generate(Kai_Tokenizer* context)
{
    Kai_Token token = ((Kai_Token){.id = KAI_TOKEN_END, .line_number = context->line_number});
    while (context->cursor<(context->source).count)
    {
        (token.string).data = (context->source).data+context->cursor;
        Kai_u8 ch = ((token.string).data)[0];
        Kai_u32 where = {0};
        if (!(ch&128))
            where = kai__token_lookup_table[ch];
        switch (where)
        {
            break; case KAI__W:
            {
                if (ch==13)
                {
                    if (kai__next_character_equals(10))
                        context->cursor += 1;
                    context->line_number += 1;
                }
                else
                if (ch==10)
                {
                    context->line_number += 1;
                }
                token.line_number = context->line_number;
                context->cursor += 1;
            }
            break; case KAI__N:
            {
                token.id = KAI_TOKEN_NUMBER;
                Kai_u32 start = context->cursor;
                if (ch==48)
                {
                    context->cursor += 1;
                    if (context->cursor<(context->source).count)
                    {
                        if (((context->source).data)[context->cursor]==98)
                        {
                            context->cursor += 1;
                            (token.value).number = kai_number_parse_whole(context->source, &context->cursor, 2);
                            (token.string).count = context->cursor-start;
                            return token;
                        }
                        if (((context->source).data)[context->cursor]==120)
                        {
                            context->cursor += 1;
                            (token.value).number = kai_number_parse_whole(context->source, &context->cursor, 16);
                            (token.string).count = context->cursor-start;
                            return token;
                        }
                    }
                }
                (token.value).number = kai_number_parse_whole(context->source, &context->cursor, 10);
                (token.value).number = kai__parse_fractional_part(context->source, &context->cursor, (token.value).number);
                (token.string).count = context->cursor-start;
                return token;
            }
            break; case 0:
            {
                token.id = KAI_TOKEN_IDENTIFIER;
                Kai_u32 start = context->cursor;
                context->cursor += 1;
                kai__tokenizer_advance_to_identifier_end(context);
                (token.string).count = context->cursor-start;
                return token;
            }
            break; case KAI__D:
            {
                token.id = KAI_TOKEN_DIRECTIVE;
                Kai_u32 start = context->cursor;
                context->cursor += 1;
                kai__tokenizer_advance_to_identifier_end(context);
                (token.string).count = context->cursor-start;
                (token.value).string = token.string;
                ((token.value).string).count -= 1;
                ((token.value).string).data += 1;
                return token;
            }
            break; case KAI__G:
            {
                token.id = KAI_TOKEN_TAG;
                Kai_u32 start = context->cursor;
                context->cursor += 1;
                kai__tokenizer_advance_to_identifier_end(context);
                (token.string).count = context->cursor-start;
                (token.value).string = token.string;
                ((token.value).string).count -= 1;
                ((token.value).string).data += 1;
                return token;
            }
            break; case KAI__K:
            {
                token.id = KAI_TOKEN_IDENTIFIER;
                Kai_u32 start = context->cursor;
                context->cursor += 1;
                while (context->cursor<(context->source).count)
                {
                    Kai_u8 c = ((context->source).data)[context->cursor];
                    if (c<128&&kai__token_lookup_table[c]&1)
                        break;
                    context->cursor += 1;
                }
                (token.string).count = context->cursor-start;
                Kai_u32 hash = kai__hash_keyword(token.string);
                Kai_u8 keyword_index = kai__keyword_map[hash];
                if (kai_string_equals(kai__keywords[keyword_index], token.string))
                {
                    token.id = 128|keyword_index;
                    return token;
                }
                return token;
            }
            break; case KAI__S:
            {
                token.id = KAI_TOKEN_STRING;
                (token.string).count = 1;
                context->cursor += 1;
                Kai_u32 count = 0;
                (token.value).string = ((Kai_string){0});
                while (context->cursor<(context->source).count)
                {
                    if (((context->source).data)[context->cursor]==34)
                    {
                        break;
                    }
                    Kai_u8 m = ((context->source).data)[context->cursor];
                    if (m==92)
                    {
                        context->cursor += 1;
                        (token.string).count += 1;
                        if (context->cursor>=(context->source).count)
                            break;
                        m = ((context->source).data)[context->cursor];
                        switch (m)
                        {
                            break; case 92:
                            m = 92;
                            break; case 34:
                            m = 34;
                            break; case 116:
                            m = 9;
                            break; case 114:
                            m = 13;
                            break; case 110:
                            m = 10;
                            break; case 101:
                            m = 27;
                        }
                    }
                    Kai_u8* data = (Kai_u8*)(kai_fixed_allocate(&context->string_arena, 1));
                    kai_assert(data!=NULL);
                    *data = m;
                    if (((token.value).string).data==NULL)
                        ((token.value).string).data = data;
                    count += 1;
                    context->cursor += 1;
                }
                context->cursor += 1;
                (token.string).count += count+1;
                ((token.value).string).count = count;
                return token;
            }
            break; case KAI__C:
            {
                (token.string).data = (context->source).data+context->cursor;
                context->cursor += 1;
                if (context->cursor<(context->source).count&&((context->source).data)[context->cursor]==47)
                {
                    while (context->cursor<(context->source).count)
                    {
                        if (((context->source).data)[context->cursor]==13||((context->source).data)[context->cursor]==10)
                            break;
                        context->cursor += 1;
                    }
                    break;
                }
                if (((context->source).data)[context->cursor]==42)
                {
                    context->cursor += 1;
                    Kai_u32 depth = 1;
                    while (depth>0&&context->cursor<(context->source).count)
                    {
                        if ((((context->source).data)[context->cursor]==47&&context->cursor+1<(context->source).count)&&((context->source).data)[context->cursor+1]==42)
                        {
                            context->cursor += 2;
                            depth += 1;
                            continue;
                        }
                        if ((((context->source).data)[context->cursor]==42&&context->cursor+1<(context->source).count)&&((context->source).data)[context->cursor+1]==47)
                        {
                            context->cursor += 2;
                            depth -= 1;
                            continue;
                        }
                        if (((context->source).data)[context->cursor]==13)
                        {
                            if (kai__next_character_equals(10))
                                context->cursor += 1;
                            context->line_number += 1;
                        }
                        else
                        if (((context->source).data)[context->cursor]==10)
                        {
                            context->line_number += 1;
                        }
                        context->cursor += 1;
                    }
                    break;
                }
                token.id = 47;
                (token.string).count = 1;
                return token;
            }
            break; case KAI__T:
            {
                token.id = (Kai_u32)(ch);
                (token.string).count = 1;
                context->cursor += 1;
                if (kai__make_multi_token(context, &token, ch))
                {
                    context->cursor += 1;
                    (token.string).count += 1;
                }
                return token;
            }
            break; case KAI__Z:
            {
                if (kai__next_character_equals(46))
                {
                    context->cursor += 2;
                    (token.string).count = 2;
                    token.id = 11822;
                    return token;
                }
                if ((context->cursor+1<(context->source).count&&((context->source).data)[context->cursor+1]>=48)&&((context->source).data)[context->cursor+1]<=57)
                {
                    token.id = KAI_TOKEN_NUMBER;
                    Kai_u32 start = context->cursor;
                    (token.value).number = kai__parse_fractional_part(context->source, &context->cursor, (token.value).number);
                    (token.string).count = context->cursor-start;
                    return token;
                }
                context->cursor += 1;
                (token.string).count = 1;
                token.id = (Kai_u32)(ch);
                return token;
            }
        }
    }
    return token;
}

KAI_API(Kai_Token*) kai_tokenizer_next(Kai_Tokenizer* context)
{
    if (!context->peeking)
    {
        context->current_token = kai_tokenizer_generate(context);
        return &context->current_token;
    }
    context->peeking = KAI_FALSE;
    context->current_token = context->peeked_token;
    return &context->current_token;
}

KAI_API(Kai_Token*) kai_tokenizer_peek(Kai_Tokenizer* context)
{
    if (context->peeking)
        return &context->peeked_token;
    context->peeking = KAI_TRUE;
    context->peeked_token = kai_tokenizer_generate(context);
    return &context->peeked_token;
}

KAI_INTERNAL Kai_Expr* kai__error_unexpected(Kai_Parser* parser, Kai_Token* token, Kai_string where, Kai_string wanted)
{
    if ((parser->error)->result!=KAI_SUCCESS)
        return NULL;
    Kai_Buffer buffer = ((Kai_Buffer){.allocator = (parser->arena).base});
    Kai_u8 temp[32] = {0};
    Kai_string temp_string = ((Kai_string){.count = sizeof(temp), .data = temp});
    kai__buffer_append_string(&buffer, KAI_STRING("unexpected "));
    kai__buffer_append_string(&buffer, kai_token_string(token->id, temp_string));
    kai__buffer_append_string(&buffer, KAI_STRING(" "));
    kai__buffer_append_string(&buffer, where);
    Kai_Range message = kai__buffer_end(&buffer);
    Kai_Memory memory = kai__buffer_done(&buffer);
    *(parser->error) = ((Kai_Error){.result = KAI_ERROR_SYNTAX, .location = ((Kai_Location){.string = token->string, .line = token->line_number}), .message = kai__range_to_string(message, memory), .context = wanted, .memory = memory});
    return NULL;
}

KAI_INTERNAL Kai__Operator kai__operator_info(Kai_u32 op)
{
    switch (op)
    {
        break; case 31868:
        return ((Kai__Operator){16, KAI__OPERATOR_TYPE_BINARY});
        break; case 9766:
        return ((Kai__Operator){17, KAI__OPERATOR_TYPE_BINARY});
        break; case 124:
        return ((Kai__Operator){32, KAI__OPERATOR_TYPE_BINARY});
        break; case 94:
        return ((Kai__Operator){33, KAI__OPERATOR_TYPE_BINARY});
        break; case 38:
        return ((Kai__Operator){34, KAI__OPERATOR_TYPE_BINARY});
        break; case 15676:
        return ((Kai__Operator){64, KAI__OPERATOR_TYPE_BINARY});
        break; case 15678:
        return ((Kai__Operator){64, KAI__OPERATOR_TYPE_BINARY});
        break; case 60:
        return ((Kai__Operator){64, KAI__OPERATOR_TYPE_BINARY});
        break; case 62:
        return ((Kai__Operator){64, KAI__OPERATOR_TYPE_BINARY});
        break; case 15677:
        return ((Kai__Operator){80, KAI__OPERATOR_TYPE_BINARY});
        break; case 15649:
        return ((Kai__Operator){80, KAI__OPERATOR_TYPE_BINARY});
        break; case 43:
        return ((Kai__Operator){256, KAI__OPERATOR_TYPE_BINARY});
        break; case 45:
        return ((Kai__Operator){256, KAI__OPERATOR_TYPE_BINARY});
        break; case 42:
        return ((Kai__Operator){512, KAI__OPERATOR_TYPE_BINARY});
        break; case 47:
        return ((Kai__Operator){512, KAI__OPERATOR_TYPE_BINARY});
        break; case 37:
        return ((Kai__Operator){512, KAI__OPERATOR_TYPE_BINARY});
        break; case 15420:
        return ((Kai__Operator){512, KAI__OPERATOR_TYPE_BINARY});
        break; case 15934:
        return ((Kai__Operator){512, KAI__OPERATOR_TYPE_BINARY});
        break; case 46:
        return ((Kai__Operator){65535, KAI__OPERATOR_TYPE_BINARY});
        break; case 15917:
        return ((Kai__Operator){KAI__PREC_CAST, KAI__OPERATOR_TYPE_BINARY});
        break; case 91:
        return ((Kai__Operator){48879, KAI__OPERATOR_TYPE_INDEX});
        break; case 40:
        return ((Kai__Operator){48879, KAI__OPERATOR_TYPE_PROCEDURE_CALL});
        break; default:
        return ((Kai__Operator){0});
    }
}

KAI_INTERNAL Kai_Expr* kai__parser_create_identifier(Kai_Parser* parser, Kai_Token token)
{
    Kai_Expr* node = (Kai_Expr*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr)));
    node->id = KAI_EXPR_IDENTIFIER;
    node->source_code = token.string;
    node->line_number = token.line_number;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_string(Kai_Parser* parser, Kai_Token token)
{
    Kai_Expr_String* node = (Kai_Expr_String*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_String)));
    node->id = KAI_EXPR_STRING;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->value = (token.value).string;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_number(Kai_Parser* parser, Kai_Token token)
{
    Kai_Expr_Number* node = (Kai_Expr_Number*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Number)));
    node->id = KAI_EXPR_NUMBER;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->value = (token.value).number;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_literal(Kai_Parser* parser, Kai_Token token, Kai_Expr* head, Kai_u32 count)
{
    Kai_Expr_Literal* node = (Kai_Expr_Literal*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Literal)));
    node->id = KAI_EXPR_LITERAL;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->head = head;
    node->count = count;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_unary(Kai_Parser* parser, Kai_Token op_token, Kai_Expr* expr)
{
    Kai_Expr_Unary* node = (Kai_Expr_Unary*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Unary)));
    node->id = KAI_EXPR_UNARY;
    node->source_code = kai_merge_strings(op_token.string, expr->source_code);
    node->line_number = kai__min_u32(op_token.line_number, expr->line_number);
    node->op = op_token.id;
    node->expr = expr;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_binary(Kai_Parser* parser, Kai_Expr* left, Kai_Expr* right, Kai_u32 op)
{
    Kai_Expr_Binary* node = (Kai_Expr_Binary*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Binary)));
    node->id = KAI_EXPR_BINARY;
    node->source_code = kai_merge_strings(left->source_code, right->source_code);
    node->line_number = kai__min_u32(left->line_number, right->line_number);
    node->op = op;
    node->left = left;
    node->right = right;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_array(Kai_Parser* parser, Kai_Token op_token, Kai_Expr* expr, Kai_Expr* rows, Kai_Expr* cols, Kai_u8 flags)
{
    Kai_Expr_Array* node = (Kai_Expr_Array*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Array)));
    node->id = KAI_EXPR_ARRAY;
    node->source_code = kai_merge_strings(op_token.string, expr->source_code);
    node->line_number = kai__min_u32(op_token.line_number, expr->line_number);
    node->flags = flags;
    node->rows = rows;
    node->cols = cols;
    node->expr = expr;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_special(Kai_Parser* parser, Kai_Token token, Kai_u8 kind)
{
    Kai_Expr_Special* node = (Kai_Expr_Special*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Special)));
    node->id = KAI_EXPR_SPECIAL;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->kind = kind;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_procedure_type(Kai_Parser* parser, Kai_Expr* in_out, Kai_u8 in_count, Kai_u8 out_count)
{
    Kai_Expr_Procedure_Type* node = (Kai_Expr_Procedure_Type*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Procedure_Type)));
    node->id = KAI_EXPR_PROCEDURE_TYPE;
    node->source_code = in_out->source_code;
    node->line_number = in_out->line_number;
    node->in_out_expr = in_out;
    node->in_count = in_count;
    node->out_count = out_count;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_procedure_call(Kai_Parser* parser, Kai_Expr* proc, Kai_Expr* args, Kai_u8 arg_count)
{
    Kai_Expr_Procedure_Call* node = (Kai_Expr_Procedure_Call*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Procedure_Call)));
    node->id = KAI_EXPR_PROCEDURE_CALL;
    node->source_code = proc->source_code;
    node->line_number = proc->line_number;
    node->proc = proc;
    node->arg_head = args;
    node->arg_count = arg_count;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_procedure(Kai_Parser* parser, Kai_Token token, Kai_Expr* in_out, Kai_Stmt* body, Kai_u8 in_count, Kai_u8 out_count)
{
    Kai_Expr_Procedure* node = (Kai_Expr_Procedure*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Procedure)));
    node->id = KAI_EXPR_PROCEDURE;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->in_out_expr = in_out;
    node->in_count = in_count;
    node->out_count = out_count;
    node->body = body;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_import(Kai_Parser* parser, Kai_Token token, Kai_Token import)
{
    Kai_Expr* node = (Kai_Expr*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr)));
    node->id = KAI_EXPR_IMPORT;
    node->source_code = kai_merge_strings(token.string, import.string);
    node->line_number = token.line_number;
    node->name = (import.value).string;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_struct(Kai_Parser* parser, Kai_Token token, Kai_u32 field_count, Kai_Stmt* body)
{
    Kai_Expr_Struct* node = (Kai_Expr_Struct*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Struct)));
    node->id = KAI_EXPR_STRUCT;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->field_count = field_count;
    node->head = body;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_enum(Kai_Parser* parser, Kai_Token token, Kai_Expr* type, Kai_u32 field_count, Kai_Stmt* body)
{
    Kai_Expr_Enum* node = (Kai_Expr_Enum*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Expr_Enum)));
    node->id = KAI_EXPR_ENUM;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->type = type;
    node->field_count = field_count;
    node->head = body;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_return(Kai_Parser* parser, Kai_Token ret_token, Kai_Expr* expr)
{
    Kai_Stmt_Return* node = (Kai_Stmt_Return*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_Return)));
    node->id = KAI_STMT_RETURN;
    node->source_code = ret_token.string;
    node->line_number = ret_token.line_number;
    node->expr = expr;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_declaration(Kai_Parser* parser, Kai_string name, Kai_Expr* type, Kai_Expr* expr, Kai_u8 flags, Kai_u32 line_number)
{
    Kai_Stmt_Declaration* node = (Kai_Stmt_Declaration*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_Declaration)));
    node->id = KAI_STMT_DECLARATION;
    node->source_code = name;
    node->line_number = line_number;
    node->name = name;
    node->type = type;
    node->expr = expr;
    node->flags = flags;
    return kai_parse_tag_to_expr(parser, (Kai_Expr*)(node));
}

KAI_INTERNAL Kai_Expr* kai__parser_create_assignment(Kai_Parser* parser, Kai_u32 op, Kai_Expr* left, Kai_Expr* expr)
{
    Kai_Stmt_Assignment* node = (Kai_Stmt_Assignment*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_Assignment)));
    node->id = KAI_STMT_ASSIGNMENT;
    node->source_code = left->source_code;
    node->line_number = left->line_number;
    node->op = op;
    node->left = left;
    node->expr = expr;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_if(Kai_Parser* parser, Kai_Token if_token, Kai_u8 flags, Kai_Expr* expr, Kai_Stmt* then_body, Kai_Stmt* else_body)
{
    Kai_Stmt_If* node = (Kai_Stmt_If*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_If)));
    node->id = KAI_STMT_IF;
    node->source_code = if_token.string;
    node->line_number = if_token.line_number;
    node->flags = flags;
    node->expr = expr;
    node->then_body = then_body;
    node->else_body = else_body;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_while(Kai_Parser* parser, Kai_Token while_token, Kai_Expr* expr, Kai_Stmt* body)
{
    Kai_Stmt_While* node = (Kai_Stmt_While*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_While)));
    node->id = KAI_STMT_WHILE;
    node->source_code = while_token.string;
    node->line_number = while_token.line_number;
    node->body = body;
    node->expr = expr;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_for(Kai_Parser* parser, Kai_Token for_token, Kai_string name, Kai_Expr* from, Kai_Expr* to, Kai_Stmt* body, Kai_u8 flags)
{
    Kai_Stmt_For* node = (Kai_Stmt_For*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_For)));
    node->id = KAI_STMT_FOR;
    node->source_code = for_token.string;
    node->line_number = for_token.line_number;
    node->body = body;
    node->from = from;
    node->to = to;
    node->iterator_name = name;
    node->flags = flags;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_control(Kai_Parser* parser, Kai_Token token, Kai_u8 kind, Kai_Expr* expr)
{
    Kai_Stmt_Control* node = (Kai_Stmt_Control*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_Control)));
    node->id = KAI_STMT_CONTROL;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->kind = kind;
    node->expr = expr;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Expr* kai__parser_create_compound(Kai_Parser* parser, Kai_Token token, Kai_Stmt* body)
{
    Kai_Stmt_Compound* node = (Kai_Stmt_Compound*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Stmt_Compound)));
    node->id = KAI_STMT_COMPOUND;
    node->source_code = token.string;
    node->line_number = token.line_number;
    node->head = body;
    return (Kai_Expr*)(node);
}

KAI_INTERNAL Kai_Tag* kai__parser_create_tag(Kai_Parser* parser, Kai_Token token, Kai_Expr* expr)
{
    Kai_Tag* tag = (Kai_Tag*)(kai_arena_allocate(&parser->arena, sizeof(Kai_Tag)));
    tag->name = (token.value).string;
    tag->expr = expr;
    return tag;
}

KAI_API(Kai_Expr*) kai_parse_procedure_call_arguments(Kai_Parser* parser, Kai_u32* arg_count)
{
    Kai_Token* current = &(parser->tokenizer).current_token;
    Kai_Expr_List args = {0};
    while (current->id!=41)
    {
        Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
        kai__expect(expr, "in argument list", "an expression");
        kai__linked_list_append(args, expr);
        kai__expect(*arg_count<255, "in argument list", "too many arguments");
        *arg_count += 1;
        kai__next_token();
        if (current->id==44)
            kai__next_token();
        else
        if (current->id!=41)
            return kai__unexpected("in argument list", "',' or ')' expected here");
    }
    return args.head;
}

KAI_API(Kai_Expr*) kai_parse_tag_to_expr(Kai_Parser* parser, Kai_Expr* expr)
{
    Kai_Token* peeked = kai__peek_token();
    if (peeked->id==KAI_TOKEN_TAG)
    {
        Kai_Token token = *peeked;
        kai__next_token();
        Kai_Expr* tag_expr = 0;
        kai__peek_token();
        if (peeked->id==40)
        {
            kai__next_token();
            kai__next_token();
            Kai_u32 arg_count = {0};
            tag_expr = kai_parse_procedure_call_arguments(parser, &arg_count);
        }
        expr->tag = kai__parser_create_tag(parser, token, tag_expr);
    }
    return expr;
}

KAI_INTERNAL Kai_bool kai__is_procedure_next(Kai_Parser* parser)
{
    Kai_Token* peeked = kai__peek_token();
    if (peeked->id==41)
        return KAI_TRUE;
    Kai_Tokenizer state = parser->tokenizer;
    Kai_Token* current = kai__next_token();
    Kai_bool found = KAI_FALSE;
    while (current->id!=KAI_TOKEN_END&&current->id!=41)
    {
        if (current->id==58)
        {
            found = KAI_TRUE;
            break;
        }
        kai__next_token();
    }
    parser->tokenizer = state;
    return found;
}

KAI_API(Kai_Expr*) kai_parse_expression(Kai_Parser* parser, Kai_u32 flags)
{
    Kai_Expr* left = 0;
    Kai_Token* current = &(parser->tokenizer).current_token;
    switch (current->id)
    {
        break; case KAI_TOKEN_IDENTIFIER:
        left = kai__parser_create_identifier(parser, *current);
        break; case KAI_TOKEN_NUMBER:
        left = kai__parser_create_number(parser, *current);
        break; case KAI_TOKEN_STRING:
        left = kai__parser_create_string(parser, *current);
        break; case 40:
        {
            kai_tokenizer_next(&parser->tokenizer);
            left = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            kai__expect(left!=NULL, "in expression", "should be an expression here");
            kai_tokenizer_next(&parser->tokenizer);
            kai__expect(current->id==41, "in expression", "should be an operator or ')' here");
        }
        break; case 91:
        {
            Kai_Token* peeked = kai__peek_token();
            if (peeked->id==93||peeked->id==11822)
            {
                left = kai_parse_type_expression(parser);
            }
            else
            {
                Kai_Token op_token = *current;
                kai__next_token();
                left = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                kai__expect(left, "in dereference expression", "should be an expression here");
                kai__next_token();
                kai__expect(current->id==93, "in dereference expression", "should be an operator or ']' here");
                left = kai__parser_create_unary(parser, op_token, left);
            }
        }
        break; case 123:
        {
            Kai_Token token = *current;
            Kai_Expr_List body = {0};
            Kai_u32 count = {0};
            kai__next_token();
            while (current->id!=125)
            {
                Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                kai__expect(expr, "in literal expression", "should be an expression here");
                kai__next_token();
                if (current->id==44)
                    kai__next_token();
                else
                if (current->id==61)
                {
                    kai__next_token();
                    Kai_Expr* right = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                    kai__expect(right, "in literal expression", "should be an expression here");
                    kai__next_token();
                    if (current->id==44)
                        kai__next_token();
                    expr = kai__parser_create_assignment(parser, 61, expr, right);
                }
                else
                    kai__expect(current->id==125, "in literal expression", "'}' or ','");
                kai__linked_list_append(body, expr);
                count += 1;
            }
            kai__expect(current->id==125, "in literal expression", "should be a '}' here");
            return kai__parser_create_literal(parser, token, body.head, count);
        }
        break; case 46:
        case 126:
        case 33:
        case 45:
        case 43:
        case 42:
        {
            Kai_Token op_token = *current;
            kai__next_token();
            left = kai_parse_expression(parser, KAI__PREC_UNARY);
            kai__expect(left, "in unary expression", "should be an expression here");
            left = kai__parser_create_unary(parser, op_token, left);
        }
        break; case KAI_TOKEN_cast:
        {
            Kai_Token token = *current;
            token.id = 15917;
            kai__next_token();
            Kai_Expr* type = 0;
            if (current->id==40)
            {
                kai__expect(current->id==40, "in expression", "'(' after cast keyword");
                kai__next_token();
                type = kai_parse_type_expression(parser);
                kai__expect(type, "in expression", "type");
                kai__next_token();
                kai__expect(current->id==41, "in expression", "')' after Type in cast expression");
                kai__next_token();
            }
            Kai_Expr* expr = kai_parse_expression(parser, KAI__PREC_CAST);
            kai__expect(expr, "in expression", "should be expression or '(' here");
            if (type==NULL)
                left = kai__parser_create_unary(parser, token, expr);
            else
                left = kai__parser_create_binary(parser, expr, type, token.id);
        }
        break; case KAI_TOKEN_DIRECTIVE:
        {
            if (kai_string_equals((current->value).string, KAI_STRING("size")))
            {
                left = kai__parser_create_special(parser, *current, KAI_SPECIAL_EVAL_SIZE);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("type")))
            {
                left = kai__parser_create_special(parser, *current, KAI_SPECIAL_EVAL_TYPE);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("Type")))
            {
                return kai__parser_create_special(parser, *current, KAI_SPECIAL_TYPE);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("Number")))
            {
                return kai__parser_create_special(parser, *current, KAI_SPECIAL_NUMBER);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("Code")))
            {
                return kai__parser_create_special(parser, *current, KAI_SPECIAL_CODE);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("import")))
            {
                Kai_Token token = *current;
                kai__next_token();
                kai__expect(current->id==KAI_TOKEN_STRING, "in import", "string");
                left = kai__parser_create_import(parser, token, *current);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("through")))
            {
                left = kai__parser_create_control(parser, *current, KAI_CONTROL_THROUGH, NULL);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("char")))
            {
                kai__next_token();
                kai__expect(current->id==KAI_TOKEN_STRING, "char", "must be string");
                kai__expect(((current->value).string).count==1, "char", "string must be have length of 1");
                (current->value).number = ((Kai_Number){.n = (Kai_u64)((((current->value).string).data)[0]), .d = 1});
                left = kai__parser_create_number(parser, *current);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("multi")))
            {
                kai__next_token();
                kai__expect(current->id==KAI_TOKEN_STRING, "multi", "must be string");
                kai__expect(((current->value).string).count>0, "multi", "string cannot be empty");
                Kai_Number value = ((Kai_Number){0});
                Kai_Number base = ((Kai_Number){.n = 1, .d = 1, .e = 8});
                for (Kai_u32 i = 0; i < ((current->value).string).count; ++i)
                {
                    Kai_u32 idx = (((current->value).string).count-1)-i;
                    Kai_Number dg = kai_number_normalize(((Kai_Number){.n = (((current->value).string).data)[idx], .d = 1}));
                    value = kai_number_add(kai_number_mul(value, base), dg);
                }
                (current->value).number = value;
                left = kai__parser_create_number(parser, *current);
            }
            else
            if ((kai_string_equals((current->value).string, KAI_STRING("array"))||kai_string_equals((current->value).string, KAI_STRING("map")))||kai_string_equals((current->value).string, KAI_STRING("proc")))
            {
                kai__next_token();
                left = kai_parse_type_expression(parser);
            }
            else
            if (kai_string_equals((current->value).string, KAI_STRING("Julie")))
            {
                current->string = KAI_STRING("\"<3\"");
                (current->value).string = KAI_STRING("<3");
                left = kai__parser_create_string(parser, *current);
            }
            else
                return kai__unexpected("in expression", "unknown directive");
        }
        break; case KAI_TOKEN_union:
        case KAI_TOKEN_struct:
        {
            Kai_Token struct_token = *current;
            Kai_Stmt_List body = {0};
            Kai_u32 count = {0};
            kai__next_token();
            kai__next_token();
            while (current->id!=125)
            {
                Kai_Stmt* stmt = kai_parse_statement(parser);
                kai__expect(stmt, "in struct definition", "expected a statement here");
                kai__linked_list_append(body, stmt);
                kai__next_token();
                count += 1;
            }
            left = kai__parser_create_struct(parser, struct_token, count, body.head);
            if (struct_token.id==KAI_TOKEN_union)
                left->flags |= KAI_FLAG_STRUCT_UNION;
        }
        break; case KAI_TOKEN_enum:
        {
            Kai_Token struct_token = *current;
            Kai_Stmt_List body = {0};
            Kai_u32 count = {0};
            kai__next_token();
            Kai_Expr* type = kai_parse_type_expression(parser);
            kai__expect(type, "[todo]", "[todo]");
            kai__next_token();
            kai__next_token();
            while (current->id!=125)
            {
                Kai_Stmt* stmt = kai_parse_statement(parser);
                kai__expect(stmt, "in struct definition", "expected a statement here");
                kai__linked_list_append(body, stmt);
                kai__next_token();
                count += 1;
            }
            left = kai__parser_create_enum(parser, struct_token, type, count, body.head);
        }
        break; default:
        return NULL;
    }
    Kai_Token* peeked = kai_tokenizer_peek(&parser->tokenizer);
    Kai__Operator op_info = kai__operator_info(peeked->id);
    Kai_u32 prec = flags&KAI_PRECEDENCE_MASK;
    while (op_info.prec!=0&&op_info.prec>prec)
    {
        Kai_u32 op = peeked->id;
        if (op==15677)
        {
            Kai_Tokenizer prev_state = parser->tokenizer;
            kai_tokenizer_next(&parser->tokenizer);
            kai_tokenizer_next(&parser->tokenizer);
            if (current->id==123)
            {
                parser->tokenizer = prev_state;
                return left;
            }
        }
        else
        {
            kai_tokenizer_next(&parser->tokenizer);
            kai_tokenizer_next(&parser->tokenizer);
        }
        switch (op_info.type)
        {
            break; case KAI__OPERATOR_TYPE_BINARY:
            {
                Kai_Expr* right = kai_parse_expression(parser, op_info.prec);
                kai__expect(right!=NULL, "in binary expression", "should be an expression after binary operator");
                left = kai__parser_create_binary(parser, left, right, op);
            }
            break; case KAI__OPERATOR_TYPE_INDEX:
            {
                Kai_Expr* right = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                kai__expect(right, "in index operation", "should be an expression here");
                kai__next_token();
                kai__expect(current->id==93, "in index operation", "expected ']' here");
                left = kai__parser_create_binary(parser, left, right, op);
            }
            break; case KAI__OPERATOR_TYPE_PROCEDURE_CALL:
            {
                Kai_u32 arg_count = {0};
                Kai_Expr* args = kai_parse_procedure_call_arguments(parser, &arg_count);
                left = kai__parser_create_procedure_call(parser, left, args, (Kai_u8)(arg_count));
            }
        }
        kai_tokenizer_peek(&parser->tokenizer);
        op_info = kai__operator_info(peeked->id);
    }
    return left;
}

KAI_API(Kai_Expr*) kai_parse_type_expression(Kai_Parser* parser)
{
    Kai_Token* current = &(parser->tokenizer).current_token;
    switch (current->id)
    {
        break; case 91:
        {
            Kai_Token op_token = *current;
            kai__next_token();
            Kai_Expr* rows = 0;
            Kai_Expr* cols = 0;
            Kai_u8 expr_flags = {0};
            if (current->id==11822)
            {
                kai__next_token();
                expr_flags |= KAI_FLAG_ARRAY_DYNAMIC;
            }
            else
            if (current->id!=93)
            {
                rows = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                kai__expect(rows, "in array type", "[todo]");
                kai__next_token();
                if (current->id==44)
                {
                    kai__next_token();
                    cols = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                    kai__expect(cols, "in array type", "[todo]");
                    kai__next_token();
                }
            }
            kai__expect(current->id==93, "in array type", "array");
            kai__next_token();
            Kai_Expr* type = kai_parse_type_expression(parser);
            kai__expect(type, "in array type", "expected type here");
            return kai__parser_create_array(parser, op_token, type, rows, cols, expr_flags);
        }
        break; case 42:
        {
            Kai_Token op_token = *current;
            kai__next_token();
            Kai_Expr* expr = kai_parse_type_expression(parser);
            kai__expect(expr, "in unary expression", "should be an expression here");
            return kai__parser_create_unary(parser, op_token, expr);
        }
    }
    if (current->id!=40)
        return kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
    kai__next_token();
    Kai_u8 in_count = {0};
    Kai_u8 out_count = {0};
    Kai_Expr_List in_out = {0};
    while (current->id!=41)
    {
        Kai_string name = {0};
        if (current->id==KAI_TOKEN_IDENTIFIER)
        {
            Kai_Token* peeked = kai__peek_token();
            if (peeked->id==58)
            {
                name = current->string;
                kai__next_token();
                kai__next_token();
            }
        }
        Kai_Expr* type = kai_parse_type_expression(parser);
        kai__expect(type, "in procedure type", "should be a type here");
        type->name = name;
        kai__linked_list_append(in_out, type);
        kai__expect(in_count!=255, "in procedure type", "too many inputs to procedure");
        in_count += 1;
        kai__next_token();
        if (current->id==44)
            kai__next_token();
        else
        if (current->id!=41)
            return kai__unexpected("in procedure type", "',' or ')' expected here");
    }
    Kai_Token* peek = kai__peek_token();
    if (peek->id==15917)
    {
        kai__next_token();
        kai__next_token();
        Kai_bool enclosed_return = KAI_FALSE;
        if (current->id==40)
        {
            enclosed_return = KAI_TRUE;
            kai__next_token();
        }
        while (current->id!=41)
        {
            Kai_Expr* type = kai_parse_type_expression(parser);
            if (!type)
                return NULL;
            kai__linked_list_append(in_out, type);
            kai__expect(out_count!=255, "in procedure type", "too many inputs to procedure");
            out_count += 1;
            kai__peek_token();
            if (peek->id==44)
            {
                kai__next_token();
                kai__next_token();
            }
            else
                break;
        }
        if (enclosed_return)
        {
            if (peek->id!=41)
            {
                kai__next_token();
                return kai__unexpected("in procedure type", "should be ')' after return types");
            }
            else
                kai__next_token();
        }
    }
    return kai__parser_create_procedure_type(parser, in_out.head, in_count, out_count);
}

KAI_API(Kai_Expr*) kai_parse_procedure(Kai_Parser* parser)
{
    Kai_Token* current = &(parser->tokenizer).current_token;
    Kai_Token token = *current;
    kai__expect(current->id==40, "", "this is likely a compiler bug, sorry :c");
    kai__next_token();
    Kai_u8 in_count = {0};
    Kai_u8 out_count = {0};
    Kai_Expr_List in_out = {0};
    while (current->id!=41)
    {
        Kai_u8 flags = {0};
        if (current->id==KAI_TOKEN_using)
        {
            flags |= KAI_FLAG_PROC_USING;
            kai__next_token();
        }
        kai__expect(current->id==KAI_TOKEN_IDENTIFIER, "in procedure input", "should be an identifier");
        Kai_string name = current->string;
        kai__next_token();
        kai__expect(current->id==58, "in procedure input", "wanted a ':' here");
        kai__next_token();
        Kai_Expr* type = kai_parse_type_expression(parser);
        kai__expect(type, "in procedure input", "should be type");
        type->name = name;
        type->flags = flags;
        kai__linked_list_append(in_out, type);
        kai__expect(in_count!=255, "in procedure call", "too many inputs to procedure");
        in_count += 1;
        kai__next_token();
        switch (current->id)
        {
            break; case 41:
            break; case 44:
            kai__next_token();
            break; default:
            return kai__unexpected("in procedure input", "wanted ')' or ',' here");
        }
    }
    kai__next_token();
    if (current->id==15917)
    {
        kai__next_token();
        Kai_Expr* type = kai_parse_type_expression(parser);
        kai__expect(type, "in procedure return type", "should be type");
        kai__linked_list_append(in_out, type);
        kai__expect(out_count!=255, "in procedure call", "too many inputs to procedure");
        out_count += 1;
        kai__next_token();
    }
    Kai_Stmt* body = 0;
    if (current->id==KAI_TOKEN_DIRECTIVE&&kai_string_equals(KAI_STRING("host"), current->string))
    {
        kai__next_token();
        kai__expect(current->id==59, "???", "???");
    }
    else
    {
        body = kai_parse_statement(parser);
        if (!body)
            return kai__unexpected("[todo: remove this]", "");
    }
    return kai__parser_create_procedure(parser, token, in_out.head, body, in_count, out_count);
}

KAI_API(Kai_Stmt*) kai_parse_declaration(Kai_Parser* parser)
{
    Kai_Token* current = &(parser->tokenizer).current_token;
    if (current->id==KAI_TOKEN_DIRECTIVE)
        return kai_parse_statement(parser);
    if (current->id!=KAI_TOKEN_IDENTIFIER)
        return kai__unexpected("in declaration", "expected an identifier");
    Kai_string name = current->string;
    Kai_u32 line_number = current->line_number;
    kai__next_token();
    if (current->id!=58)
        return kai__unexpected("in declaration", "expected ':' here");
    kai__next_token();
    Kai_u8 flags = 0;
    Kai_Expr* type = kai_parse_type_expression(parser);
    if (type)
        kai__next_token();
    Kai_Expr* expr = 0;
    switch (current->id)
    {
        break; case 58:
        flags |= KAI_FLAG_DECL_CONST;
        break; case 61:
        break; case 59:
        kai__expect(type!=NULL, "in declaration", "should be '=', ':', or expression here");
        return kai__parser_create_declaration(parser, name, type, expr, flags, line_number);
        break; default:
        return kai__unexpected("in declaration", "should be '=', ':', or ';'");
    }
    kai__next_token();
    Kai_bool require_semicolon = KAI_FALSE;
    if (current->id==KAI_TOKEN_DIRECTIVE&&kai_string_equals((current->value).string, KAI_STRING("host_import")))
    {
        flags |= KAI_FLAG_DECL_HOST_IMPORT;
        require_semicolon = KAI_TRUE;
    }
    else
    if (current->id==40&&kai__is_procedure_next(parser))
    {
        expr = kai_parse_procedure(parser);
        if (!expr)
            return kai__unexpected("[todo]", "[todo]");
    }
    else
    {
        require_semicolon = KAI_TRUE;
        if ((current->id==KAI_TOKEN_struct||current->id==KAI_TOKEN_union)||current->id==KAI_TOKEN_enum)
            require_semicolon = KAI_FALSE;
        expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
        kai__expect(expr, "in declaration", "should be an expression here");
    }
    if (require_semicolon)
    {
        kai__next_token();
        kai__expect(current->id==59, "after declaration", "there should be a ';' before this");
    }
    else
    {
        Kai_Token* peeked = kai__peek_token();
        if (peeked->id==59)
            kai__next_token();
    }
    return kai__parser_create_declaration(parser, name, type, expr, flags, line_number);
}

KAI_API(Kai_Stmt*) kai_parse_statement(Kai_Parser* parser)
{
    Kai_Token* current = &(parser->tokenizer).current_token;
    switch (current->id)
    {
        break; case 123:
        {
            Kai_Token token = *current;
            kai__next_token();
            Kai_Stmt_List body = {0};
            while (current->id!=125)
            {
                Kai_Stmt* statement = kai_parse_statement(parser);
                if (!statement)
                    return NULL;
                kai__linked_list_append(body, statement);
                kai__next_token();
            }
            return kai__parser_create_compound(parser, token, body.head);
        }
        break; case KAI_TOKEN_case:
        case KAI_TOKEN_break:
        case KAI_TOKEN_continue:
        case KAI_TOKEN_defer:
        {
            Kai_Token token = *current;
            kai__next_token();
            Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            if (expr!=NULL)
                kai__next_token();
            kai__expect(current->id==59, "after control statement", "there should be a ';' before this");
            Kai_u8 kind = KAI_CONTROL_CASE;
            if (token.id==KAI_TOKEN_break)
                kind = KAI_CONTROL_BREAK;
            if (token.id==KAI_TOKEN_continue)
                kind = KAI_CONTROL_CONTINUE;
            if (token.id==KAI_TOKEN_defer)
                kind = KAI_CONTROL_DEFER;
            return kai__parser_create_control(parser, token, kind, expr);
        }
        break; case KAI_TOKEN_using:
        {
            kai__next_token();
            Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            kai__expect(expr, "in using statement", "should be an expression");
            kai__next_token();
            kai__expect(current->id==59, "after statement", "there should be a ';' before this");
            expr->flags = KAI_FLAG_EXPR_USING;
            return expr;
        }
        break; case KAI_TOKEN_ret:
        {
            Kai_Token ret_token = *current;
            kai__next_token();
            if (current->id==59)
                return kai__parser_create_return(parser, ret_token, NULL);
            Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            kai__expect(expr, "in return statement", "should be an expression");
            kai__next_token();
            kai__expect(current->id==59, "after statement", "there should be a ';' before this");
            return kai__parser_create_return(parser, ret_token, expr);
        }
        break; case KAI_TOKEN_if:
        {
            Kai_Token if_token = *current;
            kai__next_token();
            Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            kai__expect(expr, "in if statement", "should be an expression here");
            kai__next_token();
            Kai_u8 flags = 0;
            if (current->id==15677)
            {
                kai__next_token();
                flags |= KAI_FLAG_IF_CASE;
            }
            Kai_Stmt* then_body = kai_parse_statement(parser);
            if (!then_body)
                return NULL;
            Kai_Token* peeked = kai__peek_token();
            Kai_Stmt* else_body = 0;
            if (peeked->id==KAI_TOKEN_else)
            {
                kai__next_token();
                kai__next_token();
                else_body = kai_parse_statement(parser);
                if (!else_body)
                    return NULL;
            }
            return kai__parser_create_if(parser, if_token, flags, expr, then_body, else_body);
        }
        break; case KAI_TOKEN_while:
        {
            Kai_Token while_token = *current;
            kai__next_token();
            Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            kai__next_token();
            Kai_Stmt* body = kai_parse_statement(parser);
            return kai__parser_create_while(parser, while_token, expr, body);
        }
        break; case KAI_TOKEN_for:
        {
            Kai_Token for_token = *current;
            kai__next_token();
            kai__expect(current->id==KAI_TOKEN_IDENTIFIER, "in for statement", "should be the name of the iterator");
            Kai_string iterator_name = current->string;
            kai__next_token();
            kai__expect(current->id==58, "in for statement", "should be ':' here");
            kai__next_token();
            Kai_Expr* from = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            kai__expect(from, "in for statement", "should be an expression here");
            Kai_Expr* to = 0;
            kai__next_token();
            Kai_u8 flags = {0};
            if (current->id==11822)
            {
                kai__next_token();
                if (current->id==60)
                {
                    flags |= KAI_FLAG_FOR_LESS_THAN;
                    kai__next_token();
                }
                to = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                kai__expect(to, "in for statement", "should be an expression here");
                kai__next_token();
            }
            Kai_Stmt* body = kai_parse_statement(parser);
            if (!body)
                return NULL;
            return kai__parser_create_for(parser, for_token, iterator_name, from, to, body, flags);
        }
        break; case KAI_TOKEN_DIRECTIVE:
        {
            if (kai_string_equals((current->value).string, KAI_STRING("export")))
            {
                kai__next_token();
                Kai_Stmt* decl = kai_parse_declaration(parser);
                decl->flags |= KAI_FLAG_DECL_EXPORT;
                return decl;
            }
            else
            {
                Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                kai__expect(expr, "in statement", "there should be an expression here");
                kai__next_token();
                kai__expect(current->id==59, "after statement", "there should be a ';' before this");
                return expr;
            }
        }
        break; case KAI_TOKEN_IDENTIFIER:
        {
            Kai_Token* peeked = kai__peek_token();
            if (peeked->id==58)
                return kai_parse_declaration(parser);
        }
        default:
        {
            Kai_bool requires_semicolon = KAI_TRUE;
            Kai_Expr* expr = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
            kai__expect(expr, "in statement", "should be an expression or statement");
            Kai_Token* peeked = kai__peek_token();
            switch (peeked->id)
            {
                break; case 61:
                case 15740:
                case 15654:
                case 15659:
                case 15661:
                case 15658:
                case 15663:
                {
                    kai__next_token();
                    Kai_u32 op = current->id;
                    kai__next_token();
                    Kai_Expr* right = kai_parse_expression(parser, KAI_TOP_PRECEDENCE);
                    kai__expect(right, "in assignment statement", "should be an expression");
                    expr = kai__parser_create_assignment(parser, op, expr, right);
                    kai__next_token();
                    kai__expect(current->id==59, "in assignment statement", "should be ';' after expression");
                }
                break; default:
                {
                    if (requires_semicolon)
                    {
                        kai__next_token();
                        kai__expect(current->id==59, "in expression statement", "should be ';' after expression");
                    }
                }
            }
            return expr;
        }
    }
}

KAI_API(Kai_Result) kai_create_syntax_tree(Kai_Syntax_Tree_Create_Info* info, Kai_Syntax_Tree* out_tree)
{
    Kai_Parser parser = {0};
    (parser.tokenizer).source = (info->source).contents;
    (parser.tokenizer).line_number = 1;
    parser.error = info->error;
    (parser.tokenizer).string_arena = ((Kai_Fixed_Allocator){.data = (info->allocator).heap_allocate((info->allocator).user, NULL, ((info->source).contents).count, 0), .size = ((info->source).contents).count});
    kai_arena_create(&parser.arena, &info->allocator);
    Kai_Stmt_List statements = {0};
    Kai_Token* token = kai_tokenizer_next(&parser.tokenizer);
    while (token->id!=KAI_TOKEN_END)
    {
        Kai_Stmt* statement = kai_parse_declaration(&parser);
        if (statement==NULL)
            break;
        kai__linked_list_append(statements, statement);
        kai_tokenizer_next(&parser.tokenizer);
    }
    (out_tree->root).id = KAI_STMT_COMPOUND;
    (out_tree->root).head = statements.head;
    out_tree->source = info->source;
    out_tree->allocator = parser.arena;
    if ((parser.error)->result!=KAI_SUCCESS)
        ((parser.error)->location).source = info->source;
    return (parser.error)->result;
}

KAI_API(void) kai_destroy_syntax_tree(Kai_Syntax_Tree* tree)
{
    (void)(tree);
}

KAI_INTERNAL Kai_bool kai__create_syntax_trees(Kai_Compiler_Context* context, Kai_Source_Slice sources)
{
    Kai_Allocator* allocator = &context->allocator;
    (((context->program)->code).trees).data = (Kai_Syntax_Tree*)(kai__allocate(NULL, sources.count*sizeof(Kai_Syntax_Tree), 0));
    (((context->program)->code).trees).count = sources.count;
    for (Kai_u32 i = 0; i < sources.count; ++i)
    {
        Kai_Syntax_Tree_Create_Info info = ((Kai_Syntax_Tree_Create_Info){.source = sources.data[i], .allocator = context->allocator, .error = context->error});
        if (kai_create_syntax_tree(&info, &kai_array_last(&((context->program)->code).trees))!=KAI_SUCCESS)
            return KAI_TRUE;
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__inside_procedure_scope(Kai_Compiler_Context* context)
{
    for (Kai_u32 i = 0; i < (context->scopes).count; ++i)
    {
        Kai_u32 ri = ((context->scopes).count-1)-i;
        Kai_Scope* scope = &((context->scopes).data)[ri];
        if (scope->is_proc_scope)
            return KAI_TRUE;
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__error_redefinition(Kai_Compiler_Context* context, Kai_Location location, Kai_u32 original)
{
    *(context->error) = ((Kai_Error){.result = KAI_ERROR_SEMANTIC, .location = location});
    Kai_Buffer buffer = ((Kai_Buffer){.allocator = context->allocator});
    {
        kai__buffer_append_string(&buffer, KAI_STRING("indentifier \""));
        kai__buffer_append_string(&buffer, location.string);
        kai__buffer_append_string(&buffer, KAI_STRING("\" has already been declared"));
        Kai_Range range = kai__buffer_end(&buffer);
        (context->error)->memory = kai__buffer_done(&buffer);
        (context->error)->message = kai__range_to_string(range, (context->error)->memory);
    }
    {
        Kai_Range info_range = kai__buffer_push(&buffer, sizeof(Kai_Error));
        kai__buffer_append_string(&buffer, KAI_STRING("see original definition of \""));
        kai__buffer_append_string(&buffer, location.string);
        kai__buffer_append_string(&buffer, KAI_STRING("\""));
        Kai_Range message_range = kai__buffer_end(&buffer);
        Kai_Memory memory = kai__buffer_done(&buffer);
        Kai_Node* existing = &((context->nodes).data)[original];
        Kai_Error* info = (Kai_Error*)((Kai_u8*)(memory.data)+info_range.start);
        *info = ((Kai_Error){.result = KAI_ERROR_INFO, .location = existing->location, .message = kai__range_to_string(message_range, memory), .memory = memory});
        (context->error)->next = info;
    }
    return KAI_TRUE;
}

KAI_INTERNAL Kai_bool kai__error_not_declared(Kai_Compiler_Context* context, Kai_Location location)
{
    *(context->error) = ((Kai_Error){.result = KAI_ERROR_SEMANTIC, .location = location});
    Kai_Buffer buffer = ((Kai_Buffer){.allocator = context->allocator});
    kai__buffer_append_string(&buffer, KAI_STRING("indentifier \""));
    kai__buffer_append_string(&buffer, location.string);
    kai__buffer_append_string(&buffer, KAI_STRING("\" not declared"));
    Kai_Range range = kai__buffer_end(&buffer);
    (context->error)->memory = kai__buffer_done(&buffer);
    (context->error)->message = kai__range_to_string(range, (context->error)->memory);
    return KAI_TRUE;
}

KAI_INTERNAL Kai_bool kai__create_nodes(Kai_Compiler_Context* context, Kai_Expr* expr)
{
    Kai_Allocator* allocator = &context->allocator;
    (void)(allocator);
    switch (expr->id)
    {
        break; case KAI_EXPR_PROCEDURE:
        {
        }
        break; case KAI_STMT_IF:
        {
        }
        break; case KAI_STMT_FOR:
        {
        }
        break; case KAI_STMT_DECLARATION:
        {
            Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)(expr);
            if (!(d->flags&KAI_FLAG_DECL_CONST)&&kai__inside_procedure_scope(context))
                return KAI_FALSE;
            Kai_Location location = ((Kai_Location){.source = context->current_source, .string = d->name, .line = d->line_number});
            Kai_Scope* scope = &kai_array_last(&context->scopes);
            {
                Kai_int index = kai_table_find(&scope->identifiers, d->name);
                if (index!=-1)
                {
                    Kai_Node_Reference reference = ((scope->identifiers).values)[index];
                    (context->error)->result = KAI_ERROR_FATAL;
                    return kai__error_redefinition(context, location, reference.index);
                }
            }
            {
                Kai_Node_Reference reference = ((Kai_Node_Reference){.index = (context->nodes).count});
                Kai_Node node = ((Kai_Node){.location = location, .expr = d->expr, .type_expr = d->type});
                if (d->flags&KAI_FLAG_DECL_HOST_IMPORT)
                    node.flags |= KAI_NODE_IMPORT;
                if (d->flags&KAI_FLAG_DECL_EXPORT)
                    node.flags |= KAI_NODE_EXPORT;
                kai_array_push(&node.value_dependencies, ((Kai_Node_Reference){.flags = KAI_NODE_TYPE, .index = reference.index}));
                kai_array_push(&context->nodes, node);
                kai_table_set(&scope->identifiers, d->name, reference);
            }
            return KAI_FALSE;
        }
        break; case KAI_STMT_COMPOUND:
        {
            Kai_Stmt_Compound* c = (Kai_Stmt_Compound*)(expr);
            kai_array_push(&context->scopes, ((Kai_Scope){0}));
            Kai_Stmt* current = c->head;
            while (current)
            {
                if (kai__create_nodes(context, current))
                    return KAI_TRUE;
                current = current->next;
            }
            kai_array_pop(&context->scopes);
            return KAI_FALSE;
        }
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_Node_Reference kai__lookup_node(Kai_Compiler_Context* context, Kai_string name)
{
    for (Kai_u32 i = 0; i < (context->scopes).count; ++i)
    {
        Kai_u32 ri = ((context->scopes).count-1)-i;
        Kai_Scope* scope = &((context->scopes).data)[ri];
        Kai_int k = kai_table_find(&scope->identifiers, name);
        if (k==-1)
            continue;
        return ((scope->identifiers).values)[k];
    }
    return ((Kai_Node_Reference){.flags = KAI_NODE_NOT_FOUND});
}

KAI_API(void) kai_add_dependency(Kai_Compiler_Context* context, Kai_Node_Reference ref)
{
    Kai_Allocator* allocator = &context->allocator;
    Kai_Node_Reference_DynArray* deps = 0;
    Kai_Node* node = &((context->nodes).data)[(context->current_node).index];
    if ((context->current_node).flags&KAI_NODE_TYPE)
        deps = &node->type_dependencies;
    else
        deps = &node->value_dependencies;
    for (Kai_u32 i = 0; i < deps->count; ++i)
    {
        Kai_Node_Reference other = (deps->data)[i];
        if (ref.index==other.index&&ref.flags==other.flags)
            return;
    }
    kai_array_push(deps, ref);
}

KAI_INTERNAL Kai_bool kai__insert_value_dependencies(Kai_Compiler_Context* context, Kai_Expr* expr)
{
    Kai_Allocator* allocator = &context->allocator;
    if (expr==NULL)
    {
        kai__todo("null expression\n");
    }
    switch (expr->id)
    {
        break; case KAI_EXPR_IDENTIFIER:
        {
            Kai_Node_Reference ref = kai__lookup_node(context, expr->source_code);
            if (ref.flags&KAI_NODE_NOT_FOUND)
            {
                Kai_Node* node = &((context->nodes).data)[(context->current_node).index];
                Kai_Location location = ((Kai_Location){.source = (node->location).source, .string = expr->source_code, .line = expr->line_number});
                return kai__error_not_declared(context, location);
            }
            if (ref.flags&KAI_NODE_LOCAL)
                break;
            kai_add_dependency(context, ref);
        }
        break; case KAI_EXPR_UNARY:
        {
            Kai_Expr_Unary* u = (Kai_Expr_Unary*)(expr);
            return kai__insert_value_dependencies(context, u->expr);
        }
        break; case KAI_EXPR_BINARY:
        {
            Kai_Expr_Binary* b = (Kai_Expr_Binary*)(expr);
            if (kai__insert_value_dependencies(context, b->left))
                return KAI_TRUE;
            if (kai__insert_value_dependencies(context, b->right))
                return KAI_TRUE;
        }
        break; case KAI_EXPR_PROCEDURE:
        {
            Kai_Expr_Procedure* p = (Kai_Expr_Procedure*)(expr);
            kai_array_push(&context->scopes, ((Kai_Scope){0}));
            Kai_Scope* scope = &kai_array_last(&context->scopes);
            Kai_Expr* current = p->in_out_expr;
            while (current)
            {
                if ((current->name).count!=0)
                {
                    Kai_Node_Reference ref = ((Kai_Node_Reference){.flags = KAI_NODE_LOCAL});
                    kai_table_set(&scope->identifiers, current->name, ref);
                }
                current = current->next;
            }
            if (p->body!=NULL&&kai__insert_value_dependencies(context, p->body))
                return KAI_TRUE;
            kai_array_pop(&context->scopes);
        }
        break; case KAI_STMT_RETURN:
        {
            Kai_Stmt_Return* r = (Kai_Stmt_Return*)(expr);
            return kai__insert_value_dependencies(context, r->expr);
        }
        break; case KAI_STMT_IF:
        {
            Kai_Stmt_If* i = (Kai_Stmt_If*)(expr);
            if (kai__insert_value_dependencies(context, i->expr))
                return KAI_TRUE;
            if (i->then_body!=NULL&&kai__insert_value_dependencies(context, i->then_body))
                return KAI_TRUE;
            if (i->else_body!=NULL&&kai__insert_value_dependencies(context, i->else_body))
                return KAI_TRUE;
        }
        break; case KAI_STMT_COMPOUND:
        {
            Kai_Stmt_Compound* c = (Kai_Stmt_Compound*)(expr);
            Kai_Expr* current = c->head;
            while (current!=NULL)
            {
                if (kai__insert_value_dependencies(context, current))
                    return KAI_TRUE;
                current = current->next;
            }
        }
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__insert_type_dependencies(Kai_Compiler_Context* context, Kai_Expr* expr)
{
    if (expr==NULL)
    {
        kai__todo("null expression\n");
    }
    switch (expr->id)
    {
        break; case KAI_EXPR_IDENTIFIER:
        {
            Kai_Node_Reference ref = kai__lookup_node(context, expr->source_code);
            if (ref.flags&KAI_NODE_NOT_FOUND)
            {
                Kai_Node* node = &((context->nodes).data)[(context->current_node).index];
                Kai_Location location = ((Kai_Location){.source = (node->location).source, .string = expr->source_code, .line = expr->line_number});
                return kai__error_not_declared(context, location);
            }
            if (ref.flags&KAI_NODE_LOCAL)
                break;
            ref.flags |= KAI_NODE_TYPE;
            kai_add_dependency(context, ref);
        }
        break; case KAI_EXPR_NUMBER:
        break; case KAI_EXPR_STRING:
        break; case KAI_EXPR_SPECIAL:
        break; case KAI_EXPR_UNARY:
        {
            Kai_Expr_Unary* u = (Kai_Expr_Unary*)(expr);
            return kai__insert_type_dependencies(context, u->expr);
        }
        break; case KAI_EXPR_BINARY:
        {
            Kai_Expr_Binary* b = (Kai_Expr_Binary*)(expr);
            if (kai__insert_type_dependencies(context, b->left))
                return KAI_TRUE;
            if (b->op==15917)
            {
                if (kai__insert_value_dependencies(context, b->right))
                    return KAI_TRUE;
            }
            else
            {
                if (kai__insert_type_dependencies(context, b->right))
                    return KAI_TRUE;
            }
        }
        break; case KAI_EXPR_PROCEDURE:
        {
            Kai_Expr_Procedure* p = (Kai_Expr_Procedure*)(expr);
            Kai_Expr* current = p->in_out_expr;
            while (current!=NULL)
            {
                if (kai__insert_value_dependencies(context, current))
                    return KAI_TRUE;
                current = current->next;
            }
        }
        break; case KAI_EXPR_STRUCT:
        {
            Kai_Expr_Struct* s = (Kai_Expr_Struct*)(expr);
            Kai_Stmt* current = s->head;
            while (current)
            {
                if (kai__insert_value_dependencies(context, current))
                    return KAI_TRUE;
                current = current->next;
            }
            return KAI_FALSE;
        }
        break; case KAI_EXPR_PROCEDURE_TYPE:
        {
            return KAI_FALSE;
        }
        break; default:
        {
            kai__todo("need to implement id = %i", expr->id);
        }
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__generate_dependency_builtin_types(Kai_Compiler_Context* context)
{
    Kai_Allocator* allocator = &context->allocator;
    Kai_Scope* scope = &kai_array_last(&context->scopes);
    Kai_Node_Reference ref = {0};
    ref.index = (context->nodes).count;
    kai_table_set(&scope->identifiers, KAI_STRING("void"), ((Kai_Node_Reference){.index = (context->nodes).count+1}));
    kai_table_set(&scope->identifiers, KAI_STRING("s8"), ((Kai_Node_Reference){.index = (context->nodes).count+2}));
    kai_table_set(&scope->identifiers, KAI_STRING("s16"), ((Kai_Node_Reference){.index = (context->nodes).count+3}));
    kai_table_set(&scope->identifiers, KAI_STRING("s32"), ((Kai_Node_Reference){.index = (context->nodes).count+4}));
    kai_table_set(&scope->identifiers, KAI_STRING("s64"), ((Kai_Node_Reference){.index = (context->nodes).count+5}));
    kai_table_set(&scope->identifiers, KAI_STRING("u8"), ((Kai_Node_Reference){.index = (context->nodes).count+6}));
    kai_table_set(&scope->identifiers, KAI_STRING("u16"), ((Kai_Node_Reference){.index = (context->nodes).count+7}));
    kai_table_set(&scope->identifiers, KAI_STRING("u32"), ((Kai_Node_Reference){.index = (context->nodes).count+8}));
    kai_table_set(&scope->identifiers, KAI_STRING("u64"), ((Kai_Node_Reference){.index = (context->nodes).count+9}));
    kai_table_set(&scope->identifiers, KAI_STRING("f32"), ((Kai_Node_Reference){.index = (context->nodes).count+10}));
    kai_table_set(&scope->identifiers, KAI_STRING("f64"), ((Kai_Node_Reference){.index = (context->nodes).count+11}));
    kai_table_set(&scope->identifiers, KAI_STRING("bool"), ((Kai_Node_Reference){.index = (context->nodes).count+12}));
    kai_table_set(&scope->identifiers, KAI_STRING("string"), ((Kai_Node_Reference){.index = (context->nodes).count+14}));
    Kai_Type_Info* type_type = (Kai_Type_Info*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info)));
    type_type->id = KAI_TYPE_ID_TYPE;
    kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = type_type}), .flags = KAI_NODE_EVALUATED}));
    context->type_type = type_type;
    Kai_Type_Info* void_type = (Kai_Type_Info*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info)));
    void_type->id = KAI_TYPE_ID_VOID;
    kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = void_type}), .flags = KAI_NODE_EVALUATED}));
    Kai_u8 bits = 8;
    while (bits<=64)
    {
        Kai_Type_Info_Integer* type = (Kai_Type_Info_Integer*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Integer)));
        type->id = KAI_TYPE_ID_INTEGER;
        type->is_signed = KAI_TRUE;
        type->bits = bits;
        kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = (Kai_Type)(type)}), .flags = KAI_NODE_EVALUATED}));
        bits *= 2;
    }
    Kai_Type u8_type = {0};
    Kai_Type uint_type = {0};
    bits = 8;
    while (bits<=64)
    {
        Kai_Type_Info_Integer* type = (Kai_Type_Info_Integer*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Integer)));
        type->id = KAI_TYPE_ID_INTEGER;
        type->is_signed = KAI_FALSE;
        type->bits = bits;
        if (bits==8*sizeof(Kai_uint))
        {
            uint_type = (Kai_Type)(type);
        }
        if (bits==8)
        {
            u8_type = (Kai_Type)(type);
        }
        kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = (Kai_Type)(type)}), .flags = KAI_NODE_EVALUATED}));
        bits *= 2;
    }
    bits = 32;
    while (bits<=64)
    {
        Kai_Type_Info_Float* type = (Kai_Type_Info_Float*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Float)));
        type->id = KAI_TYPE_ID_FLOAT;
        type->bits = bits;
        kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = (Kai_Type)(type)}), .flags = KAI_NODE_EVALUATED}));
        bits *= 2;
    }
    Kai_Type_Info* bool_type = (Kai_Type_Info*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info)));
    bool_type->id = KAI_TYPE_ID_BOOLEAN;
    kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = (Kai_Type)(bool_type)}), .flags = KAI_NODE_EVALUATED}));
    context->bool_type = bool_type;
    Kai_Type_Info* number_type = (Kai_Type_Info*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info)));
    number_type->id = KAI_TYPE_ID_NUMBER;
    kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = number_type}), .flags = KAI_NODE_EVALUATED}));
    context->number_type = number_type;
    Kai_Type_Info_Pointer* pu8_type = (Kai_Type_Info_Pointer*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Pointer)));
    pu8_type->id = KAI_TYPE_ID_POINTER;
    pu8_type->sub_type = u8_type;
    Kai_Type_Info_Struct* string_type = (Kai_Type_Info_Struct*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Struct)));
    string_type->id = KAI_TYPE_ID_STRING;
    string_type->size = sizeof(Kai_uint)+sizeof(Kai_u8*);
    (string_type->fields).count = 2;
    (string_type->fields).data = (Kai_Struct_Field*)(kai_arena_allocate(&context->type_allocator, (string_type->fields).count*sizeof(Kai_Struct_Field)));
    ((string_type->fields).data)[0] = ((Kai_Struct_Field){.name = KAI_STRING("count"), .offset = 0, .type = uint_type});
    ((string_type->fields).data)[1] = ((Kai_Struct_Field){.name = KAI_STRING("data"), .offset = sizeof(Kai_uint), .type = (Kai_Type)(pu8_type)});
    kai_array_push(&context->nodes, ((Kai_Node){.type = type_type, .value = ((Kai_Value){.type = (Kai_Type)(string_type)}), .flags = KAI_NODE_EVALUATED}));
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__generate_dependency_graph(Kai_Compiler_Context* context)
{
    Kai_Allocator* allocator = &context->allocator;
    kai_array_push(&context->scopes, ((Kai_Scope){.is_proc_scope = KAI_FALSE}));
    if (kai__generate_dependency_builtin_types(context))
        return KAI_TRUE;
    for (Kai_u32 i = 0; i < (((context->program)->code).trees).count; ++i)
    {
        Kai_Syntax_Tree* tree = &((((context->program)->code).trees).data)[i];
        context->current_source = tree->source;
        Kai_Stmt* current = (tree->root).head;
        while (current)
        {
            if (kai__create_nodes(context, current))
                return KAI_TRUE;
            current = current->next;
        }
    }
    for (Kai_u32 i = 0; i < (context->nodes).count; ++i)
    {
        Kai_Node* node = &((context->nodes).data)[i];
        (context->current_node).index = i;
        if (node->flags&KAI_NODE_EVALUATED||node->flags&KAI_NODE_IMPORT)
            continue;
        if (node->expr!=NULL)
        {
            (context->current_node).flags = 0;
            if (kai__insert_value_dependencies(context, node->expr))
                return KAI_TRUE;
            (context->current_node).flags = KAI_NODE_TYPE;
            if (kai__insert_type_dependencies(context, node->expr))
                return KAI_TRUE;
        }
        (context->current_node).flags = KAI_NODE_TYPE;
        if (node->type_expr!=NULL)
        {
            if (kai__insert_value_dependencies(context, node->type_expr))
                return KAI_TRUE;
        }
    }
    if ((context->options).flags&KAI_COMPILE_DEBUG)
        for (Kai_u32 i = 14; i < (context->nodes).count; ++i)
        {
            Kai_Node* node = &((context->nodes).data)[i];
            printf("node (%i) %.*s", i, (Kai_s32)(((node->location).string).count), ((node->location).string).data);
            for (Kai_u32 i = 0; i < 32-((node->location).string).count; ++i)
                putchar(32);
            printf(" V{ ");
            for (Kai_u32 j = 0; j < (node->value_dependencies).count; ++j)
            {
                Kai_Node_Reference ref = ((node->value_dependencies).data)[j];
                Kai_u8 id = 118;
                if (ref.flags&KAI_NODE_TYPE)
                    id = 116;
                printf("%i%c", ref.index, id);
                if (j+1<(node->value_dependencies).count)
                    printf(", ");
            }
            printf(" } T{ ");
            for (Kai_u32 j = 0; j < (node->type_dependencies).count; ++j)
            {
                Kai_Node_Reference ref = ((node->type_dependencies).data)[j];
                Kai_u8 id = 118;
                if (ref.flags&KAI_NODE_TYPE)
                    id = 116;
                printf("%i%c", ref.index, id);
                if (j+1<(node->type_dependencies).count)
                    printf(", ");
            }
            printf(" }\n");
        }
    return KAI_FALSE;
}

KAI_INTERNAL void kai__explore_dependencies(Kai__DFS_Context* dfs, Kai_Node_Reference ref)
{
    Kai_u32 index = (ref.index)<<1|(ref.flags&KAI_NODE_TYPE);
    (dfs->visited)[index] = KAI_TRUE;
    Kai_Node* node = &(((dfs->context)->nodes).data)[ref.index];
    Kai_Node_Reference_DynArray* deps = 0;
    if (ref.flags&KAI_NODE_TYPE)
        deps = &node->type_dependencies;
    else
        deps = &node->value_dependencies;
    for (Kai_u32 d = 0; d < deps->count; ++d)
    {
        Kai_Node_Reference dep = (deps->data)[d];
        Kai_u32 d_index = (dep.index)<<1|(dep.flags&KAI_NODE_TYPE);
        if (!(dfs->visited)[d_index])
        {
            (dfs->prev)[d_index] = index;
            kai__explore_dependencies(dfs, dep);
        }
    }
    (dfs->post)[index] = dfs->next;
    dfs->next += 1;
}

KAI_INTERNAL Kai_bool kai__generate_compilation_order(Kai_Compiler_Context* context)
{
    Kai_Allocator* allocator = &context->allocator;
    Kai__DFS_Context dfs = ((Kai__DFS_Context){.context = context, .next = 0});
    dfs.post = (Kai_u32*)(kai__allocate(NULL, ((context->nodes).count*2)*sizeof(Kai_u32), 0));
    dfs.prev = (Kai_u32*)(kai__allocate(NULL, ((context->nodes).count*2)*sizeof(Kai_u32), 0));
    dfs.visited = (Kai_bool*)(kai__allocate(NULL, ((context->nodes).count*2)*sizeof(Kai_bool), 0));
    kai__memory_fill(dfs.prev, 255, ((context->nodes).count*2)*sizeof(Kai_u32));
    for (Kai_u32 i = 0; i < (context->nodes).count; ++i)
    {
        Kai_Node_Reference ref = ((Kai_Node_Reference){.index = i});
        Kai_u32 v = (ref.index)<<1|(ref.flags&KAI_NODE_TYPE);
        if (!(dfs.visited)[v])
            kai__explore_dependencies(&dfs, ref);
        ref.flags = KAI_NODE_TYPE;
        Kai_u32 t = (ref.index)<<1|(ref.flags&KAI_NODE_TYPE);
        if (!(dfs.visited)[t])
            kai__explore_dependencies(&dfs, ref);
    }
    (context->compilation_order).count = (context->nodes).count*2;
    (context->compilation_order).data = (Kai_u32*)(kai__allocate(NULL, (context->compilation_order).count*sizeof(Kai_u32), 0));
    for (Kai_u32 i = 0; i < (context->compilation_order).count; ++i)
    {
        ((context->compilation_order).data)[(dfs.post)[i]] = i;
    }
    if ((context->options).flags&KAI_COMPILE_DEBUG)
    {
        for (Kai_u32 i = 28; i < (context->compilation_order).count; ++i)
        {
            Kai_u32 k = ((context->compilation_order).data)[i];
            Kai_u32 index = k>>1;
            Kai_u8 ch = 118;
            if (k&1)
            {
                ch = 116;
            }
            printf("%i%c ", index, ch);
        }
        printf("\n");
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__error_fatal(Kai_Compiler_Context* context, Kai_string message)
{
    (context->error)->result = KAI_ERROR_FATAL;
    (context->error)->message = message;
    return KAI_TRUE;
}

KAI_INTERNAL Kai_bool kai__value_to_number(Kai_Value value, Kai_Type_Info* type, Kai_Number* out_number)
{
    switch (type->id)
    {
        break; case KAI_TYPE_ID_INTEGER:
        {
            Kai_Type_Info_Integer* info = (Kai_Type_Info_Integer*)(type);
            Kai_Number num = ((Kai_Number){.d = 1});
            if (info->is_signed)
            {
                num.is_neg = value.s64<0;
                if (num.is_neg)
                {
                    num.n = (Kai_u64)(-value.s64);
                }
                else
                {
                    num.n = (Kai_u64)(value.s64);
                }
            }
            else
            {
                num.n = value.u64;
            }
            *out_number = kai_number_normalize(num);
            return KAI_FALSE;
        }
        break; default:
        {
            return KAI_TRUE;
        }
    }
}

KAI_INTERNAL Kai_Value kai__evaluate_binary_operation(Kai_u32 op, Kai_Type_Info* type, Kai_Value a, Kai_Value b)
{
    switch (type->id)
    {
        break; case KAI_TYPE_ID_INTEGER:
        {
            Kai_Type_Info_Integer* info = (Kai_Type_Info_Integer*)(type);
            if (info->is_signed)
            {
                switch (info->bits)
                {
                    break; case 32:
                    {
                        switch (op)
                        {
                            break; case 43:
                            return ((Kai_Value){.s32 = a.s32+b.s32});
                            break; case 45:
                            return ((Kai_Value){.s32 = a.s32-b.s32});
                        }
                    }
                }
            }
            kai__todo("integer op = %i, is_signed = %i, bits = %i", op, info->is_signed, info->bits);
        }
        break; case KAI_TYPE_ID_FLOAT:
        {
            Kai_Type_Info_Float* info = (Kai_Type_Info_Float*)(type);
            switch (info->bits)
            {
                break; case 32:
                {
                    switch (op)
                    {
                        break; case 43:
                        return ((Kai_Value){.f32 = a.f32+b.f32});
                        break; case 45:
                        return ((Kai_Value){.f32 = a.f32-b.f32});
                    }
                }
            }
            kai__todo("float op = %i, bits = %i", op, info->bits);
        }
        break; case KAI_TYPE_ID_NUMBER:
        {
            switch (op)
            {
                break; case 43:
                return ((Kai_Value){.number = kai_number_add(a.number, b.number)});
                break; case 45:
                return ((Kai_Value){.number = kai_number_sub(a.number, b.number)});
                break; case 42:
                return ((Kai_Value){.number = kai_number_mul(a.number, b.number)});
                break; case 47:
                return ((Kai_Value){.number = kai_number_div(a.number, b.number)});
                break; case 15420:
                return ((Kai_Value){.number = kai_number_mul(a.number, ((Kai_Number){1, 1, (b.number).n, 0}))});
                break; case 15934:
                return ((Kai_Value){.number = kai_number_div(a.number, ((Kai_Number){1, 1, (b.number).n, 0}))});
            }
            kai__todo("number op = %i", op);
        }
        break; default:
        {
            kai__todo("type.id = %i", type->id);
        }
    }
    return ((Kai_Value){0});
}

KAI_INTERNAL Kai_bool kai__type_check(Kai_Compiler_Context* context, Kai_Expr* expr, Kai_Type_Info** out_or_expected)
{
    Kai_Type_Info* expected = *out_or_expected;
    Kai_Allocator* allocator = &context->allocator;
    switch (expr->id)
    {
        break; case KAI_EXPR_IDENTIFIER:
        {
            Kai_Node_Reference ref = kai__lookup_node(context, expr->source_code);
            if (ref.flags&KAI_NODE_NOT_FOUND)
            {
                Kai_Node* node = &((context->nodes).data)[(context->current_node).index];
                Kai_Location location = ((Kai_Location){.source = (node->location).source, .string = expr->source_code, .line = expr->line_number});
                return kai__error_not_declared(context, location);
            }
            if (ref.flags&KAI_NODE_LOCAL)
            {
                Kai_Local_Node* local_node = &((context->local_nodes).data)[ref.index];
                if (local_node->type==NULL)
                {
                    return kai__error_fatal(context, KAI_STRING("local node type cannot be null"));
                }
                if (expected==NULL)
                {
                    *out_or_expected = local_node->type;
                    expr->this_type = local_node->type;
                    return KAI_FALSE;
                }
                if (local_node->type!=expected)
                    return kai__error_fatal(context, KAI_STRING("oh no, not same type, bad"));
                expr->this_type = expected;
                return KAI_FALSE;
            }
            Kai_Node* node = &((context->nodes).data)[ref.index];
            if (node->type==NULL)
            {
                return kai__error_fatal(context, KAI_STRING("node type cannot be null"));
            }
            if (expected==NULL)
            {
                expr->this_type = node->type;
                return KAI_FALSE;
            }
            if (node->type!=expected)
                return kai__error_fatal(context, KAI_STRING("oh no, types not same for node"));
            expr->this_type = expected;
        }
        break; case KAI_EXPR_PROCEDURE:
        {
            Kai_Expr_Procedure* p = (Kai_Expr_Procedure*)(expr);
            kai_assert(expected->id==KAI_TYPE_ID_PROCEDURE);
            Kai_Type_Info_Procedure* pt = (Kai_Type_Info_Procedure*)(expected);
            kai_array_push(&context->scopes, ((Kai_Scope){0}));
            Kai_Scope* scope = &kai_array_last(&context->scopes);
            Kai_u32 local_node_count = (context->local_nodes).count;
            Kai_Expr* current = p->in_out_expr;
            for (Kai_u32 i = 0; i < p->in_count; ++i)
            {
                Kai_Type type = ((pt->inputs).data)[i];
                Kai_Node_Reference ref = ((Kai_Node_Reference){.flags = KAI_NODE_LOCAL, .index = (context->local_nodes).count});
                kai_array_push(&context->local_nodes, ((Kai_Local_Node){.type = type, .location = ((Kai_Location){.string = current->name, .line = current->line_number})}));
                kai_table_set(&scope->identifiers, current->name, ref);
                current = current->next;
            }
            kai_assert((pt->outputs).count<=1);
            if (kai__type_check(context, p->body, &((pt->outputs).data)[0]))
                return KAI_TRUE;
            (context->local_nodes).count = local_node_count;
            kai_array_pop(&context->scopes);
            p->this_type = expected;
        }
        break; case KAI_EXPR_NUMBER:
        {
            Kai_Expr_Number* n = (Kai_Expr_Number*)(expr);
            if (expected==NULL)
            {
                *out_or_expected = context->number_type;
                n->this_type = context->number_type;
                return KAI_FALSE;
            }
            switch (expected->id)
            {
                break; case KAI_TYPE_ID_INTEGER:
                {
                    if (!kai_number_is_integer(n->value))
                        return kai__error_fatal(context, KAI_STRING("not integer"));
                }
                break; case KAI_TYPE_ID_FLOAT:
                break; case KAI_TYPE_ID_NUMBER:
                break; default:
                {
                    return kai__error_fatal(context, KAI_STRING("cannot convert from number to unknown type"));
                }
            }
            n->this_type = expected;
        }
        break; case KAI_EXPR_BINARY:
        {
            Kai_Expr_Binary* b = (Kai_Expr_Binary*)(expr);
            switch (b->op)
            {
                break; case 15420:
                case 15934:
                {
                    Kai_Type_Info* lt = expected;
                    if (kai__type_check(context, b->left, &lt))
                        return KAI_TRUE;
                    Kai_Type_Info* rt = 0;
                    if (kai__type_check(context, b->right, &rt))
                        return KAI_TRUE;
                    *out_or_expected = lt;
                    b->this_type = lt;
                    if (expected==NULL)
                        return KAI_FALSE;
                    if (lt!=expected)
                        return kai__error_fatal(context, KAI_STRING("shift not good type"));
                    return KAI_FALSE;
                }
                break; case 15677:
                case 15649:
                case 60:
                case 62:
                case 15676:
                case 15678:
                {
                    if (expected!=NULL&&expected->id!=KAI_TYPE_ID_BOOLEAN)
                        return kai__error_fatal(context, KAI_STRING("must expect bool here"));
                    Kai_Type_Info* lt = 0;
                    Kai_Type_Info* rt = 0;
                    if (kai__type_check(context, b->left, &lt))
                        return KAI_TRUE;
                    if (lt->id==KAI_TYPE_ID_NUMBER)
                    {
                        if (kai__type_check(context, b->right, &rt))
                            return KAI_TRUE;
                        lt = rt;
                        if (kai__type_check(context, b->left, &lt))
                            return KAI_TRUE;
                    }
                    else
                    {
                        rt = lt;
                        if (kai__type_check(context, b->right, &rt))
                            return KAI_TRUE;
                    }
                    if (lt!=rt)
                        return kai__error_fatal(context, KAI_STRING("types no match, comparison"));
                    *out_or_expected = context->bool_type;
                    b->this_type = context->bool_type;
                    return KAI_FALSE;
                }
                break; case 15917:
                {
                    Kai_Type_Info* lt = NULL;
                    if (kai__type_check(context, b->left, &lt))
                        return KAI_TRUE;
                    Kai_Type_Info* rt = context->type_type;
                    if (kai__type_check(context, b->right, &rt))
                        return KAI_TRUE;
                    Kai_Value rv = {0};
                    if (kai__value_of_expression(context, b->right, &rv, &rt))
                        return KAI_TRUE;
                    *out_or_expected = rv.type;
                    if (expected==NULL)
                    {
                        b->this_type = rv.type;
                        return KAI_FALSE;
                    }
                    if (rv.type!=expected)
                        return kai__error_fatal(context, KAI_STRING("cast invalid"));
                    b->this_type = expected;
                    return KAI_FALSE;
                }
                break; default:
                {
                    Kai_Type_Info* lt = expected;
                    Kai_Type_Info* rt = expected;
                    if (expected!=NULL)
                    {
                        if (kai__type_check(context, b->right, &rt))
                            return KAI_TRUE;
                        if (kai__type_check(context, b->left, &lt))
                            return KAI_TRUE;
                        if (lt!=rt)
                            return kai__error_fatal(context, KAI_STRING("types no match, binary"));
                    }
                    else
                    {
                        if (kai__type_check(context, b->left, &lt))
                            return KAI_TRUE;
                        if (lt->id==KAI_TYPE_ID_NUMBER)
                        {
                            if (kai__type_check(context, b->right, &rt))
                                return KAI_TRUE;
                            lt = rt;
                            if (kai__type_check(context, b->left, &lt))
                                return KAI_TRUE;
                        }
                        else
                        {
                            rt = lt;
                            if (kai__type_check(context, b->right, &rt))
                                return KAI_TRUE;
                        }
                    }
                    b->this_type = lt;
                }
            }
        }
        break; case KAI_STMT_RETURN:
        {
            Kai_Stmt_Return* r = (Kai_Stmt_Return*)(expr);
            if (kai__type_check(context, r->expr, out_or_expected))
                return KAI_TRUE;
        }
        break; case KAI_STMT_IF:
        {
            Kai_Stmt_If* i = (Kai_Stmt_If*)(expr);
            Kai_Type_Info* expected = context->bool_type;
            if (kai__type_check(context, i->expr, &expected))
                return KAI_TRUE;
            if (i->then_body!=NULL&&kai__type_check(context, i->then_body, out_or_expected))
                return KAI_TRUE;
            if (i->else_body!=NULL&&kai__type_check(context, i->else_body, out_or_expected))
                return KAI_TRUE;
        }
        break; case KAI_STMT_COMPOUND:
        {
            Kai_Stmt_Compound* c = (Kai_Stmt_Compound*)(expr);
            Kai_Stmt* current = c->head;
            while (current)
            {
                if (kai__type_check(context, current, out_or_expected))
                    return KAI_TRUE;
                current = current->next;
            }
        }
        break; default:
        {
            kai__todo("expr.id = %i", expr->id);
        }
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__value_of_expression(Kai_Compiler_Context* context, Kai_Expr* expr, Kai_Value* out_value, Kai_Type* out_type)
{
    kai_assert(expr!=NULL);
    switch (expr->id)
    {
        break; case KAI_EXPR_IDENTIFIER:
        {
            Kai_Node_Reference ref = kai__lookup_node(context, expr->source_code);
            if (ref.flags&KAI_NODE_NOT_FOUND)
                return kai__error_fatal(context, KAI_STRING("cant find node [todo]"));
            if (ref.flags&KAI_NODE_LOCAL)
                return kai__error_fatal(context, KAI_STRING("expression cannot have local identifiers"));
            Kai_Node* node = &((context->nodes).data)[ref.index];
            if (!(node->flags&KAI_NODE_EVALUATED))
                return kai__error_fatal(context, KAI_STRING("node not evaluated [todo]"));
            *out_value = node->value;
            *out_type = node->type;
            return KAI_FALSE;
        }
        break; case KAI_EXPR_NUMBER:
        {
            Kai_Expr_Number* n = (Kai_Expr_Number*)(expr);
            *out_value = ((Kai_Value){.number = n->value});
            *out_type = context->number_type;
            return KAI_FALSE;
        }
        break; case KAI_EXPR_SPECIAL:
        {
            Kai_Expr_Special* s = (Kai_Expr_Special*)(expr);
            switch (s->kind)
            {
                break; case KAI_SPECIAL_TYPE:
                {
                    out_value->type = context->type_type;
                    *out_type = context->type_type;
                    return KAI_FALSE;
                }
            }
            kai__todo("special value not implemented");
        }
        break; case KAI_EXPR_UNARY:
        {
            Kai_Expr_Unary* u = (Kai_Expr_Unary*)(expr);
            Kai_Type_Info* et = 0;
            Kai_Value ev = {0};
            if (kai__value_of_expression(context, u->expr, &ev, &et))
                return KAI_TRUE;
            if (u->op==42)
            {
                if (et->id==KAI_TYPE_ID_TYPE)
                {
                    Kai_Type_Info_Pointer* pt = (Kai_Type_Info_Pointer*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Pointer)));
                    pt->id = KAI_TYPE_ID_POINTER;
                    pt->sub_type = ev.type;
                    out_value->type = (Kai_Type)(pt);
                    *out_type = context->type_type;
                    return KAI_FALSE;
                }
                else
                {
                    kai__todo("pointer to value");
                }
            }
            kai__todo("unary operator");
        }
        break; case KAI_EXPR_BINARY:
        {
            Kai_Expr_Binary* b = (Kai_Expr_Binary*)(expr);
            Kai_Type_Info* lt = 0;
            Kai_Type_Info* rt = 0;
            Kai_Value lv = {0};
            Kai_Value rv = {0};
            if (kai__value_of_expression(context, b->left, &lv, &lt))
                return KAI_TRUE;
            if (kai__value_of_expression(context, b->right, &rv, &rt))
                return KAI_TRUE;
            if (lt!=rt)
            {
                if (lt->id==KAI_TYPE_ID_NUMBER)
                {
                    if (kai__value_to_number(rv, rt, &rv.number))
                        return kai__error_fatal(context, KAI_STRING("cannot convert value to number"));
                    rt = context->number_type;
                }
                else
                if (rt->id==KAI_TYPE_ID_NUMBER)
                {
                    if (kai__value_to_number(lv, lt, &lv.number))
                        return kai__error_fatal(context, KAI_STRING("cannot convert value to number"));
                    lt = context->number_type;
                }
                else
                    kai__error_fatal(context, KAI_STRING("invalid binary expression [todo]"));
            }
            *out_value = kai__evaluate_binary_operation(b->op, lt, lv, rv);
            *out_type = lt;
            return KAI_FALSE;
        }
        break; case KAI_EXPR_PROCEDURE:
        {
            Kai_Node* current_node = &((context->nodes).data)[(context->current_node).index];
            Kai_Type_Info* expected_type = current_node->type;
            kai__type_check(context, expr, &expected_type);
            out_value->ptr = expr;
            return KAI_FALSE;
        }
        break; case KAI_EXPR_STRUCT:
        {
            Kai_Expr_Struct* s = (Kai_Expr_Struct*)(expr);
            Kai_Type_Info_Struct* st = (Kai_Type_Info_Struct*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Struct)));
            st->id = KAI_TYPE_ID_STRUCT;
            (st->fields).count = s->field_count;
            (st->fields).data = (Kai_Struct_Field*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Struct_Field)*(st->fields).count));
            st->size = 0;
            Kai_Expr* current = s->head;
            for (Kai_u32 i = 0; i < s->field_count; ++i)
            {
                kai_assert(current->id==KAI_STMT_DECLARATION);
                Kai_Stmt_Declaration* d = (Kai_Stmt_Declaration*)(current);
                Kai_Type_Info* type = 0;
                Kai_Value value = {0};
                if (kai__value_of_expression(context, d->type, &value, &type))
                    return KAI_TRUE;
                if (type->id!=KAI_TYPE_ID_TYPE)
                    return KAI_TRUE;
                ((st->fields).data)[i] = ((Kai_Struct_Field){.name = d->name, .offset = st->size, .type = value.type});
                st->size += kai__type_size(value.type);
                current = current->next;
            }
            out_value->type = (Kai_Type)(st);
            *out_type = context->type_type;
            return KAI_FALSE;
        }
        break; case KAI_EXPR_PROCEDURE_TYPE:
        {
            Kai_Expr_Procedure_Type* p = (Kai_Expr_Procedure_Type*)(expr);
            Kai_Type_Info_Procedure* pt = (Kai_Type_Info_Procedure*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Procedure)));
            pt->id = KAI_TYPE_ID_PROCEDURE;
            (pt->inputs).count = p->in_count;
            (pt->inputs).data = (Kai_Type*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type)*(pt->inputs).count));
            (pt->outputs).count = p->out_count;
            (pt->outputs).data = (Kai_Type*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type)*(pt->outputs).count));
            Kai_Expr* current = p->in_out_expr;
            for (Kai_u32 i = 0; i < p->in_count; ++i)
            {
                Kai_Type_Info* type = 0;
                Kai_Value value = {0};
                if (kai__value_of_expression(context, current, &value, &type))
                    return KAI_TRUE;
                if (type->id!=KAI_TYPE_ID_TYPE)
                    return KAI_TRUE;
                ((pt->inputs).data)[i] = value.type;
                current = current->next;
            }
            for (Kai_u32 i = 0; i < p->out_count; ++i)
            {
                Kai_Type_Info* type = 0;
                Kai_Value value = {0};
                if (kai__value_of_expression(context, current, &value, &type))
                    return KAI_TRUE;
                if (type->id!=KAI_TYPE_ID_TYPE)
                    return KAI_TRUE;
                if ((value.type)->id==KAI_TYPE_ID_VOID&&p->out_count==1)
                {
                    (pt->outputs).count = 0;
                    break;
                }
                ((pt->outputs).data)[i] = value.type;
                current = current->next;
            }
            out_value->type = (Kai_Type)(pt);
            *out_type = context->type_type;
            return KAI_FALSE;
        }
        break; default:
        {
            kai__todo("%s (expr.id = %i)", __FUNCTION__, expr->id);
        }
    }
    return KAI_TRUE;
}

KAI_INTERNAL Kai_Type kai__type_of_expression(Kai_Compiler_Context* context, Kai_Expr* expr)
{
    kai_assert(expr!=NULL);
    switch (expr->id)
    {
        break; case KAI_EXPR_IDENTIFIER:
        {
            Kai_Node_Reference ref = kai__lookup_node(context, expr->source_code);
            if (ref.flags&KAI_NODE_NOT_FOUND)
            {
                kai__error_fatal(context, KAI_STRING("cant find node [todo]"));
                return NULL;
            }
            Kai_Node* node = &((context->nodes).data)[ref.index];
            if (!(node->flags&KAI_NODE_TYPE_EVALUATED))
            {
                kai__error_fatal(context, KAI_STRING("node type not evaluated [todo]"));
                return NULL;
            }
            return node->type;
        }
        break; case KAI_EXPR_NUMBER:
        {
            Kai_Node_Reference ref = kai__lookup_node(context, KAI_STRING("s32"));
            if (ref.flags&KAI_NODE_NOT_FOUND)
                return NULL;
            Kai_Node* node = &((context->nodes).data)[ref.index];
            return (node->value).type;
        }
        break; case KAI_EXPR_STRING:
        {
            kai__todo("strings");
        }
        break; case KAI_EXPR_SPECIAL:
        {
            Kai_Expr_Special* s = (Kai_Expr_Special*)(expr);
            switch (s->kind)
            {
                break; case KAI_SPECIAL_TYPE:
                {
                    return context->type_type;
                }
            }
            kai__todo("special value not implemented");
        }
        break; case KAI_EXPR_UNARY:
        {
            Kai_Expr_Unary* u = (Kai_Expr_Unary*)(expr);
            Kai_Type_Info* et = kai__type_of_expression(context, u->expr);
            if (et==NULL)
                if (u->op==42)
                {
                    Kai_Type_Info_Pointer* pt = (Kai_Type_Info_Pointer*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Pointer)));
                    pt->id = KAI_TYPE_ID_POINTER;
                    pt->sub_type = et;
                    return (Kai_Type)(pt);
                }
            return et;
        }
        break; case KAI_EXPR_BINARY:
        {
            Kai_Expr_Binary* b = (Kai_Expr_Binary*)(expr);
            if (b->op==15917)
            {
                Kai_Value rv = {0};
                Kai_Type_Info* rt = 0;
                if (kai__value_of_expression(context, b->right, &rv, &rt))
                    return NULL;
                if (rt->id!=KAI_TYPE_ID_TYPE)
                {
                    kai__todo("must cast to a type");
                    return NULL;
                }
                return rv.type;
            }
            Kai_Type_Info* lt = kai__type_of_expression(context, b->left);
            Kai_Type_Info* rt = kai__type_of_expression(context, b->right);
            if (lt!=rt)
            {
                if (lt->id==KAI_TYPE_ID_NUMBER)
                    return rt;
                if (rt->id==KAI_TYPE_ID_NUMBER)
                    return lt;
                kai__todo("binary expression with different types");
                return NULL;
            }
            return lt;
        }
        break; case KAI_EXPR_PROCEDURE:
        {
            Kai_Expr_Procedure* p = (Kai_Expr_Procedure*)(expr);
            Kai_Type_Info_Procedure* pt = (Kai_Type_Info_Procedure*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type_Info_Procedure)));
            pt->id = KAI_TYPE_ID_PROCEDURE;
            (pt->inputs).count = p->in_count;
            (pt->inputs).data = (Kai_Type*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type)*(pt->inputs).count));
            (pt->outputs).count = p->out_count;
            (pt->outputs).data = (Kai_Type*)(kai_arena_allocate(&context->type_allocator, sizeof(Kai_Type)*(pt->outputs).count));
            Kai_Expr* current = p->in_out_expr;
            for (Kai_u32 i = 0; i < p->in_count; ++i)
            {
                Kai_Type_Info* type = 0;
                Kai_Value value = {0};
                if (kai__value_of_expression(context, current, &value, &type))
                    return NULL;
                if (type->id!=KAI_TYPE_ID_TYPE)
                    return NULL;
                ((pt->inputs).data)[i] = value.type;
                current = current->next;
            }
            for (Kai_u32 i = 0; i < p->out_count; ++i)
            {
                Kai_Type_Info* type = 0;
                Kai_Value value = {0};
                if (kai__value_of_expression(context, current, &value, &type))
                    return NULL;
                if (type->id!=KAI_TYPE_ID_TYPE)
                    return NULL;
                ((pt->outputs).data)[i] = value.type;
                current = current->next;
            }
            return (Kai_Type)(pt);
        }
        break; case KAI_EXPR_STRUCT:
        case KAI_EXPR_PROCEDURE_TYPE:
        {
            return context->type_type;
        }
        break; default:
        {
            kai__todo("%s (expr.id = %i)", __FUNCTION__, expr->id);
        }
    }
    return NULL;
}

KAI_INTERNAL Kai_bool kai__compile_node_value(Kai_Compiler_Context* context, Kai_Node* node)
{
    Kai_Type_Info* type = node->type;
    Kai_Value value = {0};
    if (node->expr!=NULL&&kai__value_of_expression(context, node->expr, &value, &type))
        return KAI_TRUE;
    if (type==NULL&&(node->type)->id==KAI_TYPE_ID_PROCEDURE)
    {
        node->value = value;
        return KAI_FALSE;
    }
    if (type!=node->type)
    {
        if (type->id==KAI_TYPE_ID_NUMBER)
        {
            switch ((node->type)->id)
            {
                break; case KAI_TYPE_ID_BOOLEAN:
                {
                    if ((value.number).n!=0&&((((value.number).n!=1||(value.number).d!=1)||(value.number).e!=0)||(value.number).is_neg!=0))
                        return kai__error_fatal(context, KAI_STRING("cannot convert number to bool"));
                    (node->value).u8 = (Kai_u8)((value.number).n);
                    return KAI_FALSE;
                }
                break; case KAI_TYPE_ID_INTEGER:
                {
                    (node->value).u64 = kai_number_to_u64(value.number);
                    return KAI_FALSE;
                }
                break; case KAI_TYPE_ID_FLOAT:
                {
                    Kai_Type_Info_Float* type_info = (Kai_Type_Info_Float*)(node->type);
                    Kai_f64 fv = kai_number_to_f64(value.number);
                    if (type_info->bits==32)
                        (node->value).f32 = (Kai_f32)(fv);
                    else
                        (node->value).f64 = fv;
                    return KAI_FALSE;
                }
            }
        }
        return kai__error_fatal(context, KAI_STRING("cannot convert [todo]"));
    }
    node->value = value;
    return KAI_FALSE;
}

KAI_INTERNAL Kai_Import* kai__find_import(Kai_Compiler_Context* context, Kai_string name)
{
    for (Kai_u32 i = 0; i < (context->imports).count; ++i)
    {
        Kai_Import* import = &((context->imports).data)[i];
        if (kai_string_equals(import->name, name))
            return import;
    }
    return NULL;
}

KAI_INTERNAL Kai_Expr* kai__expression_from_string(Kai_Compiler_Context* context, Kai_string s)
{
    Kai_Parser parser = {0};
    (parser.tokenizer).source = s;
    (parser.tokenizer).line_number = 1;
    parser.error = context->error;
    parser.arena = context->temp_allocator;
    kai_tokenizer_next(&parser.tokenizer);
    Kai_Expr* type = kai_parse_type_expression(&parser);
    if (type==NULL)
    {
        kai__error_fatal(context, KAI_STRING("could not parse string"));
    }
    context->temp_allocator = parser.arena;
    return type;
}

KAI_INTERNAL Kai_bool kai__compile_node_type(Kai_Compiler_Context* context, Kai_Node* node)
{
    Kai_Import* import = 0;
    if (node->flags&KAI_NODE_IMPORT)
    {
        import = kai__find_import(context, (node->location).string);
        if (import==NULL)
            return kai__error_fatal(context, KAI_STRING("cannot find import"));
        if (node->type_expr==NULL)
        {
            if ((import->type).count==0)
                return kai__error_fatal(context, KAI_STRING("host import must be typed"));
            node->type_expr = kai__expression_from_string(context, import->type);
            if (node->type_expr==NULL)
                return KAI_TRUE;
        }
        node->value = import->value;
    }
    if (node->type_expr!=NULL)
    {
        Kai_Type_Info* type = 0;
        Kai_Value value = {0};
        if (kai__value_of_expression(context, node->type_expr, &value, &type))
            return KAI_TRUE;
        if (type==NULL)
            return KAI_TRUE;
        if (type->id!=KAI_TYPE_ID_TYPE)
            return kai__error_fatal(context, KAI_STRING("type is not type"));
        if (node->flags&KAI_NODE_IMPORT&&(import->type).count!=0)
        {
            Kai_Expr* import_type_expr = kai__expression_from_string(context, import->type);
            if (import_type_expr==NULL)
                return KAI_TRUE;
            Kai_Type_Info* import_type_type = 0;
            Kai_Value import_type_value = {0};
            if (kai__value_of_expression(context, import_type_expr, &import_type_value, &import_type_type))
                return KAI_TRUE;
            if (import_type_type==NULL)
                return KAI_TRUE;
            if (import_type_type->id!=KAI_TYPE_ID_TYPE)
                return kai__error_fatal(context, KAI_STRING("import expr type is not type"));
            if (value.type!=import_type_value.type)
                return kai__error_fatal(context, KAI_STRING("import type does not match declaration"));
        }
        node->type = value.type;
        node->flags |= KAI_NODE_TYPE_EVALUATED;
        return KAI_FALSE;
    }
    Kai_Type type = kai__type_of_expression(context, node->expr);
    if (type==NULL)
        return KAI_TRUE;
    node->type = type;
    node->flags |= KAI_NODE_TYPE_EVALUATED;
    return KAI_FALSE;
}

KAI_INTERNAL Kai_u32 kai__type_size(Kai_Type_Info* type)
{
    switch (type->id)
    {
        break; case KAI_TYPE_ID_TYPE:
        {
            return sizeof(Kai_Type);
        }
        break; case KAI_TYPE_ID_BOOLEAN:
        {
            return 1;
        }
        break; case KAI_TYPE_ID_INTEGER:
        {
            Kai_Type_Info_Integer* info = (Kai_Type_Info_Integer*)(type);
            return info->bits/8;
        }
        break; case KAI_TYPE_ID_FLOAT:
        {
            Kai_Type_Info_Float* info = (Kai_Type_Info_Float*)(type);
            return info->bits/8;
        }
        break; case KAI_TYPE_ID_POINTER:
        case KAI_TYPE_ID_PROCEDURE:
        {
            return sizeof(void*);
        }
        break; case KAI_TYPE_ID_STRING:
        case KAI_TYPE_ID_STRUCT:
        {
            Kai_Type_Info_Struct* info = (Kai_Type_Info_Struct*)(type);
            return info->size;
        }
    }
    kai__todo("type.id = %i", type->id);
    return 0;
}

KAI_INTERNAL void kai__copy_value(Kai_u8* out, Kai_Type_Info* type, Kai_Value value)
{
    switch (type->id)
    {
        break; case KAI_TYPE_ID_TYPE:
        {
            *((Kai_Type*)(out)) = value.type;
        }
        break; case KAI_TYPE_ID_BOOLEAN:
        {
            *((Kai_bool*)(out)) = value.u8;
        }
        break; case KAI_TYPE_ID_INTEGER:
        {
            Kai_Type_Info_Integer* info = (Kai_Type_Info_Integer*)(type);
            if (info->is_signed)
            {
                switch (info->bits)
                {
                    break; case 8:
                    *((Kai_s8*)(out)) = value.s8;
                    break; case 16:
                    *((Kai_s16*)(out)) = value.s16;
                    break; case 32:
                    *((Kai_s32*)(out)) = value.s32;
                    break; case 64:
                    *((Kai_s64*)(out)) = value.s64;
                }
            }
            else
            {
                switch (info->bits)
                {
                    break; case 8:
                    *((Kai_u8*)(out)) = value.u8;
                    break; case 16:
                    *((Kai_u16*)(out)) = value.u16;
                    break; case 32:
                    *((Kai_u32*)(out)) = value.u32;
                    break; case 64:
                    *((Kai_u64*)(out)) = value.u64;
                }
            }
        }
        break; case KAI_TYPE_ID_FLOAT:
        {
            Kai_Type_Info_Float* info = (Kai_Type_Info_Float*)(type);
            switch (info->bits)
            {
                break; case 32:
                *((Kai_f32*)(out)) = value.f32;
                break; case 64:
                *((Kai_f64*)(out)) = value.f64;
            }
        }
        break; case KAI_TYPE_ID_POINTER:
        case KAI_TYPE_ID_PROCEDURE:
        {
            *((void**)(out)) = value.ptr;
        }
        break; case KAI_TYPE_ID_STRING:
        {
            *((Kai_string*)(out)) = value.string;
        }
        break; case KAI_TYPE_ID_STRUCT:
        {
            Kai_Type_Info_Struct* info = (Kai_Type_Info_Struct*)(type);
            if (info->size<=sizeof(Kai_Value))
                memcpy(out, value.inline_struct, info->size);
            else
                memcpy(out, value.ptr, info->size);
        }
        break; default:
        {
            kai__todo("type.id = %i", type->id);
        }
    }
}

KAI_INTERNAL Kai_u32 kai__push_value(Kai_Compiler_Context* context, Kai_Type_Info* type, Kai_Value value)
{
    Kai_Allocator* allocator = &context->allocator;
    Kai_u32 size = kai__max_u32(kai__type_size(type), 8);
    Kai_u32 location = ((context->program)->data).count;
    kai_array_grow(&(context->program)->data, size);
    kai__copy_value(((context->program)->data).data+location, type, value);
    ((context->program)->data).count += size;
    return location;
}

KAI_INTERNAL Kai_bool kai__compile_all_nodes(Kai_Compiler_Context* context)
{
    Kai_Allocator* allocator = &context->allocator;
    for (Kai_u32 i = 0; i < (context->compilation_order).count; ++i)
    {
        Kai_u32 k = ((context->compilation_order).data)[i];
        Kai_Node_Reference ref = ((Kai_Node_Reference){.index = k>>1, .flags = k&1});
        Kai_Node* node = &((context->nodes).data)[ref.index];
        if ((node->flags&KAI_NODE_EVALUATED)==KAI_NODE_EVALUATED)
            continue;
        context->current_source = (node->location).source;
        context->current_node = ref;
        Kai_Writer* writer = &context->debug_writer;
        if (ref.flags&KAI_NODE_TYPE)
        {
            if ((context->options).flags&KAI_COMPILE_DEBUG)
            {
                printf("compiling typeof(%.*s)\n", (Kai_s32)(((node->location).string).count), ((node->location).string).data);
            }
            if (kai__compile_node_type(context, node))
                return KAI_TRUE;
            if ((context->options).flags&KAI_COMPILE_DEBUG)
            {
                printf("=> ");
                kai_write_type(&context->debug_writer, node->type);
                printf("\n");
            }
        }
        else
        {
            if ((context->options).flags&KAI_COMPILE_DEBUG)
            {
                printf("compiling (%.*s)\n", (Kai_s32)(((node->location).string).count), ((node->location).string).data);
            }
            if (!(node->flags&KAI_NODE_IMPORT))
            {
                if (kai__compile_node_value(context, node))
                    return KAI_TRUE;
            }
            if ((context->options).flags&KAI_COMPILE_DEBUG)
            {
                printf("=> ");
                switch ((node->type)->id)
                {
                    break; case KAI_TYPE_ID_NUMBER:
                    {
                        kai_write_number(writer, (node->value).number);
                    }
                    break; case KAI_TYPE_ID_INTEGER:
                    {
                        Kai_Type_Info_Integer* info = (Kai_Type_Info_Integer*)(node->type);
                        if (info->is_signed)
                            kai__write_s64((node->value).s64);
                        else
                            kai__write_u64((node->value).u64);
                    }
                    break; case KAI_TYPE_ID_FLOAT:
                    {
                        kai__write_f64((Kai_f64)((node->value).f32));
                    }
                    break; case KAI_TYPE_ID_PROCEDURE:
                    {
                        kai__write("0x");
                        (context->debug_writer).write_value((context->debug_writer).user, KAI_U64, ((Kai_Value){.u64 = (node->value).u64}), ((Kai_Write_Format){.flags = KAI_WRITE_FLAGS_HEXIDECIMAL}));
                    }
                }
                printf("\n");
            }
            if (node->flags&KAI_NODE_EXPORT)
            {
                Kai_u32 location = kai__push_value(context, node->type, node->value);
                kai_table_set(&(context->program)->variable_table, (node->location).string, ((Kai_Variable){.type = node->type, .location = location}));
            }
        }
    }
    return KAI_FALSE;
}

KAI_INTERNAL Kai_bool kai__generate_compiler_ir(Kai_Compiler_Context* context)
{
    (void)(context);
    return KAI_FALSE;
}

KAI_API(Kai_Result) kai_create_program(Kai_Program_Create_Info* info, Kai_Program* out_program)
{
    Kai_Compiler_Context context = ((Kai_Compiler_Context){.error = info->error, .allocator = info->allocator, .program = out_program, .options = info->options, .imports = info->imports, .debug_writer = kai_writer_stdout()});
    kai_arena_create(&context.type_allocator, &info->allocator);
    kai_arena_create(&context.temp_allocator, &info->allocator);
    if (!((info->options).flags&KAI_COMPILE_NO_CODE_GEN))
    {
        (context.error)->message = KAI_STRING("Code generation not currently supported :(");
        (context.error)->result = KAI_ERROR_FATAL;
        return KAI_ERROR_FATAL;
    }
    while ((context.error)->result==KAI_SUCCESS)
    {
        if (kai__create_syntax_trees(&context, info->sources))
            break;
        if (kai__generate_dependency_graph(&context))
            break;
        if (kai__generate_compilation_order(&context))
            break;
        if (kai__compile_all_nodes(&context))
            break;
        break;
    }
    return (context.error)->result;
}

KAI_API(void) kai_destroy_program(Kai_Program* program)
{
    (void)(program);
}

KAI_API(void*) kai_find_variable(Kai_Program* program, Kai_string name, Kai_Type* out_type)
{
    Kai_int index = kai_table_find(&program->variable_table, name);
    if (index==-1)
        return NULL;
    Kai_Variable var = ((program->variable_table).values)[index];
    if (out_type!=NULL)
    {
        *out_type = var.type;
    }
    return (program->data).data+var.location;
}

KAI_API(void*) kai_find_procedure(Kai_Program* program, Kai_string name, Kai_string type)
{
    (void)(type);
    Kai_Type_Info* t = 0;
    void* ptr = kai_find_variable(program, name, &t);
    if (t==NULL||t->id!=KAI_TYPE_ID_PROCEDURE)
        return NULL;
    return ptr;
}

#ifndef KAI_DONT_USE_WRITER_API

#if defined(KAI_PLATFORM_WINDOWS)
__declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);
KAI_INTERNAL void kai__setup_utf8_stdout(void) {
    SetConsoleOutputCP(65001);
    setlocale(LC_CTYPE, ".UTF8");
}
#else
KAI_INTERNAL void kai__setup_utf8_stdout(void) {
    setlocale(LC_CTYPE, ".UTF8");
}
#endif
#if defined(KAI_PLATFORM_WINDOWS)
KAI_INTERNAL FILE* kai__stdc_file_open(char const* path, char const* mode) {
    FILE* handle = NULL;
    fopen_s(&handle, path, mode); // this is somehow more safe? :/
    return (void*)handle;
}
#else
#    define kai__stdc_file_open fopen
#endif

static Kai_cstring kai__term_debug_colors[6] = {
    "\x1b[0;37m", "\x1b[1;97m", "\x1b[1;91m", "\x1b[1;94m", "\x1b[0;90m", "\x1b[0;92m"
};

KAI_INTERNAL void kai__file_writer_write_value(void* user, Kai_u32 type, Kai_Value value, Kai_Write_Format format)
{
    if (user==NULL)
        return;
    FILE* f = (FILE*)(user);
    if (type==KAI_FILL)
    {
        if (format.min_count==0)
            return;
        for (Kai_u32 i = 0; i < format.min_count; ++i)
        {
            fputc(format.fill_character, f);
        }
    }
    else
    if (type==KAI_F64)
    {
        fprintf(f, "%f", value.f64);
    }
    else
    if (type==KAI_S32)
    {
        fprintf(f, "%i", value.s32);
    }
    else
    if (type==KAI_U32)
    {
        fprintf(f, "%u", value.u32);
    }
    else
    if (type==KAI_S64)
    {
        fprintf(f, "%lli", value.s64);
    }
    else
    if (type==KAI_U64)
    {
        Kai_u8 base = 117;
        if (format.flags&KAI_WRITE_FLAGS_HEXIDECIMAL)
            base = 88;
        if (format.min_count!=0)
        {
            if (format.fill_character!=0)
            {
                char fmt[7] = "%.*ll.";
                fmt[1] = format.fill_character;
                fmt[5] = base;
                fprintf(f, fmt, format.min_count, value.u64);
            }
            else
            {
                char fmt[7] = "%*ll.";
                fmt[4] = base;
                fprintf(f, fmt, format.min_count, value.u64);
            }
        }
        else
        {
            char fmt[5] = "%ll.";
            fmt[3] = base;
            fprintf(f, fmt, value.u64);
        }
    }
    else
    {
        fprintf(f, "[Writer] -- Case not defined for type %i\n", type);
    }
}

KAI_INTERNAL void kai__file_writer_write_string(void* user, Kai_string s)
{
    if (user==NULL)
        return;
    FILE* f = (FILE*)(user);
    fwrite(s.data, 1, s.count, f);
}

KAI_INTERNAL void kai__stdout_writer_write_value(void* user, Kai_u32 type, Kai_Value value, Kai_Write_Format format)
{
    (void)(user);
    kai__file_writer_write_value(stdout, type, value, format);
}

KAI_INTERNAL void kai__stdout_writer_write_string(void* user, Kai_string s)
{
    (void)(user);
    fwrite(s.data, 1, s.count, stdout);
}

KAI_INTERNAL void kai__stdout_writer_set_color(void* user, Kai_Write_Color color)
{
    (void)(user);
    printf("%s", kai__term_debug_colors[color]);
}

KAI_API(Kai_Writer) kai_writer_stdout(void)
{
    kai__setup_utf8_stdout();
    return ((Kai_Writer){.write_string = kai__stdout_writer_write_string, .write_value = kai__stdout_writer_write_value, .set_color = kai__stdout_writer_set_color, .user = NULL});
}

KAI_API(Kai_Writer) kai_writer_file_open(Kai_cstring path)
{
    return ((Kai_Writer){.write_string = kai__file_writer_write_string, .write_value = kai__file_writer_write_value, .set_color = NULL, .user = kai__stdc_file_open(path, "wb")});
}

KAI_API(void) kai_writer_file_close(Kai_Writer* writer)
{
    FILE* f = (FILE*)(writer->user);
    if (f!=NULL)
        fclose(f);
}

#endif
#ifndef KAI_DONT_USE_MEMORY_API

#if defined(KAI_PLATFORM_LINUX) || defined(KAI_PLATFORM_APPLE)
#include <sys/mman.h> // -> mmap
#include <unistd.h>   // -> getpagesize
KAI_INTERNAL void* kai__memory_platform_allocate(void* user, void* ptr, Kai_u32 size, Kai_u32 op)
{
	Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(user);
	switch (op) {
	case KAI_MEMORY_COMMAND_ALLOCATE_WRITE_ONLY: {
        void* new_ptr = mmap(NULL, size, PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
        if (new_ptr == MAP_FAILED)
            return NULL;
        metadata->total_allocated += size;
        return new_ptr;
	}
	case KAI_MEMORY_COMMAND_SET_EXECUTABLE: {
        kai_assert(mprotect(ptr, size, PROT_EXEC) == 0);
		return NULL;
	}
	case KAI_MEMORY_COMMAND_FREE: {
        kai_assert(munmap(ptr, size) == 0);
        metadata->total_allocated -= size;
	}
	}
    return NULL;
}
#define kai__page_size() (Kai_u32)sysconf(_SC_PAGESIZE)
#elif defined(KAI_PLATFORM_WINDOWS)
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef uintptr_t           SIZE_T;
typedef struct _SYSTEM_INFO SYSTEM_INFO;
__declspec(dllimport) void* __stdcall VirtualAlloc(void* lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
__declspec(dllimport) BOOL __stdcall VirtualProtect(void* lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD* lpflOldProtect);
__declspec(dllimport) BOOL __stdcall VirtualFree(void* lpAddress, SIZE_T dwSize, DWORD dwFreeType);
__declspec(dllimport) void __stdcall GetSystemInfo(SYSTEM_INFO* lpSystemInfo);
KAI_INTERNAL void* kai__memory_platform_allocate(void* user, void* ptr, Kai_u32 size, Kai_u32 op)
{
	Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(user);
	switch (op) {
	case KAI_MEMORY_COMMAND_ALLOCATE_WRITE_ONLY: {
		void* result = VirtualAlloc(NULL, size, 0x1000|0x2000, 0x04);
		kai_assert(result != NULL);
		metadata->total_allocated += size;
		return result;
	}
	case KAI_MEMORY_COMMAND_SET_EXECUTABLE: {
		DWORD old;
		kai_assert(VirtualProtect(ptr, size, 0x10, &old) != 0);
		return NULL;
	}
	case KAI_MEMORY_COMMAND_FREE: {
		kai_assert(VirtualFree(ptr, 0, 0x8000) != 0);
		metadata->total_allocated -= size;
	}
	}
    return NULL;
}
KAI_INTERNAL Kai_u32 kai__page_size(void)
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
KAI_INTERNAL void* kai__memory_heap_allocate(void* user, void* old_ptr, Kai_u32 new_size, Kai_u32 old_size)
{
    void* ptr = NULL;
    if (new_size==0)
    {
        free(old_ptr);
    }
    else
    {
        kai_assert(old_ptr!=NULL||old_size==0);
        ptr = realloc(old_ptr, new_size);
        kai_assert(ptr!=NULL);
        if (ptr!=NULL)
            kai__memory_zero((Kai_u8*)(ptr)+old_size, new_size-old_size);
    }
    Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(user);
    metadata->total_allocated += new_size;
    metadata->total_allocated -= old_size;
    return ptr;
}

KAI_API(Kai_Result) kai_memory_create(Kai_Allocator* out_allocator)
{
    kai_assert(out_allocator!=NULL);
    out_allocator->platform_allocate = kai__memory_platform_allocate;
    out_allocator->heap_allocate = kai__memory_heap_allocate;
    out_allocator->page_size = kai__page_size();
    out_allocator->user = realloc(NULL, sizeof(Kai_Memory_Metadata));
    if (!out_allocator->user)
    {
        return KAI_MEMORY_ERROR_OUT_OF_MEMORY;
    }
    Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(out_allocator->user);
    metadata->total_allocated = 0;
    return KAI_SUCCESS;
}

KAI_API(Kai_Result) kai_memory_destroy(Kai_Allocator* allocator)
{
    kai_assert(allocator!=NULL);
    if (allocator->user==NULL)
        return KAI_ERROR_FATAL;
    Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(allocator->user);
    if (metadata->total_allocated!=0)
        return KAI_MEMORY_ERROR_MEMORY_LEAK;
    free(allocator->user);
    *allocator = ((Kai_Allocator){0});
    return KAI_SUCCESS;
}

KAI_API(Kai_u64) kai_memory_usage(Kai_Allocator* allocator)
{
    Kai_Memory_Metadata* metadata = (Kai_Memory_Metadata*)(allocator->user);
    return metadata->total_allocated;
}

#endif
#endif // KAI_IMPLEMENTATION

#ifdef __cplusplus
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
