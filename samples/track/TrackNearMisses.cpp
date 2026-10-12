// Near-miss Track.cpp candidates, kept out of src/reconstructed until they
// match. See docs/TRACK.md.
//
// Track::IsInsideStrip (0x00516ef0, 1042 bytes): 1032/1042. Edge 2
// (retail computes b.x - a.x before p.z - a.z) matches with the first factor
// written `((b->x - a->x))` (docs/VC6_OPERAND_ORDER.md section 3); edge 6
// still evaluates p.x - a.x before b.z - a.z (retail the other way). All 256
// combinations of factor/product parentheses, factor order and comparison
// direction give 1032 at best; no declaration count (k = 0..63) changes it.
// Factor and comparison order in the source do not move it. A macro over
// plain floats is much further off, because VC6 then loads the segment
// operands before p; the inline helper with a by-value TrackVec3 and
// pointers to the edge points fixes that. Edge points by reference give the
// same 1023; by value, or p by reference or pointer, the helper is no longer
// inlined; p after the edge pointers in the parameter list is worse (994).
// Also 1032 at best: 100 argument parenthesis/cast forms for edge 6, 648
// parenthesis depths of the four factors, both products and `p.z >= a->z`,
// 256 forms of the condition comparisons, and the edge points as struct
// locals, inline struct casts, TrackVec3 locals, `+ i`/`[i]` indexing or six
// named locals, each with 324 parenthesis forms (4536 compiles). Retail
// computes b.z - a.z first in edges 2, 5 and 6 and p.x - a.x first in 1, 3
// and 4; edges 3 and 6 differ only in their base pointer, so the tie-break
// is not structural.
// 2026-10-11: 1..31 dead initialised int locals before any of the six edge tests repeat
// 1002, 992, 1012, 1032 with period 4 wherever they go (top or before edge 2..6);
// locals inside UnknownEdgeCrosses stop it being inlined from 4 on.
//
// Track::PlaceNear (0x00517340, 1509 bytes; the candidate is
// 1533): the walk, the strip test, the candidate list and the acceptance
// tests follow retail instruction for instruction, but VC6 here gives the
// candidate list a frame slot (0x24 bytes of locals against retail's 0x20)
// and assigns the path loop's pointer, node and segment to edi/esi/ebx
// with start.t in ebp, where retail uses ebx/esi/edi and reloads start.t
// from the frame. A do-while walk, separate or shared loop variables, an
// aggregate TrackPos, function-scope distances and an inline accept helper
// do not change it. The fifth parameter is the work list (RaceStatus.cpp's
// binding declares it int).
//
// TrackRecordDlg::ListHighScoreTracks (TrackRecord.cpp, 0x0051ffe0, 932
// bytes): the candidate is 964 bytes. The frame (0x350, so one buffer is
// 260 bytes), stack slots, calls and the body of the loop agree. Retail
// cross-jumps the two copies of the digit-suffix branch: the first-file copy
// jumps into the later copy's strncpy call, and the later copy then jumps
// back to the first copy's `track[n] = 0`. VC6 here keeps n in esi in the
// later copy (edi in retail), which needs a reload of `directory` and blocks
// the merge, so 32 bytes stay duplicated. A separate or block-scoped n, the
// branches swapped, `continue`, an `ok` variable and track[n] = 0 in both
// first-file branches do not move it; a goto into the first copy rearranges
// the prologue (edi pushed late) and is further off.
// Session 10: compiled inside TrackRecord.cpp it is unchanged (195 of 800
// masked, 18 instruction diffs); also unchanged: `while (1)`, an unsigned
// n, a length local, a DirectoryList local for NextName, n declared first,
// `*(found + n)` and track[n] = 0 before the digit in the later copy.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Track.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/TrackRecordDlg.h"
#include "../../src/krusty2/math/FastMath.h"

// One edge of the crossing-number test in 0x00516ef0: whether the edge a-b
// crosses the ray from p.
static inline int UnknownEdgeCrosses(TrackVec3 p, const TrackVec3* a, const TrackVec3* b)
{
    return (a->x <= p.x || b->x <= p.x)
        && (p.z >= a->z && p.z < b->z || p.z >= b->z && p.z < a->z)
        && (((b->x - a->x)) * (p.z - a->z) <= (p.x - a->x) * (b->z - a->z)) == (p.z >= a->z);
}

// 0x00516ef0: whether p lies, in the horizontal plane, inside the strip
// between segment and next (edge points at +0x0c and +0x18, centre at
// +0x00), by the crossing-number rule over six edges.
int Track::IsInsideStrip(TrackVec3 p, TrackSegment* segment, TrackSegment* next)
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

// 0x0051ffe0: lists the tracks with a high-score file. For series 1 and 5
// the name's last character is a digit that selects the variant.
void TrackRecordDlg::ListHighScoreTracks(UIControl* list, DirectoryList* directory, int series)
{
    char found[128];
    char track[128];
    char display[64];
    char path[256];
    char pattern[260];
    int digit = 0;
    int count = 0;
    int prefixed;
    int n;
    if (series == 5 || series == 1)
        prefixed = 1;
    else
        prefixed = 0;
    UnknownFunction51ff40();
    directory->SetDirectory((const char*)g_TrackGame->mode.currentSeriesDirectory);
    sprintf(pattern, "*%s", g_TrackGame->field_0x3400->field_0x04[g_TrackGame->field_0x3400->field_0x00]);
    directory->SetPattern(pattern, 1);
    directory->UnknownVirtualSlot1();
    if (directory->CurrentName(found)) {
        if (prefixed) {
            n = strlen(found) - 1;
            strncpy(track, found, n);
            digit = found[n] - '0';
        } else {
            int length = strlen(found);
            n = length > 127 ? 127 : length;
            strncpy(track, found, n);
        }
        track[n] = 0;
        for (;;) {
            int directoryIndex = g_TrackGame->mode.GetSelectedSeries();
            sprintf(path, "%s\\%s%s", (const char*)g_TrackGame->mode.currentSeriesDirectory, found,
                    g_TrackGame->field_0x3400->field_0x04[g_TrackGame->field_0x3400->field_0x00]);
            FILE* file = fopen(path, "r");
            if (file) {
                fread(&directoryIndex, 4, 1, file);
                fclose(file);
            }
            g_TrackGame->mode.FindFileDirectory((int)g_TrackGame->mode.seriesDirectories[directoryIndex],
                                                              track, "env", pattern);
            g_TrackGame->sceneObject->SetSceneDirectory(pattern);
            g_TrackGame->sceneObject->ReadSceneName(display, track, digit, "scn", 0, 0);
            static_cast<UIListBox*>(list)->AddRow(display, field_0x7f64, 0);
            count++;
            field_0x7f60 = (UnknownTrackRecordRow**)DebugRealloc(field_0x7f60, (field_0x7f64 + 1) * 4, __FILE__, 534);
            field_0x7f60[field_0x7f64] = new(__FILE__, 535) UnknownTrackRecordRow;
            field_0x7f60[field_0x7f64]->field_0x00 = strdup(found);
            field_0x7f60[field_0x7f64]->field_0x04 = 0;
            field_0x7f64++;
            if (!directory->NextName(found))
                break;
            if (prefixed) {
                n = strlen(found) - 1;
                strncpy(track, found, n);
                digit = found[n] - '0';
                track[n] = 0;
            } else {
                digit = 0;
                strcpy(track, found);
            }
        }
    }
    static_cast<UIListBox*>(list)->Sort(1);
    if (count == 0) {
        g_TrackGame->LoadResourceString(0x142e, path, 128);
        static_cast<UIListBox*>(list)->AddRow(path, 0, 0);
        static_cast<UIListBox*>(list)->SetSelectable(0);
    } else {
        static_cast<UIListBox*>(list)->SetSelectable(1);
    }
}

// 0x00517340
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
            list = list->field_0x0c;
            DebugFree(item, __FILE__, 865);
        } else {
            node->flags |= 4;
            TrackSegment* next;
            for (TrackSegment* segment = list->node->firstSegment; segment && (next = segment->next) != 0;
                 segment = segment->next) {
                if (IsInsideStrip(*(TrackVec3*)&p, segment, next)) {
                    float ex = next->field_0x00 - segment->field_0x00;
                    float ez = next->field_0x08 - segment->field_0x08;
                    if (ex == 0.0f && ez == 0.0f) {
                        t = 0.0f;
                    } else {
                        t = ((p.x - segment->field_0x00) * ex + (p.z - segment->field_0x08) * ez) / (ez * ez + ex * ex);
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
                    item->field_0x0c = list;
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
            for (TrackListItem* step = (TrackListItem*)path; !found && step; step = step->field_0x0c) {
                TrackPos start;
                start.node = step->node;
                start.segment = start.node->firstSegment;
                start.t = 0.0f;
                ahead = DistanceAlong(start, candidates->pos);
                behind = DistanceAlong(candidates->pos, start);
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
// Draft near miss (117 of 2400 compared bytes; retail 2342): the chunk
// dispatch, calls and arithmetic follow retail, but the frame is 0x64
// (retail 0x48) and retail keeps `this` in ebp (spilled to the frame for the
// probe chunks), so every frame offset differs.
// The start / finish probe the loader fills (bikerace.cpp's 0x3c-byte gate).
struct TrackProbe {
    TrackVec3 centre;               // +0x00
    TrackVec3 direction;            // +0x0c
    TrackVec3 halfExtents;          // +0x18
    float field_0x24;
    float field_0x28;               // 150 after loading
    TrackPos pos;                   // +0x2c
};

static inline void ReadProbe(UnknownTextureStream* stream, TrackProbe* probe, float scale,
                             float offsetX, float offsetZ)
{
    stream->Read(&probe->centre.x, 4, 1);
    stream->Read(&probe->centre.y, 4, 1);
    stream->Read(&probe->centre.z, 4, 1);
    stream->Read(&probe->direction.x, 4, 1);
    stream->Read(&probe->direction.y, 4, 1);
    stream->Read(&probe->direction.z, 4, 1);
    stream->Read(&probe->halfExtents.x, 4, 1);
    stream->Read(&probe->halfExtents.y, 4, 1);
    stream->Read(&probe->halfExtents.z, 4, 1);
    stream->Read(&probe->field_0x24, 4, 1);
    probe->halfExtents.x *= scale;
    probe->halfExtents.y *= scale;
    probe->halfExtents.z *= scale;
    probe->field_0x24 *= scale;
    probe->centre.x = probe->centre.x * scale + offsetX;
    probe->centre.y *= scale;
    probe->centre.z = probe->centre.z * scale + offsetZ;
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
    if (!stream)
        return 0;
    char chunk;
    stream->Read(&chunk, 1, 1);
    index = 0;
    while (!stream->UnknownFunction430ff0()) {
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
            if (index < 0 || index >= count)
                goto fail;
            nodes[index] = (TrackNode*)DebugCalloc(1, sizeof(TrackNode), __FILE__, 108);
            if (!nodes[index])
                goto fail;
            link = &nodes[index]->firstSegment;
        } else if (chunk == 6) {
            if (!nodes[index])
                goto fail;
            int flags;
            stream->Read(&flags, 4, 1);
            nodes[index]->flags = (unsigned char)(flags | 1);
        } else if (chunk == 7) {
            if (!nodes[index])
                goto fail;
            stream->Read(&nodes[index]->pathLength, 4, 1);
            nodes[index]->pathLength *= scale;
        } else if (chunk == 8) {
            if (!nodes[index] || !link)
                goto fail;
            TrackSegment* segment = (TrackSegment*)DebugCalloc(1, sizeof(TrackSegment), __FILE__, 141);
            if (!segment)
                goto fail;
            segment->previous = nodes[index]->lastSegment;
            *link = segment;
            link = &segment->next;
            nodes[index]->lastSegment = segment;
        } else if (chunk == 9) {
            if (!nodes[index] || !nodes[index]->lastSegment)
                goto fail;
            stream->Read(&nodes[index]->lastSegment->field_0x00, 4, 1);
            stream->Read(&nodes[index]->lastSegment->field_0x04, 4, 1);
            stream->Read(&nodes[index]->lastSegment->field_0x08, 4, 1);
            nodes[index]->lastSegment->field_0x00 = scale * nodes[index]->lastSegment->field_0x00 + offsetX;
            nodes[index]->lastSegment->field_0x04 = scale * nodes[index]->lastSegment->field_0x04;
            nodes[index]->lastSegment->field_0x08 = scale * nodes[index]->lastSegment->field_0x08 + offsetZ;
            TrackSegment* segment = nodes[index]->lastSegment;
            TrackSegment* previous = segment->previous;
            if (previous) {
                float dx = segment->field_0x00 - previous->field_0x00;
                float dz = segment->field_0x08 - previous->field_0x08;
                previous->horizontalLength = (float)sqrt(dz * dz + dx * dx);
            }
        } else if (chunk == 10) {
            if (!nodes[index] || !nodes[index]->lastSegment)
                goto fail;
            stream->Read(&nodes[index]->lastSegment->field_0x0c, 4, 1);
            stream->Read(&nodes[index]->lastSegment->field_0x10, 4, 1);
            stream->Read(&nodes[index]->lastSegment->field_0x14, 4, 1);
            nodes[index]->lastSegment->field_0x0c = scale * nodes[index]->lastSegment->field_0x0c + offsetX;
            nodes[index]->lastSegment->field_0x10 = scale * nodes[index]->lastSegment->field_0x10;
            nodes[index]->lastSegment->field_0x14 = scale * nodes[index]->lastSegment->field_0x14 + offsetZ;
        } else if (chunk == 11) {
            if (!nodes[index] || !nodes[index]->lastSegment)
                goto fail;
            stream->Read(&nodes[index]->lastSegment->field_0x18, 4, 1);
            stream->Read(&nodes[index]->lastSegment->field_0x1c, 4, 1);
            stream->Read(&nodes[index]->lastSegment->field_0x20, 4, 1);
            nodes[index]->lastSegment->field_0x18 = scale * nodes[index]->lastSegment->field_0x18 + offsetX;
            nodes[index]->lastSegment->field_0x1c = scale * nodes[index]->lastSegment->field_0x1c;
            nodes[index]->lastSegment->field_0x20 = scale * nodes[index]->lastSegment->field_0x20 + offsetZ;
        } else if (chunk == 12) {
            int unused;
            stream->Read(&unused, 4, 1);
        } else if (chunk == 13) {
            if (!nodes[index])
                goto fail;
            stream->Read(&linkCount, 4, 1);
            if (linkCount < 0)
                goto fail;
            nodes[index]->linkCount = linkCount;
            nodes[index]->links = (TrackNode**)DebugCalloc(linkCount, 4, __FILE__, 231);
            if (!nodes[index]->links)
                goto fail;
        } else if (chunk == 14) {
            if (!nodes[index] || !nodes[index]->links)
                goto fail;
            int slot;
            int target;
            stream->Read(&slot, 4, 1);
            stream->Read(&target, 4, 1);
            if (slot < 0 || slot > linkCount || target < 0 || target > count)
                goto fail;
            nodes[index]->links[slot] = (TrackNode*)target;
        } else if (chunk == 15) {
            *hasStart = 1;
            ReadProbe(stream, (TrackProbe*)start, scale, offsetX, offsetZ);
        } else if (chunk == 16) {
            *hasFinish = 1;
            ReadProbe(stream, (TrackProbe*)finish, scale, offsetX, offsetZ);
        }
        stream->Read(&chunk, 1, 1);
    }
    {
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->links) {
                for (int j = 0; j < nodes[i]->linkCount; j++) {
                    int k = (int)nodes[i]->links[j];
                    if (k >= 0 && k < count)
                        nodes[i]->links[j] = nodes[k];
                }
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
fail:
    FreeNodeArray((UnknownStream*)stream, nodes, count);
    return 0;
}
