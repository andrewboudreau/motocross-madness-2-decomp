#include <float.h>
#include <stdio.h>
#include <string.h>

#include "Track.h"

#include "DebugAlloc.h"
#include "GameUiHelpers.h"
#include "../krusty2/math/FastMath.h"

// Vector helpers for 0x00518230. They are file-local: declared in Track.h
// they shift operand order in other TUs that include it (docs/TRACK.md).
static inline TrackVec3 MakeTrackVec3(float x, float y, float z)
{
    TrackVec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static inline TrackVec3 operator-(const TrackVec3& a, const TrackVec3& b)
{
    return MakeTrackVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline TrackVec3 operator+(const TrackVec3& a, const TrackVec3& b)
{
    return MakeTrackVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline TrackVec3 operator*(const TrackVec3& v, float s)
{
    return MakeTrackVec3(v.x * s, v.y * s, v.z * s);
}

static inline TrackVec3& operator+=(TrackVec3& a, const TrackVec3& b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

// 0x00515dc0: an empty string leaves nothing to split.
UnknownTokenizer::UnknownTokenizer(char* text)
{
    if (strlen(text) != 0)
        field_0x04 = text;
    else
        field_0x04 = 0;
}

// 0x00515df0: returns the next token and skips the delimiters after it.
char* UnknownTokenizer::NextToken(const char* delimiters)
{
    if (!field_0x04)
        return 0;
    field_0x00 = field_0x04;
    field_0x04 = strpbrk(field_0x04, delimiters);
    if (field_0x04) {
        *field_0x04++ = 0;
        if (*field_0x04 == 0)
            field_0x04 = 0;
        else
            while (strchr(delimiters, *field_0x04))
                field_0x04++;
    }
    return field_0x00;
}

// 0x00515e70: frees a node array (the loader's failure path).
int Track::FreeNodeArray(UnknownStream* stream, TrackNode** nodes, int count)
{
    if (!stream || !nodes)
        return 0;
    for (int i = 0; i < count; i++) {
        if (nodes[i])
            FreeNode(nodes[i]);
    }
    DebugFree(nodes, __FILE__, 30);
    return 1;
}

// 0x00516800: frees a node, its segments and its link array.
int Track::FreeNode(TrackNode* node)
{
    if (!node)
        return 0;
    while (node->firstSegment) {
        TrackSegment* segment = node->firstSegment;
        node->firstSegment = segment->next;
        DebugFree(segment, __FILE__, 353);
    }
    if (node->links)
        DebugFree(node->links, __FILE__, 359);
    DebugFree(node, __FILE__, 361);
    return 1;
}

// 0x00516870: frees every node reachable from *start. A node is marked on
// its first visit and freed when its list entry comes back to the top.
int Track::FreeReachableNodes(TrackNode** start)
{
    TrackListItem* list = 0;
    if (!start || !*start)
        return 0;
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 383);
    if (!item)
        return 0;
    item->node = *start;
    item->field_0x0c = list;
    list = item;
    while (list) {
        TrackNode* node = list->node;
        if (node->flags & 4) {
            FreeNode(node);
            item = list;
            list = list->field_0x0c;
            DebugFree(item, __FILE__, 400);
        } else {
            node->flags |= 4;
            int count = list->node->linkCount;
            TrackNode** links = list->node->links;
            for (int i = 0; i < count; i++) {
                if (!(links[i]->flags & 4)) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 410);
                    if (!item) {
                        FreeWorkList(&list, 0);
                        return 0;
                    }
                    item->node = links[i];
                    item->field_0x0c = list;
                    list = item;
                }
            }
        }
    }
    return 1;
}

// 0x00516980: the closest position on the track to p, in the horizontal
// plane, over every node reachable from the start node (the depth-first walk
// of 0x00516870), and optionally its distance. As in 0x00516ca0 the
// parameter is clamped first and the difference chosen after the clamp.
int Track::ClosestPointOnTrack(Vector3 p, TrackPos* out, float* outDistance)
{
    float best = -1.0f;
    if (!out)
        return 0;
    if (!startNode)
        return 0;
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 514);
    if (!item)
        return 0;
    item->node = startNode;
    TrackListItem* list = item;
    while (list) {
        TrackNode* node = list->node;
        if (node->flags & 4) {
            node->flags &= ~4;
            item = list;
            list = list->field_0x0c;
            DebugFree(item, __FILE__, 530);
        } else {
            node->flags |= 4;
            TrackSegment* segment = list->node->firstSegment;
            if (segment) {
                if (!segment->next) {
                    float dx = p.x - segment->field_0x00;
                    float dz = p.z - segment->field_0x08;
                    float distanceSquared = dz * dz + dx * dx;
                    if (distanceSquared < best || best < 0.0f) {
                        best = distanceSquared;
                        out->node = list->node;
                        out->segment = segment;
                        out->t = 0.0f;
                    }
                }
                for (; segment; segment = segment->next) {
                    TrackSegment* next = segment->next;
                    if (!next)
                        break;
                    float ex = next->field_0x00 - segment->field_0x00;
                    float ez = next->field_0x08 - segment->field_0x08;
                    float t;
                    float dx;
                    float dz;
                    if (ex == 0.0f && ez == 0.0f)
                        t = 0.0f;
                    else {
                        t = ((p.z - segment->field_0x08) * ez + (p.x - segment->field_0x00) * ex) / (ez * ez + ex * ex);
                        if (t > 1.0f)
                            t = 1.0f;
                        else if (t < 0.0f)
                            t = 0.0f;
                    }
                    if (t == 1.0f) {
                        dx = p.x - next->field_0x00;
                        dz = p.z - next->field_0x08;
                    } else if (t == 0.0f) {
                        dx = p.x - segment->field_0x00;
                        dz = p.z - segment->field_0x08;
                    } else {
                        dx = (p.x - segment->field_0x00) - t * ex;
                        dz = (p.z - segment->field_0x08) - t * ez;
                    }
                    float distanceSquared = dz * dz + dx * dx;
                    if (distanceSquared < best || best < 0.0f) {
                        best = distanceSquared;
                        out->node = list->node;
                        out->segment = segment;
                        out->t = t;
                    }
                }
            }
            int count = list->node->linkCount;
            TrackNode** links = list->node->links;
            for (int i = 0; i < count; i++) {
                if (!(links[i]->flags & 4)) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 588);
                    if (!item)
                        return 0;
                    item->node = links[i];
                    item->field_0x0c = list;
                    list = item;
                }
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

// 0x00516ca0: the closest position on `node` to p, in the horizontal
// plane, and optionally its distance. The segment parameter t is clamped
// to [0, 1] first and the difference picked afterwards (t == 1: the next
// point, t == 0: the segment start, otherwise the interpolation). VC6 then
// recomputes `p - segment` in the t == 0 copies as retail does and reuses
// the projection's differences only in the interpolation.
int Track::ClosestPointOnNode(TrackVec3 p, TrackNode* node, TrackPos* out, float* outDistance)
{
    float best = -1.0f;
    if (!node || !out)
        return 0;
    out->node = node;
    TrackSegment* segment = node->firstSegment;
    if (segment) {
        if (!segment->next) {
            out->segment = segment;
            out->t = 0.0f;
            if (outDistance) {
                float dx = p.x - segment->field_0x00;
                float dz = p.z - segment->field_0x08;
                *outDistance = FastSqrt(dz * dz + dx * dx);
            }
            return 1;
        }
        for (; segment; segment = segment->next) {
            TrackSegment* next = segment->next;
            if (!next)
                break;
            float ex = next->field_0x00 - segment->field_0x00;
            float ez = next->field_0x08 - segment->field_0x08;
            float t;
            float dx;
            float dz;
            if (ex == 0.0f && ez == 0.0f)
                t = 0.0f;
            else {
                t = ((p.z - segment->field_0x08) * ez + (p.x - segment->field_0x00) * ex) / (ez * ez + ex * ex);
                if (t > 1.0f)
                    t = 1.0f;
                else if (t < 0.0f)
                    t = 0.0f;
            }
            if (t == 1.0f) {
                dx = p.x - next->field_0x00;
                dz = p.z - next->field_0x08;
            } else if (t == 0.0f) {
                dx = p.x - segment->field_0x00;
                dz = p.z - segment->field_0x08;
            } else {
                dx = (p.x - segment->field_0x00) - t * ex;
                dz = (p.z - segment->field_0x08) - t * ez;
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

// 0x00517310: whether `segment` belongs to `node`.
int Track::NodeHasSegment(TrackSegment* segment, TrackNode* node)
{
    for (TrackSegment* s = node->firstSegment; s; s = s->next) {
        if (s == segment)
            return 1;
    }
    return 0;
}

// 0x00517930: frees a work list; its nodes too unless they came from the
// file (bit 0), or all of them when `all` is set.
int Track::FreeWorkList(TrackListItem** list, int all)
{
    if (!list)
        return 0;
    TrackListItem* item;
    while ((item = *list) != 0) {
        if (item->node && (!(item->node->flags & 1) || all))
            FreeNode(item->node);
        *list = (*list)->field_0x0c;
        DebugFree(item, __FILE__, 1021);
    }
    return 1;
}

// 0x005179a0: whether b is at or after a on the same node.
int Track::IsAtOrAfter(TrackPos a, TrackPos b)
{
    if (a.node == b.node) {
        if (a.segment == b.segment) {
            if (a.t <= b.t)
                return 1;
        } else {
            for (TrackSegment* s = a.segment; s; s = s->next) {
                if (s == b.segment)
                    return 1;
            }
        }
    }
    return 0;
}

// 0x005179f0: the shortest distance from a to b through the node graph,
// and optionally the node path. A depth-first walk: a node is marked on its
// first visit and unmarked when its list entry comes back to the top.
// `length` is initialised at the top although the value is dead: that first
// reference makes `fromStart` the newer leaf, so the arrival sum loads it
// first (`fld fromStart; fadd length`) as retail does.
int Track::ShortestPath(TrackPos a, TrackPos b, TrackListItem** path, float* distance)
{
    TrackListItem* list = 0;
    float length = 0.0f;
    if (!distance)
        return 0;
    *distance = -1.0f;
    if (a.node == b.node && IsAtOrAfter(a, b)) {
        if (path) {
            *path = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1175);
            if (!*path)
                return 0;
            (*path)->node = b.node;
        }
        *distance = DistanceAlong(a, b);
        return 1;
    }
    TrackPos pos;
    pos.node = a.node;
    pos.segment = a.node->lastSegment;
    pos.t = 1.0f;
    float toEnd = DistanceAlong(a, pos);
    pos.node = b.node;
    pos.segment = b.node->firstSegment;
    pos.t = 0.0f;
    float fromStart = DistanceAlong(pos, b);
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1191);
    if (!item) {
        FreeWorkList(&list, 0);
        FreeWorkList(path, 0);
        return 0;
    }
    item->node = a.node;
    item->field_0x0c = list;
    list = item;
    length = toEnd;
    while (list) {
        TrackNode* node = list->node;
        if (node->flags & 4) {
            length -= node->pathLength;
            item = list;
            list = list->field_0x0c;
            item->node->flags &= ~4;
            DebugFree(item, __FILE__, 1212);
        } else {
            if (node != a.node && node != b.node)
                length += node->pathLength;
            node->flags |= 4;
            int count = list->node->linkCount;
            TrackNode** links = list->node->links;
            for (int i = 0; i < count; i++) {
                if (!(links[i]->flags & 4) && links[i] != b.node) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1228);
                    if (!item) {
                        FreeWorkList(&list, 0);
                        FreeWorkList(path, 0);
                        return 0;
                    }
                    item->node = links[i];
                    item->field_0x0c = list;
                    list = item;
                } else if (links[i] == b.node) {
                    length = fromStart + length;
                    if (*distance == -1.0f || length < *distance) {
                        *distance = length;
                        if (path) {
                            FreeWorkList(path, 0);
                            item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1247);
                            if (!item) {
                                FreeWorkList(&list, 0);
                                FreeWorkList(path, 0);
                                return 0;
                            }
                            item->node = b.node;
                            item->field_0x0c = *path;
                            *path = item;
                            for (TrackListItem* s = list; s; s = s->field_0x0c) {
                                if (s->node->flags & 4) {
                                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1258);
                                    if (!item) {
                                        FreeWorkList(&list, 0);
                                        FreeWorkList(path, 0);
                                        return 0;
                                    }
                                    item->node = s->node;
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

// 0x00517da0: distance along the track from a to b.
float Track::DistanceAlong(TrackPos a, TrackPos b)
{
    float distance = 0.0f;
    if (a.node == b.node) {
        if (a.segment == b.segment) {
            if (a.t <= b.t)
                return (b.t - a.t) * a.segment->horizontalLength;
        } else {
            TrackSegment* s;
            for (s = a.segment; s; s = s->next) {
                if (s == b.segment)
                    break;
                distance += s->horizontalLength;
            }
            if (s == 0)
                goto search;
            return b.t * b.segment->horizontalLength - a.t * a.segment->horizontalLength + distance;
        }
    search:
        ShortestPath(a, b, 0, &distance);
        return distance;
    }
    ShortestPath(a, b, 0, &distance);
    return distance;
}

// 0x00517ea0: moves pos `distance` along a node path (*path, linked through
// +0x08/+0x0c). Bit 1 of `flags`, combined with each entry's bit 1, picks
// the direction.
int Track::UnknownFunction517ea0(TrackPos pos, TrackPos* out, TrackListItem** path, float distance, unsigned char flags)
{
    TrackListItem* item;
    if (!path || (item = *path) == 0 || pos.node != item->node)
        return 0;
    TrackPos cur = pos;
    float travelled = 0.0f;
    while (travelled < distance) {
        if ((item->flags & 2) && !(flags & 2) || !(item->flags & 2) && (flags & 2)) {
            if (cur.t == 0.0f) {
                if (!cur.segment->previous) {
                    if (flags & 2)
                        item = item->field_0x08;
                    else
                        item = item->field_0x0c;
                    if (!item)
                        break;
                    cur.node = item->node;
                    if ((item->flags & 2) && !(flags & 2) || !(item->flags & 2) && (flags & 2))
                        cur.segment = cur.node->lastSegment;
                    else
                        cur.segment = cur.node->firstSegment;
                } else {
                    cur.segment = cur.segment->previous;
                    travelled += cur.segment->horizontalLength;
                }
            } else {
                travelled += cur.t * cur.segment->horizontalLength;
                cur.t = 0.0f;
            }
            if (travelled > distance && cur.segment->horizontalLength != 0.0f)
                cur.t = (travelled - distance) / cur.segment->horizontalLength;
        } else {
            if (cur.t == 1.0f) {
                if (!cur.segment->next) {
                    if (flags & 2)
                        item = item->field_0x08;
                    else
                        item = item->field_0x0c;
                    if (!item)
                        break;
                    cur.node = item->node;
                    if ((item->flags & 2) && !(flags & 2) || !(item->flags & 2) && (flags & 2))
                        cur.segment = cur.node->lastSegment;
                    else
                        cur.segment = cur.node->firstSegment;
                } else {
                    cur.segment = cur.segment->next;
                    travelled += cur.segment->horizontalLength;
                }
            } else {
                travelled += (1.0f - cur.t) * cur.segment->horizontalLength;
                cur.t = 1.0f;
            }
            if (travelled > distance && cur.segment->horizontalLength != 0.0f)
                cur.t = 1.0f - (travelled - distance) / cur.segment->horizontalLength;
        }
    }
    *out = cur;
    return 1;
}

// 0x00518080: the point at pos.
int Track::PointAt(TrackPos pos, TrackVec3* out)
{
    TrackSegment* segment = pos.segment;
    if (!segment)
        return 0;
    TrackSegment* next = segment->next;
    if (!next) {
        if (pos.t == 0.0f) {
            out->x = segment->field_0x00;
            out->y = segment->field_0x04;
            out->z = segment->field_0x08;
        }
        return 0;
    }
    if (pos.t < 0.0f)
        pos.t = 0.0f;
    else if (pos.t > 1.0f)
        pos.t = 1.0f;
    TrackVec3 d;
    d.x = next->field_0x00 - segment->field_0x00;
    d.y = next->field_0x04 - segment->field_0x04;
    d.z = next->field_0x08 - segment->field_0x08;
    out->x = d.x * pos.t + segment->field_0x00;
    out->y = d.y * pos.t + segment->field_0x04;
    out->z = d.z * pos.t + segment->field_0x08;
    return 1;
}

// TrackVec3 built by a constructor. The constructor's by-value parameters
// give the components fresh leaves (docs/VC6_OPERAND_ORDER.md), which
// 0x00518130 needs; MakeTrackVec3's copy through a local does not. Kept
// file-local like the operators above: a constructor in Track.h adds symbols
// to every unit that includes it.
struct TrackVector : TrackVec3 {
    TrackVector(float x_, float y_, float z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }
};

// The unit vector along v (v itself when it already has unit length). The
// squared length needs the (y*y + x*x) + z*z grouping.
static inline TrackVec3 TrackNormalized(const TrackVec3& v)
{
    float lengthSquared = (v.y * v.y + v.x * v.x) + v.z * v.z;
    if (lengthSquared == 1.0f)
        return v;
    float scale = FastInvSqrt(lengthSquared);
    return v * scale;
}

// 0x00518130: the unit direction of a segment. Built through TrackVector's
// constructor, the difference gives retail's scaling: x scalar-first
// (`fld st(0); fmul x`), y and z component-first (`fld y; fmul st(1)`), the
// scale kept on the x87 stack until a final `fstp st(0)`. Per-component
// stores into a named local scale all three scalar-first.
int Track::SegmentDirection(TrackSegment* segment, TrackVec3* out)
{
    if (out && segment && segment->next && segment->horizontalLength > 0.0f) {
        TrackSegment* next = segment->next;
        TrackVec3 d = TrackVector(next->field_0x00 - segment->field_0x00, next->field_0x04 - segment->field_0x04,
                                  next->field_0x08 - segment->field_0x08);
        *out = TrackNormalized(d);
        return 1;
    }
    return 0;
}

// 0x00518230: the horizontal distance from `p` to the strip edge of
// `segment` chosen by `mode` (10: the +0x0c edge, 11: the +0x18 edge, 17:
// the nearer of both). The edge is cut with the perpendicular through p in
// the x/z plane (the y/z plane for an edge with no x/z extent) and the cut is
// clamped to the edge's ends. BikeAI.cpp calls it with modes 10 and 11
// (0x00415326, 0x0041535c).
int Track::UnknownFunction518230(TrackVec3 p, TrackSegment* segment, float* out, int mode)
{
    TrackVec3 a;
    TrackVec3 d;
    TrackVec3 a2;
    TrackVec3 d2;
    TrackVec3 n;
    TrackVec3 q;
    float t;
    float u;    // the second parameter of the cut, then the second distance
    if (!segment || !segment->next || !out)
        return 0;
    TrackSegment* next = segment->next;
    switch (mode) {
    case 17:
        a = *(TrackVec3*)&segment->field_0x0c;
        d = *(TrackVec3*)&next->field_0x0c - *(TrackVec3*)&segment->field_0x0c;
        a2 = *(TrackVec3*)&segment->field_0x18;
        d2 = *(TrackVec3*)&next->field_0x18 - *(TrackVec3*)&segment->field_0x18;
        break;
    case 11:
        a = *(TrackVec3*)&segment->field_0x18;
        d = *(TrackVec3*)&next->field_0x18 - *(TrackVec3*)&segment->field_0x18;
        break;
    case 10:
        a = *(TrackVec3*)&segment->field_0x0c;
        d = *(TrackVec3*)&next->field_0x0c - *(TrackVec3*)&segment->field_0x0c;
        break;
    }
    if (d.x == 0.0f && d.z == 0.0f) {
        if (d.y == 0.0f)
            return 0;
        n.x = d.z;
        n.z = -d.y;
        IntersectLines(a.y, a.z, d.y, d.z, p.y, p.z, n.x, n.z, &t, &u);
    } else {
        n.x = d.z;
        n.z = -d.x;
        IntersectLines(a.x, a.z, d.x, d.z, p.x, p.z, n.x, n.z, &t, &u);
    }
    if (t < 0.0f)
        t = 0.0f;
    else if (t > 1.0f)
        t = 1.0f;
    d = d * t;
    a += d;
    a.x -= p.x;
    a.y -= p.y;
    a.z -= p.z;
    *out = a.x * a.x + a.z * a.z;
    if (mode == 17) {
        if (d2.x == 0.0f && d2.z == 0.0f) {
            if (d2.y == 0.0f)
                return 0;
            n.x = d2.z;
            n.z = -d2.y;
            IntersectLines(a2.y, a2.z, d2.y, d2.z, p.y, p.z, n.x, n.z, &t, &u);
        } else {
            n.x = d2.z;
            n.z = -d2.x;
            IntersectLines(a2.x, a2.z, d2.x, d2.z, p.x, p.z, n.x, n.z, &t, &u);
        }
        if (t < 0.0f)
            t = 0.0f;
        else if (t > 1.0f)
            t = 1.0f;
        q = d2 * t + a2;
        float dx = q.x - p.x;
        float dz = q.z - p.z;
        u = dx * dx;
        u += dz * dz;
        if (u < *out)
            *out = u;
    }
    *out = FastSqrt(*out);
    return 1;
}

void FormatMinutesSeconds(char* text, float seconds)
{
    int minutes = 0;
    while (seconds >= 60.0f) {
        seconds -= 60.0f;
        minutes++;
    }
    sprintf(text, "%2.2d:%2.2d", minutes, (int)seconds);
}

void FormatLapTime(char* text, float seconds)
{
    if (seconds == 0.0f || seconds == FLT_MAX) {
        strcpy(text, "--:--.--");
        return;
    }
    int minutes = 0;
    while (seconds >= 60.0f) {
        seconds -= 60.0f;
        minutes++;
    }
    sprintf(text, "%2d:%05.2f", minutes, seconds);
}
