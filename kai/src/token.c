#include "config.h"
#include "token.h"

//! @TODO: Implement Comments (multi-line)
//! @TODO: String Escape \"
//! @TODO: Add "false" and "true" keywords (re2c)

Kai_str token_type_string(Kai_u32 type) {
    switch (type) {
        default:               return KAI_EMPTY_STRING; // single character
        case T_IDENTIFIER:     return KAI_STR("identifier");
        case T_DIRECTIVE:      return KAI_STR("directive");
        case T_STRING:         return KAI_STR("string");
        case T_NUMBER:         return KAI_STR("number");
#define X(SYMBOL) case SYMBOL: return KAI_STR(#SYMBOL);
        XTOKEN_SYMBOLS
#undef X
#define X(NAME,ID) case ID:    return KAI_STR("'" #NAME "'");
        XTOKEN_KEYWORDS
#undef X
    }
}

void insert_token_type_string(Kai_str* out, Kai_u32 type) {
    Kai_str s = token_type_string(type);
    if (s.count == 0 && type < 0x100) {
        out->data[out->count++] = '\'';
        out->data[out->count++] = (char)type;
        out->data[out->count++] = '\'';
        return;
    }
    memcpy(out->data + out->count, s.data, s.count);
    out->count += s.count;
}

Kai_str keyword_map[] = {
#define X(NAME, ID) {.data = (Kai_u8*)#NAME, .count = sizeof(#NAME)-1},
    XTOKEN_KEYWORDS
#undef X
};

int hash_keyword(Kai_str const s) {
    if (s.count < 2) return 6;
    return (s.count&3) | ((s.data[0]&2) << 2) | ((s.data[1]&2) << 1);
}
Kai_u8 hash_keyword_map[16] = {
    4, 3, 3, 3, 7, 10, 6, 6, 1, 11, 9, 8, 2, 0, 0, 5,
};

enum {
    // Last Bit == 0 means that the "character" can be used in an identifier
    Identifier = 0,

    W = 1 << 1 | 1,
    T = 2 << 1 | 1,
    D = 3 << 1 | 1,
    K = 2,
    C = 5 << 1 | 1,
    N = 4,
    S = 7 << 1 | 1,
    Z = 8 << 1 | 1,

    White_Space     = W, // white space is ignored
    Character_Token = T, // example: ! <= != * +
    Directive       = D,
    Keyword         = K,
    Number          = N,
    Comment         = C, // can be comment, or just a divide symbol
    String          = S,
    Dot             = Z, // Dot is complicated..
};

Kai_u8 lex_lookup_table[128] = {
//  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W, // 0
    W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W, // 1
    W,  T,  S,  D,  T,  T,  T,  T,  T,  T,  T,  T,  T,  T,  Z,  C, // 2
    N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  T,  T,  T,  T,  T,  T, // 3
    T,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 4
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  T,  T,  T,  T,  0, // 5
    T,  0,  K,  K,  K,  K,  K,  0,  0,  K,  0,  0,  K,  0,  0,  0, // 6
    0,  0,  K,  K,  0,  K,  0,  K,  0,  0,  0,  T,  T,  T,  T,  W, // 7
};

#define next_character_equals(C) \
    ( (cursor+1) < source.count && C == source.data[cursor+1] )

void parse_number_bin(Tokenization_Context* context, Kai_u64* n);
void parse_number_dec(Tokenization_Context* context, Kai_u64* n);
void parse_number_hex(Tokenization_Context* context, Kai_u64* n);
void parse_multi_token(Tokenization_Context* context, Token* t, Kai_u8);

// I forge my own "using"
#define line    context->line_number
#define cursor  context->cursor
#define source  context->source
Token generate_token(Tokenization_Context* context) {
    Token token = (Token) {
        .type = T_END,
        .string = {.count = 1},
        .line_number = line,
    };

    while (cursor < source.count) {
        token.string.data = source.data + cursor;
        Kai_u8 ch = *token.string.data;

        if (ch&0b10000000) {
            goto case_identifier;
        }

        switch (lex_lookup_table[ch])
        {

        case White_Space: {
            if (ch == '\r') {
                // Handle Windows: CR LF
                if (next_character_equals('\n')) ++cursor;
                line += 1;
            }
            else if (ch == '\n') {
                line += 1;
            }
            token.line_number = line;
            ++cursor;
            break;
        }

        case Keyword: {
            token.type = T_IDENTIFIER;
            ++cursor;
            while (cursor < source.count) {
                Kai_u8 c = source.data[cursor];
                if (c < 128 && (lex_lookup_table[c]&1)) {
                    break;
                }
                ++cursor;
                ++token.string.count;
            }

            // Check if the string we just parsed is a keyword
            int hash = hash_keyword(token.string);
            int keyword_index = hash_keyword_map[hash];
            if (kai_string_equals(keyword_map[keyword_index], token.string)) {
                token.type = KEYWORD_START | keyword_index;
                return token;
            }
            return token;
        }

        case Directive: {
            token.type = T_DIRECTIVE;
            ++token.string.data;
            token.string.count = 0;
            ++cursor;
            goto parse_identifier;
        }

        case Identifier: {
        case_identifier:
            token.type = T_IDENTIFIER;
            ++cursor;
        parse_identifier:
            while (cursor < source.count) {
                Kai_u8 c = source.data[cursor];
                if (c < 128 && (lex_lookup_table[c]&1)) {
                    break;
                }
                ++cursor;
                ++token.string.count;
            }
            return token;
        }

        case String: {
            token.type = T_STRING;
            token.string.count = 0;
            ++cursor;
            ++token.string.data;
            Kai_int start = cursor;
            while (cursor < source.count) {
                if (source.data[cursor++] == '\"') {
                    break;
                }
            }
            token.string.count = cursor - start - 1;
            return token;
        }

        case Number: {
            ++cursor;
            token.type = T_NUMBER;
            token.number.Whole_Part = ch - '0';

            // Integer
            if (token.number.Whole_Part == 0 && cursor < source.count) {
                if (source.data[cursor] == 'b') {
                    ++cursor;
                    parse_number_bin(context, &token.number.Whole_Part);
                    token.string.count = (source.data + cursor) - token.string.data;
                    return token;
                }
                if (source.data[cursor] == 'x') {
                    ++cursor;
                    parse_number_hex(context, &token.number.Whole_Part);
                    token.string.count = (source.data + cursor) - token.string.data;
                    return token;
                }
            }

            parse_number_dec(context, &token.number.Whole_Part);

        parse_fraction:
            // Fractional Part
            if (cursor < source.count && source.data[cursor] == '.' && !next_character_equals('.')) {
                ++cursor;
                Kai_int start = cursor;
                parse_number_dec(context, &token.number.Frac_Part);
                token.number.Frac_Denom = (Kai_u16)(cursor - start);
            }

            // Parse Exponential Part
            if (cursor < source.count && (source.data[cursor] == 'e' || source.data[cursor] == 'E')) {
                Kai_u64 n = 0;
                Kai_s32 factor = 1;

                ++cursor;
                if (cursor < source.count) {
                    if (source.data[cursor] == '-') { ++cursor; factor = -1; }
                    if (source.data[cursor] == '+') ++cursor; // skip
                }

                parse_number_dec(context, &n);

                if( n > INT32_MAX ) token.number.Exp_Part = factor * INT32_MAX;
                else                token.number.Exp_Part = factor * (Kai_s32)n;
            }

            token.string.count = (source.data + cursor) - token.string.data;
            return token;
        }

        case Comment: {
            ++cursor;
            if (source.data[cursor] == '/' && cursor < source.count) {
                // single line comment
                while (cursor < source.count) {
                    if (source.data[cursor] == '\r' ||
                        source.data[cursor] == '\n')
                        break;
                    ++cursor;
                }
                break;
            }

            // multi-line comment
            if (source.data[cursor] == '*') {
				//! @TODO: need to skip "/*" and "*/" in strings
                //int depth = 1;
                //while (depth > 0 && cursor < source.count) {
                //    if (source.data[cursor++] == '*') break;
                //}
            }

            // otherwise just a divide symbol
            token.type = '/';
            return token;
        }

        case Dot: {
            if (next_character_equals('.')) {
                cursor += 2;
                ++token.string.count;
                token.type = '..';
                return token;
            }
            if ((cursor + 1) < source.count &&
                source.data[cursor + 1] >= '0' &&
                source.data[cursor + 1] <= '9') {
                token.type = T_NUMBER;
                goto parse_fraction;
            }
			// Not ".." or Number? then single character token
            token.type = (Kai_u32)ch;
            ++cursor;
			return token;
        }

        case Character_Token: {
            token.type = (Kai_u32)ch;
            ++cursor;
            parse_multi_token(context, &token, ch);
            return token;
        }

        default: {
            panic_with_message("lexer error");
            return token;
        }

        }
    }
    return token;
}

//! @TODO: Handle overflow of integers (how? lol idk, token_error_overflow?)

void parse_number_bin(Tokenization_Context* context, Kai_u64* n) {
    for(; cursor < source.count; ++cursor) {
        Kai_u8 ch = source.data[cursor];

        if( ch < '0' ) break;
        if( ch > '1' ) {
            if( ch == '_' ) continue;
            else break;
        }

        *n = *n << 1;
        *n += (Kai_u64)ch - '0';
    }
}

void parse_number_dec(Tokenization_Context* context, Kai_u64* n) {
    for(; cursor < source.count; ++cursor) {
        Kai_u8 ch = source.data[cursor];

        if( ch < '0' ) break;
        if( ch > '9' ) {
            if( ch == '_' ) continue;
            else break;
        }

        *n = *n * 10;
        *n += (Kai_u64)ch - '0';
    }
}

void parse_number_hex(Tokenization_Context* context, Kai_u64* n) {
    enum {
        X = 16, /* BREAK */ S = 17, /* SKIP */
        A = 0xA, B = 0xB, C = 0xC, D = 0xD, E = 0xE, F = 0xF,
    };
    Kai_u8 lookup_table[56] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, X, X, X, X, X, X,
        X, A, B, C, D, E, F, X, X, X, X, X, X, X, X, X,
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, S,
        X, A, B, C, D, E, F, X,
    };
    
    for(; cursor < source.count; ++cursor) {
        Kai_u8 ch = source.data[cursor];

        if( ch < '0' || ch > 'f' ) break;

        Kai_u8 what = lookup_table[ch - '0'];

        if( what == S ) continue;
        if( what == X ) break;

        *n = *n << 4;
        *n += what;
    }
}

void parse_multi_token(Tokenization_Context* context, Token* t, Kai_u8 current) {
    if(cursor >= source.count) return;

    switch(current)
    {
    default:
	
	break; case '&': {
        if (source.data[cursor] == '&') {
            t->type = '&&';
            ++cursor;
            ++t->string.count;
        }
	}
	break; case '|': {
        if (source.data[cursor] == '|') {
            t->type = '||';
            ++cursor;
            ++t->string.count;
        }
	}
    break; case '=': {
        if (source.data[cursor] == '=') {
            t->type = '==';
            ++cursor;
            ++t->string.count;
        }
	}
    break; case '>': {
        if (source.data[cursor] == '=') {
            t->type = '>=';
            ++cursor;
            ++t->string.count;
        } else
		if (source.data[cursor] == '>') {
            t->type = '>>';
            ++cursor;
            ++t->string.count;
        }
	}
    break; case '<': {
        if (source.data[cursor] == '=') {
            t->type = '<=';
            ++cursor;
            ++t->string.count;
        } else
		if (source.data[cursor] == '<') {
            t->type = '<<';
            ++cursor;
            ++t->string.count;
        }
	}
    break; case '!': {
        if (source.data[cursor] == '=') {
            t->type = '!=';
            ++cursor;
            ++t->string.count;
        }
	}
    break; case '-': {
        if (source.data[cursor] == '>') {
            t->type = '->';
            ++cursor;
            ++t->string.count;
        } else
        if (cursor + 1 < source.count &&
            source.data[cursor] == '-' &&
            source.data[cursor+1] == '-'
        ) {
            t->type = '---';
            cursor += 2;
            t->string.count += 2;
        }
    }
    break;
    }
}

Token* next_token(Tokenization_Context* context) {
    if (!context->peeking) {
        context->current_token = generate_token(context);
        return &context->current_token;
    }
    context->peeking = KAI_FALSE;
    context->current_token = context->peeked_token;
    return &context->current_token;
}

Token* peek_token(Tokenization_Context* context) {
    if (context->peeking) return &context->peeked_token;
    context->peeking = KAI_TRUE;
    context->peeked_token = generate_token(context);
    return &context->peeked_token;
}
