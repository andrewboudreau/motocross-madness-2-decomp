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
    rgbColor = color & 0xffffff;
}

// 0x0049de90
void UnknownLightEmitterTarget::UnknownFunction49de90(const Vector3* position) {
    field_0xb8 = *position;
}

// 0x0049deb0
LightEmitter::LightEmitter(int flags) : GameObject(flags) {
    lightType = 0;
    debugSphere = 0;
    field_0x9c = 0;
    field_0x30 = 3;
    colorRGBA[0] = 1.0f;
    colorRGBA[1] = 1.0f;
    colorRGBA[2] = 1.0f;
    colorRGBA[3] = 0.0f;
    field_0x44[0] = 1.0f;
    field_0x44[1] = 1.0f;
    field_0x44[2] = 1.0f;
    field_0x44[3] = 0.0f;
    field_0x54[0] = 1.0f;
    field_0x54[1] = 1.0f;
    field_0x54[2] = 1.0f;
    field_0x54[3] = 0.0f;
    lightPosition = Vector3(0.0f, 0.0f, 0.0f);
    lightDirection = Vector3(1.0f, 0.0f, 0.0f);
    lightRange = (float)sqrt(FLT_MAX);
    field_0x80 = 1.0f;
    field_0x84 = 1.0f;
    field_0x88 = 0.0f;
    field_0x8c = 0.0f;
    field_0x94 = 1.5707964f;
    field_0x90 = 0.7853982f;
    field_0xa0 = -1;
    manager = 0;
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
    return (((((int)(colorRGBA[3] * 255.0f) << 8) | (int)(colorRGBA[0] * 255.0f)) << 8 |
             (int)(colorRGBA[1] * 255.0f)) << 8) | (int)(colorRGBA[2] * 255.0f);
}

// 0x0049e020
void LightEmitter::UnknownFunction49e020(unsigned int color) {
    if (color == field_0xa8)
        return;
    colorRGBA[0] = (float)((color >> 16) & 0xff) * (1.0f / 255.0f);
    colorRGBA[1] = (float)((color >> 8) & 0xff) * (1.0f / 255.0f);
    colorRGBA[2] = (float)(color & 0xff) * (1.0f / 255.0f);
    colorRGBA[3] = 0.0f;
    field_0x44[0] = colorRGBA[0];
    field_0x44[1] = colorRGBA[1];
    field_0x44[2] = colorRGBA[2];
    field_0x44[3] = 0.0f;
    field_0x54[0] = colorRGBA[0];
    field_0x54[1] = colorRGBA[1];
    field_0x54[2] = colorRGBA[2];
    field_0x54[3] = 0.0f;
    if (field_0x9c)
        field_0x9c->UnknownFunction49de70(color);
    if (manager)
        manager->UnknownFunction49e490();
}

// 0x0049e0f0
void LightEmitter::SetRange(float range) {
    if (range == 0.0f || range > (float)sqrt(FLT_MAX))
        range = (float)sqrt(FLT_MAX);
    if (range != lightRange) {
        lightRange = range;
        if (manager)
            manager->UnknownFunction49e490();
    }
}

// 0x0049e150
void LightEmitter::SetPosition(const Vector3* position) {
    if (position->x != lightPosition.x || position->y != lightPosition.y || position->z != lightPosition.z) {
        if (debugSphere)
            debugSphere->UnknownFunction4fc630(*position);
        lightPosition = *position;
        if (field_0x9c)
            field_0x9c->UnknownFunction49de90(position);
        if (manager)
            manager->UnknownFunction49e490();
    }
}

// 0x0049e1e0
void LightEmitter::SetDirection(const Vector3* direction) {
    if (direction->x != lightDirection.x || direction->y != lightDirection.y || direction->z != lightDirection.z) {
        lightDirection = *direction;
        if (manager)
            manager->UnknownFunction49e490();
    }
}

// 0x0049e230
LightEmitter* LightEmitter::UnknownFunction49e230(void* value, int type, unsigned int color,
                                                  const Vector3* position, const Vector3* direction,
                                                  float range, int sphere, int a8, int a9, int a10,
                                                  int index) {
    GameObject::UnknownVirtualSlot8(value);
    lightType = type;
    field_0xa0 = index;
    if (sphere) {
        debugSphere = new(__FILE__, 183) D3DIMSoultreeObject(1);
        UnknownFunction469190(debugSphere->UnknownVirtualSlot9(field_0x18, "sphere.slt", 0, 0, 1), -1);
        if (debugSphere)
            debugSphere->UnknownFunction4fd340(0.4f, 0.4f, 0.4f);
    }
    UnknownFunction49e020(color);
    SetRange(range);
    if (position)
        SetPosition(position);
    if (direction)
        SetDirection(direction);
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
    lightCount = 0;
    vertexCapacity = 0;
    vertexRed = 0;
    vertexGreen = 0;
    vertexBlue = 0;
    field_0x104 = 0;
    field_0x108 = 0;
    field_0x10c = 0;
    changeCount = 0;
}

// 0x0049e400
LightManager::~LightManager() {
    if (vertexRed)
        DebugFree(vertexRed, __FILE__, 271);
}

// 0x004452e0 (shared body)
GameObject* LightManager::UnknownVirtualSlot8(void* value) {
    return GameObject::UnknownVirtualSlot8(value);
}

// 0x0049e470
void LightManager::UnknownFunction49e470(LightEmitter* light) {
    field_0x30[lightCount] = light;
    light->manager = this;
    lightCount++;
}

// 0x0049e490
void LightManager::UnknownFunction49e490() {
    changeCount++;
}

// 0x0049f180
Vector3 operator-(const Vector3& v) {
    return Vector3(-v.x, -v.y, -v.z);
}

// 0x004a0190
LightEmitter* LightManager::UnknownFunction4a0190(int type) {
    int i;

    for (i = 0; i < lightCount; i++) {
        if (field_0x30[i]->lightType == type)
            return field_0x30[i];
    }
    return 0;
}
