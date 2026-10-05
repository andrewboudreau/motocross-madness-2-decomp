// Near-miss Track.cpp candidates, kept out of src/reconstructed until they
// match. See docs/TRACK.md.
//
// Track::UnknownFunction518130 (0x00518130, 250 bytes): the candidate is
// 246 bytes. Everything up to the FastInvSqrt call matches (the squared
// length needs the (y*y + x*x) + z*z grouping). Retail then keeps the scale
// on the x87 stack until a final `fstp st(0)` and multiplies y and z as
// `fld d.y; fmul st(1)`; VC6 here consumes the scale with the last multiply.
// A scaling helper (by reference, by value or through an out pointer), a
// Vec3-style constructor/operator* and writing straight into *out leave it
// unchanged.
#include "../../src/reconstructed/Track.h"
#include "../../src/krusty2/math/FastMath.h"

// 0x00518130: the unit direction of a segment.
int Track::UnknownFunction518130(TrackSegment* segment, TrackVec3* out)
{
    if (out && segment && segment->field_0x2c && segment->field_0x24 > 0.0f) {
        TrackSegment* next = segment->field_0x2c;
        TrackVec3 d;
        d.x = next->field_0x00 - segment->field_0x00;
        d.y = next->field_0x04 - segment->field_0x04;
        d.z = next->field_0x08 - segment->field_0x08;
        float lengthSquared = (d.y * d.y + d.x * d.x) + d.z * d.z;
        if (lengthSquared == 1.0f) {
            *out = d;
            return 1;
        }
        float scale = FastInvSqrt(lengthSquared);
        TrackVec3 r;
        r.x = d.x * scale;
        r.y = d.y * scale;
        r.z = d.z * scale;
        *out = r;
        return 1;
    }
    return 0;
}
