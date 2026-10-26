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
void SoultreePhysicsCharacter::UnknownVirtualSlot1(float extraWeight)
{
    SoultreePhysicsBaseObject::UnknownVirtualSlot1(extraWeight);
    motionActive = 0;
    motionCompleted = 0;
    motionJustStarted = 0;
    motionIndex = 0;
}

// slot 8 (0x005041c0)
void SoultreePhysicsCharacter::UnknownVirtualSlot8()
{
    rotationPivot = localCenterOfMass = poseNode->WorldToLocalPoint(centerOfMass);
}

// slot 42 (0x00504470)
int SoultreePhysicsCharacter::UnknownVirtualSlot42()
{
    if (motionIndex >= 0 && motionActive) {
        return 1;
    }
    return 0;
}

// slot 33 (0x005040f0), retail `ret 0x18`
int SoultreePhysicsCharacter::UnknownVirtualSlot33(const Vec3* newPosition, const Vec3* newForward,
                                                   const Vec3* newUp, const Vec3* settleUp,
                                                   int keepPlacement, float extraWeight)
{
    UnknownVirtualSlot1(extraWeight);
    bodyForward = *newForward;
    bodyUp = *newUp;
    sceneNode->SetPosition(newPosition->x, newPosition->y, newPosition->z);
    sceneNode->GetPosition(&position);
    UnknownVirtualSlot36();
    if (centerNode) {
        centerNode->GetPositionIn(0, &centerOfMass);
    } else {
        sceneNode->GetPositionIn(0, &centerOfMass);
    }
    ApplyRestPose();
    collisionObject->SnapPlacement();
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
    motionActive = 0;
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
