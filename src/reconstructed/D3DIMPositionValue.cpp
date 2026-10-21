// Complete local-position setter for the existing rendering-model call view.
// The primary SoultreeObject base has offset zero; the value argument is twelve bytes.
// Original overload spelling and TU ownership are unproven.
#include "D3DIMSoulTree.h"

// 0x004fc630, 38 bytes, thiscall ret12. Retain the by-value copy boundary
// used by LightEmitter and share the actual hierarchy-cache implementation.
void D3DIMSoultreeObject::SetPositionVec3(Vector3 position)
{
    localMatrix._41 = position.x;
    localMatrix._42 = position.y;
    localMatrix._43 = position.z;
    InvalidateWorldMatrix();
}
