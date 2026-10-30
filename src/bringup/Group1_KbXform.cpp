// Group1_KbXform.cpp -- bring-up body for KrustyBikeTypes.h's KbXform view.
//
// Retail 0x004fd710 is SoultreeObject::WorldToLocalDirection (soultree.cpp, already
// reconstructed): it updates the world matrix (0x004fb4f0), copies it, transposes the
// rotation and returns (row0.v, row1.v, row2.v).  KbXform is KrustyBike's view of the
// same scene node, so the view's method forwards to the canonical body.  The pointer
// argument and the reference parameter pass the same address.

#include "vehicle/KrustyBikeTypes.h"
#include "core/SoultreeObject.h"

// 0x004fd710 (thiscall, hidden return pointer, ret 8)
Vec3 KbXform::WorldToLocalDirection(const Vec3* v)
{
    return reinterpret_cast<SoultreeObject*>(this)->WorldToLocalDirection(*v);
}
