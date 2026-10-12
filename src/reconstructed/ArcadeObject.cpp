#include "ArcadeObject.h"

#include <string.h>

#include "DebugAlloc.h"
#include "LightEmitter.h"
#include "MemTag.h"
#include "ObjectPicker.h"
#include "TextureMap.h"
#include "TrackGame.h"

// 0x00401260
ArcadeObject::ArcadeObject(int flags) : GameObject(flags) {
    arcadeModel = 0;
    isVisible = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    arcadeView = 0;
    collisionObject = 0;
    collisionBuilt = 0;
    lifetimeEnabled = 0;
    lifetime = 0;
    field_0x6c = 0;
    elapsedLifetime = 0;
    field_0x8c = 0;
    spinning = 0;
    spinAngle = 0;
    spinSpeed = 0;
    delayEnabled = 0;
    delay = 0;
    repeatLimit = 0;
    repeatCount = 0;
    modelWidth = 0;
    modelHeight = 0;
    transparency = 0;
    modelScale = 1.0f;
    field_0x74 = 1;
}

// 0x00401300
ArcadeObject::~ArcadeObject() {
}

// 0x00401310
ArcadeObject* ArcadeObject::Create(void* value, int a, int b, const char* name,
                                                  Vector3 position, int pixels, UnknownArcadeView* view,
                                                  float size, float c, float d, unsigned char alpha) {
    Vector3 low;
    Vector3 extent;

    GameObject::UnknownVirtualSlot8(value);
    arcadeModel = new (__FILE__, 78) D3DIMSoultreeObject(1);
    if (!AppendChild(arcadeModel->UnknownVirtualSlot9(field_0x18, name, a, b, 1), -1)) {
        Release();
        return 0;
    }
    SetPosition(&position);
    isVisible = 1;
    field_0x44 = size;
    arcadeView = view;
    arcadeModel->GetSubtreeBounds(&low, &extent);
    modelWidth = extent.x + extent.x;
    modelHeight = extent.y + extent.y;
    if (pixels && arcadeView) {
        field_0x48 = c;
        field_0x4c = d;
        float largest = extent.x > extent.y ? extent.x : extent.y;
        if (!(largest > extent.z))
            largest = extent.z;
        field_0x3c = largest + largest;
        field_0x38 = (float)pixels;
        float scale = field_0x38 * field_0x44 / (field_0x3c * arcadeView->field_0x198);
        modelScale = scale;
        arcadeModel->Scale(scale, scale, scale);
    }
    if (alpha) {
        transparency = new (__FILE__, 104) TransparencyMod(1);
        AppendChild(transparency, -1);
        transparency->field_0x48 = alpha;
        arcadeModel->UnknownFunction444d80(transparency);
    }
    return this;
}

// 0x004014f0
void ArcadeObject::SetPosition(const Vector3* position) {
    arcadeModel->SetPosition(position);
    if (collisionBuilt)
        collisionObject->UnknownFunction435fe0();
}

// 0x00401520
void ArcadeObject::SetAxes(const Vector3* a, const Vector3* b, int c, int d) {
    arcadeModel->SetAxesPtr(a, b, c, d);
}

// 0x00401540
int ArcadeObject::BuildCollision(const char* name, int value, UnknownPickCallback callback,
                                        void* context, unsigned char a, unsigned char b) {
    char tag[0x80];
    char found[0x104];
    char path[0x104];

    g_MemTagStack->CopyCurrentName(tag);
    g_MemTagStack->Push("Collision");
    collisionObject = new (__FILE__, 154) CollisionObject(1);
    collisionObject->Configure(field_0x18, 1, a, b);
    strcpy(path, name);
    strcpy(strrchr(path, '.'), ".col");
    UnknownTextureStream* stream = new (__FILE__, 160) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)found))
        found[0] = 0;
    if (stream)
        delete stream;
    if (found[0])
        collisionObject->LoadShape(arcadeModel, found);
    else
        collisionObject->SetHullShape(arcadeModel, 1, 0, 0, 0);
    collisionObject->SetOwner(arcadeModel, value);
    collisionObject->UnknownFunction435fe0();
    collisionObject->field_0x88 = callback;
    collisionObject->field_0x8c = context;
    AppendChild(collisionObject, -1);
    g_MemTagStack->Push(tag);
    collisionBuilt = 1;
    return 1;
}

// 0x004017a0
void ArcadeObject::StartSpin(Vector3 axis, float speed) {
    spinning = 1;
    if (speed != 0.0f) {
        spinAxis = axis;
        spinAngle = 0;
        spinSpeed = speed * 0.01745329f;
    }
}

// 0x00401800
int ArcadeObject::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (lifetimeEnabled) {
        if (elapsedLifetime > lifetime)
            isVisible = 0;
        else
            elapsedLifetime += g_TrackGame->frameTime;
    }
    if (delayEnabled && !field_0x74) {
        elapsedDelay += g_TrackGame->frameTime;
        if (elapsedDelay > delay) {
            if (repeatCount < repeatLimit || repeatLimit == 999) {
                repeatCount++;
                field_0x74 = 1;
            }
            elapsedDelay = 0;
        }
    }
    if (field_0x74 && spinning) {
        spinAngle = g_TrackGame->frameTime * spinSpeed;
        while (spinAngle > 6.2831855f)
            spinAngle -= 6.2831855f;
        arcadeModel->Rotate(spinAxis, spinAngle);
    }
    return 1;
}

// 0x00401920
int ArcadeObject::UnknownVirtualSlot14() {
    if (field_0x74 && isVisible)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}
