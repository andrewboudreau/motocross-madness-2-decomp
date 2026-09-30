// ConstraintMethodCollisionModel -- PROVISIONAL (tier 2/3).
// Evidence: RTTI base CollisionObject; secondary vtable 0x005511f0 at object offset 12
// (COL 0x0055b098); primary vtable 0x00551260 written at +0 by 0x0043b980.
// Slot 2 of the primary vtable (0x0044d710, a bare `ret`) is introduced here.
#ifndef CONSTRAINT_METHOD_COLLISION_MODEL_H
#define CONSTRAINT_METHOD_COLLISION_MODEL_H

#include "CollisionObject.h"

class ConstraintMethodCollisionModel : public CollisionObject {
public:
    virtual void UnknownVirtualSlot2();   // 0x0044d710
};

#endif
