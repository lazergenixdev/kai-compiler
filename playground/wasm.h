// Header for easier compilation to WASM modules
// Clang Flags: --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-dynamic

#ifndef WASM_H
#define WASM_H
#include <stddef.h> // --> NULL, size_t

#define WASM_EXPORT __attribute__((__visibility__("default"))) extern
#define WASM_IMPORT(RET,NAME) __attribute__((import_module("env"), import_name(#NAME))) extern RET __env_ ## NAME
#define WASM_PAGE_SIZE 65536

void *memset(void *dest, int c, size_t n)
{
	unsigned char *s = dest;
	for (; n; n--, s++) *s = c;
	return dest;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;
	for (; n; n--) *d++ = *s++;
	return dest;
}

// NOTE: start is not checked for nullptr
size_t strlen(const char* start)
{
    const char* end = start;
    while (*end != '\0') ++end;
    return end - start;
}

#endif // WASM_H
