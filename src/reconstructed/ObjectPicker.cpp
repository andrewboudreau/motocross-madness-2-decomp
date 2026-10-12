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
    lastPickX = 0;
    lastPickY = 0;
    cursorBindingX = 0;
    cursorBindingY = 0;
}

// 0x004b04c0
int ObjectPicker::PickAtCursor() {
    UnknownCursorPosition position = field_0x34->CursorPosition();

    return PickAt(position.x, ((RenderTarget*)field_0x18)->targetHeight - position.y - 1);
}

// 0x004b0730
ObjectPicker::~ObjectPicker() {
    if (cursorBindingX)
        delete cursorBindingX;
    if (cursorBindingY)
        delete cursorBindingY;
}

// 0x004b0500
int ObjectPicker::PickAt(int x, int y) {
    UnknownPickCamera* camera;
    Vector3 direction;
    Vector3 position;
    Vector3 segment[2];
    float u;
    float v;
    float w;
    float length;

    lastPickX = x;
    lastPickY = y;
    camera = (UnknownPickCamera*)((RenderTarget*)field_0x18)->currentCamera;
    position = camera->eyePosition;
    u = (float)x - camera->viewportX - camera->viewportWidth / 2;
    v = (float)y - (((RenderTarget*)field_0x18)->targetHeight - camera->viewportY - camera->viewportHeight) -
        camera->viewportHeight / 2;
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
    pickRay->SetMeshShape(1, segment);
    pickRay->UpdatePlacement();
    if (pickRay->QueryCollisions())
        return field_0x2c;
    return 0;
}

// 0x004b0210
ObjectPicker* ObjectPicker::Create(void* target, TextureMapManager* textures,
                                                  UnknownPickCallback callback, GameCursor* cursor) {
    Vector3 segment[2];

    GameObject::UnknownVirtualSlot8(target);
    pickRay = new(__FILE__, 36) CollisionObject(1);
    pickRay->Configure(target, 1, 1, 1);
    segment[0] = Vector3(0.0f, 0.0f, 0.0f);
    segment[1] = Vector3(0.0f, 0.0f, 0.0f);
    pickRay->SetMeshShape(1, segment);
    pickRay->onHitCallback = callback;
    pickRay->ignoreVegetation = 1;
    AppendChild(pickRay, -1);
    if (cursor) {
        field_0x34 = cursor;
    } else {
        cursorBindingX = new(__FILE__, 59)
            UnknownControlBinding(0.0f, (float)((UnknownPickCamera*)((RenderTarget*)field_0x18)->currentCamera)->cursorRangeX,
                                  0.0f, g_TrackGame->controlInterface->UnknownFunction43ce90(), 0, 0.1f);
        cursorBindingY = new(__FILE__, 60)
            UnknownControlBinding(0.0f, (float)((UnknownPickCamera*)((RenderTarget*)field_0x18)->currentCamera)->cursorRangeY,
                                  0.0f, g_TrackGame->controlInterface->UnknownFunction43ce90(), 1, 0.1f);
        if (g_TrackGame->controlInterface->mouse) {
            g_TrackGame->controlInterface->mouse->AttachBinding(cursorBindingX);
            g_TrackGame->controlInterface->mouse->AttachBinding(cursorBindingY);
        }
        field_0x34 = (GameCursor*)(new(__FILE__, 66) GameCursor(1))
                         ->CreateBound(field_0x18, cursorBindingX, cursorBindingY, "cursor.tga", textures, 0, 0, 0);
        AppendChild(field_0x34, -1);
    }
    if (!field_0x34) {
        delete this;
        return 0;
    }
    return this;
}
