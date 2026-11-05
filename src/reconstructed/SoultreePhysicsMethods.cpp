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
int SoultreePhysicsBaseObject::UnknownVirtualSlot5(int value)
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

int SoultreePhysicsBaseObject::ClassifyOutOfBounds(int value)
{
    return 0;
}
