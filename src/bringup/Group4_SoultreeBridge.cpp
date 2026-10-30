// Bring-up definition (best-guess-unreviewed): the Vector3-typed view of
// SoultreeObject::LocalToWorldPoint declared in src/reconstructed/D3DIMSoulTree.h.
// Retail has one function, 0x004fd660 (p * worldMatrix, with translation),
// already provided by soultree.cpp under the Vec3 mangling; this forwards to it
// through src/bringup/Group4_SoultreeReal.cpp.
#include "../reconstructed/D3DIMSoulTree.h"

void Group4Veh_LocalToWorldPoint(void* node, const float* in, float* out);

Vector3 SoultreeObject::LocalToWorldPoint(const Vector3& p)
{
    Vector3 result;
    Group4Veh_LocalToWorldPoint(this, &p.x, &result.x);
    return result;
}
