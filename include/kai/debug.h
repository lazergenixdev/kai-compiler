#ifndef KAI_DEBUG_H
#define KAI_DEBUG_H
#include "parser.h"
__KAI_BEGIN_API__

enum {
	KAI_DEBUG_COLOR_PRIMARY,
	KAI_DEBUG_COLOR_SECONDARY,
	KAI_DEBUG_COLOR_IMPORTANT,
	KAI_DEBUG_COLOR_IMPORTANT_2,
	KAI_DEBUG_COLOR_DECORATION,

	KAI_DEBUG_COLOR_COUNT,
};
typedef Kai_u32 Kai_Debug_Color;

typedef void (*Kai_P_Write_String  )(Kai_ptr User, Kai_str String);
typedef void (*Kai_P_Write_C_String)(Kai_ptr User, char const* C_String);
typedef void (*Kai_P_Write_Char    )(Kai_ptr User, Kai_u8 Char);
typedef void (*Kai_P_Set_Color     )(Kai_ptr User, Kai_Debug_Color Color);

typedef struct {
	Kai_P_Write_String   write_string;
	Kai_P_Write_C_String write_c_string;
	Kai_P_Write_Char     write_char;
	Kai_P_Set_Color      set_color; // optional, this value may be NULL
	Kai_ptr              user;
} Kai_Debug_String_Writer;

KAI_API(Kai_Debug_String_Writer*)
	kai_debug_clib_writer();

KAI_API(Kai_Debug_String_Writer*)
	kai_debug_file_writer(char const* C_Filename);

KAI_API(void)
	kai_debug_destroy_file_writer(Kai_Debug_String_Writer* C_Filename);


KAI_API(void)
	kai_debug_write_syntax_tree(Kai_Debug_String_Writer* Writer, Kai_AST* Tree);

KAI_API(void)
	kai_debug_write_error(Kai_Debug_String_Writer* Writer, Kai_Error* Error);

KAI_API(void)
	kai_debug_write_type(Kai_Debug_String_Writer* Writer, Kai_Type Type);

KAI_API(void)
	kai_debug_write_expression(Kai_Debug_String_Writer* Writer, Kai_Expr Expr);

__KAI_END_API__
#endif//KAI_DEBUG_H
