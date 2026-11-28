#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Track.h"

#include "DebugAlloc.h"
#include "BikeRaceNode.h"
#include "GameUiHelpers.h"
#include "TextureMap.h"
#include "../krusty2/math/FastMath.h"

// Vector helpers for 0x00518230. They are file-local: declared in Track.h
// they shift operand order in other TUs that include it (docs/TRACK.md).
static inline Vec3 MakeVec3(float x, float y, float z)
{
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static inline Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return MakeVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return MakeVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vec3 operator*(const Vec3& v, float s)
{
    return MakeVec3(v.x * s, v.y * s, v.z * s);
}

// 0x00515dc0: an empty string leaves nothing to split.
Tokenizer::Tokenizer(char* text)
{
    if (strlen(text) != 0)
        rest = text;
    else
        rest = 0;
}

// 0x00515df0: returns the next token and skips the delimiters after it.
char* Tokenizer::NextToken(const char* delimiters)
{
    if (!rest)
        return 0;
    token = rest;
    rest = strpbrk(rest, delimiters);
    if (rest) {
        *rest++ = 0;
        if (*rest == 0)
            rest = 0;
        else
            while (strchr(delimiters, *rest))
                rest++;
    }
    return token;
}

// 0x00515e70: frees a node array (the loader's failure path).
int Track::FreeNodeArray(FileStream* stream, TrackNode** nodes, int count)
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

// 0x00515ed0: the track loader. A chunk stream: one id byte, then the
// chunk's data, until the stream ends (the inline end test 0x00430ff0):
// 1 node count (allocates the node table), 2/3 x/z offset, 4 scale, 5 node
// index (allocates the node), 6 node flags (bit 0 set), 7 node length,
// 8 new segment, 9/10/11 the last segment's centre / left / right point
// (scaled and offset; 9 also sets the previous segment's horizontal length),
// 12 skipped, 13 link count (allocates the links), 14 one link (slot and
// node index, range-checked against the count inclusively as retail does),
// 15/16 the start / finish probe. Afterwards the link indices become node
// pointers, the start node is kept, the probes are placed on the track and
// the lap length is measured from just after the start round to it.
// Retail repeats the cleanup at every failure (VC6 cross-jumps the copies
// into three blocks by the register holding the count), reuses `index` and
// `slot` as the link pass's counters (both stay in their frame slots) and
// scales the probe extents through a vector pointer (one base register).
// A shared `goto fail`, block-scoped counters or per-field scaling move the
// frame and registers off retail's.
// The start / finish probes the loader fills are BikeRace's startProbe and
// finishProbe, RaceGates (bikerace.cpp's 0x3c-byte gate, BikeRaceNode.h).
static inline void ScaleVec3(Vector3* v, float scale)
{
    v->x *= scale;
    v->y *= scale;
    v->z *= scale;
}

int Track::LoadTrack(FileStream* stream, void* start, void* finish, int* hasStart,
                     int* hasFinish)
{
    int count = 0;
    float offsetX = 0.0f;
    float offsetZ = 0.0f;
    float scale = 1.0f;
    TrackNode** nodes;
    int index;
    TrackSegment** link;
    int linkCount;
    int slot;
    int target;
    if (!stream)
        return 0;
    char chunk;
    stream->Read(&chunk, 1, 1);
    index = 0;
    while (!stream->AtEnd()) {
        if (chunk == 1) {
            stream->Read(&count, 4, 1);
            if (count <= 0)
                return 0;
            nodes = (TrackNode**)DebugCalloc(count, 4, __FILE__, 81);
            if (!nodes)
                return 0;
        } else if (chunk == 2) {
            stream->Read(&offsetX, 4, 1);
        } else if (chunk == 3) {
            stream->Read(&offsetZ, 4, 1);
        } else if (chunk == 4) {
            stream->Read(&scale, 4, 1);
        } else if (chunk == 5) {
            stream->Read(&index, 4, 1);
            if (index < 0 || index >= count) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            nodes[index] = (TrackNode*)DebugCalloc(1, sizeof(TrackNode), __FILE__, 108);
            if (!nodes[index]) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            link = &nodes[index]->firstSegment;
        } else if (chunk == 6) {
            if (!nodes[index]) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            int flags;
            stream->Read(&flags, 4, 1);
            nodes[index]->flags = (unsigned char)(flags | 1);
        } else if (chunk == 7) {
            if (!nodes[index]) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->pathLength, 4, 1);
            nodes[index]->pathLength *= scale;
        } else if (chunk == 8) {
            if (!nodes[index] || !link) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            TrackSegment* segment = (TrackSegment*)DebugCalloc(1, sizeof(TrackSegment), __FILE__, 141);
            if (!segment) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            segment->previous = nodes[index]->lastSegment;
            *link = segment;
            link = &segment->next;
            nodes[index]->lastSegment = segment;
        } else if (chunk == 9) {
            if (!nodes[index] || !nodes[index]->lastSegment) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->lastSegment->point.x, 4, 1);
            stream->Read(&nodes[index]->lastSegment->point.y, 4, 1);
            stream->Read(&nodes[index]->lastSegment->point.z, 4, 1);
            nodes[index]->lastSegment->point.x = scale * nodes[index]->lastSegment->point.x + offsetX;
            nodes[index]->lastSegment->point.y = scale * nodes[index]->lastSegment->point.y;
            nodes[index]->lastSegment->point.z = scale * nodes[index]->lastSegment->point.z + offsetZ;
            TrackSegment* segment = nodes[index]->lastSegment;
            TrackSegment* previous = segment->previous;
            if (previous) {
                float dx = segment->point.x - previous->point.x;
                float dz = segment->point.z - previous->point.z;
                previous->horizontalLength = (float)sqrt(dz * dz + dx * dx);
            }
        } else if (chunk == 10) {
            if (!nodes[index] || !nodes[index]->lastSegment) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->lastSegment->edgeA.x, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeA.y, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeA.z, 4, 1);
            nodes[index]->lastSegment->edgeA.x = scale * nodes[index]->lastSegment->edgeA.x + offsetX;
            nodes[index]->lastSegment->edgeA.y = scale * nodes[index]->lastSegment->edgeA.y;
            nodes[index]->lastSegment->edgeA.z = scale * nodes[index]->lastSegment->edgeA.z + offsetZ;
        } else if (chunk == 11) {
            if (!nodes[index] || !nodes[index]->lastSegment) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->lastSegment->edgeB.x, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeB.y, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeB.z, 4, 1);
            nodes[index]->lastSegment->edgeB.x = scale * nodes[index]->lastSegment->edgeB.x + offsetX;
            nodes[index]->lastSegment->edgeB.y = scale * nodes[index]->lastSegment->edgeB.y;
            nodes[index]->lastSegment->edgeB.z = scale * nodes[index]->lastSegment->edgeB.z + offsetZ;
        } else if (chunk == 12) {
            int unused;
            stream->Read(&unused, 4, 1);
        } else if (chunk == 13) {
            if (!nodes[index]) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            stream->Read(&linkCount, 4, 1);
            if (linkCount < 0) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            nodes[index]->linkCount = linkCount;
            nodes[index]->links = (TrackNode**)DebugCalloc(linkCount, 4, __FILE__, 231);
            if (!nodes[index]->links) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
        } else if (chunk == 14) {
            if (!nodes[index] || !nodes[index]->links) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            stream->Read(&slot, 4, 1);
            stream->Read(&target, 4, 1);
            if (slot < 0 || slot > linkCount || target < 0 || target > count) {
                FreeNodeArray(stream, nodes, count);
                return 0;
            }
            nodes[index]->links[slot] = (TrackNode*)target;
        } else if (chunk == 15) {
            *hasStart = 1;
            RaceGate* probe = (RaceGate*)start;
            stream->Read(&probe->center.x, 4, 1);
            stream->Read(&probe->center.y, 4, 1);
            stream->Read(&probe->center.z, 4, 1);
            stream->Read(&probe->direction.x, 4, 1);
            stream->Read(&probe->direction.y, 4, 1);
            stream->Read(&probe->direction.z, 4, 1);
            stream->Read(&probe->halfExtents.x, 4, 1);
            stream->Read(&probe->halfExtents.y, 4, 1);
            stream->Read(&probe->halfExtents.z, 4, 1);
            stream->Read(&probe->width, 4, 1);
            ScaleVec3(&probe->halfExtents, scale);
            probe->width *= scale;
            probe->center.x = probe->center.x * scale + offsetX;
            probe->center.y *= scale;
            probe->center.z = probe->center.z * scale + offsetZ;
        } else if (chunk == 16) {
            *hasFinish = 1;
            RaceGate* probe = (RaceGate*)finish;
            stream->Read(&probe->center.x, 4, 1);
            stream->Read(&probe->center.y, 4, 1);
            stream->Read(&probe->center.z, 4, 1);
            stream->Read(&probe->direction.x, 4, 1);
            stream->Read(&probe->direction.y, 4, 1);
            stream->Read(&probe->direction.z, 4, 1);
            stream->Read(&probe->halfExtents.x, 4, 1);
            stream->Read(&probe->halfExtents.y, 4, 1);
            stream->Read(&probe->halfExtents.z, 4, 1);
            stream->Read(&probe->width, 4, 1);
            ScaleVec3(&probe->halfExtents, scale);
            probe->width *= scale;
            probe->center.x = probe->center.x * scale + offsetX;
            probe->center.y *= scale;
            probe->center.z = probe->center.z * scale + offsetZ;
        }
        stream->Read(&chunk, 1, 1);
    }
    for (index = 0; index < count; index++) {
        if (nodes[index] && nodes[index]->links) {
            for (slot = 0; slot < nodes[index]->linkCount; slot++) {
                int k = (int)nodes[index]->links[slot];
                if (k >= 0 && k < count)
                    nodes[index]->links[slot] = nodes[k];
            }
        }
    }
    startNode = nodes[0];
    DebugFree(nodes, __FILE__, 315);
    if (*hasStart) {
        RaceGate* probe = (RaceGate*)start;
        probe->height = 150.0f;
        ClosestPointOnTrack(probe->center, &probe->trackPos, 0);
    }
    if (*hasFinish) {
        RaceGate* probe = (RaceGate*)finish;
        probe->height = 150.0f;
        ClosestPointOnTrack(probe->center, &probe->trackPos, 0);
    }
    {
        TrackPos from;
        from.node = startNode;
        from.segment = startNode->firstSegment;
        from.t = 0.0001f;
        TrackPos to;
        to.node = startNode;
        to.segment = startNode->firstSegment;
        to.t = 0.0f;
        ShortestPath(from, to, 0, &lapLength);
    }
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
    item->next = list;
    list = item;
    while (list) {
        TrackNode* node = list->node;
        if (node->flags & 4) {
            FreeNode(node);
            item = list;
            list = list->next;
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
                    item->next = list;
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
            list = list->next;
            DebugFree(item, __FILE__, 530);
        } else {
            node->flags |= 4;
            TrackSegment* segment = list->node->firstSegment;
            if (segment) {
                if (!segment->next) {
                    float dx = p.x - segment->point.x;
                    float dz = p.z - segment->point.z;
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
                    float ex = next->point.x - segment->point.x;
                    float ez = next->point.z - segment->point.z;
                    float t;
                    float dx;
                    float dz;
                    if (ex == 0.0f && ez == 0.0f)
                        t = 0.0f;
                    else {
                        t = ((p.z - segment->point.z) * ez + (p.x - segment->point.x) * ex) / (ez * ez + ex * ex);
                        if (t > 1.0f)
                            t = 1.0f;
                        else if (t < 0.0f)
                            t = 0.0f;
                    }
                    if (t == 1.0f) {
                        dx = p.x - next->point.x;
                        dz = p.z - next->point.z;
                    } else if (t == 0.0f) {
                        dx = p.x - segment->point.x;
                        dz = p.z - segment->point.z;
                    } else {
                        dx = (p.x - segment->point.x) - t * ex;
                        dz = (p.z - segment->point.z) - t * ez;
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
                    item->next = list;
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
int Track::ClosestPointOnNode(Vec3 p, TrackNode* node, TrackPos* out, float* outDistance)
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
                float dx = p.x - segment->point.x;
                float dz = p.z - segment->point.z;
                *outDistance = FastSqrt(dz * dz + dx * dx);
            }
            return 1;
        }
        for (; segment; segment = segment->next) {
            TrackSegment* next = segment->next;
            if (!next)
                break;
            float ex = next->point.x - segment->point.x;
            float ez = next->point.z - segment->point.z;
            float t;
            float dx;
            float dz;
            if (ex == 0.0f && ez == 0.0f)
                t = 0.0f;
            else {
                t = ((p.z - segment->point.z) * ez + (p.x - segment->point.x) * ex) / (ez * ez + ex * ex);
                if (t > 1.0f)
                    t = 1.0f;
                else if (t < 0.0f)
                    t = 0.0f;
            }
            if (t == 1.0f) {
                dx = p.x - next->point.x;
                dz = p.z - next->point.z;
            } else if (t == 0.0f) {
                dx = p.x - segment->point.x;
                dz = p.z - segment->point.z;
            } else {
                dx = (p.x - segment->point.x) - t * ex;
                dz = (p.z - segment->point.z) - t * ez;
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
// crosses the ray from p. A macro over the edge pointers: written as an
// inline function, VC6's parameter copies make the factor order of the
// second product a tie-break that no spelling of the six expansions got
// right (1032/1042 at best).
#define TRACK_EDGE_CROSSES(a, b) \
    (((a)->x <= p.x || (b)->x <= p.x) \
     && (p.z >= (a)->z && p.z < (b)->z || p.z >= (b)->z && p.z < (a)->z) \
     && ((((b)->x - (a)->x) * (p.z - (a)->z) <= (p.x - (a)->x) * ((b)->z - (a)->z)) == (p.z >= (a)->z)))

// 0x00516ef0: whether p lies, in the horizontal plane, inside the strip
// between segment and next (edge points at +0x0c and +0x18, centre at
// +0x00), by the crossing-number rule over six edges. The two centres go
// through pointer locals: retail computes b.z - a.z first in the second
// product of edge 3 (edge B to the centre), which VC6 does only when the
// centre is a local pointer (MoveBox 0x00428db0 uses the same lever).
int Track::IsInsideStrip(Vec3 p, TrackSegment* segment, TrackSegment* next)
{
    int inside = 0;
    const Vec3* centre = &segment->point;
    const Vec3* nextCentre = &next->point;
    if (TRACK_EDGE_CROSSES(&segment->edgeA, &next->edgeA))
        inside = 1 - inside;
    if (TRACK_EDGE_CROSSES(&segment->edgeA, centre))
        inside = 1 - inside;
    if (TRACK_EDGE_CROSSES(&segment->edgeB, centre))
        inside = 1 - inside;
    if (TRACK_EDGE_CROSSES(&segment->edgeB, &next->edgeB))
        inside = 1 - inside;
    if (TRACK_EDGE_CROSSES(&next->edgeA, nextCentre))
        inside = 1 - inside;
    if (TRACK_EDGE_CROSSES(&next->edgeB, nextCentre))
        inside = 1 - inside;
    return inside;
}

#undef TRACK_EDGE_CROSSES

// 0x00517310: whether `segment` belongs to `node`.
int Track::NodeHasSegment(TrackSegment* segment, TrackNode* node)
{
    for (TrackSegment* s = node->firstSegment; s; s = s->next) {
        if (s == segment)
            return 1;
    }
    return 0;
}

// 0x00517340: places a position near p, `distance` along the track from
// `from` and within `range`, on the strip or on the work list's path.
// Not exact (1509 bytes, 1533 compiled: VC6 gives the candidate list a frame
// slot and other registers in the path loop; behaviour matches, see
// tools/emu_cases/track_place_near.py; docs/TRACK.md lists the forms tried).
int Track::PlaceNear(Vector3 p, TrackPos from, float distance, float range, int path, TrackPos* out)
{
    float best;
    float t;
    TrackListItem* list = 0;
    int found = 0;
    TrackCandidate* candidates = 0;
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 843);
    if (!item)
        return 0;
    if (out) {
        *out = from;
        best = -range;
    }
    item->node = startNode;
    list = item;
    do {
        TrackNode* node = list->node;
        if (node->flags & 4) {
            node->flags &= ~4;
            item = list;
            list = list->next;
            DebugFree(item, __FILE__, 865);
        } else {
            node->flags |= 4;
            TrackSegment* next;
            for (TrackSegment* segment = list->node->firstSegment; segment && (next = segment->next) != 0;
                 segment = segment->next) {
                if (IsInsideStrip(*(Vec3*)&p, segment, next)) {
                    float ex = next->point.x - segment->point.x;
                    float ez = next->point.z - segment->point.z;
                    if (ex == 0.0f && ez == 0.0f) {
                        t = 0.0f;
                    } else {
                        t = ((p.x - segment->point.x) * ex + (p.z - segment->point.z) * ez) / (ez * ez + ex * ex);
                        if (t > 1.0f)
                            t = 1.0f;
                        else if (t < 0.0f)
                            t = 0.0f;
                    }
                    TrackCandidate* candidate = (TrackCandidate*)DebugCalloc(1, sizeof(TrackCandidate), __FILE__, 889);
                    if (!candidate)
                        return 0;
                    candidate->next = candidates;
                    candidate->pos.node = list->node;
                    candidate->pos.segment = segment;
                    candidate->pos.t = t;
                    candidates = candidate;
                }
            }
            int count = list->node->linkCount;
            TrackNode** links = list->node->links;
            for (int i = 0; i < count; i++) {
                if (!(links[i]->flags & 4)) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 907);
                    if (!item) {
                        FreeWorkList(&list, 0);
                        return 0;
                    }
                    item->node = links[i];
                    item->next = list;
                    list = item;
                }
            }
        }
    } while (list);
    while (candidates) {
        if (distance < 0.0f || range < 0.0f) {
            if (!found && out)
                *out = candidates->pos;
            found = 1;
        }
        if (!found || out) {
            float ahead = DistanceAlong(from, candidates->pos);
            float behind = DistanceAlong(candidates->pos, from);
            if (ahead >= 0.0f && ahead <= distance || behind >= 0.0f && behind <= range) {
                found = 1;
                if (out && (ahead > best || -behind > best)) {
                    *out = candidates->pos;
                    if (ahead > best)
                        best = ahead;
                    else
                        best = -behind;
                }
            }
            for (TrackListItem* step = (TrackListItem*)path; !found && step; step = step->next) {
                TrackPos start;
                start.node = step->node;
                start.segment = start.node->firstSegment;
                start.t = 0.0f;
                // Retail measures the second distance from the path node to `from`
                // (0x00517737..0x0051776b: arguments start, from), not back from the
                // candidate; the tests below then compare the two distances from start.
                ahead = DistanceAlong(start, candidates->pos);
                behind = DistanceAlong(start, from);
                if (NodeHasSegment(candidates->pos.segment, step->node)
                    && (!NodeHasSegment(from.segment, step->node)
                        || ahead >= 0.0f && behind >= 0.0f && ahead < behind)) {
                    found = 1;
                    if (out && (ahead > best || -behind > best)) {
                        *out = candidates->pos;
                        if (ahead > best)
                            best = ahead;
                        else
                            best = -behind;
                    }
                }
                if (ahead >= 0.0f && ahead <= distance || behind >= 0.0f && behind <= range) {
                    found = 1;
                    if (out && (ahead > best || -behind > best)) {
                        *out = candidates->pos;
                        if (ahead > best)
                            best = ahead;
                        else
                            best = -behind;
                    }
                }
            }
        }
        TrackCandidate* done = candidates;
        candidates = candidates->next;
        DebugFree(done, __FILE__, 994);
    }
    return found;
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
        *list = (*list)->next;
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
// `length` is initialised at the top although that value is never read:
// the early reference makes `fromStart` the newer leaf, so the arrival sum
// loads it first (`fld fromStart; fadd length`) as retail does. Inferred
// from that operand order (no other local's init moves it), not seen
// directly in the bytes; without it the function is 927/929.
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
    item->next = list;
    list = item;
    length = toEnd;
    while (list) {
        TrackNode* node = list->node;
        if (node->flags & 4) {
            length -= node->pathLength;
            item = list;
            list = list->next;
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
                    item->next = list;
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
                            item->next = *path;
                            *path = item;
                            for (TrackListItem* s = list; s; s = s->next) {
                                if (s->node->flags & 4) {
                                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1258);
                                    if (!item) {
                                        FreeWorkList(&list, 0);
                                        FreeWorkList(path, 0);
                                        return 0;
                                    }
                                    item->node = s->node;
                                    item->next = *path;
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
int Track::MoveAlongPath(TrackPos pos, TrackPos* out, TrackListItem** path, float distance, unsigned char flags)
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
                        item = item->previous;
                    else
                        item = item->next;
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
                        item = item->previous;
                    else
                        item = item->next;
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
int Track::PointAt(TrackPos pos, Vec3* out)
{
    TrackSegment* segment = pos.segment;
    if (!segment)
        return 0;
    TrackSegment* next = segment->next;
    if (!next) {
        if (pos.t == 0.0f) {
            out->x = segment->point.x;
            out->y = segment->point.y;
            out->z = segment->point.z;
        }
        return 0;
    }
    if (pos.t < 0.0f)
        pos.t = 0.0f;
    else if (pos.t > 1.0f)
        pos.t = 1.0f;
    Vec3 d;
    d.x = next->point.x - segment->point.x;
    d.y = next->point.y - segment->point.y;
    d.z = next->point.z - segment->point.z;
    out->x = d.x * pos.t + segment->point.x;
    out->y = d.y * pos.t + segment->point.y;
    out->z = d.z * pos.t + segment->point.z;
    return 1;
}

// The unit vector along v (v itself when it already has unit length). The
// squared length needs the (y*y + x*x) + z*z grouping.
static inline Vec3 TrackNormalized(const Vec3& v)
{
    float lengthSquared = (v.y * v.y + v.x * v.x) + v.z * v.z;
    if (lengthSquared == 1.0f)
        return v;
    float scale = FastInvSqrt(lengthSquared);
    return v * scale;
}

// 0x00518130: the unit direction of a segment. Built through Vec3's
// constructor (whose by-value parameters give the components fresh leaves,
// docs/VC6_OPERAND_ORDER.md; MakeVec3's copy through a local does not), the
// difference gives retail's scaling: x scalar-first
// (`fld st(0); fmul x`), y and z component-first (`fld y; fmul st(1)`), the
// scale kept on the x87 stack until a final `fstp st(0)`. Per-component
// stores into a named local scale all three scalar-first.
int Track::SegmentDirection(TrackSegment* segment, Vec3* out)
{
    if (out && segment && segment->next && segment->horizontalLength > 0.0f) {
        TrackSegment* next = segment->next;
        Vec3 d = Vec3(next->point.x - segment->point.x, next->point.y - segment->point.y,
                         next->point.z - segment->point.z);
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
int Track::DistanceToStripEdge(Vec3 p, TrackSegment* segment, float* out, int mode)
{
    Vec3 startA;
    Vec3 directionA;
    Vec3 startB;
    Vec3 directionB;
    Vec3 n;
    Vec3 q;
    float t;
    float u;    // the second parameter of the cut, then the second distance
    if (!segment || !segment->next || !out)
        return 0;
    TrackSegment* next = segment->next;
    switch (mode) {
    case 17:
        startA = segment->edgeA;
        directionA = next->edgeA - segment->edgeA;
        startB = segment->edgeB;
        directionB = next->edgeB - segment->edgeB;
        break;
    case 11:
        startA = segment->edgeB;
        directionA = next->edgeB - segment->edgeB;
        break;
    case 10:
        startA = segment->edgeA;
        directionA = next->edgeA - segment->edgeA;
        break;
    }
    if (directionA.x == 0.0f && directionA.z == 0.0f) {
        if (directionA.y == 0.0f)
            return 0;
        n.x = directionA.z;
        n.z = -directionA.y;
        IntersectLines(startA.y, startA.z, directionA.y, directionA.z, p.y, p.z, n.x, n.z, &t, &u);
    } else {
        n.x = directionA.z;
        n.z = -directionA.x;
        IntersectLines(startA.x, startA.z, directionA.x, directionA.z, p.x, p.z, n.x, n.z, &t, &u);
    }
    if (t < 0.0f)
        t = 0.0f;
    else if (t > 1.0f)
        t = 1.0f;
    directionA = directionA * t;
    startA += directionA;
    startA.x -= p.x;
    startA.y -= p.y;
    startA.z -= p.z;
    *out = startA.x * startA.x + startA.z * startA.z;
    if (mode == 17) {
        if (directionB.x == 0.0f && directionB.z == 0.0f) {
            if (directionB.y == 0.0f)
                return 0;
            n.x = directionB.z;
            n.z = -directionB.y;
            IntersectLines(startB.y, startB.z, directionB.y, directionB.z, p.y, p.z, n.x, n.z, &t, &u);
        } else {
            n.x = directionB.z;
            n.z = -directionB.x;
            IntersectLines(startB.x, startB.z, directionB.x, directionB.z, p.x, p.z, n.x, n.z, &t, &u);
        }
        if (t < 0.0f)
            t = 0.0f;
        else if (t > 1.0f)
            t = 1.0f;
        q = directionB * t + startB;
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
