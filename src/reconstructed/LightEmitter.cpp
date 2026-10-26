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
void UnknownLightEmitterTarget::SetRgbColor(unsigned int color) {
    rgbColor = color & 0xffffff;
}

// 0x0049de90
void UnknownLightEmitterTarget::SetPosition(const Vector3* position) {
    targetPosition = *position;
}

// 0x0049deb0
LightEmitter::LightEmitter(int flags) : GameObject(flags) {
    lightType = 0;
    debugSphere = 0;
    linkedTarget = 0;
    d3dLightType = 3;
    colorRGBA[0] = 1.0f;
    colorRGBA[1] = 1.0f;
    colorRGBA[2] = 1.0f;
    colorRGBA[3] = 0.0f;
    specularRGBA[0] = 1.0f;
    specularRGBA[1] = 1.0f;
    specularRGBA[2] = 1.0f;
    specularRGBA[3] = 0.0f;
    ambientRGBA[0] = 1.0f;
    ambientRGBA[1] = 1.0f;
    ambientRGBA[2] = 1.0f;
    ambientRGBA[3] = 0.0f;
    lightPosition = Vector3(0.0f, 0.0f, 0.0f);
    lightDirection = Vector3(1.0f, 0.0f, 0.0f);
    lightRange = (float)sqrt(FLT_MAX);
    falloff = 1.0f;
    attenuation0 = 1.0f;
    attenuation1 = 0.0f;
    attenuation2 = 0.0f;
    spotOuterAngle = 1.5707964f;
    spotInnerAngle = 0.7853982f;
    lightIndex = -1;
    manager = 0;
    field_0xa8 = 0;
}

// 0x0049dfc0
LightEmitter::~LightEmitter() {
}

// 0x00467ae0 (shared `return 1` body)
int LightEmitter::PrepareGeometry() {
    return 1;
}

// 0x0049dfd0
unsigned int LightEmitter::PackedColor() {
    return (((((int)(colorRGBA[3] * 255.0f) << 8) | (int)(colorRGBA[0] * 255.0f)) << 8 |
             (int)(colorRGBA[1] * 255.0f)) << 8) | (int)(colorRGBA[2] * 255.0f);
}

// 0x0049e020
void LightEmitter::SetColor(unsigned int color) {
    if (color == field_0xa8)
        return;
    colorRGBA[0] = (float)((color >> 16) & 0xff) * (1.0f / 255.0f);
    colorRGBA[1] = (float)((color >> 8) & 0xff) * (1.0f / 255.0f);
    colorRGBA[2] = (float)(color & 0xff) * (1.0f / 255.0f);
    colorRGBA[3] = 0.0f;
    specularRGBA[0] = colorRGBA[0];
    specularRGBA[1] = colorRGBA[1];
    specularRGBA[2] = colorRGBA[2];
    specularRGBA[3] = 0.0f;
    ambientRGBA[0] = colorRGBA[0];
    ambientRGBA[1] = colorRGBA[1];
    ambientRGBA[2] = colorRGBA[2];
    ambientRGBA[3] = 0.0f;
    if (linkedTarget)
        linkedTarget->SetRgbColor(color);
    if (manager)
        manager->MarkChanged();
}

// 0x0049e0f0
void LightEmitter::SetRange(float range) {
    if (range == 0.0f || range > (float)sqrt(FLT_MAX))
        range = (float)sqrt(FLT_MAX);
    if (range != lightRange) {
        lightRange = range;
        if (manager)
            manager->MarkChanged();
    }
}

// 0x0049e150
void LightEmitter::SetPosition(const Vector3* position) {
    if (position->x != lightPosition.x || position->y != lightPosition.y || position->z != lightPosition.z) {
        if (debugSphere)
            debugSphere->SetPositionVec3(*position);
        lightPosition = *position;
        if (linkedTarget)
            linkedTarget->SetPosition(position);
        if (manager)
            manager->MarkChanged();
    }
}

// 0x0049e1e0
void LightEmitter::SetDirection(const Vector3* direction) {
    if (direction->x != lightDirection.x || direction->y != lightDirection.y || direction->z != lightDirection.z) {
        lightDirection = *direction;
        if (manager)
            manager->MarkChanged();
    }
}

// 0x0049e230
LightEmitter* LightEmitter::Create(void* context, int type, unsigned int color,
                                                  const Vector3* position, const Vector3* direction,
                                                  float range, int sphere, int a8, int a9, int a10,
                                                  int index) {
    GameObject::SetRenderContext(context);
    lightType = type;
    lightIndex = index;
    if (sphere) {
        debugSphere = new(__FILE__, 183) D3DIMSoultreeObject(1);
        AppendChild(debugSphere->UnknownVirtualSlot9(renderContext, "sphere.slt", 0, 0, 1), -1);
        if (debugSphere)
            debugSphere->Scale(0.4f, 0.4f, 0.4f);
    }
    SetColor(color);
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
        d3dLightType = 1;
        break;
    case 5:
        d3dLightType = 2;
        break;
    case 4:
        d3dLightType = 3;
        break;
    case 2:
        d3dLightType = 4;
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
GameObject* LightManager::SetRenderContext(void* context) {
    return GameObject::SetRenderContext(context);
}

// 0x0049e470
void LightManager::AddLight(LightEmitter* light) {
    lights[lightCount] = light;
    light->manager = this;
    lightCount++;
}

// 0x0049e490
void LightManager::MarkChanged() {
    changeCount++;
}

// 0x0049f180
Vector3 operator-(const Vector3& v) {
    return Vector3(-v.x, -v.y, -v.z);
}

// 0x004a0190
LightEmitter* LightManager::FindLightOfType(int type) {
    int i;

    for (i = 0; i < lightCount; i++) {
        if (lights[i]->lightType == type)
            return lights[i];
    }
    return 0;
}
