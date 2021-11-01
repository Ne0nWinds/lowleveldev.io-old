#pragma once
#include "WebAssemblyConstants.h"
#include <wasm_simd128.h>

#define is_digit(c) (c >= '0' && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\r' || c == '\n' || c == '\t')
#define len(arr) (sizeof(arr) / sizeof(*arr))
#define is_punct(c) (c == '+' || c == '-' || c == '/' || c == '*' || c == '(' || c == ')' || c == '>' || c == '<')

#include <stdarg.h>
#include <stdbool.h>

void *memset(void *str, int c, unsigned int n);
void *memcpy(void *dest, const void *src, unsigned int n);
unsigned int strlen(const char *str);
bool startswith(const char *p, const char *q);
int vprintf(const char *fmt, va_list ap);
int printf(const char *fmt, ...);
void print(const char *src);
void print_int(int s);
void print_uint(unsigned int s);
unsigned int str_lu(char *str, char **end);

__attribute__((always_inline))
void *memset(void *str, int c, unsigned int n) {
	v128_t c_vectorized = wasm_i8x16_splat(c);

	unsigned int i = 0;
	for (;i < n; i += 16) {
		wasm_v128_store(str + i, c_vectorized);
	}

	for (; i < n; ++i) {
		((unsigned char *)str)[i] = c;
	}

	return str;
}

void *memcpy(void *dest, const void *src, unsigned int n) {
	unsigned int i = 0;
	for (; i < n; i += 16) {
		v128_t src_vec = wasm_v128_load(src + i);
		wasm_v128_store(dest + i, src_vec);
	}
	for (; i < n; ++i) {
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
	}
	return dest;
}

unsigned int strlen(const char *str) {
	unsigned int n = 0;
	while (*str++) ++n;
	return n;
}

bool startswith(const char *p, const char *q) {
	while (*q) {
		if (*q != *p) return false;
		q += 1;
		p += 1;
	}
	return true;
}

int vprintf(const char *fmt, va_list ap) {
	static char buffer[512] = {0};
	unsigned int b = 0;
	for (unsigned int f = 0; fmt[f] && f < len(buffer); ++f) {
		if (fmt[f] != '%') {
			buffer[b++] = fmt[f];
		} else {
			f += 1;
			switch (fmt[f]) {
				case 's': {
					const char *str = va_arg(ap, const char *);
					while (*str) buffer[b++] = *str++;
				} break;
				case '%': {
					buffer[b++] = '%';
				} break;
				case 'd': {
					int d = va_arg(ap, int);
					buffer[b++] = d + '0';
				} break;
				default: {
					return -1;
				} break;
			}
		}
	}
	_print(buffer, b);
	return b;
}
int printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int b = vprintf(fmt, ap);
	va_end(ap);
	return b;
}

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

unsigned int str_lu(char *str, char **end) {
	unsigned int num = 0;
	while (*str >= '0' && *str <= '9') {
		num *= 10;
		num += *str - '0';
		str += 1;
		*end += 1;
	}
	return num;
}
