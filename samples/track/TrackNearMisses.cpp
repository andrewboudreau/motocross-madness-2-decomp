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
// bytes): same size and layout as retail; 926 of 932 bytes match. Setting
// `digit = 0` in the first file's unprefixed branch (as the loop's later
// copy does), instead of initialising it at the declaration, keeps n in
// edi in both digit-suffix copies, so VC6 cross-jumps them as retail does
// (186 -> 926; before this the later copy kept n in esi and reloaded
// `directory`). The one remaining difference: retail compares the
// CurrentName result with the zero in ebx (`cmp eax, ebx`) where VC6 here
// emits `test eax, eax`; `!= 0`, `!= count` and an `ok` local leave it.
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
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->edgeAX, (TrackVec3*)&next->edgeAX))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->edgeAX, (TrackVec3*)&segment->pointX))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->edgeBX, (TrackVec3*)&segment->pointX))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&segment->edgeBX, (TrackVec3*)&next->edgeBX))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&next->edgeAX, (TrackVec3*)&next->pointX))
        inside = 1 - inside;
    if (UnknownEdgeCrosses(p, (TrackVec3*)&next->edgeBX, (TrackVec3*)&next->pointX))
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
    int digit;
    int count = 0;
    int prefixed;
    int n;
    if (series == 5 || series == 1)
        prefixed = 1;
    else
        prefixed = 0;
    FreeRows();
    directory->SetDirectory((const char*)g_TrackGame->mode.currentSeriesDirectory);
    sprintf(pattern, "*%s", g_TrackGame->highScores->extensions[g_TrackGame->highScores->extensionIndex]);
    directory->SetPattern(pattern, 1);
    directory->Scan();
    if (directory->CurrentName(found)) {
        if (prefixed) {
            n = strlen(found) - 1;
            strncpy(track, found, n);
            digit = found[n] - '0';
        } else {
            digit = 0;
            int length = strlen(found);
            n = length > 127 ? 127 : length;
            strncpy(track, found, n);
        }
        track[n] = 0;
        for (;;) {
            int directoryIndex = g_TrackGame->mode.GetSelectedSeries();
            sprintf(path, "%s\\%s%s", (const char*)g_TrackGame->mode.currentSeriesDirectory, found,
                    g_TrackGame->highScores->extensions[g_TrackGame->highScores->extensionIndex]);
            FILE* file = fopen(path, "r");
            if (file) {
                fread(&directoryIndex, 4, 1, file);
                fclose(file);
            }
            g_TrackGame->mode.FindFileDirectory((int)g_TrackGame->mode.seriesDirectories[directoryIndex],
                                                              track, "env", pattern);
            g_TrackGame->sceneObject->SetSceneDirectory(pattern);
            g_TrackGame->sceneObject->ReadSceneName(display, track, digit, "scn", 0, 0);
            static_cast<UIListBox*>(list)->AddRow(display, rowCount, 0);
            count++;
            rows = (UnknownTrackRecordRow**)DebugRealloc(rows, (rowCount + 1) * 4, __FILE__, 534);
            rows[rowCount] = new(__FILE__, 535) UnknownTrackRecordRow;
            rows[rowCount]->fileName = strdup(found);
            rows[rowCount]->field_0x04 = 0;
            rowCount++;
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

