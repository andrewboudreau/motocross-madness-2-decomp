#include "ObjectPicker.h"

#include "ControlInterface.h"
#include "DebugAlloc.h"
#include "GUIManager.h"
#include "MouseDevice.h"
#include "RenderTarget.h"
#include "TrackGame.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

float FastInvSqrt(float x); // 0x00460c00

// 0x004b01c0
ObjectPicker::ObjectPicker(int flags) : GameObject(flags) {
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
}

// 0x004b04c0
int ObjectPicker::UnknownFunction4b04c0() {
    UnknownCursorPosition position = field_0x34->UnknownFunction43f100();

    return UnknownFunction4b0500(position.x, ((RenderTarget*)field_0x18)->field_0x10 - position.y - 1);
}

// 0x004b0730
ObjectPicker::~ObjectPicker() {
    if (field_0x40)
        delete field_0x40;
    if (field_0x44)
        delete field_0x44;
}

// 0x004b0500
int ObjectPicker::UnknownFunction4b0500(int x, int y) {
    UnknownPickCamera* camera;
    Vector3 direction;
    Vector3 position;
    Vector3 segment[2];
    float u;
    float v;
    float w;
    float length;

    field_0x38 = x;
    field_0x3c = y;
    camera = (UnknownPickCamera*)((RenderTarget*)field_0x18)->field_0x08;
    position = camera->field_0x170;
    u = (float)x - camera->field_0x1a0 - camera->field_0x1a8 / 2;
    v = (float)y - (((RenderTarget*)field_0x18)->field_0x10 - camera->field_0x1a4 - camera->field_0x1ac) -
        camera->field_0x1ac / 2;
    w = camera->field_0x198;
    direction.x = u * camera->field_0x0ac[0][0] + v * camera->field_0x0ac[0][1] + w * camera->field_0x0ac[0][2] +
                  camera->field_0x0ac[0][3];
    direction.y = u * camera->field_0x0ac[1][0] + v * camera->field_0x0ac[1][1] + w * camera->field_0x0ac[1][2] +
                  camera->field_0x0ac[1][3];
    direction.z = u * camera->field_0x0ac[2][0] + v * camera->field_0x0ac[2][1] + w * camera->field_0x0ac[2][2] +
                  camera->field_0x0ac[2][3];
    length = direction.z * direction.z + (direction.x * direction.x + direction.y * direction.y);
    if (length == 0.0f) {
        direction = kVec3Zero;
    } else {
        length = FastInvSqrt(length);
        direction.x *= length;
        direction.y *= length;
        direction.z *= length;
    }
    direction.x *= 800.0f;
    direction.y *= 800.0f;
    direction.z *= 800.0f;
    direction += position;
    segment[0] = position;
    segment[1] = direction;
    field_0x30->UnknownFunction432ab0(1, segment);
    field_0x30->UnknownFunction435fb0();
    if (field_0x30->UnknownFunction438e70())
        return field_0x2c;
    return 0;
}

// 0x004b0210
ObjectPicker* ObjectPicker::UnknownFunction4b0210(void* target, TextureMapManager* textures,
                                                  UnknownPickCallback callback, GameCursor* cursor) {
    Vector3 segment[2];

    GameObject::UnknownVirtualSlot8(target);
    field_0x30 = new(__FILE__, 36) CollisionObject(1);
    field_0x30->UnknownFunction4320f0(target, 1, 1, 1);
    segment[0] = Vector3(0.0f, 0.0f, 0.0f);
    segment[1] = Vector3(0.0f, 0.0f, 0.0f);
    field_0x30->UnknownFunction432ab0(1, segment);
    field_0x30->field_0x88 = callback;
    field_0x30->field_0x68 = 1;
    UnknownFunction469190(field_0x30, -1);
    if (cursor) {
        field_0x34 = cursor;
    } else {
        field_0x40 = new(__FILE__, 59)
            UnknownControlBinding(0.0f, (float)((UnknownPickCamera*)((RenderTarget*)field_0x18)->field_0x08)->field_0x1c4,
                                  0.0f, g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 0, 0.1f);
        field_0x44 = new(__FILE__, 60)
            UnknownControlBinding(0.0f, (float)((UnknownPickCamera*)((RenderTarget*)field_0x18)->field_0x08)->field_0x1c8,
                                  0.0f, g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 1, 0.1f);
        if (g_UnknownGlobal56e26c->field_0x14->mouse) {
            g_UnknownGlobal56e26c->field_0x14->mouse->UnknownFunction48a420(field_0x40);
            g_UnknownGlobal56e26c->field_0x14->mouse->UnknownFunction48a420(field_0x44);
        }
        field_0x34 = (GameCursor*)(new(__FILE__, 66) GameCursor(1))
                         ->UnknownFunction43ea70(field_0x18, field_0x40, field_0x44, "cursor.tga", textures, 0, 0, 0);
        UnknownFunction469190(field_0x34, -1);
    }
    if (!field_0x34) {
        delete this;
        return 0;
    }
    return this;
}
