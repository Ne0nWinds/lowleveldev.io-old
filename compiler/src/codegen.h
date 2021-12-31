#pragma once
#include "tokenize.h"
#include "defines.h"

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
	ND_BLOCK,
	ND_IF,
	ND_FOR,
	ND_ADDR,
	ND_DEREF,
	ND_FUNCCALL
} NodeKind;

typedef enum {
	TYPE_INT = 1,
	TYPE_PTR,
} TypeKind;

typedef struct Node Node;
typedef struct Obj Obj;
typedef struct Function Function;
typedef struct Type Type;

struct Node {
	NodeKind kind;
	Node *next;
	Node *lhs;
	Node *rhs;
	Node *body;
	Token *tok;
	Type *type;

	// complex statements
	union {
		struct _if {
			Node *condition;
			Node *then;
			Node *els;
		} _if;
		struct _for {
			Node *init;
			Node *condition;
			Node *increment;
			Node *then;
		} _for;
	};

	union {
		Obj *var; // ND_VAR
		int val; // ND_NUM
		char *funcname; // ND FUNCCALL
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

struct Type {
	TypeKind kind;
	Type *base;
	Token *name;
};

void gen_expr(Function *prog, unsigned int *byte_length, unsigned char *c);
Function *ParseTokens();
void print_tree(Node *node);

void add_type(Node *node);
