// DebugAlloc.h -- the retail debug allocator entry points (all cdecl).  One shared set of
// declarations; the addresses are the call targets (tier 1), the names are tier 3.
#ifndef DEBUG_ALLOC_H
#define DEBUG_ALLOC_H

// 0x004a2e20: malloc(size, file, line) with category accounting; the plain-array sibling of
// operator new (same name as src/reconstructed/DebugAlloc.h).
void* DebugMalloc(unsigned int size, const char* file, int line);

// 0x004a3010: operator new(size, __FILE__, __LINE__).  The (size, file, line) push order
// is confirmed at 0x0043a344.
void* operator new(unsigned int size, const char* file, int line);

// 0x004a2e60: the matching delete (ptr, file, line).  It also unwinds a failed
// constructor after the placement form of new.
void operator delete(void* p, const char* file, int line);

// 0x004a2ec0: realloc(ptr, size, __FILE__, __LINE__).
void* DebugRealloc(void* p, unsigned int size, const char* file, int line);

#endif
