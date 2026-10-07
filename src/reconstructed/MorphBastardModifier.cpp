// MorphBastardModifier.cpp -- see MorphBastardModifier.h for the evidence.

#include <math.h>
#include <string.h>

#include "MorphBastardModifier.h"

#include "DebugAlloc.h"

// The four vector constants that open about 73 retail files (see
// src/krusty2/math/Math3D.h): 0x006850f0, 0x00685100, 0x00685110 and
// 0x006850e0, initialised by 0x004a54f0..0x004a562b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x004a3150
MorphBastardModifier::MorphBastardModifier(int flags)
    : D3DIMSoultreeModifier(flags)
{
    field_0x40 = 0;
    field_0x44 = 0;
    field_0x48 = 0;
    field_0x4c = 0;
    field_0x50 = 0;
    field_0x54 = 0;
    field_0x3c = 0;
}

// 0x004a31a0
MorphBastardModifier::~MorphBastardModifier()
{
    if (field_0x48) {
        for (int i = 0; i < field_0x44; i++) {
            for (int j = 0; j < field_0x48[i].field_0x00; j++) {
                for (int k = 0; k < field_0x48[i].field_0x04[j].field_0x4c; k++) {
                    DebugFree(field_0x48[i].field_0x04[j].field_0x50[k].field_0x08, __FILE__, 28);
                    DebugFree(field_0x48[i].field_0x04[j].field_0x50[k].field_0x0c, __FILE__, 29);
                    DebugFree(field_0x48[i].field_0x04[j].field_0x50[k].field_0x10, __FILE__, 30);
                }
                DebugFree(field_0x48[i].field_0x04[j].field_0x50, __FILE__, 32);
                DebugFree(field_0x48[i].field_0x04[j].field_0x48, __FILE__, 33);
            }
            DebugFree(field_0x48[i].field_0x04, __FILE__, 35);
        }
        DebugFree(field_0x48, __FILE__, 37);
    }
    DebugFree(field_0x4c, __FILE__, 40);
    DebugFree(field_0x50, __FILE__, 41);
    DebugFree(field_0x54, __FILE__, 42);
    if (field_0x40) {
        delete field_0x40->field_0x10;
        delete field_0x40->field_0x14;
        delete field_0x40->field_0x18;
        delete field_0x40->field_0x1c;
        delete field_0x40->field_0x28;
    }
}

// 0x004a3bb0
int UnknownFunction4a3bb0(const void* a, const void* b)
{
    if (((const MorphBastardTarget*)a)->field_0x00 > ((const MorphBastardTarget*)b)->field_0x00)
        return 1;
    if (((const MorphBastardTarget*)a)->field_0x00 < ((const MorphBastardTarget*)b)->field_0x00)
        return -1;
    return 0;
}

// 0x004a3be0
float UnknownFunction4a3be0(Vector3 a, Vector3 b)
{
    float dot = a.z * b.z + a.x * b.x + a.y * b.y;
    if (dot + 0.001f > 1.0f)
        dot = 1.0f;
    if (dot - 0.001f < -1.0f)
        dot = -1.0f;
    if (dot == 1.0)
        return 0.0f;
    if (dot == 0.0)
        return 1.5707964f;
    return (float)acos(dot);
}

// 0x004a4bb0
void MorphBastardModifier::UnknownFunction4a4bb0(MorphBastardObject* object)
{
    Matrix4 m;
    object->field_0x08->UnknownFunction4fca60(&m);
    object->field_0x08->UnknownFunction4fca80(0, &m);
    MorphBastardTransposeRotation(m);
    for (int i = 0; i < object->field_0x00; i++)
        UnknownFunction4a3c80(&object->field_0x04[i], m);
}

// 0x004a5290
void MorphBastardModifier::UnknownFunction4a5290(UnknownSoultreeMesh* mesh)
{
    field_0x40 = new (__FILE__, 854) UnknownSoultreeMesh;
    field_0x40->field_0x00 = mesh->field_0x00;
    field_0x40->field_0x08 = mesh->field_0x08;
    field_0x40->field_0x0c = mesh->field_0x0c;
    field_0x40->field_0x10 = new (__FILE__, 858) MorphBastardVertex[field_0x40->field_0x08];
    field_0x40->field_0x14 = new (__FILE__, 859) MorphBastardVertex[field_0x40->field_0x08];
    field_0x40->field_0x18 = new (__FILE__, 860) Vector3[field_0x40->field_0x08];
    field_0x40->field_0x1c = new (__FILE__, 861) MorphBastardFace[field_0x40->field_0x0c];
    field_0x40->field_0x20 = mesh->field_0x20;
    field_0x40->field_0x28 = new (__FILE__, 863) int[field_0x40->field_0x08 * 2];
    memcpy(field_0x40->field_0x10, mesh->field_0x10, field_0x40->field_0x08 * sizeof(MorphBastardVertex));
    memcpy(field_0x40->field_0x14, mesh->field_0x14, field_0x40->field_0x08 * sizeof(MorphBastardVertex));
    memcpy(field_0x40->field_0x18, mesh->field_0x18, field_0x40->field_0x08 * sizeof(Vector3));
    memcpy(field_0x40->field_0x1c, mesh->field_0x1c, field_0x40->field_0x0c * sizeof(MorphBastardFace));
    memcpy(field_0x40->field_0x28, mesh->field_0x28, field_0x40->field_0x08 * sizeof(int));
    field_0x40->field_0x04 = new (__FILE__, 871) MorphBastardMeshGroup[field_0x40->field_0x00];
    int offset = 0;
    for (int i = 0; i < field_0x40->field_0x00; i++) {
        field_0x40->field_0x04[i].field_0x00 = mesh->field_0x04[i].field_0x00;
        field_0x40->field_0x04[i].field_0x04 = mesh->field_0x04[i].field_0x04;
        field_0x40->field_0x04[i].field_0x08 = field_0x40->field_0x10 + offset;
        field_0x40->field_0x04[i].field_0x0c = field_0x40->field_0x14 + offset;
        field_0x40->field_0x04[i].field_0x10 = field_0x40->field_0x18 + offset;
        offset += field_0x40->field_0x04[i].field_0x04;
    }
}
