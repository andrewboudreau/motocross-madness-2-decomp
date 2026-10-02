// CrossProduct.cpp -- kept out of VectorOps.cpp: that file uses inline_depth(0), which would turn
// the Vec3 default constructor into a call.
#include "../common/Math3D.h"

// 0x00515600. CrossProduct(const Vec3&, const Vec3&), cdecl, hidden result first.
// Retail's body is the out-of-line COMDAT of d3dvec.inl-style CrossProduct. The
// compound form (r.x = ...; r.x -= ...) is what reproduces its operand order under VC6;
// the single-expression form in Math3D.h gets the second product's operands swapped.
Vec3 CrossProductCall(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.y * b.z; r.x -= a.z * b.y;
    r.y = a.z * b.x; r.y -= a.x * b.z;
    r.z = a.x * b.y; r.z -= a.y * b.x;
    return r;
}
