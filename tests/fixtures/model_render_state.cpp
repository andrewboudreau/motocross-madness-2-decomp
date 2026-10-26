// Executes unchanged production bodies; Python verifies both production and
// extracted objects against the complete retail extents before native execution.
#include "../../src/reconstructed/D3DIMSoulTree.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LAYOUT(name, expression) typedef char name[(expression) ? 1 : -1]
LAYOUT(model_size, sizeof(D3DIMSoultreeObject) == 0x2d8);
LAYOUT(dirty_offset, offsetof(D3DIMSoultreeObject, subtreeDirty) == 0x18c);
LAYOUT(lighting_offset, offsetof(D3DIMSoultreeObject, useLighting) == 0x1a4);
LAYOUT(perspective_offset, offsetof(D3DIMSoultreeObject, texturePerspective) == 0x1b4);
LAYOUT(shading_offset, offsetof(D3DIMSoultreeObject, gouraudShading) == 0x1b8);
LAYOUT(count_offset, offsetof(D3DIMSoultreeObject, lodCount) == 0x274);
LAYOUT(lowest_offset, offsetof(D3DIMSoultreeObject, lowestLod) == 0x278);
LAYOUT(current_offset, offsetof(D3DIMSoultreeObject, currentLod) == 0x27c);
LAYOUT(threshold_offset, offsetof(D3DIMSoultreeObject, inverseLodThreshold) == 0x284);
LAYOUT(screen_offset, offsetof(D3DIMSoultreeObject, screenRect) == 0x298);
LAYOUT(second_screen_offset, offsetof(D3DIMSoultreeObject, field_0x2a8) == 0x2a8);
LAYOUT(setting_size, sizeof(SoultreeLodSetting) == 16);

static unsigned int actual[0x2d8 / 4 + 8], expected[0x2d8 / 4 + 8];
static SoultreeLodSetting tables[2][10];
SoultreeLodSetting* g_ActiveLodSettings;
static D3DIMSoultreeObject* model = (D3DIMSoultreeObject*)(actual + 4);
static D3DIMSoultreeObject* wanted = (D3DIMSoultreeObject*)(expected + 4);
static int cases;
static unsigned int Next(unsigned int& state) {
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state;
}
static void Reset(unsigned int seed) {
    for (int i = 0; i < sizeof(actual) / 4; ++i) actual[i] = Next(seed);
    memcpy(expected, actual, sizeof(actual));
}
static void Check(int line) {
    if (memcmp(actual, expected, sizeof(actual))) {
        printf("model state mismatch at line %d case %d\n", line, cases); exit(1);
    }
    ++cases;
}
#define CHECK() Check(__LINE__)
int main() {
    const int counts[] = {-2, 0, 1, 2, 5, 10, 1024, 0x7fffffff};
    const int levels[] = {-2, -1, 0, 1, 3, 10, 0x7fffffff};
    const int requests[] = {-0x7fffffff, -2, -1, 0, 1, 4, 10, 1024, 0x7fffffff};
    const float thresholds[] = {0.125f, 0.5f, 1.0f, 2.0f, 8.0f, 128.0f, 1024.0f, -2.0f};
    unsigned int state = 0x87a52c13;
    for (int table = 0; table < 2; ++table) for (int row = 0; row < 10; ++row) {
        tables[table][row].lodThreshold = thresholds[(table + row) & 7];
        tables[table][row].texturePerspective = Next(state);
        tables[table][row].gouraudShading = Next(state);
        tables[table][row].useLighting = Next(state);
    }
    for (int seed = 1; seed <= 8; ++seed) {
        for (int c = 0; c < 8; ++c) for (int value = 0; value < 9; ++value) {
            Reset(seed); model->lodCount = wanted->lodCount = counts[c];
            int minimum = requests[value] < 0 ? 0 : requests[value];
            if (minimum >= counts[c]) minimum = counts[c] - 1;
            wanted->lowestLod = minimum;
            model->SetLowestLod(requests[value]); CHECK();
            for (int low = 0; low < 7; ++low) for (int current = 0; current < 7; ++current) {
                Reset(seed); model->lodCount = wanted->lodCount = counts[c];
                model->lowestLod = wanted->lowestLod = levels[low];
                model->currentLod = wanted->currentLod = levels[current];
                int selected = requests[value];
                if (selected < levels[low]) selected = levels[low];
                if (selected >= counts[c]) selected = counts[c] - 1;
                if (selected != levels[current]) {
                    wanted->subtreeDirty = 1; wanted->currentLod = selected;
                }
                model->SetLod(requests[value]); CHECK();
            }
        }
        for (int lightingValue = 0; lightingValue < 9; ++lightingValue) {
            Reset(seed); wanted->useLighting = requests[lightingValue]; wanted->subtreeDirty = 1;
            model->SetUseLighting(requests[lightingValue]); CHECK();
        }
        Reset(seed); memset(&wanted->screenRect, 0, 16); memset(&wanted->field_0x2a8, 0, 16);
        wanted->hidden = 1; model->ClearScreenRects(); CHECK();
        for (table = 0; table < 2; ++table) for (int setting = 0; setting < 10; ++setting) {
            Reset(seed); g_ActiveLodSettings = tables[table];
            wanted->inverseLodThreshold = 1.0f / tables[table][setting].lodThreshold;
            wanted->texturePerspective = tables[table][setting].texturePerspective;
            wanted->gouraudShading = tables[table][setting].gouraudShading;
            wanted->useLighting = tables[table][setting].useLighting; wanted->subtreeDirty = 1;
            model->ApplyLodSettings(setting); CHECK();
        }
    }
    printf("Model render state: %d complete storage cases passed\n", cases);
    return 0;
}
