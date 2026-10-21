// Exercise the actual complete BikeNumberPainter object with scripted resource
// loading and texture Release. Rendering is outside this lifetime probe.
#include "../../src/reconstructed/BikeNumberPainterCore.h"
#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/TextureLoader.h"
#include "../../src/krusty2/motion/D3DIMSoultreeCharacter.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char PainterSize[sizeof(UnknownBikeNumberPainter) == 0x2c ? 1 : -1];
typedef char SheetOffset[offsetof(UnknownBikeNumberPainter, sheet) == 0 ? 1 : -1];
typedef char WidthOffset[offsetof(UnknownBikeNumberPainter, digitWidths) == 4 ? 1 : -1];
typedef char CharacterSize[sizeof(D3DIMSoultreeCharacter) == 0x240 ? 1 : -1];
typedef char ModelOffset[offsetof(D3DIMSoultreeCharacter, modelNode) == 0x1a0 ? 1 : -1];

static unsigned textureStorage[2][sizeof(PCTextureMap) / 4];
static void* textureVtable[4];
static TextureMapManager* expectedManager;
static PCTextureMap* expectedTexture;
static int loadThrows;
static int events[4];
static int eventCount;
static int releaseCount;

static int __fastcall ReleaseTexture(void* self, void*) {
    if (self != expectedTexture || eventCount != 1) exit(10);
    events[eventCount++] = 2;
    ++releaseCount;
    return 0;
}

TextureMap* TerrainLoadTexture(TextureMapManager* manager, const char* name, int format,
                              Palette8* palette, int a, int addressU, int addressV,
                              UnknownTextureFormatChoice* choice, int alphaThreshold,
                              unsigned int key, int b, int c) {
    if (manager != expectedManager || strcmp(name, "BikeNumbers128.tga") || format != 0x115c ||
        palette || a || addressU != 5 || addressV != 6 || choice || alphaThreshold != 0x80 ||
        key != 0xff00ff || b != 1 || c != 1 || eventCount != 0) exit(11);
    events[eventCount++] = 1;
    if (loadThrows) throw 17;
    return expectedTexture;
}

// A complete production object also contains PaintNumber. This unexercised
// graphics boundary must fail if accidentally reached by this lifetime test.
void ManagedTexture::Reregister() { exit(12); }

static void ScopedPainter(int unwind) {
    UnknownBikeNumberPainter painter(expectedManager);
    unsigned expected[11];
    expected[0] = (unsigned)expectedTexture;
    for (int i = 0; i < 10; ++i) expected[i + 1] = i == 1 ? 13 : 19;
    if (memcmp(&painter, expected, sizeof(expected))) exit(13);
    if (unwind) throw 23;
}

int main() {
    unsigned cases = 0;
    memset(textureVtable, 0, sizeof(textureVtable));
    textureVtable[2] = (void*)&ReleaseTexture;
    for (int seed = 0; seed < 64; ++seed) {
        for (int mode = 0; mode < 3; ++mode) {
            memset(textureStorage, seed, sizeof(textureStorage));
            textureStorage[0][0] = textureStorage[1][0] = (unsigned)textureVtable;
            expectedTexture = (PCTextureMap*)textureStorage[seed & 1];
            expectedManager = (TextureMapManager*)(0x1000 + 16 * seed);
            loadThrows = mode == 2;
            eventCount = releaseCount = 0;
            int exception = 0;
            try { ScopedPainter(mode == 1); }
            catch (int value) { exception = value; }
            if (exception != (mode == 2 ? 17 : mode == 1 ? 23 : 0) ||
                releaseCount != (mode != 2) || eventCount != (mode == 2 ? 1 : 2) ||
                events[0] != 1 || (mode != 2 && events[1] != 2)) return 1;
            ++cases;
        }
    }
    printf("Garage painter: %u resource, normal cleanup and exception lifetime cases passed\n", cases);
    return 0;
}
