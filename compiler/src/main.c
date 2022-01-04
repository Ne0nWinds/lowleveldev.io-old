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

__attribute__((export_name("compile")))
extern unsigned int compile() {

	char *ct = (char *)compile_text;

	n_byte_length = 0;

	Token *t = tokenize(ct);
	if (!t) return 0;

	Function *prog = ParseTokens();

	if (!prog) return 0;

#if _DEBUG
	print_tree(prog->body);
#endif

	if (CurrentToken()->kind != TK_EOF)
		error_tok(CurrentToken(), "extra token");

	memset(c, 0, sizeof(compiled_code));

	return gen_expr(compiled_code);
}

__attribute__((export_name("get_compiled_code")))
unsigned char *get_compiled_code() {
	return compiled_code;
}
