// Group5 bring-up definitions (best-guess, link frontier).
//
// SoultreePhysicsCharacter's constructor (0x00503c70) and destructor core
// (0x00503d40), retail TU SoulTreePhysics.cpp, from
// samples/physics/hierarchy/LayoutProbe.cpp (which proves the class layout):
//  * the constructor builds the virtual GameObject base with GameObject(1)
//    (`push 1` before 0x00468ca0), passes the caller's argument to both
//    non-virtual bases (0x00500aa0, 0x004455b0), and clears the node pointer;
//  * the destructor has no own cleanup: VC6 restores the vptrs/vtordisp and
//    runs ~D3DIMSoultreeCharacter (0x004459a0), then
//    ~SoultreePhysicsBaseObject (0x00501260).
#include "soultree/SoultreePhysicsCharacter.h"

SoultreePhysicsCharacter::SoultreePhysicsCharacter(int flags)
    : GameObject(1), SoultreePhysicsBaseObject(flags), D3DIMSoultreeCharacter(flags)
{
    poseNode = 0;
}

SoultreePhysicsCharacter::~SoultreePhysicsCharacter()
{
}

// 0x00504210 (reached through the vtordisp thunk 0x00504350), also from
// LayoutProbe.cpp; the constructor's vtable needs it and nothing else in src/
// defines it. When slot 42 reports the motion done, motionActive is cleared
// and motionCompleted set; then the physics update and the plain GameObject
// update run with the same frame time.
int SoultreePhysicsCharacter::Tick(float dt)
{
    if (UnknownVirtualSlot42()) {
        motionActive = 0;
        motionCompleted = 1;
    }
    SoultreePhysicsBaseObject::Tick(dt);
    return GameObject::Tick(dt);
}
