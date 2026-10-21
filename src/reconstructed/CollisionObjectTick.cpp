// CollisionObject's secondary GameObject table (0x00551148, object offset 12)
// identifies slot 10 at 0x00499ae0. The complete 13-byte body forwards to
// GameObject::Tick. The shared retail address does not establish original TU
// ownership or equate this method with other classes' forwarding methods.
#include "../krusty2/collision/CollisionObject.h"

int CollisionObject::Tick(float frameTime) {
    return GameObject::Tick(frameTime);
}
