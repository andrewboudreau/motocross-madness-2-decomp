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
//
// Track::UnknownFunction5179f0 (0x005179f0, 929 bytes): 929/929 bytes, two
// differ. When the walk reaches b's node, retail adds the two floats as
// `fld fromStart; fadd length`; VC6 here always loads `length` first, for
// `length += fromStart`, `length = fromStart + length`, a separate total, a
// const fromStart and with the declarations reordered. Everything else,
// including the stack slots, matches. All six DebugCalloc line numbers
// (1175, 1191, 1212, 1228, 1247, 1258) are confirmed by the pushes.
//
// Track::UnknownFunction516ca0 (0x00516ca0, 586 bytes): the candidate is 576
// bytes and the frame, stack slots and x87 shape agree. VC6 here reuses the
// projection's `p - segment` differences (CSE temps at esp+8/+0xc) in the
// t < 0 and t == 0 branches; retail recomputes them there with
// `fld p.x; fsub [segment]` but does reuse them in the final interpolating
// branch. Named px/pz locals, TrackVec3 temporaries, an inline Delta helper,
// a TrackVec3 cast in those branches and reusing dx/dz all keep the CSE
// (named locals add a second copy and grow the frame to 0x14).
//
// Track::UnknownFunction516ef0 (0x00516ef0, 1042 bytes): 1042/1042 bytes,
// 98.2%. Two of the six inlined edge tests evaluate the two factors of one
// product in the other order (edge 2: retail computes b.x - a.x before
// p.z - a.z; edge 6: p.x - a.x before b.z - a.z). The other four match.
// Factor and comparison order in the source do not move it. A macro over
// plain floats is much further off, because VC6 then loads the segment
// operands before p; the inline helper with a by-value TrackVec3 and
// pointers to the edge points fixes that.
#include "../../src/reconstructed/DebugAlloc.h"
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

// 0x005179f0: the shortest distance from a to b through the node graph,
// and optionally the node path. A depth-first walk: a node is marked on its
// first visit and unmarked when its list entry comes back to the top.
int Track::UnknownFunction5179f0(TrackPos a, TrackPos b, TrackListItem** path, float* distance)
{
    TrackListItem* list = 0;
    if (!distance)
        return 0;
    *distance = -1.0f;
    if (a.node == b.node && UnknownFunction5179a0(a, b)) {
        if (path) {
            *path = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1175);
            if (!*path)
                return 0;
            (*path)->field_0x04 = b.node;
        }
        *distance = UnknownFunction517da0(a, b);
        return 1;
    }
    TrackPos pos;
    pos.node = a.node;
    pos.segment = a.node->field_0x0c;
    pos.t = 1.0f;
    float toEnd = UnknownFunction517da0(a, pos);
    pos.node = b.node;
    pos.segment = b.node->field_0x08;
    pos.t = 0.0f;
    float fromStart = UnknownFunction517da0(pos, b);
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1191);
    if (!item) {
        UnknownFunction517930(&list, 0);
        UnknownFunction517930(path, 0);
        return 0;
    }
    item->field_0x04 = a.node;
    item->field_0x0c = list;
    list = item;
    float length = toEnd;
    while (list) {
        TrackNode* node = list->field_0x04;
        if (node->field_0x00 & 4) {
            length -= node->field_0x04;
            item = list;
            list = list->field_0x0c;
            item->field_0x04->field_0x00 &= ~4;
            operator delete(item, __FILE__, 1212);
        } else {
            if (node != a.node && node != b.node)
                length += node->field_0x04;
            node->field_0x00 |= 4;
            int count = list->field_0x04->field_0x10;
            TrackNode** links = list->field_0x04->field_0x14;
            for (int i = 0; i < count; i++) {
                if (!(links[i]->field_0x00 & 4) && links[i] != b.node) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1228);
                    if (!item) {
                        UnknownFunction517930(&list, 0);
                        UnknownFunction517930(path, 0);
                        return 0;
                    }
                    item->field_0x04 = links[i];
                    item->field_0x0c = list;
                    list = item;
                } else if (links[i] == b.node) {
                    length = fromStart + length;
                    if (*distance == -1.0f || length < *distance) {
                        *distance = length;
                        if (path) {
                            UnknownFunction517930(path, 0);
                            item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1247);
                            if (!item) {
                                UnknownFunction517930(&list, 0);
                                UnknownFunction517930(path, 0);
                                return 0;
                            }
                            item->field_0x04 = b.node;
                            item->field_0x0c = *path;
                            *path = item;
                            for (TrackListItem* s = list; s; s = s->field_0x0c) {
                                if (s->field_0x04->field_0x00 & 4) {
                                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1258);
                                    if (!item) {
                                        UnknownFunction517930(&list, 0);
                                        UnknownFunction517930(path, 0);
                                        return 0;
                                    }
                                    item->field_0x04 = s->field_0x04;
                                    item->field_0x0c = *path;
                                    *path = item;
                                }
                            }
                        }
                    }
                    length -= fromStart;
                }
            }
        }
    }
    return 1;
}

// 0x00516ca0: the closest position on `node` to p, in the horizontal
// plane, and optionally its distance.
int Track::UnknownFunction516ca0(TrackVec3 p, TrackNode* node, TrackPos* out, float* outDistance)
{
    float best = -1.0f;
    if (!node || !out)
        return 0;
    out->node = node;
    TrackSegment* segment = node->field_0x08;
    if (segment) {
        if (!segment->field_0x2c) {
            out->segment = segment;
            out->t = 0.0f;
            if (outDistance) {
                float dx = p.x - segment->field_0x00;
                float dz = p.z - segment->field_0x08;
                *outDistance = FastSqrt(dz * dz + dx * dx);
            }
            return 1;
        }
        for (; segment; segment = segment->field_0x2c) {
            TrackSegment* next = segment->field_0x2c;
            if (!next)
                break;
            float ex = next->field_0x00 - segment->field_0x00;
            float ez = next->field_0x08 - segment->field_0x08;
            float t;
            float dx;
            float dz;
            if (ex == 0.0f && ez == 0.0f) {
                t = 0.0f;
                dx = p.x - segment->field_0x00;
                dz = p.z - segment->field_0x08;
            } else {
                t = ((p.z - segment->field_0x08) * ez + (p.x - segment->field_0x00) * ex) / (ez * ez + ex * ex);
                if (t > 1.0f) {
                    t = 1.0f;
                    dx = p.x - next->field_0x00;
                    dz = p.z - next->field_0x08;
                } else if (t < 0.0f) {
                    t = 0.0f;
                    dx = p.x - segment->field_0x00;
                    dz = p.z - segment->field_0x08;
                } else if (t == 1.0f) {
                    dx = p.x - next->field_0x00;
                    dz = p.z - next->field_0x08;
                } else if (t == 0.0f) {
                    dx = p.x - segment->field_0x00;
                    dz = p.z - segment->field_0x08;
                } else {
                    dx = (p.x - segment->field_0x00) - t * ex;
                    dz = (p.z - segment->field_0x08) - t * ez;
                }
            }
            float distanceSquared = dz * dz + dx * dx;
            if (distanceSquared < best || best < 0.0f) {
                best = distanceSquared;
                out->segment = segment;
                out->t = t;
            }
        }
    }
    if (outDistance) {
        if (best > 0.0f)
            *outDistance = FastSqrt(best);
        else if (best == 0.0f)
            *outDistance = 0.0f;
    }
    return 1;
}

// One edge of the crossing-number test in 0x00516ef0: whether the edge a-b
// crosses the ray from p.
static inline int UnknownEdgeCrosses(TrackVec3 p, const TrackVec3* a, const TrackVec3* b)
{
    return (a->x <= p.x || b->x <= p.x)
        && (p.z >= a->z && p.z < b->z || p.z >= b->z && p.z < a->z)
        && ((b->x - a->x) * (p.z - a->z) <= (p.x - a->x) * (b->z - a->z)) == (p.z >= a->z);
}

// 0x00516ef0: whether p lies, in the horizontal plane, inside the strip
// between segment and next (edge points at +0x0c and +0x18, centre at
// +0x00), by the crossing-number rule over six edges.
int Track::UnknownFunction516ef0(TrackVec3 p, TrackSegment* segment, TrackSegment* next)
{
    int inside = 0;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->field_0x0c, (TrackVec3*)&next->field_0x0c))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->field_0x0c, (TrackVec3*)&segment->field_0x00))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->field_0x18, (TrackVec3*)&segment->field_0x00))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->field_0x18, (TrackVec3*)&next->field_0x18))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&next->field_0x0c, (TrackVec3*)&next->field_0x00))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&next->field_0x18, (TrackVec3*)&next->field_0x00))
        inside = 1 - inside;
    return inside;
}
