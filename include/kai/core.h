#ifndef KAI_CORE_H
#define KAI_CORE_H
#include <kai/config.h>
#include <stdint.h>
__KAI_BEGIN_API__

// Primitive Types

typedef uint8_t   kai_bool;

enum: kai_bool {
	kai_bool_false = 0,
	kai_bool_true  = 1,
};

typedef int64_t   kai_s64;
typedef int32_t   kai_s32;
typedef int16_t   kai_s16;
typedef int8_t    kai_s8;

typedef uint64_t  kai_u64;
typedef uint32_t  kai_u32;
typedef uint16_t  kai_u16;
typedef uint8_t   kai_u8;

typedef kai_s64   kai_int;

typedef double    kai_f64;
typedef float     kai_f32;

typedef kai_u64   kai_enum;
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


// Type Info

typedef enum {
	kai_Type_Integer   = 0,
	kai_Type_Float     = 1,
	kai_Type_Pointer   = 2,
	kai_Type_Procedure = 3,
	kai_Type_Type      = 4, // No Type_Info struct for this
	kai_Type_Struct    = 5,
} kai_Type_Enum;

typedef struct kai_Type_Info {
	kai_u32 type;
} kai_Type_Info;

typedef struct {
	kai_u32 type;
	kai_u8 bits;
	kai_bool is_signed;
} kai_Type_Info_Integer;

typedef struct {
	kai_u32 type;
	kai_u8 bits;
} kai_Type_Info_Float;

typedef struct {
	kai_u32 type;
	kai_Type sub_type;
} kai_Type_Info_Pointer;

typedef struct {
	kai_u32 type;
	kai_u16 param_count;
	kai_u16 ret_count;
	kai_Type* param_ret_types; // list of parameters types, then return types
} kai_Type_Info_Procedure;

typedef struct {
	kai_u32 type;
	void* members;
} kai_Type_Info_Struct;

// Core Functions
// String Functions

KAI_API bool kai_string_equals(kai_str a, kai_str b);


//////////////////////////////////////////////////////////
// VERSION INFO

typedef struct {
	kai_s32 major;
	kai_s32 minor;
	kai_s32 patch;
} kai_Version;

KAI_API void kai_version(kai_Version* ver);

KAI_API char const* kai_version_string();


//////////////////////////////////////////////////////////
// MEMORY

// Allocate "Byte_Count" number of bytes
typedef kai_ptr (*kai_fn_Memory_Allocate)(kai_ptr user, kai_int Byte_Count);

typedef struct {
	// There is NO "free" function
	//   Memory "object" is required to keep track of all the memory
	//   it allocates and free it when the compiler is no longer using it.
	kai_fn_Memory_Allocate alloc;
	kai_ptr                temperary;
	kai_int                temperary_size;
	kai_ptr                user;
} kai_Memory;

KAI_API void kai_Lib_create_memory(kai_Memory* mem);
KAI_API void kai_Lib_reset_memory(kai_Memory* mem);
KAI_API void kai_Lib_destroy_memory(kai_Memory* mem);

//////////////////////////////////////////////////////////
// ERRORS

typedef enum {
	kai_Result_Success = 0,

	kai_Result_Error_Syntax,
	kai_Result_Error_Semantic,
	kai_Result_Error_Type_Cast,
	kai_Result_Error_Type_Check,

	// 'Meta' Errors
	kai_Result_Error_Fatal, // compiler bug probably
	kai_Result_Error_Internal, // compiler error unrelated to source code
	
	kai_Result_Error_Info, // to be implemented...
} kai_result;

#define KAI_FAILED(RESULT) (RESULT != kai_Result_Success)

typedef struct {
	kai_u8* source;
	kai_str string;
	kai_int line;
} kai_Location;

// @TODO: make this a linked list for chaining errors togther with more info
typedef struct {
	kai_result    value;
	kai_Location  loc;
	kai_str       file; // copied from source
	kai_str       what; // uses temperary memory
	kai_str       context; // static memory
} kai_Error_Info;

/////////////////////////////

__KAI_END_API__
#ifdef  KAI_CPP_API
namespace kai {

// Static String Helper
#if 1
#define kai_static_string(STRING) kai_str{ .count = std::size(STRING)-1, .data = (kai_u8*)STRING }
#else
template <kai_int Size, typename CharT>
static inline constexpr kai_str static_string( CharT const( &Data )[Size] ) {
	static_assert( sizeof(CharT) == 1 ); // CharT should not be any of [wchar_t, char16_t, char32_t]
	kai_str out;
	out.count = Size-1;
	out.data  = (kai_u8*)Data; // C++, you are stupid, just let me do what I WANT TO DO!
	return out;
}
#endif

}
#endif//KAI_CPP_API
#endif//KAI_CORE_H