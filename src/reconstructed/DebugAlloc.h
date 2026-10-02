#pragma once

// Retail debug allocator entry points (all cdecl). Call targets and argument
// order are decoded from retail call sites; the names are provisional. The
// file/line arguments are the original __FILE__/__LINE__ values, so callers
// pass the retail line number literally. See docs/ALLOCATION.md.

// 0x004a2e20: malloc(size, file, line) with category accounting.
void* DebugMalloc(unsigned int size, const char* file, int line);

// 0x004a2fc0: calloc(count, size, file, line) with category accounting.
void* DebugCalloc(unsigned int count, unsigned int size, const char* file, int line);

// 0x004a3010: operator new(size, __FILE__, __LINE__).
void* operator new(unsigned int size, const char* file, int line);

// 0x004a2e60: the matching delete (ptr, file, line); also frees DebugMalloc blocks.
void operator delete(void* p, const char* file, int line);

// 0x004a2ec0: realloc(ptr, size, __FILE__, __LINE__).
void* DebugRealloc(void* p, unsigned int size, const char* file, int line);
