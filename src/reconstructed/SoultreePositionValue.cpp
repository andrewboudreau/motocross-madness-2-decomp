// Complete local-position setter for the existing by-value node call view.
// The twelve-byte value argument preserves the observed callers' copy boundary.
// Original overload spelling and TU ownership are unproven.
#include "../krusty2/math/Math3D.h"
#include "../krusty2/core/SoultreeObject.h"

// 0x004fc630, 38 bytes, thiscall ret12. Copy all three argument words before
// invalidating this node's world matrix and the dependent hierarchy caches.
void SoultreeObject::SetPositionVec3(Vec3 position)
{
    localMatrix._41 = position.x;
    localMatrix._42 = position.y;
    localMatrix._43 = position.z;
    InvalidateWorldMatrix();
}
