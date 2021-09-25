#define export __attribute__((visibility("default")))
#include "WebAssemblyConstants.h"

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

export char *get_mem_addr() {
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

int atoi(const char *str) {
	int num = 0;
	while (*str >= '0' && *str <= '9') {
		num *= 10;
		num += *str - '0';
		++str;
	}
	return num;
}

int get_int_byte_length(int n) {
	if (n & 0xFF000000) return 4;
	if (n & 0x00FF0000) return 3;
	if (n & 0x0000FF00) return 2;
	return 1;
}

export unsigned int compile() {

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

	int n = atoi(compile_text);
	int n_byte_length = EncodeLEB128(c + 6, n);
	// print("Byte Length:");
	// print_int(n_byte_length);

	c[0] = SECTION_CODE;
	c[1] = 5 + n_byte_length;
	c[2] = 0x1;
	c[3] = 3 + n_byte_length;
	c[4] = 0x0;
	c[5] = OP_I32_CONST;
	c[6 + n_byte_length] = OP_END;
	c += 7 + n_byte_length;

	return c - compiled_code;
}

export unsigned char *get_compiled_code() {
	return compiled_code;
}
