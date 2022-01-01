#pragma once
#include "WebAssemblyConstants.h"
#include <wasm_simd128.h>

#define is_digit(c) (c >= '0' && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\r' || c == '\n' || c == '\t')
#define len(arr) (sizeof(arr) / sizeof(*arr))
#define is_punct(c) (c == '+' || c == '-' || c == '/' || c == '*' || c == '(' || c == ')' || c == '>' || c == '<' || c == ';' || c == '=' || c == '{' || c == '}' || c == '*' || c == '&' || c == ',')

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;
