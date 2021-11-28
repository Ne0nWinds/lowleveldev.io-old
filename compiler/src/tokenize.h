#pragma once

typedef enum {
	TK_EOF = 0,
	TK_PUNCT,
	TK_IDENTIFIER,
	TK_KEYWORD,
	TK_NUM,
} TokenKind;

typedef struct Token Token;
struct Token {
	TokenKind kind;
	unsigned int val;
	char *loc;
	unsigned int len;
};

Token *tokenize(char *p);

const Token *CurrentToken();
const Token *NextToken();
void ResetCurrentToken();
