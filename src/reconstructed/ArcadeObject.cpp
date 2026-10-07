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
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x68 = 0;
    field_0x6c = 0;
    field_0x70 = 0;
    field_0x8c = 0;
    field_0x90 = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0x78 = 0;
    field_0x7c = 0;
    field_0x84 = 0;
    field_0x88 = 0;
    modelWidth = 0;
    field_0x54 = 0;
    field_0x58 = 0;
    modelScale = 1.0f;
    field_0x74 = 1;
}

// 0x00401300
ArcadeObject::~ArcadeObject() {
}

// 0x00401310
ArcadeObject* ArcadeObject::UnknownFunction401310(void* value, int a, int b, const char* name,
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
    UnknownFunction4014f0(&position);
    isVisible = 1;
    field_0x44 = size;
    arcadeView = view;
    arcadeModel->UnknownFunction4fe850(&low, &extent);
    modelWidth = extent.x + extent.x;
    field_0x54 = extent.y + extent.y;
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
        arcadeModel->UnknownFunction4fd340(scale, scale, scale);
    }
    if (alpha) {
        field_0x58 = new (__FILE__, 104) TransparencyMod(1);
        AppendChild(field_0x58, -1);
        field_0x58->field_0x48 = alpha;
        arcadeModel->UnknownFunction444d80(field_0x58);
    }
    return this;
}

// 0x004014f0
void ArcadeObject::UnknownFunction4014f0(const Vector3* position) {
    arcadeModel->UnknownFunction4fc660(position);
    if (field_0x60)
        field_0x5c->UnknownFunction435fe0();
}

// 0x00401520
void ArcadeObject::UnknownFunction401520(const Vector3* a, const Vector3* b, int c, int d) {
    arcadeModel->UnknownFunction4fbd70(a, b, c, d);
}

// 0x00401540
int ArcadeObject::UnknownFunction401540(const char* name, int value, UnknownPickCallback callback,
                                        void* context, unsigned char a, unsigned char b) {
    char tag[0x80];
    char found[0x104];
    char path[0x104];

    g_MemTagStack->UnknownFunction4a2da0(tag);
    g_MemTagStack->Push("Collision");
    field_0x5c = new (__FILE__, 154) CollisionObject(1);
    field_0x5c->UnknownFunction4320f0(field_0x18, 1, a, b);
    strcpy(path, name);
    strcpy(strrchr(path, '.'), ".col");
    UnknownTextureStream* stream = new (__FILE__, 160) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)found))
        found[0] = 0;
    if (stream)
        delete stream;
    if (found[0])
        field_0x5c->UnknownFunction432800(arcadeModel, found);
    else
        field_0x5c->UnknownFunction432720(arcadeModel, 1, 0, 0, 0);
    field_0x5c->UnknownFunction4394d0(arcadeModel, value);
    field_0x5c->UnknownFunction435fe0();
    field_0x5c->field_0x88 = callback;
    field_0x5c->field_0x8c = context;
    AppendChild(field_0x5c, -1);
    g_MemTagStack->Push(tag);
    field_0x60 = 1;
    return 1;
}

// 0x004017a0
void ArcadeObject::UnknownFunction4017a0(Vector3 axis, float speed) {
    field_0x90 = 1;
    if (speed != 0.0f) {
        field_0x94 = axis;
        field_0xa0 = 0;
        field_0xa4 = speed * 0.01745329f;
    }
}

// 0x00401800
int ArcadeObject::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x64) {
        if (field_0x70 > field_0x68)
            isVisible = 0;
        else
            field_0x70 += g_TrackGame->frameTime;
    }
    if (field_0x78 && !field_0x74) {
        field_0x80 += g_TrackGame->frameTime;
        if (field_0x80 > field_0x7c) {
            if (field_0x88 < field_0x84 || field_0x84 == 999) {
                field_0x88++;
                field_0x74 = 1;
            }
            field_0x80 = 0;
        }
    }
    if (field_0x74 && field_0x90) {
        field_0xa0 = g_TrackGame->frameTime * field_0xa4;
        while (field_0xa0 > 6.2831855f)
            field_0xa0 -= 6.2831855f;
        arcadeModel->UnknownFunction4fceb0(field_0x94, field_0xa0);
    }
    return 1;
}

// 0x00401920
int ArcadeObject::UnknownVirtualSlot14() {
    if (field_0x74 && isVisible)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}
