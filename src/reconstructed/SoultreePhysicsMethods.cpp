// Exact primary-vtable methods of SoultreePhysicsBaseObject.
// Vtable 0x00557d90 has object offset 0; slots are verified independently.
// This is a reconstruction grouping, not a claim about original TU ownership.
// Names and semantic field roles remain provisional.
#include <math.h>
#include <string.h>
#include "soultree/SoultreePhysicsBaseObject.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "soultree/SoultreePhysicsContact.h"

int SoultreePhysicsBaseObject::IsAtBaseWeight()
{
    if (totalWeight == baseWeight)
        return 1;
    return 0;
}

int SoultreePhysicsBaseObject::ShouldUpdateShadowLight()
{
    return staggerCounter == staggerPhase;
}

void SoultreePhysicsBaseObject::UpdateSlideEffect(SoultreeAttachment* attachment)
{
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot26()
{
}

// slot 5 (0x004aa150): shared with Vehicle/Character, compares its argument with zero.
int SoultreePhysicsBaseObject::ShouldRefreshContacts(int value)
{
    return value == 0;
}

int SoultreePhysicsBaseObject::ShouldEmitMovingEffects()
{
    return !airborne && linearSpeed >= 1.0f;
}

int SoultreePhysicsBaseObject::ShouldRestartSteamEffect()
{
    return 0;
}

float SoultreePhysicsBaseObject::EffectiveLoadWeight()
{
    if (IsAtBaseWeight())
        return 0.0f;
    return loadWeight;
}

void SoultreePhysicsBaseObject::LoadBodyAxes()
{
    sceneNode->GetAxesIn(0, &bodyForward, &bodyUp);
}

// slot 35 (0x004aa1e0)
void SoultreePhysicsBaseObject::StoreBodyAxes(int orthogonalize, int keepZ)
{
    sceneNode->SetAxesIn(0, &bodyForward, &bodyUp, orthogonalize, keepZ);
}

// slot 36 (0x004aa210): when the forward axis is vertical it becomes -/+ the up axis; the up
// axis is reset to Math3D.h's kVec3YAxis, forward is flattened, slots 35 and 34 re-derive the
// axes, then the pose angles and their saved copies are reset.  Retail's copy reads
// MSZoneInterface.cpp's kVec3YAxis (0x00685190) and sits in that unit's code range
// (0x004aa150..0x004aa340), so it looks like a header inline the linker kept from that unit
// (inference); this grouping's own kVec3YAxis holds the same (0, 1, 0).
void SoultreePhysicsBaseObject::ResetBodyOrientation()
{
    if (bodyForward.x == 0.0f && bodyForward.z == 0.0f) {
        float s = (bodyForward.y >= 0.0f) ? -1.0f : 1.0f;
        bodyForward = s * bodyUp;
    }
    bodyUp = kVec3YAxis;
    bodyForward.y = 0.0f;
    StoreBodyAxes(1, 0);
    LoadBodyAxes();
    bodyRoll = 0.0f;
    bodyPitch = 0.0f;
    bodySinRoll = 0.0f;
    bodyCosRoll = 1.0f;
    savedForward = bodyForward;
    savedUp = bodyUp;
    bodySinPitch = 0.0f;
    bodyCosPitch = 1.0f;
    savedPitch = 0.0f;
    savedRoll = 0.0f;
    savedYaw = bodyYaw;
    savedSinRoll = 0.0f;
    savedCosRoll = 1.0f;
    savedCosPitch = 1.0f;
    savedSinPitch = 0.0f;
}

int SoultreePhysicsBaseObject::ClassifyOutOfBounds(int value)
{
    return 0;
}
