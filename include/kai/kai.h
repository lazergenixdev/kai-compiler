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

#ifdef __cplusplus
#    define KAI_STRUCT(X) X
#else
#    define KAI_STRUCT(X) (X)
#endif

#define KAI_API(RETURN_TYPE) extern RETURN_TYPE

#define KAI_EMPTY_STRING    KAI_STRUCT(Kai_str){0}
#define KAI_STRING(LITERAL) KAI_STRUCT(Kai_str){.count = sizeof(LITERAL)-1, .data = (Kai_u8*)(LITERAL)}

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
	Kai_u16   param_count;
	Kai_u16   ret_count;
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

typedef void* Kai_P_Memory_Allocate   (Kai_ptr User, Kai_u32 Num_Bytes, Kai_u32 access);
typedef void  Kai_P_Memory_Free       (Kai_ptr User, Kai_ptr Ptr, Kai_u32 Num_Bytes);
typedef void  Kai_P_Memory_Set_Access (Kai_ptr User, Kai_ptr Ptr, Kai_u32 Num_Bytes, Kai_u32 access);

typedef struct {
	Kai_P_Memory_Allocate*   allocate;
	Kai_P_Memory_Free*       free;
	Kai_P_Memory_Set_Access* set_access;
	Kai_ptr                  user;
	Kai_u32                  page_size;
} Kai_Memory_Allocator;

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
#ifndef KAI__SECTION_INTERNAL_API_STRUCTS

typedef struct Kai__Arena_Bucket {
    struct Kai__Arena_Bucket* prev;
} Kai__Arena_Bucket;

typedef struct {
    Kai__Arena_Bucket*   current_bucket;
    Kai_u32              current_allocated;
    Kai_u32              bucket_size;
    Kai_Memory_Allocator allocator;
} Kai__Dynamic_Arena_Allocator;

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
    Kai_str               source_code; // input
    Kai_Memory_Allocator  allocator;   // input
    Kai_Error*            error;       // output [optional]
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
	Kai_Memory_Allocator  memory;
	Kai_Error*            error;
	Kai_Native_Procedure* native_procedures;
	Kai_u32               native_procedure_count;
} Kai_Program_Create_Info;

typedef struct Kai_Program_Impl* Kai_Program;

#endif
#ifndef KAI__SECTION_CORE_API

// ------ Core Library of some sort? ----------------------------------------------------

KAI_API(Kai_bool)        kai_string_equals    (Kai_str A, Kai_str B);
KAI_API(Kai_str)         kai_string_slice     (Kai_str String, Kai_u32 offset, Kai_u32 end);
KAI_API(Kai_vector3_f32) kai_color_rgb_to_hsv (Kai_vector3_f32 RGB);
KAI_API(Kai_vector3_f32) kai_color_hsv_to_rgb (Kai_vector3_f32 HSV);

// --------------------------------------------------------------------------------------

KAI_API (Kai_str) kai_str_from_cstring(char const* String);

KAI_API (void) kai_get_version(Kai_u32* out_Major, Kai_u32* out_Minor, Kai_u32* out_Patch);

KAI_API (Kai_str) kai_get_version_string(void);

KAI_API (Kai_Result) kai_create_syntax_tree(
	Kai_Syntax_Tree_Create_Info* Info,
	Kai_Syntax_Tree*             Syntax_Tree);

KAI_API (void) kai_destroy_syntax_tree(Kai_Syntax_Tree* Syntax_Tree);

KAI_API (Kai_Result) kai_create_program(Kai_Program_Create_Info* Info, Kai_Program* out_Program);

KAI_API (Kai_Result) kai_create_program_from_source(
	Kai_str               Source,
	Kai_Memory_Allocator* Allocator,
	Kai_Error*            out_Error,
	Kai_Program*          out_Program);

KAI_API (void) kai_destroy_program(Kai_Program Program);

KAI_API (void*) kai_find_procedure(Kai_Program Program, char const* Name, char const* opt_Type);

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

KAI_API (Kai_Result) kai_create_memory(Kai_Memory_Allocator* out_Allocator);
KAI_API (Kai_Result) kai_destroy_memory(Kai_Memory_Allocator* Allocator);
KAI_API (void) kai_memory_set_debug(Kai_Memory_Allocator* Allocator, Kai_u32 Debug_Level);
KAI_API (Kai_u32) kai_memory_usage(Kai_Memory_Allocator* Allocator);

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
	struct Default_Allocator : public Kai_Memory_Allocator {
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

        Result compile_from_file(String path) {
			std::ifstream f { path.string() };
			f.seekg(0, std::ios::end);
			auto size = f.tellg();
			f.seekg(0, std::ios::beg);
			std::string source(size, '\0');
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
