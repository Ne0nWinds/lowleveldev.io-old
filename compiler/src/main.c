#include "WebAssemblyConstants.h"

#define is_digit(c) (c >= '0' && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\r' || c == '\n' || c == '\t')
#define len(arr) (sizeof(arr) / sizeof(arr[0]))
#define is_punct(c) (c == '+' || c == '-' || c == '/' || c == '*' || c == '(' || c == ')')

#include <stdarg.h>
#include <stdbool.h>
// clang generates seems to expect memset/memcpy to be defined like this?
void *memset(void *str, int c, unsigned int n) {
	for (unsigned int i = 0; i < n; ++i) {
		((unsigned char *)str)[i] = c;
	}
	return str;
}
void *memcpy(void *dest, const void *src, unsigned int n) {
	for (unsigned int i = 0; i < n; ++i) {
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
	}
	return dest;
}
unsigned int strlen(const char *str) {
	unsigned int n = 0;
	while (*str++) ++n;
	return n;
}
int vprintf(const char *fmt, va_list ap) {
	static char buffer[512] = {0};
	unsigned int b = 0;
	for (unsigned int f = 0; fmt[f] && f < len(buffer); ++f) {
		if (fmt[f] != '%') {
			buffer[b++] = fmt[f];
		} else {
			f += 1;
			switch (fmt[f]) {
				case 's': {
					const char *str = va_arg(ap, const char *);
					while (*str) buffer[b++] = *str++;
				} break;
				case '%': {
					buffer[b++] = '%';
				} break;
				case 'd': {
					int d = va_arg(ap, int);
					buffer[b++] = d + '0';
				} break;
				default: {
					return -1;
				} break;
			}
		}
	}
	_print(buffer, b);
	return b;
}
int printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int b = vprintf(fmt, ap);
	va_end(ap);
	return b;
}

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

typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};
typedef struct Token Token;
struct Token {
	TokenKind kind;
	unsigned int val;
	char *loc;
	unsigned int len;
};

Node AllNodes[512] = {0};
Node *CurrentNode = AllNodes;

static Node *new_node(NodeKind kind) {
	Node *node = CurrentNode++;
	node->kind = kind;
	return node;
}
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}
static Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}
static bool equal(Token *token, char *op) {
	for (unsigned int i = 0; i < token->len; ++i) {
		if (token->loc[i] != op[i]) return 0;
	}
	return (op[token->len] == '\0');
}

static char compile_text[1024] = {0};
static void verror_at(char *loc, char *fmt, va_list ap) {
	int pos = loc - compile_text;
	vprintf(fmt, ap);
}
static void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror_at(loc, fmt, ap);
	va_end(ap);
}
void error_tok(Token *token, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror_at(token->loc, fmt, ap);
	va_end(ap);
}
void error(const char *str) {
	print(str);
}

Token AllTokens[512] = {0};
Token *CurrentToken = AllTokens;

static Token *skip(Token *tok, char *s) {
	if (!equal(tok, s))
		error_tok(tok, "expected '%s'", s);
	return tok + 1;
}

static bool error_parsing = false;
static Node *expr();
static Node *mul();
static Node *primary();

static Node *expr() {
	Node *node = mul();

	for (;;) {
		if (equal(CurrentToken, "+")) {
			CurrentToken += 1;
			node = new_binary(ND_ADD, node, mul());
			continue;
		}

		if (equal(CurrentToken, "-")) {
			CurrentToken += 1;
			node = new_binary(ND_SUB, node, mul());
			continue;
		}

		return node;
	}
}

static Node *mul() {
	Node *node = primary();

	for (;;) {
		if (equal(CurrentToken, "*")) {
			CurrentToken += 1;
			node = new_binary(ND_MUL, node, primary());
			continue;
		}

		if (equal(CurrentToken, "/")) {
			CurrentToken += 1;
			node = new_binary(ND_DIV, node, primary());
			continue;
		}

		return node;
	}
}

static Node *primary() {
	if (equal(CurrentToken, "(")) {
		CurrentToken += 1;
		Node *node = expr();
		CurrentToken = skip(CurrentToken, ")");
		return node;
	}

	if (CurrentToken->kind == TK_NUM) {
		Node *node = new_num(CurrentToken->val);
		CurrentToken += 1;
		return node;
	}

	error_tok(CurrentToken, "expected an expression");
	error_parsing = true;
	return 0;
}

static unsigned char compiled_code[1024] = {0};
static unsigned char *c = 0;
unsigned int n_byte_length = 0;

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

static void gen_expr(Node *node) {
	if (node->kind == ND_NUM) {
		c[n_byte_length++] = OP_I32_CONST;
		n_byte_length += EncodeLEB128(c + n_byte_length, node->val);
		print("OP_I32_CONST");
		print_int(node->val);
		return;
	}
	gen_expr(node->lhs);
	gen_expr(node->rhs);

	switch (node->kind) {
		case ND_ADD: {
			c[n_byte_length++] = OP_I32_ADD;
			print("OP_I32_ADD");
		} break;
		case ND_SUB: {
			c[n_byte_length++] = OP_I32_SUB;
			print("OP_I32_SUB");
		} break;
		case ND_MUL: {
			c[n_byte_length++] = OP_I32_MUL;
			print("OP_I32_MUL");
		} break;
		case ND_DIV: {
			c[n_byte_length++] = OP_I32_DIV_U;
			print("OP_I32_DIV");
		} break;
		default: {
			error("invalid expression");
		}
	}
}

__attribute__((export_name("get_mem_addr")))
char *get_mem_addr() {
	return compile_text;
}

static unsigned int get_number(Token *tok) {
	if (tok->kind != TK_NUM)
		error_tok(tok, "expected a number");
	return tok->val;
}

static void print_token_type(Token t) {
	switch (t.kind) {
		case TK_EOF: print("TK_EOF"); break;
		case TK_PUNCT: print("TK_PUNCT"); break;
		case TK_NUM: print("TK_NUM"); break;
	}
}

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
		} else if (is_punct(*p)) {
			current = new_token(TK_PUNCT, p, p + 1);
			++p;
		} else {
			error_at(p, "invalid token %s", __FILE_NAME__);
			return 0;
		}
	}
	current = new_token(TK_EOF, p, p);
	return AllTokens;
}

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

	c = compiled_code;

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

	n_byte_length = 0;

	if (!tokenize(ct)) return 0;

	CurrentToken = AllTokens;
	memset(AllNodes, 0, CurrentNode - AllNodes);
	CurrentNode = AllNodes;
	Node *node = expr();

	if (error_parsing) return 0;

	if (CurrentToken->kind != TK_EOF)
		error_tok(CurrentToken, "extra token");

	gen_expr(node);

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
