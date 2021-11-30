#pragma once
#include "WebAssemblyConstants.h"
#include <wasm_simd128.h>

#define is_digit(c) (c >= '0' && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\r' || c == '\n' || c == '\t')
#define len(arr) (sizeof(arr) / sizeof(*arr))
#define is_punct(c) (c == '+' || c == '-' || c == '/' || c == '*' || c == '(' || c == ')' || c == '>' || c == '<' || c == ';' || c == '=' || c == '{' || c == '}')

#include <stdarg.h>
#include <stdbool.h>
