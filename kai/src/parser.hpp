#pragma once
#include <cstddef>
#include <cstring>
#include <kai/parser.h>
#include "config.hpp"
#include "lexer.hpp"
#include "compiler_context.hpp"
using std::nullptr_t;

#define DEFAULT_PREC -6942069

template <typename T> struct Expr_ID_Map { static constexpr kai_u32 ID = -1; };

/////////////////////////////////////////////////////////////////////
#define MAP(TYPE, EXPRESSION_NAME)                                   \
template<> struct Expr_ID_Map <kai_##TYPE##_##EXPRESSION_NAME>{      \
    static constexpr kai_u32 ID = kai_##TYPE##_ID_##EXPRESSION_NAME; \
}

    MAP(Expr, Identifier);
    MAP(Expr, Number);
    MAP(Expr, String);
    MAP(Expr, Unary);
    MAP(Expr, Binary);
    MAP(Expr, Procedure_Call);
    MAP(Expr, Procedure_Type);
    MAP(Expr, Procedure);
    MAP(Stmt, Declaration);
    MAP(Stmt, Assignment);
    MAP(Stmt, Return);
    MAP(Stmt, If);
    MAP(Stmt, For);
    MAP(Stmt, Compound);

#undef MAP_STMT
#undef MAP_EXPR
/////////////////////////////////////////////////////////////////////

struct Parser_Context {
    Lexer_Context  lexer;
    kai_ptr        stack;

    template <typename ExprType>
    inline ExprType* alloc_expr() {
        static_assert(Expr_ID_Map<ExprType>::ID != -1, "Must be a Expression Type");
        auto expr = ctx_alloc_T(ExprType);
        expr->id = Expr_ID_Map<ExprType>::ID;
        return expr;
    }

    template <typename T>
    inline Temperary_Array<T> temperary_array() {
        return Temperary_Array<T>{reinterpret_cast<T*>(stack), 0};
    }

    template <typename T>
    inline void array_add(Temperary_Array<T>& a, T const& value) {
        a.base[a.count++] = value;
        stack = a.base + a.count;
    }

    template <typename T>
    inline void free_temperary_array(Temperary_Array<T> const& a) {
        stack = a.base;
    }

    kai_Expr parse_single_token_expression();
    kai_Expr parse_expression(int precedence = DEFAULT_PREC);

    // statements will always generate an error when returning nullptr
    kai_Expr parse_statement(bool is_top_level = false);

    kai_Expr parse_type();
    kai_Expr parse_procedure();

    void parse_types();

    bool is_procedure_next();
};
