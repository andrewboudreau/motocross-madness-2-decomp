// Near-miss Track.cpp candidates, kept out of src/reconstructed until they
// match. See docs/TRACK.md.
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
// binding declares it int). Retail's entry stores give the slots: list
// (+0x18 above the saved registers, immediate 0), found (+0x1c, from the
// ebp zero, so candidates = 0 shares the zero web) and this (+0x20);
// candidates lives in ebp for the whole function with no home. Here
// candidates gets ebp in the strip loop, a slot at +0x20 and is reloaded
// around the path loop, where ebp holds start.t. Dropping the dead
// `list = 0` removes its store (1525 bytes) but retail has it.
// Audited again as the linked body (KbRaceSub::PlaceNear forwards to it) on
// 2026-11-11: a masked instruction diff against retail shows only the frame
// size, register choice, load scheduling and the placement of the out-of-memory
// FreeWorkList exit (inside the walk loop here, after it in retail); calls,
// constants, branches and field offsets agree, so the 0.868 linked score is
// code shape, not behaviour.
//
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Track.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/krusty2/math/FastMath.h"

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
            list = list->next;
            DebugFree(item, __FILE__, 865);
        } else {
            node->flags |= 4;
            TrackSegment* next;
            for (TrackSegment* segment = list->node->firstSegment; segment && (next = segment->next) != 0;
                 segment = segment->next) {
                if (IsInsideStrip(*(TrackVec3*)&p, segment, next)) {
                    float ex = next->pointX - segment->pointX;
                    float ez = next->pointZ - segment->pointZ;
                    if (ex == 0.0f && ez == 0.0f) {
                        t = 0.0f;
                    } else {
                        t = ((p.x - segment->pointX) * ex + (p.z - segment->pointZ) * ez) / (ez * ez + ex * ex);
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

