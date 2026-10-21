// The generated header contains the unchanged production call-view declarations.
// The constructor definitions are linked as separately compiled objects.
#include "vector_constructor_views.h"
#include <new>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

union Cell { unsigned bits; float value; };

template<class T> static int Check(unsigned seed, int input) {
    typedef char Size[sizeof(T) == 12 ? 1 : -1];
    typedef char X[offsetof(T, x) == 0 ? 1 : -1];
    typedef char Y[offsetof(T, y) == 4 ? 1 : -1];
    typedef char Z[offsetof(T, z) == 8 ? 1 : -1];
    Cell storage[16], expected[16];
    const unsigned special[] = {
        0, 0x80000000, 0x3f800000, 0xbf800000,
        0x7f800000, 0xff800000, 0x7fc12345, 0xffc54321,
        0x7f812345, 0xff812345, 1, 0x80000001,
        0x007fffff, 0x00800000, 0x7f7fffff, 0xff7fffff
    };
    unsigned random = seed;
    for (int i = 0; i < 16; ++i) {
        random = random * 1664525u + 1013904223u;
        storage[i].bits = random;
    }
    if (seed < 16) for (i = 0; i < 3; ++i)
        storage[input + i].bits = special[(seed + i * 5) & 15];
    memcpy(expected, storage, sizeof(storage));
    for (i = 0; i < 3; ++i) expected[6 + i].bits = storage[input + i].bits;
    // The by-value arguments must be captured before any destination writes,
    // including exact and shifted overlap with the argument storage.
    T* result = new ((void*)&storage[6]) T(storage[input].value,
                                           storage[input + 1].value,
                                           storage[input + 2].value);
    if ((void*)result != &storage[6] || memcmp(storage, expected, sizeof(storage))) {
        printf("Vector constructor mismatch: seed %u, input %d\n", seed, input);
        return 0;
    }
    return 1;
}

int main() {
    const int inputs[] = {0, 4, 5, 6, 7};
    unsigned checks = 0;
    for (unsigned seed = 0; seed < 2064; ++seed)
    for (int input = 0; input < 5; ++input) {
        int index = inputs[input];
        if (!Check<CollisionVec3>(seed, index) || !Check<BikeVec3Call>(seed, index) ||
            !Check<CameraVec3Call>(seed, index) || !Check<CollisionVec3Call>(seed, index) ||
            !Check<SoultreeOolVec3>(seed, index) || !Check<Vec3Call>(seed, index) ||
            !Check<VehVec3Call>(seed, index)) return 2;
        checks += 7;
    }
    printf("Vector constructors: %u raw-bit, placement and overlap cases passed\n", checks);
    return 0;
}
