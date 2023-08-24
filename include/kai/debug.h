#ifndef KAI_DEBUG_H
#define KAI_DEBUG_H
#include <kai/parser.h>
__KAI_BEGIN_API__

typedef enum {
	kai_debug_color_primary,
	kai_debug_color_secondary,
	kai_debug_color_important,
	kai_debug_color_important_2,

	kai_debug_color_COUNT,
} kai_debug_color_enum;

typedef void (*kai_fn_write_string  )(kai_ptr User, kai_str String);
typedef void (*kai_fn_write_c_string)(kai_ptr User, char const* C_String);
typedef void (*kai_fn_write_char    )(kai_ptr User, kai_u8 Char);
typedef void (*kai_fn_set_color     )(kai_ptr User, kai_debug_color_enum Color);

typedef struct {
	kai_fn_write_string   write_string;
	kai_fn_write_c_string write_c_string;
	kai_fn_write_char     write_char;
	kai_fn_set_color      set_color; // optional, this value may be NULL
	void*                 user;
} kai_Debug_String_Writer;

KAI_API(kai_Debug_String_Writer*)
	kai_debug_clib_writer();

KAI_API(kai_Debug_String_Writer*)
	kai_debug_file_writer(char const* Filename);


KAI_API(void)
	kai_debug_write_syntax_tree(kai_Debug_String_Writer* Writer, kai_Module* Module);

KAI_API(void)
	kai_debug_write_error(kai_Debug_String_Writer* Writer, kai_Error* Error);

KAI_API(void)
	kai_debug_write_type(kai_Debug_String_Writer* Writer, kai_Type Type);

KAI_API(void)
	kai_debug_write_expression(kai_Debug_String_Writer* Writer, kai_Expr Expr);

__KAI_END_API__
#endif//KAI_DEBUG_H