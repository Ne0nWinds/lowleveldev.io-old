#pragma once

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
	ND_GT,
	ND_GE,
	ND_EXPR
} NodeKind;

typedef struct Node Node;
struct Node {
	NodeKind kind;
	Node *next;
	Node *lhs;
	Node *rhs;
	int val;
};

void gen_expr(Node *node, unsigned int *byte_length, unsigned char *c);
Node *ParseTokens();
void print_tree(Node *node);
