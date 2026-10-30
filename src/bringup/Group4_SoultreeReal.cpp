// Bring-up definitions (best-guess-unreviewed) on the real soultree.cpp view
// of SoultreeObject (SOULTREE_OBJECT_WITH_BASES, as src/krusty2/soultree/soultree.cpp).
#define SOULTREE_OBJECT_WITH_BASES
#include "math/Math3D.h"

// Primary vtable 0x00557c18 slot 5 is 0x00464e90, the shared one-instruction
// `ret` body: SoultreeObject's own UpdateLocalBounds does nothing (subclasses
// such as D3DIMSoultreeObject override it). Referenced by the vtable soultree.cpp emits.
void SoultreeObject::UpdateLocalBounds()
{
}

// Bridge for src/bringup/Group4_SoultreeBridge.cpp: the Vector3-typed
// LocalToWorldPoint view (D3DIMSoulTree.h) is the same retail function
// 0x004fd660 that soultree.cpp defines with Vec3 (same 12-byte layout).
void Group4Veh_LocalToWorldPoint(void* node, const float* in, float* out)
{
    Vec3 p = ((SoultreeObject*)node)->LocalToWorldPoint(*(const Vec3*)in);
    out[0] = p.x;
    out[1] = p.y;
    out[2] = p.z;
}
