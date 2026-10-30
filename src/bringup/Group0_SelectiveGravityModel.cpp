// Group0_SelectiveGravityModel.cpp -- bring-up body for the gravity model
// Wrecker creates at +0x3c (RTTI SelectiveGravityModel, vtable 0x00557b0c,
// D:\aardvark\VC\krusty2\SelectiveGravityModel.cpp at 0x00573f54).
// Behavior follows the retail disassembly; byte matching was not attempted.
//
// src/reconstructed/Wrecker.h declares the class as
// UnknownWreckerGravityModel with only the constructor, slot 27 and the
// +0x38 manualStep flag. Defining the constructor makes this translation
// unit emit the class's vtable, so every slot SelectiveGravityModel
// overrides in retail (0, 10, 11, 27, 28) is declared and defined here, in
// a local declaration of the same class (same name and `class` keyword, so
// the mangled names are the ones Wrecker.cpp references; same 0x3c size and
// field offsets as Wrecker.h). Wrecker.h is deliberately not included.
//
// Retail overrides (vtable 0x00557b0c):
//   slot 0  0x004f9790 deleting destructor (core 0x004f97b0)
//   slot 10 0x004f9860 Tick
//   slot 11 0x004f9890 per-step update: applies gravity to every body
//   slot 27 0x004f9820 adds a body
//   slot 28 0x004f9a50 sets the gravity constant
#include "GameObject.h"
#include "DebugAlloc.h"

static const char kSelectiveGravityModelFile[] = "D:\\aardvark\\VC\\krusty2\\SelectiveGravityModel.cpp";

// A force passed by value (three floats on the stack).
struct Group0GravityForce {
    float x, y, z;
};

// The body view slot 11 needs: PhysicsRigidBody slot 37 (0x004cc1f0,
// ret 0xc) adds a world-space force to the accumulator at +0x210, and the
// mass is at +0x178 (see Wrecker.h UnknownWreckerRigidBody,
// samples/physics/rigidbody/PhysicsBody.h AddWorldForce). Never
// instantiated; only used to call through the body's own vtable.
class Group0GravityBodyView {
public:
    virtual void Slot0() = 0;
    virtual void Slot1() = 0;
    virtual void Slot2() = 0;
    virtual void Slot3() = 0;
    virtual void Slot4() = 0;
    virtual void Slot5() = 0;
    virtual void Slot6() = 0;
    virtual void Slot7() = 0;
    virtual void Slot8() = 0;
    virtual void Slot9() = 0;
    virtual void Slot10() = 0;
    virtual void Slot11() = 0;
    virtual void Slot12() = 0;
    virtual void Slot13() = 0;
    virtual void Slot14() = 0;
    virtual void Slot15() = 0;
    virtual void Slot16() = 0;
    virtual void Slot17() = 0;
    virtual void Slot18() = 0;
    virtual void Slot19() = 0;
    virtual void Slot20() = 0;
    virtual void Slot21() = 0;
    virtual void Slot22() = 0;
    virtual void Slot23() = 0;
    virtual void Slot24() = 0;
    virtual void Slot25() = 0;
    virtual void Slot26() = 0;
    virtual void Slot27() = 0;
    virtual void Slot28() = 0;
    virtual void Slot29() = 0;
    virtual void Slot30() = 0;
    virtual void Slot31() = 0;
    virtual void Slot32() = 0;
    virtual void Slot33() = 0;
    virtual void Slot34() = 0;
    virtual void Slot35() = 0;
    virtual void Slot36() = 0;
    virtual void AddWorldForce(Group0GravityForce force) = 0; // slot 37

    unsigned char field_0x04[0x178 - 0x04];
    float mass;                               // +0x178
};

class UnknownWreckerRigidBody;

class UnknownWreckerGravityModel : public GameObject {
public:
    explicit UnknownWreckerGravityModel(int flags);           // 0x004f9760
    virtual ~UnknownWreckerGravityModel();                    // 0x004f97b0
    virtual int Tick(float frameTime);                        // slot 10, 0x004f9860
    virtual int UnknownVirtualSlot11(float frameTime);        // slot 11, 0x004f9890
    virtual void UnknownVirtualSlot27(UnknownWreckerRigidBody* body); // slot 27, 0x004f9820
    virtual void UnknownVirtualSlot28(float value);           // slot 28, 0x004f9a50

    float gravity;                            // +0x2c ctor 32.2f (0x4200cccd), set by slot 28
    UnknownWreckerRigidBody** bodies;         // +0x30 DebugRealloc'd by slot 27, freed by the destructor
    int bodyCount;                            // +0x34
    int manualStep;                           // +0x38 Tick runs slot 11 itself only while 0
};

// 0x004f9760 (ret 4)
UnknownWreckerGravityModel::UnknownWreckerGravityModel(int flags) : GameObject(flags)
{
    bodies = 0;
    bodyCount = 0;
    manualStep = 0;
    gravity = 32.2f;
}

// 0x004f97b0: frees the body array (line 0x12), then the GameObject core.
UnknownWreckerGravityModel::~UnknownWreckerGravityModel()
{
    if (bodies)
        DebugFree(bodies, kSelectiveGravityModelFile, 0x12);
}

// 0x004f9860 (ret 4): the GameObject tick, then the gravity step unless the
// owner steps the model itself (Wrecker sets manualStep).
int UnknownWreckerGravityModel::Tick(float frameTime)
{
    GameObject::Tick(frameTime);
    if (!manualStep)
        UnknownVirtualSlot11(frameTime);
    return 1;
}

// 0x004f9890 (ret 4): applies the force (0, -mass * gravity, 0) to every
// body. The timestep is not used.
int UnknownWreckerGravityModel::UnknownVirtualSlot11(float frameTime)
{
    for (int i = 0; i < bodyCount; i++) {
        Group0GravityBodyView* body = (Group0GravityBodyView*)bodies[i];
        float weight = body->mass * gravity;
        Group0GravityForce force;
        force.x = 0.0f;
        force.y = weight * -1.0f;
        force.z = 0.0f;
        body->AddWorldForce(force);
    }
    return 1;
}

// 0x004f9820 (ret 4): appends `body` to the array (DebugRealloc, line 0x18).
void UnknownWreckerGravityModel::UnknownVirtualSlot27(UnknownWreckerRigidBody* body)
{
    bodies = (UnknownWreckerRigidBody**)DebugRealloc(bodies, bodyCount * 4 + 4,
                                                     kSelectiveGravityModelFile, 0x18);
    bodies[bodyCount] = body;
    bodyCount++;
}

// 0x004f9a50 (ret 4)
void UnknownWreckerGravityModel::UnknownVirtualSlot28(float value)
{
    gravity = value;
}
