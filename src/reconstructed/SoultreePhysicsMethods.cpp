// Exact primary-vtable methods of SoultreePhysicsBaseObject.
// Vtable 0x00557d90 has object offset 0; slots are verified independently.
// This is a reconstruction grouping, not a claim about original TU ownership.
// Names and semantic field roles remain provisional.
#include <math.h>
#include <string.h>
#include "soultree/SoultreePhysicsBaseObject.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "soultree/SoultreePhysicsContact.h"

int SoultreePhysicsBaseObject::UnknownVirtualSlot10()
{
    if (totalWeight == baseWeight)
        return 1;
    return 0;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot22()
{
    return staggerCounter == staggerPhase;
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot19(SoultreeAttachment* attachment)
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

int SoultreePhysicsBaseObject::UnknownVirtualSlot23()
{
    return !airborne && linearSpeed >= 1.0f;
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot24()
{
    return 0;
}

float SoultreePhysicsBaseObject::UnknownVirtualSlot32()
{
    if (UnknownVirtualSlot10())
        return 0.0f;
    return loadWeight;
}

void SoultreePhysicsBaseObject::UnknownVirtualSlot34()
{
    sceneNode->GetAxesIn(0, &bodyForward, &bodyUp);
}

// slot 35 (0x004aa1e0)
void SoultreePhysicsBaseObject::UnknownVirtualSlot35(int orthogonalize, int keepZ)
{
    sceneNode->SetAxesIn(0, &bodyForward, &bodyUp, orthogonalize, keepZ);
}

int SoultreePhysicsBaseObject::UnknownVirtualSlot12(int value)
{
    return 0;
}
