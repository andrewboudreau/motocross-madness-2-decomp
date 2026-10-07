// MemTag.cpp -- the allocation accounting unit, 0x004a2ac0..0x004a3142: the
// category tracker (MemTagStack, src/reconstructed/MemTag.h), its static
// instance and the debug allocator entry points (DebugAlloc.h). The unit is
// bounded by its own .CRT$XCU entry 183 (0x004a2ac0, constructing the tracker
// at 0x006850c0 and registering its destructor) and MorphBastardModifier.cpp
// (first xref 0x004a3215). No __FILE__ literal or RTTI: the file name is ours
// (tier 3); the extent and the class layout are tier 2 (see
// docs/ALLOCATION.md).
//
// The three identical free bodies are told apart by their callers: the
// unwind funclets (0x005495db..) call 0x004a3060 when a constructor after
// `new(__FILE__, line)` throws, so that one is the placement operator delete;
// the deleting destructors call 0x004a30c0, the plain operator delete; the
// explicit (pointer, file, line) calls go to 0x004a2e60.

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/DebugAlloc.h"

// 0x004a3120: memcpy with the caller's file and line (DebugRealloc).
void* DebugMemcpy(void* destination, const void* source, unsigned int size, const char* file, int line);

static MemTagStack s_memTagStack;             // 0x006850c0, .CRT$XCU 183
int g_UnknownUnclaimedBytes6850dc;            // 0x006850dc: bytes counted without a tracker
MemTagStack* g_MemTagStack = &s_memTagStack;  // 0x0056df04

// 0x004a2b00
MemTagStack::MemTagStack()
{
    field_0x18 = -1;
    current = 0;
    count = 1;
    field_0x14 = 0;
    names = (char(*)[0x80])malloc(0x80);
    ours = (int*)malloc(sizeof(int));
    directx = (int*)malloc(sizeof(int));
    strcpy(names[0], "Unclaimed");
    ours[0] = 0;
    directx[0] = 0;
}

// 0x004a2b80
MemTagStack::~MemTagStack()
{
    if (names) {
        free(names);
    }
    if (ours) {
        free(ours);
    }
    if (directx) {
        free(directx);
    }
    g_MemTagStack = 0;
}

// 0x004a2bc0: totals both counters (the report that used them is compiled out).
void MemTagStack::UnknownFunction4a2bc0(const char* tag)
{
    int total = 0;
    int totalDirectX = 0;
    for (int i = 0; i < count; i++) {
        total += ours[i];
        totalDirectX += directx[i];
    }
}

// 0x004a2bf0
int MemTagStack::UnknownFunction4a2bf0(const char* tag)
{
    for (int i = 0; i < count; i++) {
        if (!strcmp(names[i], tag)) {
            return i;
        }
    }
    count++;
    names = (char(*)[0x80])realloc(names, count * 0x80);
    ours = (int*)realloc(ours, count * sizeof(int));
    directx = (int*)realloc(directx, count * sizeof(int));
    memset(names[count - 1], 0, 0x80);
    strcpy(names[count - 1], tag);
    ours[count - 1] = 0;
    directx[count - 1] = 0;
    return count - 1;
}

// 0x004a2d00
int MemTagStack::Push(const char* tag)
{
    int previous = current;
    current = UnknownFunction4a2bf0(tag);
    return previous;
}

// 0x004a2d20
int MemTagStack::UnknownFunction4a2d20(const char* tag)
{
    for (int i = 0; i < count; i++) {
        if (!strcmp(names[i], tag)) {
            return ours[i];
        }
    }
    return 0;
}

// 0x004a2d90
void MemTagStack::Pop(int previous)
{
    current = previous;
}

// 0x004a2da0
void MemTagStack::UnknownFunction4a2da0(char* name)
{
    strcpy(name, names[current]);
}

// 0x004a2de0
void MemTagStack::UnknownFunction4a2de0(int bytes)
{
    directx[current] += bytes;
}

// 0x004a2e00
void MemTagStack::UnknownFunction4a2e00(int bytes)
{
    directx[current] -= bytes;
}

// 0x004a2e20
void* DebugMalloc(unsigned int size, const char* file, int line)
{
    void* p = malloc(size);
    if (p) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            g_MemTagStack->ours[g_MemTagStack->current] += size;
        } else {
            g_UnknownUnclaimedBytes6850dc += size;
        }
    }
    return p;
}

// 0x004a2e60
void DebugFree(void* p, const char* file, int line)
{
    if (p) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            int size = _msize(p);
            g_MemTagStack->ours[g_MemTagStack->current] -= size;
        } else {
            g_UnknownUnclaimedBytes6850dc -= _msize(p);
        }
    }
    free(p);
}

// 0x004a2ec0
void* DebugRealloc(void* p, unsigned int size, const char* file, int line)
{
    int oldSize = 0;
    if (p) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            oldSize = _msize(p);
            g_MemTagStack->ours[g_MemTagStack->current] -= oldSize;
        } else {
            oldSize = _msize(p);
            g_UnknownUnclaimedBytes6850dc -= oldSize;
        }
        if (!size) {
            free(p);
            return 0;
        }
    }
    void* q = malloc(size);
    if (q) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            g_MemTagStack->ours[g_MemTagStack->current] += size;
        } else {
            g_UnknownUnclaimedBytes6850dc += size;
        }
        if (p) {
            if (oldSize >= (int)size) {
                oldSize = size;
            }
            DebugMemcpy(q, p, oldSize, file, line);
            free(p);
        }
        return q;
    }
    if (p) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            g_MemTagStack->ours[g_MemTagStack->current] += oldSize;
        } else {
            g_UnknownUnclaimedBytes6850dc += oldSize;
        }
    }
    return p;
}

// 0x004a2fc0
void* DebugCalloc(unsigned int count, unsigned int size, const char* file, int line)
{
    unsigned int bytes = count * size;
    void* p = calloc(count, size);
    if (p) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            g_MemTagStack->ours[g_MemTagStack->current] += bytes;
        } else {
            g_UnknownUnclaimedBytes6850dc += bytes;
        }
    }
    return p;
}

// 0x004a3010: counts before allocating, even when malloc fails.
void* operator new(unsigned int size, const char* file, int line)
{
    if (g_MemTagStack && g_MemTagStack->ours) {
        g_MemTagStack->ours[g_MemTagStack->current] += size;
    } else {
        g_UnknownUnclaimedBytes6850dc += size;
    }
    return malloc(size);
}

// 0x004a3060
void operator delete(void* p, const char* file, int line)
{
    if (p) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            int size = _msize(p);
            g_MemTagStack->ours[g_MemTagStack->current] -= size;
        } else {
            g_UnknownUnclaimedBytes6850dc -= _msize(p);
        }
    }
    free(p);
}

// 0x004a30c0
void operator delete(void* p)
{
    if (p) {
        if (g_MemTagStack && g_MemTagStack->ours) {
            int size = _msize(p);
            g_MemTagStack->ours[g_MemTagStack->current] -= size;
        } else {
            g_UnknownUnclaimedBytes6850dc -= _msize(p);
        }
    }
    free(p);
}

// 0x004a3120
void* DebugMemcpy(void* destination, const void* source, unsigned int size, const char* file, int line)
{
    return memcpy(destination, source, size);
}
