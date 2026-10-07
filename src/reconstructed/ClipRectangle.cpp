#include <float.h>

#include "ClipRectangle.h"

// Outcode -> region index 1..8 around the rectangle (0x00568500, 11
// entries; outcode bits 1 left, 2 right, 4 below, 8 above). -1 marks the
// impossible left+right outcodes. ClipLine reports these regions for a
// rejected segment and ClipPolygon uses them to track winding.
int s_outcodeRegion[11] = {0, 1, 5, -1, 3, 2, 4, -1, 7, 8, 6};

static ClipRectangle s_clipRectangle;         // 0x00579730
ClipRectangle* g_clipRectangle = &s_clipRectangle;

// 0x004312f0. Retail never stores top (+0x0c) here, although the corners
// assume it is FLT_MAX.
ClipRectangle::ClipRectangle()
{
    left = -FLT_MAX;
    bottom = -FLT_MAX;
    right = FLT_MAX;
    corners[0].x = -FLT_MAX;
    corners[0].y = -FLT_MAX;
    corners[0].z = -FLT_MAX;
    corners[1].x = FLT_MAX;
    corners[1].y = -FLT_MAX;
    corners[1].z = -FLT_MAX;
    corners[2].x = FLT_MAX;
    corners[2].y = FLT_MAX;
    corners[2].z = -FLT_MAX;
    corners[3].x = -FLT_MAX;
    corners[3].y = FLT_MAX;
    corners[3].z = -FLT_MAX;
    corners[4].x = -FLT_MAX;
    corners[4].y = -FLT_MAX;
    corners[4].z = -FLT_MAX;
    corners[5].x = FLT_MAX;
    corners[5].y = -FLT_MAX;
    corners[5].z = -FLT_MAX;
    corners[6].x = FLT_MAX;
    corners[6].y = FLT_MAX;
    corners[6].z = -FLT_MAX;
    corners[7].x = -FLT_MAX;
    corners[7].y = FLT_MAX;
    corners[7].z = -FLT_MAX;
}

ClipRectangle::~ClipRectangle()
{
}

// 0x00431350
void ClipRectangle::Set(float left_, float bottom_, float right_, float top_)
{
    left = left_;
    bottom = bottom_;
    right = right_;
    top = top_;
    corners[0].x = corners[3].x = left_;
    corners[1].x = corners[2].x = right_;
    corners[0].y = corners[1].y = bottom_;
    corners[2].y = corners[3].y = top_;
    for (int i = 0; i < 4; i++)
        corners[i + 4] = corners[i];
}

// 0x004313d0
int ClipRectangle::Outcode(ClipPoint point)
{
    int code = 0;
    if (point.y > top)
        code = 8;
    else if (point.y < bottom)
        code = 4;
    if (point.x > right)
        code |= 2;
    else if (point.x < left)
        code |= 1;
    return code;
}

// 0x00431430: moves the outside end of the segment's line onto the edge
// named by one outcode bit and returns that edge (1 left, 2 bottom,
// 3 right, 4 top).
int ClipRectangle::IntersectEdge(const ClipSegment* segment, int outcode, ClipPoint* out)
{
    float dx = segment->end.x - segment->start.x;
    float dy = segment->end.y - segment->start.y;
    float dz = segment->end.z - segment->start.z;
    float edge;
    int side;
    if (outcode & 0xc) {
        if (outcode & 4) {
            edge = bottom;
            side = 2;
        } else {
            edge = top;
            side = 4;
        }
        out->y = edge;
        out->x = (edge - segment->start.y) * dx / dy + segment->start.x;
        out->z = (edge - segment->start.y) * dz / dy + segment->start.z;
        return side;
    }
    if (outcode & 1) {
        edge = left;
        side = 1;
    } else {
        edge = right;
        side = 3;
    }
    out->x = edge;
    out->y = (edge - segment->start.x) * dy / dx + segment->start.y;
    out->z = (edge - segment->start.x) * dz / dx + segment->start.z;
    return side;
}

// 0x004314e0: returns 0 when the segment is inside, -1 when it was clipped
// into *out, and the start outcode when it is rejected. The edge outputs
// are the IntersectEdge sides for a clipped segment and the outcode regions
// for a rejected one.
int ClipRectangle::ClipLine(const ClipSegment* segment, ClipSegment* out, int* enterEdge, int* exitEdge)
{
    int startCode = Outcode(segment->start);
    int endCode = Outcode(segment->end);
    if ((startCode | endCode) == 0) {
        *exitEdge = 0;
        *enterEdge = 0;
        return 0;
    }
    if (startCode & endCode) {
        *enterEdge = s_outcodeRegion[startCode];
        *exitEdge = s_outcodeRegion[endCode];
        return startCode;
    }
    int exitSide = 0;
    int enterSide = 0;
    int startRegion = s_outcodeRegion[startCode];
    int endRegion = s_outcodeRegion[endCode];
    *out = *segment;
    for (;;) {
        if (startCode) {
            enterSide = IntersectEdge(segment, startCode, &out->start);
            startCode = Outcode(out->start);
        } else {
            exitSide = IntersectEdge(segment, endCode, &out->end);
            endCode = Outcode(out->end);
        }
        if ((startCode | endCode) == 0) {
            *enterEdge = enterSide;
            *exitEdge = exitSide;
            return -1;
        }
        if (startCode & endCode) {
            *enterEdge = startRegion;
            *exitEdge = endRegion;
            return startCode;
        }
    }
}

// 0x00431680: clips a closed polygon (points[count] is overwritten with
// points[0]) and returns the output vertex count. Corners are inserted
// where the polygon leaves through one edge and re-enters through another;
// a polygon that never enters but winds around the rectangle becomes the
// rectangle itself. The dst temporaries keep retail's order of reading the
// source point before advancing outCount.
int ClipRectangle::ClipPolygon(ClipPoint* points, ClipPoint* out, int count)
{
    int outCount, winding, lastExit;
    outCount = winding = lastExit = 0;
    points[count] = points[0];
    bool wrapped = false;
    int i = 0;
    for (;;) {
        if (i == count) {
            if (outCount == 0) {
                if (winding >= 8) {
                    for (outCount = 0; outCount < 4; outCount++)
                        out[outCount] = corners[outCount];
                }
                break;
            }
            i = 0;
            wrapped = true;
        }
        ClipSegment segment;
        segment.start = points[i];
        segment.end = points[i + 1];
        ClipSegment clipped;
        int exitEdge;
        int enterEdge;
        int result = ClipLine(&segment, &clipped, &enterEdge, &exitEdge);
        if (result == 0) {
            if (wrapped)
                break;
            ClipPoint* dst = &out[outCount++];
            *dst = segment.start;
        }
        if (result < 0) {
            if (lastExit > 0 && enterEdge > 0) {
                int corner = enterEdge - lastExit;
                if (corner < 0)
                    corner += 4;
                else if (corner == 0 && winding > 4)
                    corner = 4;
                for (int j = 0; j < corner; j++)
                    out[outCount++] = corners[lastExit - 1 + j];
            }
            if (wrapped)
                break;
            ClipPoint* dst = &out[outCount++];
            *dst = clipped.start;
            if (exitEdge > 0) {
                dst = &out[outCount++];
                *dst = clipped.end;
            }
            lastExit = exitEdge;
            winding = 0;
        } else {
            int turn = exitEdge - enterEdge;
            if (turn <= -4)
                turn += 8;
            else if (turn > 4)
                turn -= 8;
            if (turn == 4)
                turn = (exitEdge - s_outcodeRegion[result] == 1) ? 4 : -4;
            winding += turn;
        }
        i++;
    }
    return outCount;
}
