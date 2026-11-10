// Group5_WreckerRigidBody.cpp -- link shim between Wrecker.h's name for the rigid body
// constructor and the canonical PhysicsRigidBody (src/krusty2/rigidbody; bring-up glue).
//
// Wrecker.h's UnknownWreckerRigidBody(int) (0x004cc120) is the retail
// PhysicsRigidBody constructor (RTTI .?AVPhysicsRigidBody@@, vtable
// 0x00556ec0, 45 slots). Wrecker.cpp news 0x2a8 bytes and then calls the
// body only through vtable slot numbers, so the object must carry the real
// PhysicsRigidBody vtable. The class and every slot it overrides are the
// canonical src/krusty2/rigidbody/PhysicsBody.cpp (0x004cbc50..0x004cc05c) and
// PhysicsRigidBody.cpp (0x004cc060..0x004ccbcc), both strict exact; this shim
// only builds the PhysicsRigidBody in place under the Wrecker.h name.
#include <new.h>

#include "rigidbody/PhysicsBody.h"

// Wrecker.h's view, declared only as far as the constructor needs (the
// mangled name does not depend on its virtual slots or fields).
class UnknownWreckerRigidBody {
public:
    explicit UnknownWreckerRigidBody(int flags);  // 0x004cc120
};

typedef char group5wrecker_check_size[(sizeof(PhysicsRigidBody) == 0x2a8) ? 1 : -1];

UnknownWreckerRigidBody::UnknownWreckerRigidBody(int flags)
{
    new (this) PhysicsRigidBody(flags);
}
