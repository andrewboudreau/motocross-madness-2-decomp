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
//
// Track::UnknownFunction517340 (0x00517340, 1509 bytes; the candidate is
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
// TrackRecordDlg::UnknownFunction51ffe0 (TrackRecord.cpp, 0x0051ffe0, 932
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
#include <stdio.h>
#include <string.h>
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Track.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/TrackRecordDlg.h"
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

// 0x0051ffe0: lists the tracks with a high-score file. For series 1 and 5
// the name's last character is a digit that selects the variant.
void TrackRecordDlg::UnknownFunction51ffe0(UnknownGameUiControl* list, DirectoryList* directory, int series)
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
    directory->UnknownFunction44a1d0((const char*)g_UnknownGlobal56e26c->mode.field_0x6a0);
    sprintf(pattern, "*%s", g_UnknownGlobal56e26c->field_0x3400->field_0x04[g_UnknownGlobal56e26c->field_0x3400->field_0x00]);
    directory->UnknownFunction44a220(pattern, 1);
    directory->UnknownVirtualSlot1();
    if (directory->UnknownFunction44a550(found)) {
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
            int directoryIndex = g_UnknownGlobal56e26c->mode.UnknownFunction524100();
            sprintf(path, "%s\\%s%s", (const char*)g_UnknownGlobal56e26c->mode.field_0x6a0, found,
                    g_UnknownGlobal56e26c->field_0x3400->field_0x04[g_UnknownGlobal56e26c->field_0x3400->field_0x00]);
            FILE* file = fopen(path, "r");
            if (file) {
                fread(&directoryIndex, 4, 1, file);
                fclose(file);
            }
            g_UnknownGlobal56e26c->mode.UnknownFunction523a60((int)g_UnknownGlobal56e26c->mode.field_0xa0[directoryIndex],
                                                              track, "env", pattern);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(pattern);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(display, track, digit, "scn", 0, 0);
            static_cast<UIListBox*>(list)->UnknownFunction476d80(display, field_0x7f64, 0);
            count++;
            field_0x7f60 = (UnknownTrackRecordRow**)DebugRealloc(field_0x7f60, (field_0x7f64 + 1) * 4, __FILE__, 534);
            field_0x7f60[field_0x7f64] = new(__FILE__, 535) UnknownTrackRecordRow;
            field_0x7f60[field_0x7f64]->field_0x00 = strdup(found);
            field_0x7f60[field_0x7f64]->field_0x04 = 0;
            field_0x7f64++;
            if (!directory->UnknownFunction44a4c0(found))
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
    static_cast<UIListBox*>(list)->UnknownFunction477900(1);
    if (count == 0) {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x142e, path, 128);
        static_cast<UIListBox*>(list)->UnknownFunction476d80(path, 0, 0);
        static_cast<UIListBox*>(list)->UnknownFunction477bb0(0);
    } else {
        static_cast<UIListBox*>(list)->UnknownFunction477bb0(1);
    }
}

// 0x00517340
int Track::UnknownFunction517340(Vector3 p, TrackPos from, float distance, float range, int path, TrackPos* out)
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
    item->field_0x04 = field_0x00;
    list = item;
    do {
        TrackNode* node = list->field_0x04;
        if (node->field_0x00 & 4) {
            node->field_0x00 &= ~4;
            item = list;
            list = list->field_0x0c;
            operator delete(item, __FILE__, 865);
        } else {
            node->field_0x00 |= 4;
            TrackSegment* next;
            for (TrackSegment* segment = list->field_0x04->field_0x08; segment && (next = segment->field_0x2c) != 0;
                 segment = segment->field_0x2c) {
                if (UnknownFunction516ef0(*(TrackVec3*)&p, segment, next)) {
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
                    candidate->pos.node = list->field_0x04;
                    candidate->pos.segment = segment;
                    candidate->pos.t = t;
                    candidates = candidate;
                }
            }
            int count = list->field_0x04->field_0x10;
            TrackNode** links = list->field_0x04->field_0x14;
            for (int i = 0; i < count; i++) {
                if (!(links[i]->field_0x00 & 4)) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 907);
                    if (!item) {
                        UnknownFunction517930(&list, 0);
                        return 0;
                    }
                    item->field_0x04 = links[i];
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
            float ahead = UnknownFunction517da0(from, candidates->pos);
            float behind = UnknownFunction517da0(candidates->pos, from);
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
                start.node = step->field_0x04;
                start.segment = start.node->field_0x08;
                start.t = 0.0f;
                ahead = UnknownFunction517da0(start, candidates->pos);
                behind = UnknownFunction517da0(candidates->pos, start);
                if (UnknownFunction517310(candidates->pos.segment, step->field_0x04)
                    && (!UnknownFunction517310(from.segment, step->field_0x04)
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
        operator delete(done, __FILE__, 994);
    }
    return found;
}
