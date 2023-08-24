#pragma once
#include <array>
#include <kai/parser.h>

#define KEYWORD_START 0x80

#define XTOKEN_KEYWORDS \
X(break   , KEYWORD_START | 0) \
X(cast    , KEYWORD_START | 1) \
X(continue, KEYWORD_START | 2) \
X(defer   , KEYWORD_START | 3) \
X(for     , KEYWORD_START | 4) \
X(if      , KEYWORD_START | 5) \
X(loop    , KEYWORD_START | 6) \
X(ret     , KEYWORD_START | 7) \
X(struct  , KEYWORD_START | 8) \
X(using   , KEYWORD_START | 9) \
X(while   , KEYWORD_START |10) \


using token_type = enum : kai_u32 {
	token_end = 0,

	token_identifier = 0xC0 | 0,
	token_directive  = 0xC0 | 1,
	token_string     = 0xC0 | 2,
	token_number     = 0xC0 | 3,

#define X(NAME, ID) token_kw_ ## NAME = ID,
	XTOKEN_KEYWORDS
#undef X
};

static inline constexpr bool is_token_keyword(token_type ty) {
	return ty < token_identifier && ty >= KEYWORD_START;
}

// count <= 8  hash = (count & 0b11 << 2) | (data[0] & 0b11)
//kai_str* keyword_buckets[16] = {
//
//};

inline kai_str const keyword_map[] = {
#define X(NAME, ID) KAI_STR(#NAME),
	XTOKEN_KEYWORDS
#undef X
};

static constexpr kai_str token_type_string(token_type ty) {
	switch( ty )
	{
	default: return {0, nullptr};

	case token_identifier: return KAI_STR("identifier");
	case token_directive:  return KAI_STR("directive");
	case token_string:     return KAI_STR("string");
	case token_number:     return KAI_STR("number");

	case '->':             return KAI_STR("'->'");

#define X(NAME, ID) case ID: return KAI_STR(#NAME);
		XTOKEN_KEYWORDS
#undef X
	}
}

struct Token {
	token_type      type;
	kai_str         string;
	kai_int         line_number;
	kai_Number_Info number;
};

struct Lexer_Context {
	Token      currentToken;
	Token      peekedToken;

	kai_str    source;
	kai_int    cursor;
	kai_u32    line_number;
	bool       peeking = false;

	Lexer_Context(kai_str const& src):
		source(src),
		cursor(0),
		line_number(1)
	{}

	// Get next token after the current
	Token& next_token();

	// Get next token after the current without replacing the current
	Token& peek_token();


private:
	Token generate_token();

	void parse_number_bin(kai_u64& n);
	void parse_number_dec(kai_u64& n);
	void parse_number_hex(kai_u64& n);
	
	void parse_multi_token(Token& t, kai_u8 current);

	bool next_character_equals(kai_u8 c) {
		auto next = cursor + 1;
		if( next >= source.count )
			return false; // No characters next, can't be equal
		return c == source.data[next];
	}
};
