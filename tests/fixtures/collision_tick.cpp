// Test the actual secondary-base forwarding method; GameObject Tick is an
// explicit scripted boundary, including return values and exception propagation.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collision/CollisionObject.h"

struct Storage { unsigned char before[16], object[0xb8], after[16]; };
static Storage actual, expected;
static unsigned int timeBits;
static int calls, resultValue, fail, seed;

int GameObject::Tick(float frameTime) {
    unsigned int bits; memcpy(&bits, &frameTime, 4);
    if ((unsigned char*)this != actual.object + 12 || bits != timeBits ||
        calls++ || memcmp(&actual, &expected, sizeof actual)) exit(2);
    // A callee may mutate either base and may throw after those writes.
    actual.object[8] ^= 0x53;
    actual.object[0x34] ^= 0x97;
    if (fail) throw seed;
    return resultValue;
}

int main() {
    if (sizeof(CollisionObject) != 0xb8) return 1;
    const unsigned int times[] = {0, 0x80000000, 0x3e000000, 0xbf800000,
        0x7f800000, 0xff800000, 0x7fc12345, 0x7f812345};
    unsigned cases = 0;
    for (seed = 0; seed < 256; ++seed) for (unsigned t = 0; t < 8; ++t)
        for (fail = 0; fail < 2; ++fail) {
            for (unsigned i = 0; i < sizeof actual; ++i)
                ((unsigned char*)&actual)[i] = (unsigned char)(i * 37 + seed * 13);
            expected = actual; calls = 0; timeBits = times[t];
            resultValue = (int)(0x9e3779b9U * seed + t);
            float time; memcpy(&time, &timeBits, 4);
            int caught = -1, result = 0;
            try { result = ((CollisionObject*)actual.object)->CollisionObject::Tick(time); }
            catch (int value) { caught = value; }
            expected.object[8] ^= 0x53; expected.object[0x34] ^= 0x97;
            if (calls != 1 || caught != (fail ? seed : -1) ||
                (!fail && result != resultValue) || memcmp(&actual, &expected, sizeof actual)) return 3;
            ++cases;
        }
    printf("Collision tick: %u secondary-base, raw-argument, return and exception cases passed\n", cases);
    return 0;
}
