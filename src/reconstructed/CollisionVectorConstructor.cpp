// Complete constructor used by the collision TUs' existing 12-byte value type.
// The call-view name and this source grouping are provisional; a shared retail
// address does not establish identity with the other vector declarations.
#include "../krusty2/collision/CollisionTypes.h"

// 0x00404e60, 25 bytes: copy all three argument words, return this, ret 0xc.
CollisionVec3::CollisionVec3(float x_, float y_, float z_)
{
    x = x_;
    y = y_;
    z = z_;
}
