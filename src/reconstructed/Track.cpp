#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Track.h"

#include "DebugAlloc.h"
#include "GameUiHelpers.h"
#include "TextureMap.h"
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
        rest = text;
    else
        rest = 0;
}

// 0x00515df0: returns the next token and skips the delimiters after it.
char* UnknownTokenizer::NextToken(const char* delimiters)
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
// The start / finish probe the loader fills (bikerace.cpp's 0x3c-byte gate).
struct TrackProbe {
    TrackVec3 centre;               // +0x00
    TrackVec3 direction;            // +0x0c
    TrackVec3 halfExtents;          // +0x18
    float width;                    // +0x24, as BikeRaceNode.h
    float field_0x28;               // 150 after loading
    TrackPos pos;                   // +0x2c
};

static inline void ScaleTrackVec3(TrackVec3* v, float scale)
{
    v->x *= scale;
    v->y *= scale;
    v->z *= scale;
}

int Track::LoadTrack(UnknownTextureStream* stream, void* start, void* finish, int* hasStart,
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
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            nodes[index] = (TrackNode*)DebugCalloc(1, sizeof(TrackNode), __FILE__, 108);
            if (!nodes[index]) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            link = &nodes[index]->firstSegment;
        } else if (chunk == 6) {
            if (!nodes[index]) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            int flags;
            stream->Read(&flags, 4, 1);
            nodes[index]->flags = (unsigned char)(flags | 1);
        } else if (chunk == 7) {
            if (!nodes[index]) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->pathLength, 4, 1);
            nodes[index]->pathLength *= scale;
        } else if (chunk == 8) {
            if (!nodes[index] || !link) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            TrackSegment* segment = (TrackSegment*)DebugCalloc(1, sizeof(TrackSegment), __FILE__, 141);
            if (!segment) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            segment->previous = nodes[index]->lastSegment;
            *link = segment;
            link = &segment->next;
            nodes[index]->lastSegment = segment;
        } else if (chunk == 9) {
            if (!nodes[index] || !nodes[index]->lastSegment) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->lastSegment->pointX, 4, 1);
            stream->Read(&nodes[index]->lastSegment->pointY, 4, 1);
            stream->Read(&nodes[index]->lastSegment->pointZ, 4, 1);
            nodes[index]->lastSegment->pointX = scale * nodes[index]->lastSegment->pointX + offsetX;
            nodes[index]->lastSegment->pointY = scale * nodes[index]->lastSegment->pointY;
            nodes[index]->lastSegment->pointZ = scale * nodes[index]->lastSegment->pointZ + offsetZ;
            TrackSegment* segment = nodes[index]->lastSegment;
            TrackSegment* previous = segment->previous;
            if (previous) {
                float dx = segment->pointX - previous->pointX;
                float dz = segment->pointZ - previous->pointZ;
                previous->horizontalLength = (float)sqrt(dz * dz + dx * dx);
            }
        } else if (chunk == 10) {
            if (!nodes[index] || !nodes[index]->lastSegment) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->lastSegment->edgeAX, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeAY, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeAZ, 4, 1);
            nodes[index]->lastSegment->edgeAX = scale * nodes[index]->lastSegment->edgeAX + offsetX;
            nodes[index]->lastSegment->edgeAY = scale * nodes[index]->lastSegment->edgeAY;
            nodes[index]->lastSegment->edgeAZ = scale * nodes[index]->lastSegment->edgeAZ + offsetZ;
        } else if (chunk == 11) {
            if (!nodes[index] || !nodes[index]->lastSegment) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            stream->Read(&nodes[index]->lastSegment->edgeBX, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeBY, 4, 1);
            stream->Read(&nodes[index]->lastSegment->edgeBZ, 4, 1);
            nodes[index]->lastSegment->edgeBX = scale * nodes[index]->lastSegment->edgeBX + offsetX;
            nodes[index]->lastSegment->edgeBY = scale * nodes[index]->lastSegment->edgeBY;
            nodes[index]->lastSegment->edgeBZ = scale * nodes[index]->lastSegment->edgeBZ + offsetZ;
        } else if (chunk == 12) {
            int unused;
            stream->Read(&unused, 4, 1);
        } else if (chunk == 13) {
            if (!nodes[index]) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            stream->Read(&linkCount, 4, 1);
            if (linkCount < 0) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            nodes[index]->linkCount = linkCount;
            nodes[index]->links = (TrackNode**)DebugCalloc(linkCount, 4, __FILE__, 231);
            if (!nodes[index]->links) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
        } else if (chunk == 14) {
            if (!nodes[index] || !nodes[index]->links) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            stream->Read(&slot, 4, 1);
            stream->Read(&target, 4, 1);
            if (slot < 0 || slot > linkCount || target < 0 || target > count) {
                FreeNodeArray((UnknownStream*)stream, nodes, count);
                return 0;
            }
            nodes[index]->links[slot] = (TrackNode*)target;
        } else if (chunk == 15) {
            *hasStart = 1;
            TrackProbe* probe = (TrackProbe*)start;
            stream->Read(&probe->centre.x, 4, 1);
            stream->Read(&probe->centre.y, 4, 1);
            stream->Read(&probe->centre.z, 4, 1);
            stream->Read(&probe->direction.x, 4, 1);
            stream->Read(&probe->direction.y, 4, 1);
            stream->Read(&probe->direction.z, 4, 1);
            stream->Read(&probe->halfExtents.x, 4, 1);
            stream->Read(&probe->halfExtents.y, 4, 1);
            stream->Read(&probe->halfExtents.z, 4, 1);
            stream->Read(&probe->width, 4, 1);
            ScaleTrackVec3(&probe->halfExtents, scale);
            probe->width *= scale;
            probe->centre.x = probe->centre.x * scale + offsetX;
            probe->centre.y *= scale;
            probe->centre.z = probe->centre.z * scale + offsetZ;
        } else if (chunk == 16) {
            *hasFinish = 1;
            TrackProbe* probe = (TrackProbe*)finish;
            stream->Read(&probe->centre.x, 4, 1);
            stream->Read(&probe->centre.y, 4, 1);
            stream->Read(&probe->centre.z, 4, 1);
            stream->Read(&probe->direction.x, 4, 1);
            stream->Read(&probe->direction.y, 4, 1);
            stream->Read(&probe->direction.z, 4, 1);
            stream->Read(&probe->halfExtents.x, 4, 1);
            stream->Read(&probe->halfExtents.y, 4, 1);
            stream->Read(&probe->halfExtents.z, 4, 1);
            stream->Read(&probe->width, 4, 1);
            ScaleTrackVec3(&probe->halfExtents, scale);
            probe->width *= scale;
            probe->centre.x = probe->centre.x * scale + offsetX;
            probe->centre.y *= scale;
            probe->centre.z = probe->centre.z * scale + offsetZ;
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
        TrackProbe* probe = (TrackProbe*)start;
        probe->field_0x28 = 150.0f;
        ClosestPointOnTrack(*(Vector3*)&probe->centre, &probe->pos, 0);
    }
    if (*hasFinish) {
        TrackProbe* probe = (TrackProbe*)finish;
        probe->field_0x28 = 150.0f;
        ClosestPointOnTrack(*(Vector3*)&probe->centre, &probe->pos, 0);
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
                    float dx = p.x - segment->pointX;
                    float dz = p.z - segment->pointZ;
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
                    float ex = next->pointX - segment->pointX;
                    float ez = next->pointZ - segment->pointZ;
                    float t;
                    float dx;
                    float dz;
                    if (ex == 0.0f && ez == 0.0f)
                        t = 0.0f;
                    else {
                        t = ((p.z - segment->pointZ) * ez + (p.x - segment->pointX) * ex) / (ez * ez + ex * ex);
                        if (t > 1.0f)
                            t = 1.0f;
                        else if (t < 0.0f)
                            t = 0.0f;
                    }
                    if (t == 1.0f) {
                        dx = p.x - next->pointX;
                        dz = p.z - next->pointZ;
                    } else if (t == 0.0f) {
                        dx = p.x - segment->pointX;
                        dz = p.z - segment->pointZ;
                    } else {
                        dx = (p.x - segment->pointX) - t * ex;
                        dz = (p.z - segment->pointZ) - t * ez;
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
                float dx = p.x - segment->pointX;
                float dz = p.z - segment->pointZ;
                *outDistance = FastSqrt(dz * dz + dx * dx);
            }
            return 1;
        }
        for (; segment; segment = segment->next) {
            TrackSegment* next = segment->next;
            if (!next)
                break;
            float ex = next->pointX - segment->pointX;
            float ez = next->pointZ - segment->pointZ;
            float t;
            float dx;
            float dz;
            if (ex == 0.0f && ez == 0.0f)
                t = 0.0f;
            else {
                t = ((p.z - segment->pointZ) * ez + (p.x - segment->pointX) * ex) / (ez * ez + ex * ex);
                if (t > 1.0f)
                    t = 1.0f;
                else if (t < 0.0f)
                    t = 0.0f;
            }
            if (t == 1.0f) {
                dx = p.x - next->pointX;
                dz = p.z - next->pointZ;
            } else if (t == 0.0f) {
                dx = p.x - segment->pointX;
                dz = p.z - segment->pointZ;
            } else {
                dx = (p.x - segment->pointX) - t * ex;
                dz = (p.z - segment->pointZ) - t * ez;
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
int Track::PointAt(TrackPos pos, TrackVec3* out)
{
    TrackSegment* segment = pos.segment;
    if (!segment)
        return 0;
    TrackSegment* next = segment->next;
    if (!next) {
        if (pos.t == 0.0f) {
            out->x = segment->pointX;
            out->y = segment->pointY;
            out->z = segment->pointZ;
        }
        return 0;
    }
    if (pos.t < 0.0f)
        pos.t = 0.0f;
    else if (pos.t > 1.0f)
        pos.t = 1.0f;
    TrackVec3 d;
    d.x = next->pointX - segment->pointX;
    d.y = next->pointY - segment->pointY;
    d.z = next->pointZ - segment->pointZ;
    out->x = d.x * pos.t + segment->pointX;
    out->y = d.y * pos.t + segment->pointY;
    out->z = d.z * pos.t + segment->pointZ;
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
        TrackVec3 d = TrackVector(next->pointX - segment->pointX, next->pointY - segment->pointY,
                                  next->pointZ - segment->pointZ);
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
int Track::DistanceToStripEdge(TrackVec3 p, TrackSegment* segment, float* out, int mode)
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
        a = *(TrackVec3*)&segment->edgeAX;
        d = *(TrackVec3*)&next->edgeAX - *(TrackVec3*)&segment->edgeAX;
        a2 = *(TrackVec3*)&segment->edgeBX;
        d2 = *(TrackVec3*)&next->edgeBX - *(TrackVec3*)&segment->edgeBX;
        break;
    case 11:
        a = *(TrackVec3*)&segment->edgeBX;
        d = *(TrackVec3*)&next->edgeBX - *(TrackVec3*)&segment->edgeBX;
        break;
    case 10:
        a = *(TrackVec3*)&segment->edgeAX;
        d = *(TrackVec3*)&next->edgeAX - *(TrackVec3*)&segment->edgeAX;
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
