#include "tokenize.h"
#include "defines.h"
#include "standard_functions.h"

static Token AllTokens[512] = {0};
static Token *_CurrentToken = AllTokens;

const Token *CurrentToken() {
	return _CurrentToken;
}

const Token *NextToken() {
	return ++_CurrentToken;
}

void ResetCurrentToken() {
	_CurrentToken = AllTokens;
}

static void print_token_type(Token t) {
	switch (t.kind) {
		case TK_EOF: print("TK_EOF"); break;
		case TK_PUNCT: print("TK_PUNCT"); break;
		case TK_NUM: print("TK_NUM"); break;
		case TK_IDENTIFIER: print("TK_IDENTIFIER"); break;
		case TK_KEYWORD: print("TK_KEYWORD"); break;
	}
}

static int read_punct(char *p) {
	if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
		return 2;
	}
	return is_punct(*p);
}

static Token *new_token(TokenKind kind, char *start, unsigned int length) {
	Token *tok = _CurrentToken;
	_CurrentToken += 1;
	tok->kind = kind;
	tok->loc = start;
	tok->len = length;
#if _DEBUG
	print_token_type(*tok);
#endif
	return tok;
}

static bool is_ident1(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_ident2(char c) {
	return is_ident1(c) || (c >= '0' && c <= '9');
}

static bool is_keyword(const char *c, const unsigned int c_len) {
	static char *kw[] = {"return", "if", "else"};
	for (int i = 0; i < len(kw); ++i) {
		if (!strncmp(c, kw[i], c_len))
			return true;
	}
	return false;
}

Token *tokenize(char *p) {
	memset(AllTokens, 0, sizeof(AllTokens));
	ResetCurrentToken();
	while (*p) {
		if (is_whitespace(*p)) {
			p += 1;
			continue;
		}

		if (is_digit(*p)) {
			Token *current = new_token(TK_NUM, p, 0);
			char *q = p;
			current->val = str_lu(p, &p);
			current->len = p - q;
			continue;
		}

		int punct_len = read_punct(p);
		if (punct_len) {
			new_token(TK_PUNCT, p, punct_len);
			p += punct_len;
			continue;
		}

		if (is_ident1(*p)) {
			char *start = p;
			do {
				++p;
			} while (is_ident2(*p));
			unsigned int length = p - start;
			new_token(is_keyword(start, length) ? TK_KEYWORD : TK_IDENTIFIER, start, length);
			continue;
		}

		error_at(p, "invalid token %s, %s", __FILE_NAME__, p);
		return 0;
	}
	new_token(TK_EOF, p, 0);
	return AllTokens;
}

