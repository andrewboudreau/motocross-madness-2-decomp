// Group5 bring-up definition (best-guess, link frontier).
//
// Wrecker.h's UnknownWreckerRigidBody(int) (0x004cc120) is the retail
// PhysicsRigidBody constructor (RTTI .?AVPhysicsRigidBody@@, vtable
// 0x00556ec0, 45 slots). Wrecker.cpp news 0x2a8 bytes and then calls the
// body only through vtable slot numbers, so the object must carry the real
// PhysicsRigidBody vtable. The class and every slot it overrides are
// reconstructed in samples/physics/rigidbody (PhysicsBody.cpp covers
// 0x004cbc50..0x004cc05c, PhysicsRigidBody.cpp 0x004cc060..0x004ccbcc; both
// are byte-matching samples, not yet promoted). This unit compiles those two
// sample units unchanged and builds the PhysicsRigidBody in place from the
// Wrecker.h name. None of their symbols is defined elsewhere in src/.
#include <new.h>

#include "../../samples/physics/rigidbody/PhysicsBody.cpp"
#include "../../samples/physics/rigidbody/PhysicsRigidBody.cpp"

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
