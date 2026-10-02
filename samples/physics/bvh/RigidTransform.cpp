// RigidTransform.cpp -- the out-of-line rigid-transform inverse at 0x0042e2b0.
//
// Ownership is uncertain, so this stays a sample: the function sits after ReadPointNode
// (0x0042e190, BoundingBoxTreeBuild.cpp's last own __FILE__ xref at 0x0042e257) and before
// Camera.cpp's code (0x0042e340), but does not reference __FILE__ itself, and its only callers
// are in CollisionObject.cpp (0x004399be, 0x00439a47).  Its body is byte-identical to the
// InvertRigid inline that BuildModelBoxTree (0x0042dc90) expands, which makes it likely (tier 2)
// that the source defines it next to BoundingBoxTreeBuild.cpp's helpers; the name is ours.
#include "bvh/BoundingBoxTreeBuild.h"

// 0x0042e2b0.  In-place inverse of a rigid transform (see InvertRigid).
void InvertRigidTransform(TreeMatrix4* m)
{
    InvertRigid(m);
}
