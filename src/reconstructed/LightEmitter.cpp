#include <float.h>
#include <math.h>

#include "LightEmitter.h"

#include "DebugAlloc.h"

// The four vector constants that open about 73 retail files (see
// src/krusty2/math/Math3D.h): 0x0067c4b8, 0x0067c4c8, 0x0067c4d8 and
// 0x0067c4a8, initialised by 0x0049dd30..0x0049de6b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x0049de70
void UnknownLightEmitterTarget::UnknownFunction49de70(unsigned int color) {
    field_0xd0 = color & 0xffffff;
}

// 0x0049de90
void UnknownLightEmitterTarget::UnknownFunction49de90(const Vector3* position) {
    field_0xb8 = *position;
}

// 0x0049deb0
LightEmitter::LightEmitter(int flags) : GameObject(flags) {
    field_0x2c = 0;
    field_0x98 = 0;
    field_0x9c = 0;
    field_0x30 = 3;
    field_0x34[0] = 1.0f;
    field_0x34[1] = 1.0f;
    field_0x34[2] = 1.0f;
    field_0x34[3] = 0.0f;
    field_0x44[0] = 1.0f;
    field_0x44[1] = 1.0f;
    field_0x44[2] = 1.0f;
    field_0x44[3] = 0.0f;
    field_0x54[0] = 1.0f;
    field_0x54[1] = 1.0f;
    field_0x54[2] = 1.0f;
    field_0x54[3] = 0.0f;
    field_0x64 = Vector3(0.0f, 0.0f, 0.0f);
    field_0x70 = Vector3(1.0f, 0.0f, 0.0f);
    field_0x7c = (float)sqrt(FLT_MAX);
    field_0x80 = 1.0f;
    field_0x84 = 1.0f;
    field_0x88 = 0.0f;
    field_0x8c = 0.0f;
    field_0x94 = 1.5707964f;
    field_0x90 = 0.7853982f;
    field_0xa0 = -1;
    field_0xa4 = 0;
    field_0xa8 = 0;
}

// 0x0049dfc0
LightEmitter::~LightEmitter() {
}

// 0x00467ae0 (shared `return 1` body)
int LightEmitter::UnknownVirtualSlot12() {
    return 1;
}

// 0x0049dfd0
unsigned int LightEmitter::UnknownFunction49dfd0() {
    return (((((int)(field_0x34[3] * 255.0f) << 8) | (int)(field_0x34[0] * 255.0f)) << 8 |
             (int)(field_0x34[1] * 255.0f)) << 8) | (int)(field_0x34[2] * 255.0f);
}

// 0x0049e020
void LightEmitter::UnknownFunction49e020(unsigned int color) {
    if (color == field_0xa8)
        return;
    field_0x34[0] = (float)((color >> 16) & 0xff) * (1.0f / 255.0f);
    field_0x34[1] = (float)((color >> 8) & 0xff) * (1.0f / 255.0f);
    field_0x34[2] = (float)(color & 0xff) * (1.0f / 255.0f);
    field_0x34[3] = 0.0f;
    field_0x44[0] = field_0x34[0];
    field_0x44[1] = field_0x34[1];
    field_0x44[2] = field_0x34[2];
    field_0x44[3] = 0.0f;
    field_0x54[0] = field_0x34[0];
    field_0x54[1] = field_0x34[1];
    field_0x54[2] = field_0x34[2];
    field_0x54[3] = 0.0f;
    if (field_0x9c)
        field_0x9c->UnknownFunction49de70(color);
    if (field_0xa4)
        field_0xa4->UnknownFunction49e490();
}

// 0x0049e0f0
void LightEmitter::UnknownFunction49e0f0(float range) {
    if (range == 0.0f || range > (float)sqrt(FLT_MAX))
        range = (float)sqrt(FLT_MAX);
    if (range != field_0x7c) {
        field_0x7c = range;
        if (field_0xa4)
            field_0xa4->UnknownFunction49e490();
    }
}

// 0x0049e150
void LightEmitter::UnknownFunction49e150(const Vector3* position) {
    if (position->x != field_0x64.x || position->y != field_0x64.y || position->z != field_0x64.z) {
        if (field_0x98)
            field_0x98->UnknownFunction4fc630(*position);
        field_0x64 = *position;
        if (field_0x9c)
            field_0x9c->UnknownFunction49de90(position);
        if (field_0xa4)
            field_0xa4->UnknownFunction49e490();
    }
}

// 0x0049e1e0
void LightEmitter::UnknownFunction49e1e0(const Vector3* direction) {
    if (direction->x != field_0x70.x || direction->y != field_0x70.y || direction->z != field_0x70.z) {
        field_0x70 = *direction;
        if (field_0xa4)
            field_0xa4->UnknownFunction49e490();
    }
}

// 0x0049e230
LightEmitter* LightEmitter::UnknownFunction49e230(void* value, int type, unsigned int color,
                                                  const Vector3* position, const Vector3* direction,
                                                  float range, int sphere, int a8, int a9, int a10,
                                                  int index) {
    GameObject::UnknownVirtualSlot8(value);
    field_0x2c = type;
    field_0xa0 = index;
    if (sphere) {
        field_0x98 = new(__FILE__, 183) D3DIMSoultreeObject(1);
        UnknownFunction469190(field_0x98->UnknownVirtualSlot9(field_0x18, "sphere.slt", 0, 0, 1), -1);
        if (field_0x98)
            field_0x98->UnknownFunction4fd340(0.4f, 0.4f, 0.4f);
    }
    UnknownFunction49e020(color);
    UnknownFunction49e0f0(range);
    if (position)
        UnknownFunction49e150(position);
    if (direction)
        UnknownFunction49e1e0(direction);
    // Types 0, 3, 6 and 7 keep the current mode (they still occupy the
    // jump table, which spans 0..7).
    switch (type) {
    case 0:
    case 3:
    case 6:
    case 7:
        return this;
    case 1:
        field_0x30 = 1;
        break;
    case 5:
        field_0x30 = 2;
        break;
    case 4:
        field_0x30 = 3;
        break;
    case 2:
        field_0x30 = 4;
        break;
    }
    return this;
}

// 0x0049e390
LightManager::LightManager(int flags) : GameObject(flags) {
    field_0x2c = 0;
    field_0x110 = 0;
    field_0xf8 = 0;
    field_0xfc = 0;
    field_0x100 = 0;
    field_0x104 = 0;
    field_0x108 = 0;
    field_0x10c = 0;
    field_0x118 = 0;
}

// 0x0049e400
LightManager::~LightManager() {
    if (field_0xf8)
        DebugFree(field_0xf8, __FILE__, 271);
}

// 0x004452e0 (shared body)
GameObject* LightManager::UnknownVirtualSlot8(void* value) {
    return GameObject::UnknownVirtualSlot8(value);
}

// 0x0049e470
void LightManager::UnknownFunction49e470(LightEmitter* light) {
    field_0x30[field_0x2c] = light;
    light->field_0xa4 = this;
    field_0x2c++;
}

// 0x0049e490
void LightManager::UnknownFunction49e490() {
    field_0x118++;
}

// 0x0049f180
Vector3 operator-(const Vector3& v) {
    return Vector3(-v.x, -v.y, -v.z);
}

// 0x004a0190
LightEmitter* LightManager::UnknownFunction4a0190(int type) {
    int i;

    for (i = 0; i < field_0x2c; i++) {
        if (field_0x30[i]->field_0x2c == type)
            return field_0x30[i];
    }
    return 0;
}
