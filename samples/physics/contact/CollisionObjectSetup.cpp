// CollisionObjectSetup.cpp -- CollisionObject::Configure (0x004320f0, 40 bytes).
// Declared in collision/CollisionObject.h (src/krusty2) but not defined by that area.  The three
// int stores are tier 1 (target bytes: +0x80, +0x6c, +0x70); the call to 0x004692f0 is
// GameObject slot 8 invoked non-virtually on the GraphicsTest/GameObject subobject
// (this + 12), which stores its argument in GameObject::field_0x18.
#include "collision/CollisionObject.h"

void CollisionObject::Configure(int a, int b, int c, int d)
{
    useBroadphase = b;
    collisionEnabled = c;
    collidable = d;
    GameObject::GameObjectVirtualSlot8(a);
}
