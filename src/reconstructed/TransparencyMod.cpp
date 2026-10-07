// TransparencyMod.cpp -- see TransparencyMod.h for the evidence.

#include "TransparencyMod.h"

#include "DebugAlloc.h"
#include "SoultreeMaterial.h"

// 0x005206a0
TransparencyMod::TransparencyMod(int flags)
    : D3DIMSoultreeModifier(flags)
{
    field_0x40 = 0;
    field_0x49 = 0;
    field_0x44 = 1;
    field_0x48 = 0x80;
}

// 0x005206f0
void TransparencyMod::UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                           UnknownSoultreeMesh** out)
{
    *out = mesh;
    if (field_0x44 == field_0x40) {
        return;
    }
    field_0x49++;
    if (field_0x49 == object->field_0x28c[object->field_0x27c].field_0x00) {
        field_0x40 = field_0x44;
        field_0x49 = 0;
    }

    UnknownSoultreeVertex* vertices;
    unsigned int alpha = field_0x48 << 24;
    int i;
    if (field_0x44) {
        for (i = 0; i < object->field_0x294; i++) {
            object->field_0x290[i]->field_0xbc = 1;
        }
        for (i = 0; i < mesh->field_0x08; i++) {
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].field_0x10 &= 0xffffff;
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].field_0x10 |= alpha;
        }
    } else {
        for (i = 0; i < object->field_0x294; i++) {
            object->field_0x290[i]->field_0xbc = 0;
        }
        for (i = 0; i < mesh->field_0x08; i++) {
            ((UnknownSoultreeVertex*)mesh->field_0x10)[i].field_0x10 |= 0xff000000;
        }
    }
}

// 0x00520810
TransparencyMod::~TransparencyMod()
{
}
