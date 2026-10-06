#include <float.h>
#include <stdio.h>
#include <string.h>

#include "Track.h"

#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"

// 0x00515dc0: an empty string leaves nothing to split.
UnknownTokenizer::UnknownTokenizer(char* text)
{
    if (strlen(text) != 0)
        field_0x04 = text;
    else
        field_0x04 = 0;
}

// 0x00515df0: returns the next token and skips the delimiters after it.
char* UnknownTokenizer::UnknownFunction515df0(const char* delimiters)
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
int Track::UnknownFunction515e70(UnknownStream* stream, TrackNode** nodes, int count)
{
    if (!stream || !nodes)
        return 0;
    for (int i = 0; i < count; i++) {
        if (nodes[i])
            UnknownFunction516800(nodes[i]);
    }
    operator delete(nodes, __FILE__, 30);
    return 1;
}

// 0x00516800: frees a node, its segments and its link array.
int Track::UnknownFunction516800(TrackNode* node)
{
    if (!node)
        return 0;
    while (node->field_0x08) {
        TrackSegment* segment = node->field_0x08;
        node->field_0x08 = segment->field_0x2c;
        operator delete(segment, __FILE__, 353);
    }
    if (node->field_0x14)
        operator delete(node->field_0x14, __FILE__, 359);
    operator delete(node, __FILE__, 361);
    return 1;
}

// 0x00516870: frees every node reachable from *start. A node is marked on
// its first visit and freed when its list entry comes back to the top.
int Track::UnknownFunction516870(TrackNode** start)
{
    TrackListItem* list = 0;
    if (!start || !*start)
        return 0;
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 383);
    if (!item)
        return 0;
    item->field_0x04 = *start;
    item->field_0x0c = list;
    list = item;
    while (list) {
        TrackNode* node = list->field_0x04;
        if (node->field_0x00 & 4) {
            UnknownFunction516800(node);
            item = list;
            list = list->field_0x0c;
            operator delete(item, __FILE__, 400);
        } else {
            node->field_0x00 |= 4;
            int count = list->field_0x04->field_0x10;
            TrackNode** links = list->field_0x04->field_0x14;
            for (int i = 0; i < count; i++) {
                if (!(links[i]->field_0x00 & 4)) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 410);
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
    }
    return 1;
}

// 0x00517310: whether `segment` belongs to `node`.
int Track::UnknownFunction517310(TrackSegment* segment, TrackNode* node)
{
    for (TrackSegment* s = node->field_0x08; s; s = s->field_0x2c) {
        if (s == segment)
            return 1;
    }
    return 0;
}

// 0x00517930: frees a work list; its nodes too unless they came from the
// file (bit 0), or all of them when `all` is set.
int Track::UnknownFunction517930(TrackListItem** list, int all)
{
    if (!list)
        return 0;
    TrackListItem* item;
    while ((item = *list) != 0) {
        if (item->field_0x04 && (!(item->field_0x04->field_0x00 & 1) || all))
            UnknownFunction516800(item->field_0x04);
        *list = (*list)->field_0x0c;
        operator delete(item, __FILE__, 1021);
    }
    return 1;
}

// 0x005179a0: whether b is at or after a on the same node.
int Track::UnknownFunction5179a0(TrackPos a, TrackPos b)
{
    if (a.node == b.node) {
        if (a.segment == b.segment) {
            if (a.t <= b.t)
                return 1;
        } else {
            for (TrackSegment* s = a.segment; s; s = s->field_0x2c) {
                if (s == b.segment)
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00517da0: distance along the track from a to b.
float Track::UnknownFunction517da0(TrackPos a, TrackPos b)
{
    float distance = 0.0f;
    if (a.node == b.node) {
        if (a.segment == b.segment) {
            if (a.t <= b.t)
                return (b.t - a.t) * a.segment->field_0x24;
        } else {
            TrackSegment* s;
            for (s = a.segment; s; s = s->field_0x2c) {
                if (s == b.segment)
                    break;
                distance += s->field_0x24;
            }
            if (s == 0)
                goto search;
            return b.t * b.segment->field_0x24 - a.t * a.segment->field_0x24 + distance;
        }
    search:
        UnknownFunction5179f0(a, b, 0, &distance);
        return distance;
    }
    UnknownFunction5179f0(a, b, 0, &distance);
    return distance;
}

// 0x00517ea0: moves pos `distance` along a node path (*path, linked through
// +0x08/+0x0c). Bit 1 of `flags`, combined with each entry's bit 1, picks
// the direction.
int Track::UnknownFunction517ea0(TrackPos pos, TrackPos* out, TrackListItem** path, float distance, unsigned char flags)
{
    TrackListItem* item;
    if (!path || (item = *path) == 0 || pos.node != item->field_0x04)
        return 0;
    TrackPos cur = pos;
    float travelled = 0.0f;
    while (travelled < distance) {
        if ((item->field_0x00 & 2) && !(flags & 2) || !(item->field_0x00 & 2) && (flags & 2)) {
            if (cur.t == 0.0f) {
                if (!cur.segment->field_0x28) {
                    if (flags & 2)
                        item = item->field_0x08;
                    else
                        item = item->field_0x0c;
                    if (!item)
                        break;
                    cur.node = item->field_0x04;
                    if ((item->field_0x00 & 2) && !(flags & 2) || !(item->field_0x00 & 2) && (flags & 2))
                        cur.segment = cur.node->field_0x0c;
                    else
                        cur.segment = cur.node->field_0x08;
                } else {
                    cur.segment = cur.segment->field_0x28;
                    travelled += cur.segment->field_0x24;
                }
            } else {
                travelled += cur.t * cur.segment->field_0x24;
                cur.t = 0.0f;
            }
            if (travelled > distance && cur.segment->field_0x24 != 0.0f)
                cur.t = (travelled - distance) / cur.segment->field_0x24;
        } else {
            if (cur.t == 1.0f) {
                if (!cur.segment->field_0x2c) {
                    if (flags & 2)
                        item = item->field_0x08;
                    else
                        item = item->field_0x0c;
                    if (!item)
                        break;
                    cur.node = item->field_0x04;
                    if ((item->field_0x00 & 2) && !(flags & 2) || !(item->field_0x00 & 2) && (flags & 2))
                        cur.segment = cur.node->field_0x0c;
                    else
                        cur.segment = cur.node->field_0x08;
                } else {
                    cur.segment = cur.segment->field_0x2c;
                    travelled += cur.segment->field_0x24;
                }
            } else {
                travelled += (1.0f - cur.t) * cur.segment->field_0x24;
                cur.t = 1.0f;
            }
            if (travelled > distance && cur.segment->field_0x24 != 0.0f)
                cur.t = 1.0f - (travelled - distance) / cur.segment->field_0x24;
        }
    }
    *out = cur;
    return 1;
}

// 0x00518080: the point at pos.
int Track::UnknownFunction518080(TrackPos pos, TrackVec3* out)
{
    TrackSegment* segment = pos.segment;
    if (!segment)
        return 0;
    TrackSegment* next = segment->field_0x2c;
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

void UnknownFunction518640(char* text, float seconds)
{
    int minutes = 0;
    while (seconds >= 60.0f) {
        seconds -= 60.0f;
        minutes++;
    }
    sprintf(text, "%2.2d:%2.2d", minutes, (int)seconds);
}

void UnknownFunction518690(char* text, float seconds)
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
