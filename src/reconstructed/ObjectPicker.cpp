#include "ObjectPicker.h"

#include "ControlInterface.h"
#include "GUIManager.h"
#include "RenderTarget.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

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
