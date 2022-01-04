#pragma once
#include "defines.h"
// TODO: Assertion functions
#define memset(str, c, n) __builtin_memset(str, c, n)
#define memcpy(dst, src, n) __builtin_memcpy(dst, src, n)
unsigned int strlen(const char *str);
int strncmp(const char *str1, const char *str2, unsigned int num);
bool startswith(const char *p, const char *q);
int vprintf(const char *fmt, va_list ap);
int printf(const char *fmt, ...);

void print(const char *src);
void print_int(int s);
void print_uint(unsigned int s);
unsigned int str_lu(char *str, char **end);

#include "tokenize.h"
void verror_at(char *loc, char *fmt, va_list ap);
void error_at(char *loc, char *fmt, ...);
void error(const char *str);
void error_tok(const Token *token, char *fmt, ...);

// TODO: Look into optimizing this
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
