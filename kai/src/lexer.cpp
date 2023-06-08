#include "lexer.hpp"

// TODO: Implement Comments (multi-line)
// TODO: String Escape \"
// TODO: Add "false" and "true" keywords (re2c)

Token& Lexer_Context::next_token()
{
	if( !peeking )
		return currentToken = generate_token();
	peeking = false;
	return currentToken = peekedToken;
}

Token& Lexer_Context::peek_token()
{
	if( peeking )
		return peekedToken;
	peeking = true;
	return peekedToken = generate_token();
}

enum What {
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

kai_u8 lex_lookup_table[128] = {
//  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W, // 0
	W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W,  W, // 1
	W,  T,  S,  D,  T,  T,  T,  T,  T,  T,  T,  T,  T,  T,  Z,  C, // 2
	N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  T,  T,  T,  T,  T,  T, // 3
	T,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 4
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  T,  T,  T,  T,  0, // 5
	T,  0,  K,  K,  K,  0,  K,  0,  0,  K,  0,  0,  K,  0,  0,  0, // 6
	0,  0,  K,  0,  0,  K,  0,  K,  0,  0,  0,  T,  T,  T,  T,  W, // 7
};

Token Lexer_Context::generate_token()
{
	Token token{token_end};
	token.string.count = 1;
	token.line_number = line_number;

	while( cursor < source.count ) {
		token.string.data = source.data + cursor;

		kai_u8 ch = *token.string.data;

		if( ch & 0b1000'0000 ) {
			goto case_identifier;
		}

		switch( lex_lookup_table[ch] )
		{

		case White_Space: {
			if( ch == '\r' ) {
				// Handle Windows: CR LF
				if( next_character_equals('\n') ) ++cursor;
				line_number += 1;
			}
			else if( ch == '\n' ) {
				line_number += 1;
			}
			token.line_number = line_number;
			++cursor;
			break;
		}

		case Keyword: {
			// TODO: Use re2c for keyword lookup
			token.type = token_identifier;
			++cursor;
			while( cursor < source.count ) {
				kai_u8 ch = source.data[cursor];
				if( ch < 128 && (lex_lookup_table[ch] & 1) ) {
					break;
				}
				++cursor;
				++token.string.count;
			}

			int i = 0;
			for( auto&& kw: keyword_map ) {
				if( kai_string_equals(token.string, kw) ) {
					token.type = token_type(KEYWORD_START + i);
					return token;
				}
				++i;
			}

			return token;
		}

		case Directive: {
			token.type = token_directive;
			++token.string.data;
			token.string.count = 0;
			++cursor;
			goto parse_identifier;
		}

		case Identifier: {
		case_identifier:
			token.type = token_identifier;
			++cursor;
		parse_identifier:
			while( cursor < source.count ) {
				kai_u8 ch = source.data[cursor];
				if( ch < 128 && (lex_lookup_table[ch] & 1) ) {
					break;
				}
				++cursor;
				++token.string.count;
			}
			return token;
		}

		case String: {
			token.type = token_string;
			token.string.count = 0;
			++cursor;
			++token.string.data;
			kai_int start = cursor;

			while( cursor < source.count ) {
				if( source.data[cursor++] == '\"' ) {
					break;
				}
			}
			token.string.count = cursor - start - 1;

			return token;
		}

		case Number: {
			++cursor;
			token.type = token_number;
			token.number.Whole_Part = ch - '0';

			// Integer Constant
			if( token.number.Whole_Part == 0 && cursor < source.count ) {
				if( source.data[cursor] == 'b' ) {
					++cursor;
					parse_number_bin(token.number.Whole_Part);
					token.string.count = (source.data + cursor) - token.string.data;
					return token;
				}
				if( source.data[cursor] == 'x' ) {
					++cursor;
					parse_number_hex( token.number.Whole_Part );
					token.string.count = (source.data + cursor) - token.string.data;
					return token;
				}
			}

			parse_number_dec(token.number.Whole_Part);

			// Parse Fractional Part
			if( cursor < source.count && source.data[cursor] == '.' ) {
				++cursor;
				auto start = cursor;
				parse_number_dec(token.number.Frac_Part);
				token.number.Frac_Denom = kai_u16(cursor - start);
			}

			// Parse Exponential Part
			if( cursor < source.count && (source.data[cursor] == 'e' || source.data[cursor] == 'E') ) {
				kai_u64 n = 0;
				kai_s32 factor = 1;

				++cursor;
				if( cursor < source.count ) {
					if( source.data[cursor] == '-' ) { ++cursor; factor = -1; }
					if( source.data[cursor] == '+' ) ++cursor; // skip
				}

				parse_number_dec(n);

				if( n > INT32_MAX ) token.number.Exp_Part = factor * INT32_MAX;
				else                token.number.Exp_Part = factor * (kai_s32)n;
			}

			token.string.count = (source.data + cursor) - token.string.data;
			return token;
		}

		case Comment: {
			++cursor;
			if( cursor < source.count ) {
				// single line comment
				if( source.data[cursor] == '/' ) {
					++cursor;
					while( cursor < source.count ) {
						if( source.data[cursor] == '\r' ||
							source.data[cursor] == '\n' )
							break;
						++cursor;
					}
					break;
				}

				// multi line comment
				if( source.data[cursor] == '*' ) {
				//	break;
				}
			}

			token.type = (token_type)'/';
			return token;
		}

		case Dot: {
			// TODO: handle case where this could be a number (e.g. ".01")
		}

		case Character_Token: {
			token.type = (token_type)ch;
			++cursor;

			parse_multi_token(token, ch);
			return token;
		}

		default:
			__debugbreak();
			break;
		}
	}

	return token;
}

// TODO: Handle overflow of integers (how? lol idk, token_error_overflow?)

void Lexer_Context::parse_number_bin(kai_u64& n) {
	for(; cursor < source.count; ++cursor) {
		kai_u8 ch = source.data[cursor];

		if( ch < '0' ) break;
		if( ch > '1' ) {
			if( ch == '_' ) continue;
			else break;
		}

		n = n << 1;
		n += (kai_u64)ch - '0';
	}
}

void Lexer_Context::parse_number_dec(kai_u64& n) {
	for(; cursor < source.count; ++cursor) {
		kai_u8 ch = source.data[cursor];

		if( ch < '0' ) break;
		if( ch > '9' ) {
			if( ch == '_' ) continue;
			else break;
		}

		n = n * 10;
		n += (kai_u64)ch - '0';
	}
}

void Lexer_Context::parse_number_hex(kai_u64& n) {
	enum {
		X = 16, /* BREAK */ S = 17, /* SKIP */
		A = 0xA, B = 0xB, C = 0xC, D = 0xD, E = 0xE, F = 0xF,
	};
	static constexpr kai_u8 lookup_table[56] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, X, X, X, X, X, X,
		X, A, B, C, D, E, F, X, X, X, X, X, X, X, X, X,
		X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, S,
		X, A, B, C, D, E, F, X,
	};
	
	for(; cursor < source.count; ++cursor) {
		kai_u8 ch = source.data[cursor];

		if( ch < '0' || ch > 'f' ) break;

		auto what = lookup_table[ch - '0'];

		if( what == S ) continue;
		if( what == X ) break;

		n = n << 4;
		n += what;
	}
}

void Lexer_Context::parse_multi_token(Token& t, kai_u8 current) {
	if(cursor >= source.count) return;

	switch( current )
	{
	default:break;

	case '&':
		if(source.data[cursor] == '&') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'&&';
		}
	break;

	case '|':
		if(source.data[cursor] == '|') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'||';
		}
	break;

	case '=':
		if(source.data[cursor] == '=') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'==';
		}
	break;

	case '>':
		if(source.data[cursor] == '=') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'>=';
		}
		else if(source.data[cursor] == '>') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'>>';
		}
	break;

	case '<':
		if(source.data[cursor] == '=') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'<=';
		}
		else if(source.data[cursor] == '<') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'<<';
		}
	break;

	case '!':
		if(source.data[cursor] == '=') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'&&';
		}
	break;

	case '-': {
		if (source.data[cursor] == '>') {
			++cursor;
			++t.string.count;
			t.type = (token_type)'->';
		} else
		if(cursor + 1 < source.count &&
			source.data[cursor] == '-' &&
			source.data[cursor+1] == '-'
		) {
			cursor += 2;
			t.string.count += 2;
			t.type = (token_type)'---';
		}
		break;
	}
	}
}
