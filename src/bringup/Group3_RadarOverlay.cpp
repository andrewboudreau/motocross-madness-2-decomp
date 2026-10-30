// Bring-up (best-guess-unreviewed) RadarOverlay methods for the source-built game link.
//
// RadarOverlay::DrawRacers (0x0051bed0) is unresolved from TrackOverlay.cpp; it calls
// RadarOverlay::DrawTrackOutline (0x0051c720), which calls
// RadarOverlay::IntersectLineCircle (0x0051c4f0).  None of the three is defined in src/,
// so all three bodies are taken from the near misses in
// samples/track/TrackOverlayNearMisses.cpp (DrawRacers 1129/1160, DrawTrackOutline
// 1002/1016 candidate bytes, IntersectLineCircle 334/520 masked).  They are not
// byte-exact; control flow and calls were checked against the retail disassembly.
// Original TU: TrackOverlay.cpp.  Move these into src/reconstructed/TrackOverlay.cpp
// once they match.
#include "../krusty2/visibility/VisibilityClipper.h"
#include "../krusty2/broadphase/Terrain.h"
#include "../reconstructed/BikeRaceCore.h"
#include "../reconstructed/GUIManagerCore.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../reconstructed/TrackOverlay.h"
#include "../reconstructed/D3DIMSoulTree.h"
#include "../reconstructed/DebugAlloc.h"
#include "../reconstructed/BikeCamera.h"
#include "../reconstructed/Camera.h"
#include "../reconstructed/ControlInterface.h"
#include "../reconstructed/FollowCamera.h"
#include "../reconstructed/KeyboardDevice.h"
#include "../reconstructed/KrustyUI.h"
#include "../reconstructed/PCRenderTarget.h"
#include "../reconstructed/PCTextureMap.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/Track.h"
#include "../reconstructed/D3DConstants.h"

// 0x0051c4f0: intersects the line a-b with the circle (x, y, r); writes the
// intersection nearest to the incoming `point` back into `point`.  Returns -1 for no
// intersection (or a degenerate line), 1 for a tangent, 2 for two hits, 3 for a
// zero-length direction.
int RadarOverlay::IntersectLineCircle(const float* a, const float* b, const float* circle, float* a4, float* point)
{
    float nx;
    float ny;
    float lineOffset;
    if (LineThrough(a, b, &nx, &ny, &lineOffset))
        return -1;
    float scale = 1.0f / (nx * nx + ny * ny);
    float t = -(scale * lineOffset);
    float px = t * nx;
    float py = t * ny;
    float root = FastSqrt(scale);
    float dx = root * ny;
    float dy = -(root * nx);
    float lengthSquared = dy * dy + dx * dx;
    if (lengthSquared < 1e-14f)
        return 3;
    float ex = circle[0] - px;
    float ey = circle[1] - py;
    float cross = ey * dx - ex * dy;
    float radius = circle[2];
    float disc = radius * radius * lengthSquared - cross * cross;
    if (disc < -1e-14f)
        return -1;
    float proj = (ex * dx) + ey * dy;
    if (disc < 1e-14f) {
        float along = proj / lengthSquared;
        point[0] = along * dx + px;
        point[1] = along * dy + py;
        return 1;
    }
    float halfChord = FastSqrt(disc);
    float inverse = 1.0f / lengthSquared;
    float k1 = (proj - halfChord) * inverse;
    float k2 = (proj + halfChord) * inverse;
    float x1 = k1 * dx + px;
    float y1 = k1 * dy + py;
    float x2 = k2 * dx + px;
    float y2 = k2 * dy + py;
    float ay = point[1] - y1;
    float ax = point[0] - x1;
    float by = point[1] - y2;
    float bx = point[0] - x2;
    if (ax * ax + ay * ay < (by * by) + bx * bx) {
        point[0] = x1;
        point[1] = y1;
    } else {
        point[0] = x2;
        point[1] = y2;
    }
    return 2;
}

// 0x0051d730 (called by BaseQuarryEvent::LoadEventObjects 0x004e02c7, not defined
// elsewhere in src/): collects the racers of `view` and creates a name tag per racer, plus
// an extra fixed tag in event type 0. Near miss from TrackOverlayNearMisses.cpp (587
// against 586 bytes, same instructions); the racer iteration goes through the BikeRace
// provider (0x004204e0) as RadarOverlay::TakeRacers does.
int ChatOverlay::CollectRacers(UnknownChatView* view)
{
    int iterator;
    UnknownChatRacer* racer;
    racerCount = 0;
    iterator = 0;
    chatView = view;
    while ((racer = reinterpret_cast<UnknownChatRacer*>(
                reinterpret_cast<BikeRace*>(chatView)->NextBike(&iterator))) != 0) {
        racers[racerCount] = racer;
        racerCount++;
    }
    int i;
    for (i = 0; i < racerCount; i++) {
        nameTags[i] = (new(__FILE__, 2369) NameOverlay(1))->Create(
            Target(), overlayTexture, &nameTagRects[i], 0, &nameTagRects[i], (UnknownEventRacer*)racers[i],
            (UnknownNameOverlayWorld*)chatView, i);
        AppendChild(nameTags[i], -1);
    }
    if (g_TrackGame->mode.raceSettings.eventType == 0) {
        // best guess: the sample passes the loop index (== racerCount here) as the last
        // Create argument.
        nameTags[racerCount] = (new(__FILE__, 2376) NameOverlay(1))->Create(
            Target(), overlayTexture, &nameTagRects[racerCount], 0, &nameTagRects[racerCount], 0,
            (UnknownNameOverlayWorld*)chatView, i);
        AppendChild(nameTags[racerCount], -1);
        if (nameTags[racerCount]) {
            nameTags[racerCount]->SetFixedPosition(Vector3(cueRect.left, cueRect.top, 0));
            nameTags[racerCount]->Disable();
            nameLineRight = nameTagRects[racerCount].right;
            nameTags[racerCount]->drawWidth = 0;
        }
    }
    RedrawNameTags(-1);
    return 1;
}

// The view RadarOverlay draws (+0x12c) holds its track at +0x48.
struct UnknownRadarView {
    unsigned char field_0x00[0x48];
    Track* track;                              // +0x48
};

// 0x0051c720: draws the track outline (mode 1: the +0x18 edge of every
// segment, 2: the +0x0c edge), clipped to the map circle, walking every node
// once through a work list.
void RadarOverlay::DrawTrackOutline(int mode)
{
    TrackListItem* list = 0;
    TrackListItem* item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 1992);
    item->node = ((UnknownRadarView*)radarView)->track->startNode;
    item->next = list;
    list = item;
    while (list) {
        TrackNode*& node = list->node;
        if (node->flags & 4) {
            node->flags &= ~4;
            item = list;
            list = list->next;
            DebugFree(item, __FILE__, 2001);
        } else {
            node->flags |= 4;
            TrackSegment* first = node->firstSegment;
            Vector3 point;
            if (mode == 1)
                point = *(Vector3*)&first->edgeBX;
            else if (mode == 2)
                point = *(Vector3*)&first->edgeAX;
            int startX;
            int startY;
            float rim[2];
            int outside = WorldToMap(point, &startX, &startY, &rim[0], &rim[1]);
            lineStart.sx = (float)startX;
            lineStart.sy = (float)startY;
            lineStart.color = 0xedea5e;
            lineEnd.color = 0xedea5e;
            int x = startX;
            int y = startY;
            TrackSegment* segment = first->next;
            if (segment) do {
                if (mode == 1)
                    point = *(Vector3*)&segment->edgeBX;
                else if (mode == 2)
                    point = *(Vector3*)&segment->edgeAX;
                int wasOutside = outside;
                float from[2];
                from[0] = (float)x;
                from[1] = (float)y;
                outside = WorldToMap(point, &x, &y, &rim[0], &rim[1]);
                if (outside) {
                    if (wasOutside) {
                        lineStart.sx = (float)x;
                        lineStart.sy = (float)y;
                    } else {
                        float to[2];
                        float circle[3];
                        float hit[2];
                        to[0] = (float)x;
                        to[1] = (float)y;
                        circle[2] = mapRadius;
                        circle[0] = (float)mapCenterX;
                        circle[1] = (float)mapCenterY;
                        if (IntersectLineCircle(from, to, circle, rim, hit) != -1) {
                            lineEnd.sx = hit[0];
                            lineEnd.sy = hit[1];
                            Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
                            lineStart = lineEnd;
                        }
                    }
                } else if (wasOutside) {
                    float to[2];
                    float circle[3];
                    float hit[2];
                    to[0] = (float)x;
                    to[1] = (float)y;
                    circle[2] = mapRadius;
                    circle[0] = (float)mapCenterX;
                    circle[1] = (float)mapCenterY;
                    if (IntersectLineCircle(from, to, circle, rim, hit) != -1) {
                        lineStart.sy = hit[1];
                        lineEnd.sy = (float)y;
                        lineEnd.sx = (float)x;
                        lineStart.sx = hit[0];
                        Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
                        lineStart = lineEnd;
                    }
                } else {
                    lineEnd.sx = (float)x;
                    lineEnd.sy = (float)y;
                    Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
                    lineStart = lineEnd;
                }
                segment = segment->next;
                if (segment == node->firstSegment) {
                    lineEnd.sx = (float)startX;
                    lineEnd.sy = (float)startY;
                    break;
                }
            } while (segment);
            for (int i = 0; i < node->linkCount; i++) {
                if (!(node->links[i]->flags & 4)) {
                    item = (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 2094);
                    item->node = node->links[i];
                    item->next = list;
                    list = item;
                }
            }
        }
    }
}

// 0x0051bed0: centres the map on the followed racer, places the name tags and
// draws the map dots, then the track outline (modes 2, 3) or the gates (1, 5).
void RadarOverlay::DrawRacers(int mode)
{
    int i;
    int x;
    int y;
    int outside;
    Vector3 rim;
    Vector3 point;
    for (i = 0; i < racerCount; i++) {
        if (racers[i] == radarCamera->followedRacer) {
            mapOrigin = racers[i]->position;
            mapAngle = -racers[i]->heading;
            if (racers[i]->crashState)
                mapAngle = heldMapAngle;
            else
                heldMapAngle = mapAngle;
            followedIndex = i;
            break;
        }
    }
    mapAngleCos = cos(mapAngle);
    mapAngleSin = sin(mapAngle);
    for (i = 0; i < racerCount; i++) {
        if (i != followedIndex && racers[i]->isEnabled) {
            outside = WorldToMap(racers[i]->position, &x, &y, &rim.x, &rim.y);
            if (cueShown && cueIndex == i) {
                if (radarView->chatOverlay) {
                    Vector3& own = racers[followedIndex]->position;
                    float dx = racers[i]->position.x - own.x;
                    float dz = racers[i]->position.z - own.z;
                    float distance = dz * dz + dx * dx;
                    radarView->chatOverlay->SetCueRacer(distance, racers[i]->racerName);
                }
                if (outside && (g_TrackGame->mode.raceSettings.eventType == 3 ||
                                g_TrackGame->mode.raceSettings.eventType == 2)) {
                    point.x = -1.0f;
                    point.y = -1.0f;
                } else {
                    // Retail converts `outside` with fild and compares it with 0.0f.
                    point.x = (outside != 0.0f) ? rim.x : (float)x;
                    // Retail really tests the sum of the dword at +0x1fc (mapRect.top) and
                    // `outside` here (mov edx,[ebp+0x1fc]; add edx,esi; je) before
                    // subtracting 6.4 and truncating.
                    point.y = (float)(int)(((mapRect.top + outside) ? rim.y : (float)y) - 6.4f);
                }
                for (int k = 0; k < racerCount; k++) {
                    NameOverlay* tag = radarView->chatOverlay->nameTags[k];
                    if (tag->trackedRacer == racers[i])
                        tag->SetRadarAnchor(point, x - mapCenterX > 0, outside);
                }
            } else {
                point.x = -1.0f;
                point.y = -1.0f;
                if (radarView->chatOverlay)
                    radarView->chatOverlay->nameTags[i]->SetRadarAnchor(point, 0, outside);
            }
            if (outside) {
                if (g_TrackGame->mode.raceSettings.eventType == 3 ||
                    g_TrackGame->mode.raceSettings.eventType == 2)
                    continue;
                lineStart.color = 0x28aa28;
                lineEnd.color = 0x28aa28;
                lineStart.sx = rim.x;
                lineStart.sy = rim.y;
            } else {
                lineStart.color = 0x50e650;
                lineEnd.color = 0x50e650;
                lineStart.sx = (float)x;
                lineStart.sy = (float)y;
            }
            lineEnd = lineStart;
            lineEnd.sx += 3.0f;
            Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
            lineStart.sy += 1.0f;
            lineEnd.sy += 1.0f;
            Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
            lineStart.sy -= 2.0f;
            lineEnd.sy -= 2.0f;
            Target()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, (int)&lineStart, 2, 0);
        } else {
            point.x = -1.0f;
            point.y = -1.0f;
            if (radarView->chatOverlay)
                radarView->chatOverlay->nameTags[i]->SetRadarAnchor(point, 0, 1);
        }
    }
    if (mode == 2 || mode == 3) {
        DrawTrackOutline(1);
        DrawTrackOutline(2);
    }
    if (mode == 1 || mode == 5)
        DrawGates();
}
