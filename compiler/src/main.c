#include "WebAssemblyConstants.h"

#define is_digit(c) (c >= '0' && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\r' || c == '\n' || c == '\t')
#define len(arr) (sizeof(arr) / sizeof(arr[0]))

void print(const char *src) {
	unsigned int i = 0;
	while (src[i] != 0) ++i;
	_print(src, i);
}
void print_int(int s) {
	_print(s, 0);
}
void print_uint(unsigned int s) {
	_print(s, 0);
}

typedef enum {
	TK_EOF = 0,
	TK_PUNCT,
	TK_NUM,
} TokenKind;

typedef struct Token Token;
struct Token {
	TokenKind kind;
	Token *next;
	unsigned int val;
	char *loc;
	unsigned int len;
};

// clang generates seems to expect memset to be defined like this?
void *memset(void *str, int c, unsigned int n) {
	unsigned char *str_as_byte = str;
	for (unsigned int i = 0; i < n; ++i) {
		str_as_byte[i] = c;
	}
	return str;
}

#include <stdarg.h>
#include <stdbool.h>
static void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char buffer[1024] = {0};
	unsigned int b = 0;
	for (unsigned int f = 0; fmt[f]; ++f) {
		if (fmt[f] != '%') {
			buffer[b++] = fmt[f];
		} else {
			f += 1;
			switch (fmt[f]) {
				case 's': {
					const char *str = va_arg(ap, const char *);
					while (*str)
						buffer[b++] = *str++;
				} break;
				case '%': {
					buffer[b++] = '%';
				} break;
			}
		}
	}
	_print(buffer, b);
	va_end(ap);
}

static bool equal(Token *token, char *op) {
	for (unsigned int i = 0; i < token->len; ++i) {
		if (token->loc[i] != op[i]) return 0;
	}
	return (op[token->len] == '\0');
}

static Token *skip(Token *tok, char *s) {
	if (!equal(tok, s))
		error("expected '%s'", s);
	return tok->next;
}

static unsigned int get_number(Token *tok) {
	if (tok->kind != TK_NUM)
		error("expected a number");
	return tok->val;
}

static void print_token_type(Token t) {
	switch (t.kind) {
		case TK_EOF: print("TK_EOF"); break;
		case TK_PUNCT: print("TK_PUNCT"); break;
		case TK_NUM: print("TK_NUM"); break;
	}
}

Token AllTokens[512] = {0};
Token *CurrentToken = AllTokens;

static Token *new_token(TokenKind kind, char *start, char *end) {
	Token *tok = CurrentToken;
	CurrentToken += 1;
	tok->kind = kind;
	tok->loc = start;
	tok->len = end - start;
	return tok;
}

unsigned int str_lu(char *str, char **end) {
	unsigned int num = 0;
	while (*str >= '0' && *str <= '9') {
		num *= 10;
		num += *str - '0';
		str += 1;
		*end += 1;
	}
	return num;
}

static Token *tokenize(char *p) {
	CurrentToken = AllTokens;
	Token *current = 0;

	while (*p) {
		while (is_whitespace(*p)) p += 1;

		if (is_digit(*p)) {
			current = new_token(TK_NUM, p, p);
			char *q = p;
			current->val = str_lu(p, &p);
			current->len = p - q;
		} else if (*p == '+' || *p == '-') {
			current = new_token(TK_PUNCT, p, p + 1);
			++p;
		} else {
			error("invalid token %s", __FILE_NAME__);
			return 0;
		}
		print_token_type(*current);
	}
	current = new_token(TK_EOF, p, p);
	return AllTokens;
}

static unsigned int EncodeLEB128(unsigned char *src, unsigned int value) {
	unsigned int length = 0;
	unsigned char byte;
	do {
		byte = (value & 0x7F) | 0x80;
		value >>= 7;
		length += 1;
		*src++ = byte;
	} while (value || byte & 0x40);

	byte &= 0x7F;
	*(src - 1) = byte;

	return length;
}

char compile_text[1024] = {0};

__attribute__((export_name("get_mem_addr")))
char *get_mem_addr() {
	return compile_text;
}

unsigned char compiled_code[1024] = {0};

static unsigned long WASM_header(unsigned char *c) {
	c[0] = 0;
	c[1] = 'a';
	c[2] = 's';
	c[3] = 'm';
	c[4] = 1;
	c[5] = 0;
	c[6] = 0;
	c[7] = 0;
	return 8;
}

int atoi(char **str) {
	int num = 0;
	int is_negative = (**str == '-');
	str += is_negative;
	while (**str >= '0' && **str <= '9') {
		num *= 10;
		num += **str - '0';
		*str += 1;
	}
	if (is_negative) num *= -1;
	return num;
}

int get_int_byte_length(int n) {
	if (n & 0xFF000000) return 4;
	if (n & 0x00FF0000) return 3;
	if (n & 0x0000FF00) return 2;
	return 1;
}

__attribute__((export_name("compile")))
extern unsigned int compile() {

	unsigned char *c = compiled_code;

	c += WASM_header(c);

	c[0] = SECTION_TYPE;
	c[1] = 0x05;
	c[2] = 0x01;
	c[3] = 0x60;
	c[4] = 0;
	c[5] = 1;
	c[6] = VAL_I32;
	c += 7;

	c[0] = SECTION_FUNC;
	c[1] = 0x2;
	c[2] = 0x1;
	c[3] = 0x0;
	c += 4;

	c[0] = SECTION_EXPORT;
	c[1] = 0x08;
	c[2] = 0x01;
	c[3] = 4;
	c[4] = 'm';
	c[5] = 'a';
	c[6] = 'i';
	c[7] = 'n';
	c[8] = EXPORT_FUNC;
	c[9] = 0x0;
	c += 10;

	c += 5;

	char *ct = (char *)compile_text;

	int n_byte_length = 0;

	if (!tokenize(ct)) return 0;

	c[n_byte_length++] = OP_I32_CONST;
	n_byte_length += EncodeLEB128(c + n_byte_length, get_number(AllTokens));

	for (unsigned int i = 1; AllTokens[i].kind; ++i) {
		if (equal(AllTokens + i, "+")) {
			print("PLUS");
			c[n_byte_length++] = OP_I32_CONST;
			++i;
			n_byte_length += EncodeLEB128(c + n_byte_length, get_number(AllTokens + i));
			c[n_byte_length++] = OP_I32_ADD;
		} else if (equal(AllTokens + i, "-")) {
			print("MINUS");
			c[n_byte_length++] = OP_I32_CONST;
			++i;
			n_byte_length += EncodeLEB128(c + n_byte_length, get_number(AllTokens + i));
			c[n_byte_length++] = OP_I32_SUB;
		}
	}

	c[n_byte_length++] = OP_END;
	c -= 5;

	c[0] = SECTION_CODE;
	c[1] = 3 + n_byte_length;
	c[2] = 0x1;
	c[3] = 1 + n_byte_length;
	c[4] = 0x0;
	c += 5 + n_byte_length;

	return c - compiled_code;
}

__attribute__((export_name("get_compiled_code")))
unsigned char *get_compiled_code() {
	return compiled_code;
}
