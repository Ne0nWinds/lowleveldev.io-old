#include "defines.h"
#include "tokenize.h"
#include "standard_functions.h"
#include "codegen.h"

static char compile_text[1024] = {0};

static unsigned char compiled_code[1024] = {0};
static unsigned char *c = 0;
unsigned int n_byte_length = 0;

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

__attribute__((export_name("compile")))
extern unsigned int compile() {

	c = compiled_code;
	memset(c, 0, sizeof(compiled_code));

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

	c[0] = SECTION_MEMORY;
	c[1] = 0x3;
	c[2] = 0x1;
	c[3] = 0x0;
	c[4] = 0x1;
	c += 5;

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

	Token *t = tokenize(ct);
	if (!t) return 0;

	Node *node = ParseTokens();

	if (!node) return 0;

#if _DEBUG
	print_tree(node);
#endif

	if (CurrentToken()->kind != TK_EOF)
		error_tok(CurrentToken(), "extra token");

	gen_expr(node, &n_byte_length, c);

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
