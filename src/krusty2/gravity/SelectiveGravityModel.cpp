// SelectiveGravityModel.cpp -- reconstruction of D:\aardvark\VC\krusty2\SelectiveGravityModel.cpp
// (the SelectiveGravityModel class only).
//
// Ownership: the destructor core and AddBody reference this file's __FILE__ string
// (0x00573f54, lines 0x12 and 0x18); the remaining methods are slots of the same RTTI
// vtable laid out contiguously with them (0x004f9760..0x004f9a5a), together with the TU's
// own math/Math3D.h constant-vector initializers (0x004f9910..0x004f9a4b).
//
// The Shock family that follows at 0x004f9a60 builds a second, separate set of the same four
// constants (0x00689e48..0x00689e78, initializers at 0x004fafd0..0x004fb10b).  One TU emits
// that header's statics once, so Shock/InlineShock/RotatingShock belong to a different
// translation unit (no __FILE__ string of its own); they stay in samples/physics/suspension/.
#include "gravity/SelectiveGravityModel.h"
#include "core/DebugAlloc.h"

// 0x004f9760
SelectiveGravityModel::SelectiveGravityModel(int flags)
    : GameObject(flags)
{
    bodies = 0;
    bodyCount = 0;
    field_0x38 = 0;
    gravity = 32.2f;
}

// slot 0: scalar deleting 0x004f9790 is compiler generated; this is the core 0x004f97b0.
SelectiveGravityModel::~SelectiveGravityModel()
{
    if (bodies)
        operator delete(bodies, __FILE__, 0x12);
}

// slot 27, 0x004f9820
void SelectiveGravityModel::AddBody(GravityBody* body)
{
    bodies = (GravityBody**)DebugRealloc(bodies, bodyCount * 4 + 4, __FILE__, 0x18);
    bodies[bodyCount] = body;
    bodyCount++;
}

// slot 10, 0x004f9860
int SelectiveGravityModel::GameObjectVirtualSlot10(float dt)
{
    GameObject::GameObjectVirtualSlot10(dt);
    if (!field_0x38)
        GameObjectVirtualSlot11(dt);
    return 1;
}

// slot 11, 0x004f9890: pushes each body down along -Y with its weight.  The scaled unit
// vector (Math3D operator*) is what makes VC6 load 0.0f once from memory and store it to
// both x and z; a literal Vec3(0, -w, 0) folds those into integer stores instead.
int SelectiveGravityModel::GameObjectVirtualSlot11(float dt)
{
    for (int i = 0; i < bodyCount; i++) {
        GravityBody* b = bodies[i];
        b->AddWorldForce(Vec3(0.0f, -1.0f, 0.0f) * (b->mass * gravity));
    }
    return 1;
}
