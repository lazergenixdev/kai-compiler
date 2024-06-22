#ifndef KAI_CORE_H
#define KAI_CORE_H
#include "config.h"
#include <stdint.h>

#define KAI_STR(STRING) \
	KAI_STRUCT(Kai_str){.count = sizeof(STRING)-1, .data = (Kai_u8*)(STRING)}

#define KAI_EMPTY_STRING (KAI_STRUCT(Kai_str){})

#define KAI_FAILED(RESULT) ((RESULT) != KAI_SUCCESS)

__KAI_BEGIN_API__

// ===========================> Primitive Types <==============================

enum {
	KAI_FALSE = 0,
	KAI_TRUE  = 1,
};
typedef uint8_t   Kai_bool;

typedef int8_t    Kai_s8;
typedef int16_t   Kai_s16;
typedef int32_t   Kai_s32;
typedef int64_t   Kai_s64;

typedef uint8_t   Kai_u8;
typedef uint16_t  Kai_u16;
typedef uint32_t  Kai_u32;
typedef uint64_t  Kai_u64;

typedef Kai_s64   Kai_int;

typedef double    Kai_f64;
typedef float     Kai_f32;

typedef void*     Kai_ptr;

typedef struct Kai_Type_Info* Kai_Type;

// TODO: Add vector, matrix, quaternion types
//typedef Kai_f32 kai_f32x2[2];
//typedef Kai_f32 kai_f32x3[3];
//typedef Kai_f32 kai_f32x4[4];
//typedef Kai_f64 kai_f64x2[2];
//typedef Kai_f64 kai_f64x3[3];
//typedef Kai_f64 kai_f64x4[4];

// =============================> Struct Types <===============================

typedef struct {
	Kai_int count;
	Kai_u8* data;
} Kai_str;

typedef struct {
	Kai_u32 offset;
	Kai_u32 count;
} Kai_range;

typedef struct {
	Kai_int count;
	Kai_ptr data;
} Kai_array;


// =============================> Version Info <===============================

typedef struct {
	Kai_s32 major;
	Kai_s32 minor;
	Kai_s32 patch;
} Kai_Version;

// .version: [out]
KAI_API(void)
	kai_get_version(Kai_Version* Version);

KAI_API(Kai_str)
	kai_get_version_string();


// ==============================> Type Info <=================================

typedef enum {
	KAI_TYPE_TYPE      = 0, // No Type_Info struct for this.
	KAI_TYPE_INTEGER   = 1,
	KAI_TYPE_FLOAT     = 2,
	KAI_TYPE_POINTER   = 3,
	KAI_TYPE_PROCEDURE = 4,
	KAI_TYPE_ARRAY     = 5,
	KAI_TYPE_STRING    = 6,
	KAI_TYPE_STRUCT    = 7,
} KAI_TYPE_;

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
	Kai_u16   param_count;
	Kai_u16   ret_count;
	Kai_Type* input_output; // list of parameters types, then return types
} Kai_Type_Info_Procedure;

// unimplemented
typedef struct {
	Kai_u8 type;
	void*  members;
} Kai_Type_Info_Struct;


// ============================> Core Functions <==============================

KAI_API(Kai_bool)
	kai_string_equals(Kai_str A, Kai_str B);


// ================================> Memory <==================================

// Allocate "Byte_Count" number of bytes
typedef Kai_ptr (*Kai_P_Allocate_Memory)(Kai_ptr User, Kai_int Byte_Count);

typedef struct {
	// There is NO "free" function
	//   Memory "object" is required to keep track of all the memory
	//   it allocates and free it when the compiler is no longer using it.
	Kai_P_Allocate_Memory alloc;
	Kai_ptr temperary;
	Kai_u64 temperary_size;
	Kai_ptr user;
} Kai_Memory;


// ================================> Errors <==================================

enum {
	KAI_SUCCESS = 0,

	KAI_ERROR_SYNTAX,
	KAI_ERROR_SEMANTIC,
	KAI_ERROR_TYPE_CAST,
	KAI_ERROR_TYPE_CHECK,
	KAI_ERROR_INFO,

	// 'Meta' Errors
	KAI_ERROR_FATAL,    // => compiler bug probably
	KAI_ERROR_INTERNAL, // => compiler error unrelated to source code (e.g. out of memory)

	KAI_RESULT_COUNT,
};
typedef Kai_u32 Kai_Result;

typedef struct {
	Kai_str file_name;
	Kai_str string;
	Kai_u8* source; // source code for this file
	Kai_int line;
} Kai_Location;

typedef struct Kai_Error {
	Kai_Result        result;
	Kai_Location      location;
	Kai_str           message; // temperary or static memory
	Kai_str           context; // temperary or static memory
	struct Kai_Error* next;
} Kai_Error;

__KAI_END_API__
#endif//KAI_CORE_H
