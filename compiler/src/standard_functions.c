#include "standard_functions.h"

__attribute__((always_inline))
void *memset(void *str, int c, unsigned int n) {
	for (int i; i < n; ++i) {
		((unsigned char *)str)[i] = c;
	}

	return str;
}

void *memcpy(void * restrict dest, const void * restrict src, unsigned int n) {
	for (unsigned int i = 0; i < n; ++i) {
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
	}
	return dest;
}

unsigned int strlen(const char *str) {
	unsigned int n = 0;
	while (*str++) ++n;
	return n;
}

bool startswith(const char * restrict p, const char * restrict q) {
	while (*q) {
		if (*q != *p) return false;
		q += 1;
		p += 1;
	}
	return true;
}

#ifndef _DEBUG
#define _print(a, b)
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
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
				case 'c': {
					const char c = va_arg(ap, int);
					buffer[b++] = c;
				} break;
				case '%': {
					buffer[b++] = '%';
				} break;
				case 'i':
				case 'd': {
					int d = va_arg(ap, int);
					if (d < 0) {
						buffer[b++] = '-';
						d *= -1;
					}
					unsigned char length = 0;
					int d_len_test = d;
					do {
						++length;
						d_len_test /= 10;
					} while (d_len_test);
					unsigned char length2 = length;
					do {
						char c = d % 10 + '0';
						d /= 10;
						--length;
						buffer[b + length] = c;
					} while (length);
					b += length2;
				} break;
				case 'u': {
					unsigned int d = va_arg(ap, unsigned int);
					unsigned char length = 0;
					int d_len_test = d;
					do {
						++length;
						d_len_test /= 10;
					} while (d_len_test);
					unsigned char length2 = length;
					do {
						char c = d % 10 + '0';
						d /= 10;
						--length;
						buffer[b + length] = c;
					} while (length);
					b += length2;
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
	_print(src, strlen(src));
}

void print_int(int s) {
	_print(s, 0);
}

void print_uint(unsigned int s) {
	_print(s, 0);
}
#pragma GCC diagnostic pop

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

void verror_at(char *loc, char *fmt, va_list ap) {
	vprintf(fmt, ap);
}
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror_at(loc, fmt, ap);
	va_end(ap);
}
void error(const char *str) {
	print(str);
}

void error_tok(const Token *token, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror_at(token->loc, fmt, ap);
	va_end(ap);
}
