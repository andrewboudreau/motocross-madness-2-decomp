// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// Vegetation::TestDistance (0x00456890), the body of
// samples/ecosystem/EcoSystemNearMisses.cpp, re-checked against the retail
// disassembly: the x and z offsets from the camera are compared one-sided
// against the band's 3D distance (retail does not take their absolute
// value), then the planar distance through FastSqrt (0x00460b50).
//
// The band comes from EcoSystem.cpp's shared detail band pointer (0x0059af14,
// set when the ecosystem loads, EcoSystem.cpp:801), read twice as retail does.
// This copy used to recompute the choice from private copies of the two tables
// (linked 0.953; the sample form scores 0.991).
//
// Kept as a stand-in instead of linking the sample: the sample also carries
// EcoSystem::Render3D and the Vegetation collision placement, which other units
// of the game build provide.
#include "../krusty2/broadphase/Terrain.h"
#include "TrackGameGlobal.h"
#include <windows.h>
#include "EcoSystem.h"

float FastSqrt(float value);  // 0x00460b50

// 0x0059af14, defined in src/reconstructed/EcoSystem.cpp.
extern UnknownEcoDetailBand* g_UnknownGlobal59af14;

// 0x00456890: whether the object is beyond the detail band's 3D distance
// (then `fade` is 255), else the fade between the band's two distances.
void Vegetation::TestDistance(int* billboard, int* fade) {
    VegetationDefinition* definition = g_EcoSystem->definitionTable[definitionIndex];
    EcoCameraView* camera = ((EcoRenderTargetView*)g_EcoSystem->renderContext)->camera;
    float dx = camera->position.x - quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
    float dz = camera->position.z - quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
    *billboard = 0;
    float outer = (float)g_UnknownGlobal59af14[g_EcoSystem->detailLevel].geometryDistance;
    float inner = (float)g_UnknownGlobal59af14[g_EcoSystem->detailLevel].fadeStartDistance;
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
