// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// Vegetation::TestDistance (0x00456890), from
// samples/ecosystem/EcoSystemNearMisses.cpp, re-checked against the retail
// disassembly: the x and z offsets from the camera are compared one-sided
// against the band's 3D distance (retail does not take their absolute
// value), then the planar distance through FastSqrt (0x00460b50).
#include "../krusty2/broadphase/Terrain.h"
#include "TrackGameGlobal.h"
#include <windows.h>
#include "EcoSystem.h"

float FastSqrt(float value);  // 0x00460b50

// EcoSystem.cpp's 0x0059af14 is file-static there: it points at one of the
// two detail band tables (0x0056a600 for the "Blade" software renderer,
// 0x0056a740 otherwise), chosen when the ecosystem loads (EcoSystem.cpp:801).
// best guess: recompute the same choice from local copies of the two tables.
static UnknownEcoDetailBand s_softwareBands[10] = {
    {0, 0, 1555, 256, 0, 0, 0, 0},   {0, 0, 1555, 256, 0, 0, 0, 0},   {0, 0, 1555, 256, 0, 0, 0, 0},
    {10, 10, 1555, 384, 0, 0, 0, 0}, {20, 20, 1555, 384, 0, 1, 0, 0}, {20, 20, 1555, 448, 0, 1, 1, 0},
    {30, 30, 1555, 512, 0, 1, 1, 0}, {40, 40, 1555, 512, 0, 1, 1, 0}, {75, 75, 1555, 640, 1, 1, 1, 0},
    {150, 75, 1555, 768, 1, 1, 1, 1},
};
static UnknownEcoDetailBand s_hardwareBands[10] = {
    {20, 0, 4444, 256, 0, 1, 1, 1},   {35, 5, 4444, 256, 0, 1, 1, 1},   {45, 10, 4444, 320, 0, 1, 1, 1},
    {60, 20, 4444, 384, 0, 1, 1, 1},  {75, 30, 4444, 448, 1, 1, 1, 1},  {90, 40, 4444, 512, 1, 1, 1, 1},
    {105, 50, 4444, 576, 1, 1, 1, 1}, {120, 60, 4444, 640, 1, 1, 1, 1}, {135, 67, 4444, 768, 1, 1, 1, 1},
    {150, 75, 4444, 768, 1, 1, 1, 1},
};

static UnknownEcoDetailBand* DetailBands()
{
    return reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->bladeRenderer ? s_softwareBands
                                                                              : s_hardwareBands;
}

// 0x00456890: whether the object is beyond the detail band's 3D distance
// (then `fade` is 255), else the fade between the band's two distances.
void Vegetation::TestDistance(int* billboard, int* fade)
{
    UnknownEcoDefinition* definition = g_EcoSystem->definitionTable[definitionIndex];
    UnknownEcoCamera* camera = ((UnknownEcoRenderTarget*)g_EcoSystem->renderContext)->camera;
    float dx = camera->position.x - quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
    float dz = camera->position.z - quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
    *billboard = 0;
    UnknownEcoDetailBand* band = &DetailBands()[g_EcoSystem->detailLevel];
    float outer = (float)band->geometryDistance;
    float inner = (float)band->fadeStartDistance;
    if (dx > outer || dz > outer) {
        *billboard = 1;
        *fade = 0xff;
        return;
    }
    float distance = FastSqrt(dz * dz + dx * dx);
    if (distance > outer) {
        *billboard = 1;
        *fade = 0xff;
        return;
    }
    if (definition->blendLods) {
        if (distance < inner)
            *fade = 0;
        else
            *fade = (int)((distance - inner) * 255.0f / (outer - inner));
    } else {
        *fade = 0;
    }
}
