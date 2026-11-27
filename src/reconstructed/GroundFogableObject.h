#ifndef MCM2_GROUND_FOGABLE_OBJECT_H
#define MCM2_GROUND_FOGABLE_OBJECT_H

// Terrain and EcoSystem's RTTI identify this nonpolymorphic second base at
// +0x2c. Constructor 0x004aae20 stores one zero dword: groundFog, the
// GroundFog object 0x00484f10 caches (1: none).
struct GroundFogVertex;
struct GroundFogView;

class GroundFogableObject {
public:
    GroundFogableObject();
    // 0x00484f10 (GUIManager.cpp): fogs `count` vertices by their depth below the fog plane.
    void FogVertices(GroundFogVertex* vertices, int count);

    GroundFogView* groundFog;   // +0x00: FindByClassName(1, "GroundFog"), or 1 when none
};

#endif
