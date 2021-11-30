#include "codegen.h"
#include "tokenize.h"
#include "standard_functions.h"

Node AllNodes[512] = {0};
Node *CurrentNode = AllNodes;
static bool error_parsing = false;

static int align_to(int n, int align) {
	return (n + align - 1) / align * align;
}

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

static Obj Locals[512];
Obj *CurrentLocal = 0;
static char *Names[2048] = {0};
char *CurrentChar = (char *)Names;

static Node *new_variable(Obj *var) {
	Node *node = new_node(ND_VAR);
	node->var = var;
	return node;
}

static Obj *new_lvar(char *name, unsigned int len) {
	Obj *var = CurrentLocal;
	CurrentLocal += 1;
	memcpy(CurrentChar, name, len);
	var->name = CurrentChar;
	CurrentChar += len;
	*CurrentChar = 0;
	CurrentChar += 1;
	return var;
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
		error_tok(tok, "expected '%s' at '%s'", s, tok->loc);
		error_parsing = true;
	}
	NextToken();
}

Function *ParseTokens();
static Node *expr();
static Node *new_expr();
static Node *mul();
static Node *unary();
static Node *primary();
static Node *equality();
static Node *relational();
static Node *add();
static Node *assign();
static Node *complex_expr();

static Obj *find_var(const Token *tok) {
	for (Obj *var = Locals; var != CurrentLocal; ++var) {
		if (strlen(var->name) == tok->len && !strncmp(tok->loc, var->name, tok->len))
			return var;
	}
	return 0;
}

Function *ParseTokens() {
	CurrentLocal = Locals;
	CurrentChar = (char *)Names;
	ResetCurrentToken();
	error_parsing = false;
	memset(AllNodes, 0, sizeof(AllNodes));
	Node *head = complex_expr();
	Node *LocalCurrent = head;
	while (CurrentToken()->kind && !error_parsing) {
		LocalCurrent->next = complex_expr();
		LocalCurrent = LocalCurrent->next;
	}
	static Function prog = {0};
	prog.body = head;
	prog.locals = Locals;
	return (!error_parsing) ? &prog : 0;
}

static Node *new_expr() {
	Node *node = expr();
	skip(";");
	return node;
}

static Node *complex_expr() {
	Node head = {};
	Node *current = &head;
	NextToken();
	do {
		if (equal(CurrentToken(), "{")) {
			current->next = complex_expr();
			current = current->next;
		} else {
			current->next = expr();
			current = current->next;
			skip(";");
		}
	} while (!equal(CurrentToken(), "}") && !error_parsing);
	skip("}");
	Node *node = new_node(ND_BLOCK);
	node->body = head.next;
	return node;
}

static Node *expr() {
	if (CurrentToken()->kind == TK_KEYWORD) {
		// only handles "return"
		NextToken();
		Node *node = new_unary(ND_RETURN, assign());
		return node;
	}

	if (equal(CurrentToken(), ";")) {
		return new_node(ND_BLOCK);
	}

	Node *node = assign();
	return node;
}

static Node *assign() {
	Node *node = equality();
	if (equal(CurrentToken(), "=")) {
		NextToken();
		node = new_binary(ND_ASSIGN, node, assign());
	}
	return node;
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

	if (CurrentToken()->kind == TK_IDENTIFIER) {
		Obj *var = find_var(CurrentToken());
		if (!var)
			var = new_lvar(CurrentToken()->loc, CurrentToken()->len);
		NextToken();
		return new_variable(var);
	}

	printf("%d\n", CurrentToken()->kind);

	error_tok(CurrentToken(), "expected an expression");
	error_parsing = true;
	return 0;
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

static int depth = 0;

static void _gen_expr(Node *node) {
	switch (node->kind) {
		case ND_BLOCK: {
			for (Node *n = node->body; n; n = n->next) {
				_gen_expr(n);
				if (n->next && depth) {
					print("OP_DROP");
					c[n_byte_length++] = OP_DROP;
					--depth;
				}
			}
			return;
		}
		case ND_NUM: {
			c[n_byte_length++] = OP_I32_CONST;
			EncodeLEB128(c + n_byte_length, node->val, n_byte_length);
			printf("OP_I32_CONST: %d\n", node->val);
			++depth;
			return;
		} break;
		case ND_NEG: {
			_gen_expr(node->lhs);
			c[n_byte_length++] = OP_I32_CONST;
			EncodeLEB128(c + n_byte_length, -1, n_byte_length);
			printf("OP_I32_CONST: %d\n", -1);
			c[n_byte_length++] = OP_I32_MUL;
			print("OP_I32_MUL");
			return;
		} break;
		case ND_VAR: {
			print("ND_VAR");
			printf("Name: %s", node->var->name);
			c[n_byte_length++] = OP_I32_CONST;
			c[n_byte_length++] = 0;
			printf("OP_I32_CONST: %d\n", 0);
			c[n_byte_length++] = OP_I32_LOAD;
			c[n_byte_length++] = 2;
			EncodeLEB128(c + n_byte_length, node->var->offset, n_byte_length);
			printf("OP_I32_LOAD: %d", node->var->offset);
			++depth;
			return;
		} break;
		case ND_ASSIGN: {
			print("ND_ASSIGN");
			c[n_byte_length++] = OP_I32_CONST;
			c[n_byte_length++] = 0;
			printf("OP_I32_CONST: %d\n", 0);
			_gen_expr(node->rhs);
			c[n_byte_length++] = OP_I32_STORE;
			c[n_byte_length++] = 2;
			EncodeLEB128(c + n_byte_length, node->lhs->var->offset, n_byte_length);
			printf("OP_I32_STORE: %d", node->lhs->var->offset);
			--depth;
			return;
		} break;
		case ND_RETURN: {
			print("ND_RETURN");
			_gen_expr(node->lhs);
			c[n_byte_length++] = OP_RETURN;
			return;
		}
	}

	_gen_expr(node->lhs);
	_gen_expr(node->rhs);

	switch (node->kind) {
		case ND_ADD: {
			c[n_byte_length++] = OP_I32_ADD;
			print("OP_I32_ADD");
			--depth;
		} break;
		case ND_SUB: {
			c[n_byte_length++] = OP_I32_SUB;
			print("OP_I32_SUB");
			--depth;
		} break;
		case ND_MUL: {
			c[n_byte_length++] = OP_I32_MUL;
			print("OP_I32_MUL");
			--depth;
		} break;
		case ND_DIV: {
			c[n_byte_length++] = OP_I32_DIV_U;
			print("OP_I32_DIV");
			--depth;
		} break;
		case ND_EQ: {
			c[n_byte_length++] = OP_I32_EQ;
			print("OP_I32_EQ");
			--depth;
		} break;
		case ND_NE: {
			c[n_byte_length++] = OP_I32_NE;
			print("OP_I32_NE");
			--depth;
		} break;
		case ND_LT: {
			c[n_byte_length++] = OP_I32_LT_S;
			print("OP_I32_LT");
			--depth;
		} break;
		case ND_LE: {
			c[n_byte_length++] = OP_I32_LE_S;
			print("OP_I32_LE");
			--depth;
		} break;
		case ND_GT: {
			c[n_byte_length++] = OP_I32_GT_S;
			print("OP_I32_GT");
			--depth;
		} break;
		case ND_GE: {
			c[n_byte_length++] = OP_I32_GE_S;
			print("OP_I32_GE");
			--depth;
		} break;
	}
}

static void assign_lvar_offsets(Function *prog) {
	int offset = 0;
	for (Obj *var = Locals; var < CurrentLocal; ++var) {
		offset += 4;
		var->offset = 128 - offset;
	}
	prog->stack_size += align_to(offset, 16);
}

void gen_expr(Function *prog, unsigned int *byte_length, unsigned char *output_code) {
	n_byte_length = *byte_length;
	c = output_code;
	depth = 0;

	assign_lvar_offsets(prog);

	_gen_expr(prog->body);

	if (depth == 0) {
		c[n_byte_length++] = OP_I32_CONST;
		c[n_byte_length++] = 0;
		printf("OP_I32_CONST: %d\n", 0);
	}
	*byte_length = n_byte_length;
}
