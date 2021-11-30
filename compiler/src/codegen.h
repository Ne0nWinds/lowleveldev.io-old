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
	ND_EXPR,
	ND_ASSIGN,
	ND_RETURN,
	ND_VAR,
	ND_BLOCK
} NodeKind;

typedef struct Node Node;
typedef struct Obj Obj;
typedef struct Function Function;

struct Node {
	NodeKind kind;
	Node *next;
	Node *lhs;
	Node *rhs;
	Node *body;
	union {
		Obj *var; // ND_VAR
		int val; // ND_NUM
	};
};

struct Obj {
	char *name;
	int offset;
};

struct Function {
	Node *body;
	Obj *locals;
	int stack_size;
};

void gen_expr(Function *prog, unsigned int *byte_length, unsigned char *c);
Function *ParseTokens();
void print_tree(Node *node);
