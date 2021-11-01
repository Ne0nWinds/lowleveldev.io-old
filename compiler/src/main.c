#include "defines.h"
#include "tokenize.h"
#include "standard_functions.h"

typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NEG,
	ND_NUM,
	ND_EQ,
	ND_NE,
	ND_LT,
	ND_LE,
} NodeKind;

typedef struct Node Node;
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
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
static Node *new_unary(NodeKind kind, Node *expr) {
	Node *node = new_node(kind);
	node->lhs = expr;
	return node;
}
static Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}
static bool equal(const Token *token, char *op) {
	for (unsigned int i = 0; i < token->len; ++i) {
		if (token->loc[i] != op[i]) return 0;
	}
	return (op[token->len] == '\0');
}

static char compile_text[1024] = {0};

void skip(char *s) {
	const Token *tok = CurrentToken();
	if (!equal(tok, s))
		error_tok(tok, "expected '%s'", s);
	NextToken();
}

static bool error_parsing = false;
static Node *expr();
static Node *mul();
static Node *unary();
static Node *primary();

static Node *equality();
static Node *relational();
static Node *add();

static Node *expr() {
	return equality();
}

static Node *add() {
	Node *node = mul();

	for (;;) {
		if (equal(CurrentToken(), "+")) {
			NextToken();
			node = new_binary(ND_ADD, node, mul());
			continue;
		}

		if (equal(CurrentToken(), "-")) {
			NextToken();
			node = new_binary(ND_SUB, node, mul());
			continue;
		}

		return node;
	}
}

static Node *equality() {
	Node *node = relational();

	for (;;) {
		if (equal(CurrentToken(), "==")) {
			NextToken();
			node = new_binary(ND_EQ, node, relational());
			continue;
		}

		if (equal(CurrentToken(), "!=")) {
			NextToken();
			node = new_binary(ND_NE, node, relational());
			continue;
		}

		return node;
	}
}

static Node *relational() {
	Node *node = add();

	for (;;) {
		if (equal(CurrentToken(), "<")) {
			NextToken();
			node = new_binary(ND_LT, node, add());
			continue;
		}
		if (equal(CurrentToken(), "<=")) {
			NextToken();
			node = new_binary(ND_LE, node, add());
			continue;
		}
		if (equal(CurrentToken(), ">")) {
			NextToken();
			node = new_binary(ND_LT, node, add());
			continue;
		}
		if (equal(CurrentToken(), ">=")) {
			NextToken();
			node = new_binary(ND_LE, node, add());
			continue;
		}

		return node;
	}
}

static Node *mul() {
	Node *node = unary();

	for (;;) {
		if (equal(CurrentToken(), "*")) {
			NextToken();
			node = new_binary(ND_MUL, node, unary());
			continue;
		}

		if (equal(CurrentToken(), "/")) {
			NextToken();
			node = new_binary(ND_DIV, node, unary());
			continue;
		}

		return node;
	}
}

static Node *unary() {
	if (equal(CurrentToken(), "+")) {
		NextToken();
		return unary();
	}

	if (equal(CurrentToken(), "-")) {
		NextToken();
		return new_unary(ND_NEG, unary());
	}

	return primary();
}

static Node *primary() {
	if (equal(CurrentToken(), "(")) {
		NextToken();
		Node *node = expr();
		skip(")");
		return node;
	}

	if (CurrentToken()->kind == TK_NUM) {
		Node *node = new_num(CurrentToken()->val);
		NextToken();
		return node;
	}

	error_tok(CurrentToken(), "expected an expression");
	error_parsing = true;
	return 0;
}

static unsigned char compiled_code[1024] = {0};
static unsigned char *c = 0;
unsigned int n_byte_length = 0;

#define EncodeLEB128(writeable, x, n_byte_length) {\
	typeof(x) value = x;\
	unsigned char *src = writeable;\
	unsigned char byte;\
	do {\
		byte = (value & 0x7F) | 0x80;\
		value >>= 7;\
		n_byte_length += 1;\
		*(src)++ = byte;\
	} while ((value && !(byte & 0x40)) || (value != -1 && (byte & 0x40)));\
	byte &= 0x7F;\
	*(src - 1) = byte;\
}

static void gen_expr(Node *node) {
	switch (node->kind) {
		case ND_NUM: {
			c[n_byte_length++] = OP_I32_CONST;
			EncodeLEB128(c + n_byte_length, node->val, n_byte_length);
			print("OP_I32_CONST");
			print_int(node->val);
			return;
		} break;
		case ND_NEG: {
			gen_expr(node->lhs);
			c[n_byte_length++] = OP_I32_CONST;
			EncodeLEB128(c + n_byte_length, -1, n_byte_length);
			print("OP_I32_CONST");
			print_int(-1);
			c[n_byte_length++] = OP_I32_MUL;
			print("OP_I32_MUL");
			return;
		}
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
		case ND_EQ: {
			c[n_byte_length++] = OP_I32_EQ;
			print("OP_I32_EQ");
		} break;
		case ND_NE: {
			c[n_byte_length++] = OP_I32_NE;
			print("OP_I32_NE");
		} break;
		case ND_LT: {
			c[n_byte_length++] = OP_I32_LT_S;
			print("OP_I32_LT");
		} break;
		case ND_LE: {
			c[n_byte_length++] = OP_I32_LE_S;
			print("OP_I32_LE");
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

	error_parsing = false;
	if (!tokenize(ct)) return 0;

	ResetCurrentToken();
	memset(AllNodes, 0, sizeof(AllNodes));
	CurrentNode = AllNodes;
	Node *node = expr();

	if (error_parsing) return 0;

	if (CurrentToken()->kind != TK_EOF)
		error_tok(CurrentToken(), "extra token");

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
