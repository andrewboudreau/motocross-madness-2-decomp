// Executes unchanged, separately compiled production helpers. The Python test
// proves both the full production and extracted bodies strict before linking.
#define SOULTREE_OBJECT_WITH_BASES
#include "math/Math3D.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LAYOUT(name, expression) typedef char name[(expression) ? 1 : -1]
LAYOUT(node_size, sizeof(SoultreeObject) == 0x1a4);
LAYOUT(vector_size, sizeof(Vec3) == 12);
LAYOUT(vector_y, offsetof(Vec3, y) == 4);
LAYOUT(vector_z, offsetof(Vec3, z) == 8);
LAYOUT(node_name, offsetof(SoultreeObject, name) == 0x38);
LAYOUT(node_matrix, offsetof(SoultreeObject, localMatrix) == 0xb8);
LAYOUT(node_child, offsetof(SoultreeObject, firstChild) == 0x140);
LAYOUT(node_sibling, offsetof(SoultreeObject, nextSibling) == 0x144);
LAYOUT(bounds_flag, offsetof(SoultreeObject, localBoundsValid) == 0x154);
LAYOUT(bounds_a, offsetof(SoultreeObject, localBoundsCenter) == 0x158);
LAYOUT(bounds_b, offsetof(SoultreeObject, localBoundsHalfExtent) == 0x164);

static unsigned int storage[128], expected[128], nodes[4][0x1a4 / 4];
static unsigned int callbackSeed;
static void* callbackReceiver;
static int callbacks, cases;
static void Require(int value, int line) {
    if (!value) { printf("model node failure line %d case %d\n", line, cases); exit(1); }
}
#define CHECK(x) Require((x), __LINE__)
static unsigned int Next(unsigned int& state) {
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state;
}
static void UpdateExpected(unsigned int* data) {
    unsigned int state = callbackSeed;
    for (int i = 0; i < 6; ++i) data[0x158 / 4 + i] = Next(state);
    data[0x154 / 4] = 1;
}
// No stack arguments: this callback observes the primary receiver in ECX.
// It replaces only the virtual cache dependency, never the tested helper.
static void __fastcall UpdateBounds(void* self, void*) {
    CHECK(self == callbackReceiver);
    CHECK(((unsigned int*)self)[0x154 / 4] == 0);
    ++callbacks;
    UpdateExpected((unsigned int*)self);
}
static void CopyThree(unsigned int* data, int destination, int source) {
    for (int i = 0; i < 3; ++i) data[destination / 4 + i] = data[source / 4 + i];
}
static void Reset(unsigned int seed) {
    for (int i = 0; i < 128; ++i) storage[i] = Next(seed);
    callbacks = 0;
}
int main() {
    static void* slots[6]; slots[5] = (void*)&UpdateBounds;
    const int positionOffsets[] = {0x1b0, 0xe8, 0xe4, 0xec, 0xf0, 0x158, 0x164, 0x1bc};
    const int boundsOffsets[] = {0x1b0, 0x158, 0x164, 0x15c, 0x160, 0x168, 0x154, 0x1bc};
    const int flags[] = {0, 1, -1, 2};
    SoultreeObject* node = (SoultreeObject*)storage;
    callbackReceiver = node;
    for (int seed = 1; seed <= 128; ++seed) {
        for (int mode = 0; mode < 8; ++mode) {
            Reset(seed); memcpy(expected, storage, sizeof(storage));
            int offset = positionOffsets[mode];
            CopyThree(expected, offset, 0xe8);
            Vec3* out = (Vec3*)((char*)storage + offset);
            CHECK(node->GetPosition(out) == out);
            CHECK(!memcmp(storage, expected, sizeof(storage))); ++cases;
            for (int flag = 0; flag < 4; ++flag) {
                for (int outputMode = 0; outputMode < 4; ++outputMode) {
                    Reset(seed); storage[0] = (unsigned int)slots;
                    node->localBoundsValid = flags[flag]; callbackSeed = seed ^ 0x8eaf1307U;
                    memcpy(expected, storage, sizeof(storage));
                    int a = boundsOffsets[mode];
                    int b = outputMode == 0 ? 0x1bc : outputMode == 1 ? 0x158 : outputMode == 2 ? 0x164 : a;
                    if (!flags[flag]) UpdateExpected(expected);
                    CopyThree(expected, a, 0x158); CopyThree(expected, b, 0x164);
                    node->GetLocalBounds((Vec3*)((char*)storage + a), (Vec3*)((char*)storage + b));
                    CHECK(callbacks == (flags[flag] == 0));
                    CHECK(!memcmp(storage, expected, sizeof(storage))); ++cases;
                }
            }
        }
    }
    // Actual recursion and CRT case-insensitive comparison. Duplicate names
    // make the sibling-before-child traversal order observable.
    memset(nodes, 0, sizeof(nodes));
    SoultreeObject* n[4];
    for (int i = 0; i < 4; ++i) n[i] = (SoultreeObject*)nodes[i];
    strcpy(n[0]->name, "Root"); strcpy(n[1]->name, "Body");
    strcpy(n[2]->name, "body"); strcpy(n[3]->name, "Wheel");
    n[0]->nextSibling = n[1]; n[0]->firstChild = n[2]; n[1]->firstChild = n[3];
    const char* queries[] = {"root", "ROOT", "BoDy", "wheel", "absent", "", "Body", "Wheel"};
    SoultreeObject* results[] = {n[0], n[0], n[1], n[3], 0, 0, n[1], n[3]};
    unsigned int before[4][0x1a4 / 4]; memcpy(before, nodes, sizeof(nodes));
    for (i = 0; i < 8; ++i) {
        CHECK(n[0]->FindByName(queries[i]) == results[i]);
        CHECK(!memcmp(before, nodes, sizeof(nodes))); ++cases;
    }
    printf("Model nodes: %d lookup, position and bounds cases passed\n", cases);
    return 0;
}
