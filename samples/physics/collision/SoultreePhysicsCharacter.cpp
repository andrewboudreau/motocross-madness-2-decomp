// SoultreePhysicsCharacter small virtuals (assigned targets).  TU ownership tier 2:
// SoulTreePhysics.cpp (__FILE__ at 0x00503b13/0x00503f87).  The slot 40 loaders
// (0x00503970, 0x00503de0) load that __FILE__ string and live in
// src/krusty2/soultree/SoulTreePhysics.cpp.
#include "soultree/SoultreePhysicsCharacter.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "soultree/SoultreePhysicsObject.h"

// SoultreePhysicsBaseObject::field_0x128 is the CollisionObject that slot 2 creates
// (`new` of 0xb8 bytes, ctor 0x00431e70, tier 1).

// slot 1 (0x005040c0): base slot 1 then clears four flag bytes.
void SoultreePhysicsCharacter::UnknownVirtualSlot1(float value)
{
    SoultreePhysicsBaseObject::UnknownVirtualSlot1(value);
    field_0x430 = 0;
    field_0x431 = 0;
    field_0x432 = 0;
    field_0x433 = 0;
}

// slot 8 (0x005041c0)
void SoultreePhysicsCharacter::UnknownVirtualSlot8()
{
    rotationPivot = localCenterOfMass = poseNode->WorldToLocalPoint(centerOfMass);
}

// slot 42 (0x00504470)
int SoultreePhysicsCharacter::UnknownVirtualSlot42()
{
    if (field_0x433 >= 0 && field_0x430) {
        return 1;
    }
    return 0;
}

// slot 33 (0x005040f0), retail `ret 0x18`
int SoultreePhysicsCharacter::UnknownVirtualSlot33(const Vec3* a1, const Vec3* a2,
                                                   const Vec3* a3, const Vec3* a4,
                                                   int a5, float a6)
{
    UnknownVirtualSlot1(a6);
    bodyForward = *a2;
    bodyUp = *a3;
    sceneNode->SetPosition(a1->x, a1->y, a1->z);
    sceneNode->GetPosition(&position);
    UnknownVirtualSlot36();
    if (centerNode) {
        centerNode->GetPositionIn(0, &centerOfMass);
    } else {
        sceneNode->GetPositionIn(0, &centerOfMass);
    }
    ApplyRestPose();
    collisionObject->Fn_00435fe0();
    respawnPending = 0;
    justReset = 1;
    attachmentResetPending = 1;
    return 0;
}

// slot 41 (0x00504360)
void SoultreePhysicsCharacter::UnknownVirtualSlot41()
{
    poseNode->GetAxesIn(0, &savedForward, &savedUp);
    modelNode->SetAxesIn(0, &savedForward, &savedUp, 1, 0);
    ApplyRestPose();
    field_0x430 = 0;
    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll, &bodySinRoll,
              &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedSinRoll = bodySinRoll;
    savedCosRoll = bodyCosRoll;
    savedCosPitch = bodyCosPitch;
    savedSinPitch = bodySinPitch;
}
