// TransparencyMod.cpp -- see TransparencyMod.h for the evidence.

#include "TransparencyMod.h"

#include "DebugAlloc.h"
#include "SoultreeMaterial.h"

// 0x005206a0
TransparencyMod::TransparencyMod(int flags)
    : D3DIMSoultreeModifier(flags)
{
    appliedState = 0;
    surfacesDrawn = 0;
    isTransparent = 1;
    field_0x48 = 0x80;
}

// 0x005206f0
void TransparencyMod::UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                           UnknownSoultreeMesh** out)
{
    *out = mesh;
    if (isTransparent == appliedState) {
        return;
    }
    surfacesDrawn++;
    if (surfacesDrawn == object->lodTable[object->field_0x27c].surfaceCount) {
        appliedState = isTransparent;
        surfacesDrawn = 0;
    }

    UnknownSoultreeVertex* vertices;
    unsigned int alpha = field_0x48 << 24;
    int i;
    if (isTransparent) {
        for (i = 0; i < object->materialCount; i++) {
            object->materialTable[i]->hasAlpha = 1;
        }
        for (i = 0; i < mesh->vertexCount; i++) {
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].diffuse &= 0xffffff;
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].diffuse |= alpha;
        }
    } else {
        for (i = 0; i < object->materialCount; i++) {
            object->materialTable[i]->hasAlpha = 0;
        }
        for (i = 0; i < mesh->vertexCount; i++) {
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].diffuse |= 0xff000000;
        }
    }
}

// 0x00520810
TransparencyMod::~TransparencyMod()
{
}
