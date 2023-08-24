#ifndef KAI_CORE_H
#define KAI_CORE_H
#include <kai/config.h>
#include <stdint.h>
__KAI_BEGIN_API__

// Primitive Types

typedef uint8_t   kai_bool;

enum: kai_bool {
	kai_false = 0,
	kai_true  = 1,
};

typedef int8_t    kai_s8;
typedef int16_t   kai_s16;
typedef int32_t   kai_s32;
typedef int64_t   kai_s64;

typedef uint8_t   kai_u8;
typedef uint16_t  kai_u16;
typedef uint32_t  kai_u32;
typedef uint64_t  kai_u64;

typedef kai_s64   kai_int;

typedef double    kai_f64;
typedef float     kai_f32;

typedef void*     kai_ptr;

typedef struct kai_Type_Info* kai_Type;

// TODO: Add vector, matrix, quaternion types

typedef kai_f32 kai_f32v2[2];
typedef kai_f32 kai_f32v3[3];
typedef kai_f32 kai_f32v4[4];

typedef kai_f64 kai_f64v2[2];
typedef kai_f64 kai_f64v3[3];
typedef kai_f64 kai_f64v4[4];

// Base Language struct types

typedef struct {
	kai_int count;
	kai_u8* data;
} kai_str;

typedef struct {
	kai_int count;
	void*   data;
} kai_array;


//////////////////////////////////////////////////////////
// VERSION INFO

typedef struct {
	kai_s32 major;
	kai_s32 minor;
	kai_s32 patch;
} kai_Version;

KAI_API(void)
	kai_get_version(kai_Version* Version);

KAI_API(char const*)
	kai_get_version_string();


//////////////////////////////////////////////////////////
// Type Info

typedef enum: kai_u8 {
	kai_Type_Type      = 0, // No Type_Info struct for this
	kai_Type_Integer   = 1,
	kai_Type_Float     = 2,
	kai_Type_Pointer   = 3,
	kai_Type_Procedure = 4,
	kai_Type_Array     = 5,
	kai_Type_String    = 6,
	kai_Type_Struct    = 7,
} kai_Type_Enum;

typedef struct kai_Type_Info {
	kai_u8 type;
} kai_Type_Info;

typedef struct {
	kai_u8 type;
	kai_u8 bits;
	kai_bool is_signed;
} kai_Type_Info_Integer;

typedef struct {
	kai_u8 type;
	kai_u8 bits;
} kai_Type_Info_Float;

typedef struct {
	kai_u8 type;
	kai_Type sub_type;
} kai_Type_Info_Pointer;

typedef struct {
	kai_u8    type;
	kai_u16   param_count;
	kai_u16   ret_count;
	kai_Type* input_output; // list of parameters types, then return types
} kai_Type_Info_Procedure;

typedef struct {
	kai_u8 type;
	void* members;
} kai_Type_Info_Struct;

// Core Functions
// String Functions

KAI_API(bool)
	kai_string_equals(kai_str Left, kai_str Right);


//////////////////////////////////////////////////////////
// MEMORY

// Allocate "Byte_Count" number of bytes
typedef kai_ptr (*kai_fn_Memory_Allocate)(kai_ptr User, kai_int Byte_Count);

typedef struct {
	// There is NO "free" function
	//   Memory "object" is required to keep track of all the memory
	//   it allocates and free it when the compiler is no longer using it.
	kai_fn_Memory_Allocate alloc;
	void*                  temperary;
	kai_u64                temperary_size;
	void*                  user;
} kai_Memory;

//////////////////////////////////////////////////////////
// ERRORS
typedef enum: kai_u32 {
	kai_Result_Success = 0,

	kai_Result_Error_Syntax,
	kai_Result_Error_Semantic,
	kai_Result_Error_Type_Cast,
	kai_Result_Error_Type_Check,
	kai_Result_Error_Info,

	// 'Meta' Errors
	kai_Result_Error_Fatal,    // compiler bug probably
	kai_Result_Error_Internal, // compiler error unrelated to source code (e.g. out of memory)

	kai_Result_COUNT,
} kai_result;

#define KAI_FAILED(RESULT) (RESULT != kai_Result_Success)

typedef struct {
	kai_str file;
	kai_str string;
	kai_u8* source;
	kai_int line;
} kai_Location;

typedef struct kai_Error {
	kai_result     result;
	kai_Location   location;
	kai_str        message; // temperary or static memory
	kai_str        context; // temperary or static memory
	kai_Error*     next;
} kai_Error;

/////////////////////////////

__KAI_END_API__
#ifdef  KAI_CPP_API
namespace kai {

// Must pass in a UTF-8 string 
#define KAI_STR(STRING) kai_str{ .count = sizeof(STRING)-1, .data = (kai_u8*)STRING }

}
#endif//KAI_CPP_API
#endif//KAI_CORE_H