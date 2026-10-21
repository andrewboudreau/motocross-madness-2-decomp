// Executes unchanged extracted production bodies, each independently strict
// against the full retail extent and the actual production object.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/GhostMod.h"

typedef char GhostSize[(sizeof(GhostMod1) == 0x48) ? 1 : -1];
typedef char MorphSize[(sizeof(MorphBastardModifier) == 0x58) ? 1 : -1];
typedef char ModelSize[(sizeof(D3DIMSoultreeObject) == 0x2d8) ? 1 : -1];
typedef char GhostTime[(offsetof(GhostMod1, timeRemaining) == 0x40) ? 1 : -1];
typedef char GhostDuration[(offsetof(GhostMod1, totalDuration) == 0x44) ? 1 : -1];
typedef char ObjectList[(offsetof(D3DIMSoultreeModifier, modifiedObjects) == 0x34) ? 1 : -1];
typedef char ObjectCount[(offsetof(D3DIMSoultreeModifier, objectCount) == 0x38) ? 1 : -1];
typedef char FirstList[(offsetof(D3DIMSoultreeObject, modifierList) == 0x268) ? 1 : -1];
typedef char SecondList[(offsetof(D3DIMSoultreeObject, secondModifierList) == 0x270) ? 1 : -1];

extern "C" GameObject* GhostPrimary(GhostMod1*);
extern "C" GameObject* MorphPrimary(MorphBastardModifier*);
extern "C" GameObject* ModelSecondary(D3DIMSoultreeObject*);
static int cases, liveBlocks;
static void Require(int ok, int line) {
    if (!ok) { printf("modifier failure at %d, case %d\n", line, cases); exit(1); }
}
#define CHECK(x) Require((x), __LINE__)
void* DebugMalloc(unsigned int size, const char* file, int line) {
    CHECK(file && *file && (line == 26 || line == 41));
    CHECK(size >= 4 && !(size & 3));
    void* p = malloc(size); CHECK(p != 0); ++liveBlocks; return p;
}
void DebugFree(void* p, const char* file, int line) {
    CHECK(file && *file && (line == 33 || line == 53));
    if (p) { --liveBlocks; free(p); }
}
void* DebugRealloc(void* p, unsigned int size, const char* file, int line) {
    CHECK(file && *file && (line == 0x958 || line == 0x96d || line == 0x97f || line == 0x994));
    CHECK(!(size & 3));
    // Retain a valid allocation for a zero-length list; production never reads
    // it. This models only the allocator boundary, not allocation failure.
    if (!p) ++liveBlocks;
    p = realloc(p, size ? size : 1); CHECK(p != 0); return p;
}

int main() {
    unsigned long nodes[5][0x2d8 / 4], ghosts[5][0x48 / 4];
    D3DIMSoultreeObject* model[5]; GhostMod1* modifier[5];
    int i, j, seed, mode, models, modifiers;
    for (i = 0; i < 5; ++i) {
        model[i] = reinterpret_cast<D3DIMSoultreeObject*>(nodes[i]);
        modifier[i] = reinterpret_cast<GhostMod1*>(ghosts[i]);
        CHECK(GhostPrimary(modifier[i]) == reinterpret_cast<GameObject*>(modifier[i]));
        CHECK(MorphPrimary(reinterpret_cast<MorphBastardModifier*>(modifier[i])) == reinterpret_cast<GameObject*>(modifier[i]));
        CHECK(ModelSecondary(model[i]) == reinterpret_cast<GameObject*>((char*)model[i] + 12));
    }
    CHECK(!GhostPrimary(0) && !MorphPrimary(0) && !ModelSecondary(0));
    for (seed = 0; seed < 64; ++seed) for (mode = 0; mode < 2; ++mode)
    for (models = 1; models <= 5; ++models) for (modifiers = 1; modifiers <= 5; ++modifiers) {
        memset(nodes, seed + 17, sizeof(nodes)); memset(ghosts, seed + 33, sizeof(ghosts));
        for (i = 0; i < models; ++i) {
            model[i]->modifierCount = model[i]->secondModifierCount = 0;
            model[i]->modifierList = model[i]->secondModifierList = 0;
        }
        for (j = 0; j < modifiers; ++j) {
            modifier[j]->objectCount = 0; modifier[j]->modifiedObjects = 0;
        }
        for (i = 0; i < models; ++i) for (j = 0; j < modifiers; ++j) {
            if (mode) model[i]->AddSecondModifier(modifier[j]);
            else model[i]->AddModifier(modifier[j]);
            CHECK((mode ? model[i]->secondModifierCount : model[i]->modifierCount) == j + 1);
            CHECK((mode ? model[i]->secondModifierList : model[i]->modifierList)[j] == GhostPrimary(modifier[j]));
            CHECK(modifier[j]->objectCount == i + 1);
            CHECK(modifier[j]->modifiedObjects[i] == model[i]);
            CHECK((mode ? model[i]->modifierCount : model[i]->secondModifierCount) == 0);
        }
        // Alternate removing the first and last attached modifier. Reverse the
        // model order on odd seeds to exercise removal from both list ends.
        for (int step = 0; step < models; ++step) {
            i = (seed & 1) ? models - 1 - step : step;
            for (int remaining = modifiers; remaining; --remaining) {
                int index = (remaining + seed) & 1 ? 0 : remaining - 1;
                GameObject* selected = (mode ? model[i]->secondModifierList : model[i]->modifierList)[index];
                D3DIMSoultreeModifier* m = (D3DIMSoultreeModifier*)selected;
                if (mode) model[i]->RemoveSecondModifier(selected);
                else model[i]->RemoveModifier(selected);
                CHECK((mode ? model[i]->secondModifierCount : model[i]->modifierCount) == remaining - 1);
                CHECK(m->objectCount == models - step - 1);
                for (int k = 0; k < m->objectCount; ++k) CHECK(m->modifiedObjects[k] != model[i]);
            }
            void* list = mode ? (void*)model[i]->secondModifierList : (void*)model[i]->modifierList;
            CHECK(list != 0); free(list); --liveBlocks;
        }
        for (j = 0; j < modifiers; ++j) {
            CHECK(!modifier[j]->objectCount && !modifier[j]->modifiedObjects);
            for (int word = 0; word < 0x48 / 4; ++word)
                if (word != 0x34 / 4 && word != 0x38 / 4)
                    CHECK(ghosts[j][word] == (unsigned long)(seed + 33) * 0x01010101);
        }
        for (i = 0; i < models; ++i)
            for (int word = 0; word < 0x2d8 / 4; ++word)
                if (word < 0x264 / 4 || word > 0x270 / 4)
                    CHECK(nodes[i][word] == (unsigned long)(seed + 17) * 0x01010101);
        CHECK(liveBlocks == 0); ++cases;
    }
    const unsigned long times[] = {0,0x80000000,0x3f800000,0xbf800000,0x3eaaaaab,0x7f800000,0xff800000,1};
    for (seed = 0; seed < 64; ++seed) for (i = 0; i < 8; ++i) {
        unsigned long expected[0x48 / 4]; float value;
        memset(ghosts[0], seed + 1, sizeof(ghosts[0])); memcpy(expected, ghosts[0], sizeof(expected));
        memcpy(&value, &times[i], 4);
        expected[0x40 / 4] = expected[0x44 / 4] = (times[i] & 0x7fffffff) ? times[i] : 0x7f7fffff;
        modifier[0]->StartFade(value); CHECK(!memcmp(expected, ghosts[0], sizeof(expected))); ++cases;
    }
    printf("Bike modifiers: %d attachment, removal and fade boundary cases passed\n", cases);
    return 0;
}
