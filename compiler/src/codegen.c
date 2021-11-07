#include "codegen.h"
#include "tokenize.h"
#include "standard_functions.h"

Node AllNodes[512] = {0};
Node *CurrentNode = AllNodes;
static bool error_parsing = false;

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

void skip(char *s) {
	const Token *tok = CurrentToken();
	if (!equal(tok, s)) {
		error_tok(tok, "expected '%s'", s);
		error_parsing = true;
	}
	NextToken();
}

Node *ParseTokens();
static Node *expr();
static Node *new_expr();
static Node *mul();
static Node *unary();
static Node *primary();

static Node *equality();
static Node *relational();
static Node *add();

Node *ParseTokens() {
	ResetCurrentToken();
	error_parsing = false;
	memset(AllNodes, 0, sizeof(AllNodes));
	Node *head = new_expr();
	Node *LocalCurrent = head;
	while (CurrentToken()->kind && !error_parsing) {
		LocalCurrent->next = new_expr();
		LocalCurrent = LocalCurrent->next;
	}
	return (!error_parsing) ? head : 0;
}
static Node *new_expr() {
	Node *node = new_unary(ND_EXPR, expr());
	skip(";");
	return node;
}

static Node *expr() {
	return equality();
}

static Node *add() {
	Node *node = mul();

	for (;;) {
		if (equal(CurrentToken(), "+")) {
			NextToken();
			print("hit -- add");
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
			node = new_binary(ND_GT, node, add());
			continue;
		}
		if (equal(CurrentToken(), ">=")) {
			NextToken();
			node = new_binary(ND_GE, node, add());
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

static char *NodeKind_str[] = {
	"ND_ADD",
	"ND_SUB",
	"ND_MUL",
	"ND_DIV",
	"ND_NEG",
	"ND_NUM",
	"ND_EQ",
	"ND_NE",
	"ND_LT",
	"ND_LE",
	"ND_GT",
	"ND_GE",
	"ND_EXPR"
};

static void _print_tree(Node *node) {
	print(NodeKind_str[node->kind]);
	if (node->lhs) print_tree(node->lhs);
	if (node->rhs) print_tree(node->rhs);
}

void print_tree(Node *node) {
	while (node) {
		_print_tree(node);
		node = node->next;
	}
}

static unsigned int n_byte_length;
static unsigned char *c = 0;

static void _gen_expr(Node *node) {
	switch (node->kind) {
		case ND_NUM: {
			c[n_byte_length++] = OP_I32_CONST;
			EncodeLEB128(c + n_byte_length, node->val, n_byte_length);
			print("OP_I32_CONST");
			print_int(node->val);
			return;
		} break;
		case ND_NEG: {
			_gen_expr(node->lhs);
			c[n_byte_length++] = OP_I32_CONST;
			EncodeLEB128(c + n_byte_length, -1, n_byte_length);
			print("OP_I32_CONST");
			print_int(-1);
			c[n_byte_length++] = OP_I32_MUL;
			print("OP_I32_MUL");
			return;
		}
	}
	_gen_expr(node->lhs);
	_gen_expr(node->rhs);

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
		case ND_GT: {
			c[n_byte_length++] = OP_I32_GT_S;
			print("OP_I32_GT");
		} break;
		case ND_GE: {
			c[n_byte_length++] = OP_I32_GE_S;
			print("OP_I32_GE");
		} break;
		default: {
			error("invalid expression");
			return;
		}
	}
}

void gen_expr(Node *node, unsigned int *byte_length, unsigned char *output_code) {
	n_byte_length = *byte_length;
	c = output_code;
	for (Node *n = node; n && n->kind == ND_EXPR; n = n->next) {
		_gen_expr(n->lhs);
		if (n->next) {
			print("OP_DROP");
			c[n_byte_length++] = OP_DROP;
		}
	}
	*byte_length = n_byte_length;
}
