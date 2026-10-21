#include "../krusty2/collision/CollisionObject.h"

// Retail 0x464e90: complete one-byte return. SetRenderContext installs this
// callback in the primary CollisionObject's onHitByCallback (+0x8c); the three
// QueryCollisions dispatch sites pass (hitObject, this) and pop both arguments.
// The semantic name and original TU are provisional. Other users of this
// shared empty address do not establish method or class identity.
void CollisionEmptyCallback(CollisionObject*, CollisionObject*) {
}
