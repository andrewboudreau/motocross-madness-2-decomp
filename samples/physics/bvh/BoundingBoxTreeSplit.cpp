// BoundingBoxTreeSplit.cpp -- the partition and bounds helpers at 0x0042ad30..0x0042b5eb.
//
// Ownership is uncertain, so this stays a sample: these six functions sit in the
// BoundingBoxTreeBuild.cpp link bracket (after bmpfile.cpp's last xref 0x004245dc) but before
// the file's first own __FILE__ xref (0x0042b75e), and none of them references __FILE__.
// Strong inference (tier 2) that they are BoundingBoxTreeBuild.cpp's own leading helpers: every
// direct call to them comes from BoundingBoxTreeBuild.cpp functions (0x0042b5f0, 0x0042bb60,
// 0x0042bf90, 0x0042cfa0, 0x0042d9e0) and none from anywhere else, while the code before
// 0x0042ad30 is the run-time box/OBB query code that CollisionShapeTests calls.
#include "bvh/BoundingBoxTreeBuild.h"

// 0x0042ad30.  Partitions triangle records by the centroid against a plane on `axis`: records
// with centroid > split go to `left`, the rest to `right`.  When either side ends up empty the
// input is split in half by index instead and 0 is returned (1 = the plane separated them).
int SplitTriangles(const TreeBuildTriangle* tris, TreeBuildTriangle* left,
                   TreeBuildTriangle* right, int count, int* leftCount, int* rightCount,
                   float split, int axis)
{
    int i;
    int toLeft;

    *leftCount = 0;
    *rightCount = 0;
    toLeft = 0;
    for (i = 0; i < count; i++) {
        switch (axis) {
        case 0: toLeft = tris[i].centroid.x > split; break;
        case 1: toLeft = tris[i].centroid.y > split; break;
        case 2: toLeft = tris[i].centroid.z > split; break;
        }
        if (toLeft)
            left[(*leftCount)++] = tris[i];
        else
            right[(*rightCount)++] = tris[i];
    }
    if (*rightCount != 0 && *leftCount != 0)
        return 1;

    *leftCount = count / 2;
    *rightCount = count - count / 2;
    for (i = 0; i < *leftCount; i++)
        left[i] = tris[i];
    for (i = 0; i < *rightCount; i++)
        right[i] = tris[*leftCount + i];
    return 0;
}

// 0x0042ae70.  The box-tree counterpart of SplitTriangles: element i is the box
// boxes[2*i], boxes[2*i+1] (two corners) with centre centers[i]; boxes whose centre is > split
// on `axis` go left.  When either side ends up empty the caller gets the half-and-half counts
// (no copying here) and 0 is returned.
int SplitBoxes(const TreeVec3* boxes, const TreeVec3* centers, TreeVec3* leftBoxes,
               TreeVec3* rightBoxes, TreeVec3* leftCenters, TreeVec3* rightCenters, int count,
               int* leftCount, int* rightCount, float split, int axis)
{
    int i;
    int toLeft;

    *leftCount = 0;
    *rightCount = 0;
    for (i = 0; i < count; i++) {
        switch (axis) {
        case 0: toLeft = centers[i].x > split; break;
        case 1: toLeft = centers[i].y > split; break;
        case 2: toLeft = centers[i].z > split; break;
        }
        if (toLeft) {
            leftBoxes[2 * *leftCount] = boxes[2 * i];
            leftBoxes[2 * *leftCount + 1] = boxes[2 * i + 1];
            leftCenters[*leftCount] = centers[i];
            (*leftCount)++;
        } else {
            rightBoxes[2 * *rightCount] = boxes[2 * i];
            rightBoxes[2 * *rightCount + 1] = boxes[2 * i + 1];
            rightCenters[*rightCount] = centers[i];
            (*rightCount)++;
        }
    }
    if (*rightCount != 0 && *leftCount != 0)
        return 1;

    *leftCount = count / 2;
    *rightCount = count - count / 2;
    return 0;
}

// Edge of a bounding box used for its volume: never negative and at least 0.01 so flat
// boxes still have a volume.
inline float BoxEdge(float halfExtent)
{
    float edge = halfExtent * 2.0f;
    if (edge < 0.0f)
        edge = -edge;
    if (edge <= 0.01f)
        edge = 0.01f;
    return edge;
}

// 0x0042aff0.  Axis-aligned bounds of `count` points (optionally transformed by `m` first):
// writes the centre, the half size and the volume (edges clamped to >= 0.01).
void ComputePointBounds(const TreeVec3* points, int count, TreeVec3* center,
                        TreeVec3* halfExtents, float* volume, const TreeMatrix4* m)
{
    TreeVec3 max = points[0];
    TreeVec3 min = points[0];
    int i;

    for (i = 0; i < count; i++) {
        TreeVec3 p = points[i];
        if (m)
            TransformPoint(&p, p, m);
        if (p.x > max.x) max.x = p.x;
        if (p.y > max.y) max.y = p.y;
        if (p.z > max.z) max.z = p.z;
        if (p.x < min.x) min.x = p.x;
        if (p.y < min.y) min.y = p.y;
        if (p.z < min.z) min.z = p.z;
    }
    *center = (min + max) * 0.5f;
    *halfExtents = (max - min) * 0.5f;
    *volume = BoxEdge(halfExtents->x) * BoxEdge(halfExtents->y) * BoxEdge(halfExtents->z);
}

// 0x0042b2a0.  ComputePointBounds over the three corners of each triangle record (vertex
// indices into `verts`).
void ComputeTriangleBounds(const TreeBuildTriangle* tris, int count, const TreeVec3* verts,
                           TreeVec3* center, TreeVec3* halfExtents, float* volume,
                           const TreeMatrix4* m)
{
    TreeVec3 max = verts[tris[0].vertex[0]];
    TreeVec3 min = verts[tris[0].vertex[0]];
    int i;
    int k;

    for (i = 0; i < count; i++) {
        for (k = 0; k < 3; k++) {
            TreeVec3 p = verts[tris[i].vertex[k]];
            if (m)
                TransformPoint(&p, p, m);
            if (p.x > max.x) max.x = p.x;
            if (p.y > max.y) max.y = p.y;
            if (p.z > max.z) max.z = p.z;
            if (p.x < min.x) min.x = p.x;
            if (p.y < min.y) min.y = p.y;
            if (p.z < min.z) min.z = p.z;
        }
    }
    *center = (min + max) * 0.5f;
    *halfExtents = (max - min) * 0.5f;
    *volume = BoxEdge(halfExtents->x) * BoxEdge(halfExtents->y) * BoxEdge(halfExtents->z);
}

// 0x0042b590.  Bounding-box volume of a triangle set, the split cost the triangle tree uses.
float TriangleBoundsVolume(const TreeBuildTriangle* tris, const TreeVec3* verts, int count)
{
    TreeVec3 center;
    TreeVec3 halfExtents;
    float volume;

    ComputeTriangleBounds(tris, count, verts, &center, &halfExtents, &volume, 0);
    return volume;
}

// 0x0042b5c0.  Bounding-box volume of a point set (the box tree passes box corners).
float PointBoundsVolume(const TreeVec3* points, int count)
{
    TreeVec3 center;
    TreeVec3 halfExtents;
    float volume;

    ComputePointBounds(points, count, &center, &halfExtents, &volume, 0);
    return volume;
}
