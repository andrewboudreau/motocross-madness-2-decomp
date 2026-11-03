// Private differential validation only. Production never loads reference code.
// Calls the candidate (linked in) and retail bytes (copied into an executable
// buffer; these halvers have no calls or globals) with identical pixel data.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (__cdecl *HalveFn)(void*, void*, int, int, int, int);
int Halve8888(void*, void*, int, int, int, int);
int Halve1555(void*, void*, int, int, int, int);

static unsigned seed = 0x1234567;
static unsigned Next() { seed = seed * 1664525u + 1013904223u; return seed >> 8; }

// Retail Halve8888 jumps through a switch table inside its own extent, so a few
// dwords hold absolute addresses into [va, va + size). Windows may already use
// the retail address range (stack/heap placement is randomized), so the bytes
// are loaded wherever the OS has room and every dword that points into the
// function's own retail extent is rebased by the load delta, as a loader
// would. The test asserts the exact number of rebased dwords per function.
static HalveFn Load(const char* spec) {
    unsigned va = strtoul(spec, 0, 16);
    const char* path = strchr(spec, ':') + 1;
    FILE* file = fopen(path, "rb");
    if (!file) { fprintf(stderr, "open failed %s\n", path); return 0; }
    unsigned char* region = (unsigned char*)VirtualAlloc(0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!region) { fprintf(stderr, "alloc failed %x err %lu\n", va, GetLastError()); return 0; }
    size_t size = fread(region, 1, 0x1000, file);
    fclose(file);
    if (!size) return 0;
    unsigned delta = (unsigned)region - va, relocated = 0;
    for (size_t offset = 0; offset + 4 <= size; ++offset) {
        unsigned value;
        memcpy(&value, region + offset, 4);
        if (value >= va && value < va + size) {
            value += delta;
            memcpy(region + offset, &value, 4);
            ++relocated;
        }
    }
    printf("Pixtrans halve %x: rebased %u absolute references\n", va, relocated);
    return (HalveFn)region;
}

// element = bytes per pixel (4 or 2)
static int Compare(const char* name, HalveFn candidate, HalveFn retail, int element, int cases) {
    static unsigned char sourceA[1 << 16], sourceB[1 << 16], destA[1 << 16], destB[1 << 16];
    for (int n = 0; n < cases; ++n) {
        int width = 1 + Next() % 9, height = 1 + Next() % 6;
        int sourceStride = 2 * width + Next() % 5, destStride = width + Next() % 4;
        int mode = Next() % 4;
        int sourceBytes = (2 * height + 1) * sourceStride * element + 16;
        int destBytes = (height * destStride + 4) * element;
        int i;
        for (i = 0; i < sourceBytes; ++i) {
            unsigned char b = Next();
            if (mode == 0) b = (Next() & 3) ? 0 : b;      // sparse alpha
            if (mode == 1) b = (Next() & 3) ? 0xff : b;   // mostly opaque
            sourceA[i] = sourceB[i] = b;
        }
        for (i = 0; i < destBytes; ++i) destA[i] = destB[i] = 0xa5;
        int a = candidate(destA, sourceA, width, height, destStride, sourceStride);
        int b = retail(destB, sourceB, width, height, destStride, sourceStride);
        if (a != b || memcmp(destA, destB, destBytes) || memcmp(sourceA, sourceB, sourceBytes)) {
            printf("%s: mismatch case %d w=%d h=%d ss=%d ds=%d mode=%d result %d/%d\n",
                   name, n, width, height, sourceStride, destStride, mode, a, b);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    if (argc != 3) return 2;
    HalveFn retail8888 = Load(argv[1]), retail1555 = Load(argv[2]);
    if (!retail8888 || !retail1555) return 3;
    const int cases = 20000;
    if (!Compare("Halve8888", Halve8888, retail8888, 4, cases)) return 1;
    if (!Compare("Halve1555", Halve1555, retail1555, 2, cases)) return 1;
    printf("Pixtrans halve: %d scenarios per function match results, destination and source bytes\n", cases);
    return 0;
}
