#pragma once
#include <kai/parser.h>
#include "lexer.hpp"

template <typename T> struct expr_id_map { static constexpr kai_u32 ID = -1; };
template<> struct expr_id_map <kai_Expr_Identifier>     { static constexpr kai_u32 ID = kai_Expr_ID_Identifier;     };
template<> struct expr_id_map <kai_Expr_Number>         { static constexpr kai_u32 ID = kai_Expr_ID_Number;         };
template<> struct expr_id_map <kai_Expr_String>         { static constexpr kai_u32 ID = kai_Expr_ID_String;         };
template<> struct expr_id_map <kai_Expr_Unary>          { static constexpr kai_u32 ID = kai_Expr_ID_Unary;          };
template<> struct expr_id_map <kai_Expr_Binary>         { static constexpr kai_u32 ID = kai_Expr_ID_Binary;         };
template<> struct expr_id_map <kai_Expr_Procedure_Call> { static constexpr kai_u32 ID = kai_Expr_ID_Procedure_Call; };
template<> struct expr_id_map <kai_Expr_Procedure_Type> { static constexpr kai_u32 ID = kai_Expr_ID_Procedure_Type; };
template<> struct expr_id_map <kai_Expr_Procedure>      { static constexpr kai_u32 ID = kai_Expr_ID_Procedure;      };
template<> struct expr_id_map <kai_Stmt_Declaration>    { static constexpr kai_u32 ID = kai_Stmt_ID_Declaration;    };
template<> struct expr_id_map <kai_Stmt_Return>         { static constexpr kai_u32 ID = kai_Stmt_ID_Return;         };
template<> struct expr_id_map <kai_Stmt_Compound>       { static constexpr kai_u32 ID = kai_Stmt_ID_Compound;       };

// TODO: rewrite to use global context
struct Parser_Context {
    kai_Memory     memory;
    Lexer_Context  lexer;
    kai_ptr        stack;
    kai_int        stack_size;
    kai_Error      error_info;

    template <typename NodeType>
    inline NodeType* alloc() {
        return reinterpret_cast<NodeType*>(memory.alloc(memory.user, sizeof(NodeType)));
    }

    template <typename ExprType>
    inline ExprType* alloc_expr() {
        static_assert(expr_id_map<ExprType>::ID != -1, "Must be a Expression Type");
        auto expr = reinterpret_cast<ExprType*>(memory.alloc(memory.user, sizeof(ExprType)));
        expr->id = expr_id_map<ExprType>::ID;
        return expr;
    }

    template <size_t Size>
    nullptr_t error_internal(char const (&what)[Size]) {
        if (error_info.result != kai_Result_Success) return nullptr; // already have error value

        error_info.result = kai_Result_Error_Internal;
        error_info.message = {0, (kai_u8*)memory.temperary}; // uses temperary memory
        error_info.context.count = 0; // static memory

        // fill what buffer
        auto& w = error_info.message;
        memcpy(w.data + w.count, what, Size - 1);
        w.count += Size - 1;

        return nullptr;
    }

    // TODO: remove expected and add it to unexpected error.context
    template <size_t Size>
    nullptr_t error_expected(char const (&what)[Size]) {
        return error_expected(what, lexer.currentToken);
    }

    template <size_t Size>
    nullptr_t error_expected(char const (&what)[Size], Token const& token) {
        if (error_info.result != kai_Result_Success) return nullptr; // already have error value

        error_info.result          = kai_Result_Error_Syntax;
        error_info.location.string = token.string;
        error_info.location.line   = token.line_number;
        error_info.message         = { 0, (kai_u8*)memory.temperary }; // uses temperary memory
        error_info.context.count   = 0; // static memory
        
        adjust_source_location(error_info.location.string, token.type);

        // fill what buffer
        auto& w = error_info.message;
        memcpy(w.data, "Expected ", 9);
        w.count += 9;

        memcpy(w.data + w.count, what, Size - 1);
        w.count += Size - 1;

        return nullptr;
    }

    template <size_t Size1, size_t Size2>
    nullptr_t error_unexpected(char const (&what)[Size1], char const (&context)[Size2]) {
        return error_unexpected(what, context, lexer.currentToken);
    }

    template <size_t Size1, size_t Size2>
    nullptr_t error_unexpected(char const (&what)[Size1], char const (&context)[Size2], Token const& token)  {
        if (error_info.result != kai_Result_Success) return nullptr; // already have error value

        error_info.result = kai_Result_Error_Syntax;
        error_info.location.string = token.string;
        error_info.location.line = token.line_number;
        error_info.message = { 0, (kai_u8*)memory.temperary }; // uses temperary memory
        error_info.context = { Size2 - 1, (kai_u8*)context }; // static memory

        adjust_source_location(error_info.location.string, token.type);

        auto& w = error_info.message;
        memcpy(w.data, "Unexpected ", 11);
        w.count += 11;

        // print token we did not expect
        {
            char backup[4];
            auto token_str = token_type_string(token.type);
            if (token_str.count == 0) {
                token_str.data = (kai_u8*)backup;
                token_str.count = 3;

                backup[0] = '\'';
                backup[1] = (char)token.type;
                backup[2] = '\'';
            }

            memcpy(w.data + w.count, token_str.data, token_str.count);
            w.count += token_str.count;
        }

        w.data[w.count++] = ' ';

        memcpy(w.data + w.count, what, Size1 - 1);
        w.count += Size1 - 1;

        return nullptr;
    }

    // @TODO: Fix lexer so that this can be Deleted (adjust_source_location)
    void adjust_source_location(kai_str& src, kai_u32 type) {
        if (type == token_string) {
            src.data -= 1; // we know there will at least be one " to open the string
            src.count += 2;
        }
        if (type == token_directive) {
            src.data -= 1;
            src.count += 1;
        }
    }
};
