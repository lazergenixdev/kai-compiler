#ifndef TOKEN_H
#include <kai/parser.h>

// usage: token.type = TT2("->");
#define TT2(S) ( \
        ((Kai_u32)((S)[1]) << 8) \
    |   ((Kai_u32)((S)[0])     ))

// usage: token.type = TT3("---");
#define TT3(S) ( \
        ((Kai_u32)((S)[2]) << 16) \
    |   ((Kai_u32)((S)[1]) <<  8) \
    |   ((Kai_u32)((S)[0])      ))

#define KEYWORD_START 0x80

#define X_TOKEN_KEYWORDS \
X(break   , KEYWORD_START | 0) \
X(cast    , KEYWORD_START | 1) \
X(continue, KEYWORD_START | 2) \
X(defer   , KEYWORD_START | 3) \
X(else    , KEYWORD_START | 4) \
X(for     , KEYWORD_START | 5) \
X(if      , KEYWORD_START | 6) \
X(loop    , KEYWORD_START | 7) \
X(ret     , KEYWORD_START | 8) \
X(struct  , KEYWORD_START | 9) \
X(using   , KEYWORD_START |10) \
X(while   , KEYWORD_START |11) \

enum Token_Enum {
    T_END        = 0,
    T_IDENTIFIER = 0xC0 | 0,
    T_DIRECTIVE  = 0xC0 | 1,
    T_STRING     = 0xC0 | 2,
    T_NUMBER     = 0xC0 | 3,
#define X(NAME,ID) T_KW_ ## NAME = ID,
    X_TOKEN_KEYWORDS
#undef X
};

#define X_TOKEN_SYMBOLS \
X(TT2("->"), '->') \
X(TT2("=>"), '=>') \
X(TT2("=="), '==') \
X(TT2("!="), '!=') \
X(TT2("<="), '<=') \
X(TT2(">="), '>=') \
X(TT2("&&"), '&&') \
X(TT2("||"), '||') \
X(TT2("<<"), '<<') \
X(TT2(">>"), '>>') \
X(TT2(".."), '..') \
X(TT3("---"), '---')

#define token_is_keyword(TOKEN_TYPE) \
    ((TOKEN_TYPE) < T_IDENTIFIER && (TOKEN_TYPE) >= KEYWORD_START)

void insert_token_type_string(Kai_str* out, Kai_u32 type);

typedef struct {
    Kai_u32 type;
    Kai_str string;
    Kai_u32 line_number;
    Kai_Number_Info number;
} Token;

typedef struct {
    Token    current_token;
    Token    peeked_token;
    Kai_str  source;
    Kai_int  cursor;
    Kai_u32  line_number;
    Kai_bool peeking;
} Tokenization_Context;

Token* next_token(Tokenization_Context* context);
Token* peek_token(Tokenization_Context* context);

#endif // TOKEN_H

