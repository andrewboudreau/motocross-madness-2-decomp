// Partial traversal candidate; original TU: VisibilityQuadTree.cpp.
#include "visibility/VisibilityQuadTree.h"

// One child of the walk: skipped when absent or outside the frustum, gathered whole when all 8
// box corners are inside and recursed into otherwise.
__forceinline QuadTreeResultLink* VisibilityQuadTreeNode::VisitChild(
    int i, int x, int z, QuadTreeResultLink* tail, int nodeX, int nodeZ, int midX, int midZ,
    int half, float centerX, float centerZ, float halfExtent, VisibilityCamera* camera,
    VisibilityBoxVec* center, VisibilityBoxVec* extent, int* cornersInside)
{
    if (!children[i])
        return tail;
    center->y = children[i]->yCenter;
    extent->y = children[i]->yHalfExtent;
    center->x = (i & 2) ? centerX + halfExtent : centerX - halfExtent;
    center->z = (i & 1) ? centerZ + halfExtent : centerZ - halfExtent;
    if (!g_visibilityClipper->TestBox(camera, (const float*)((char*)camera + 0xec), &center->x,
                                      &extent->x, 0, cornersInside, 0))
        return tail;
    int childX = (i & 2) ? midX : nodeX;
    int childZ = (i & 1) ? midZ : nodeZ;
    if (*cornersInside == 8)
        return children[i]->UnknownVirtualSlot2(x, z, tail, childX, childZ, half);
    return ((VisibilityQuadTreeNode*)children[i])->Traverse(x, z, tail, childX, childZ, half, camera);
}

// Back-to-front walk: the quadrant containing the query point is visited last.
QuadTreeResultLink* VisibilityQuadTreeNode::Traverse(int x, int z, QuadTreeResultLink* tail,
                                                     int nodeX, int nodeZ, int size,
                                                     VisibilityCamera* camera)
{
    int midX = nodeX + size;
    int midZ = nodeZ + size;
    float centerX = (float)midX * g_quadTreeInvScale;
    float centerZ = (float)midZ * g_quadTreeInvScale;
    size >>= 1;
    float halfExtent = (float)size * g_quadTreeInvScale;
    VisibilityBoxVec center;
    VisibilityBoxVec extent;
    int cornersInside;
    extent.x = halfExtent;
    extent.z = halfExtent;
    if (children) {
        if (x >= midX) {
            if (z >= midZ) {
                tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            } else {
                tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
                tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            }
        } else if (z >= midZ) {
            tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        } else {
            tail = VisitChild(3, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(2, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(1, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
            tail = VisitChild(0, x, z, tail, nodeX, nodeZ, midX, midZ, size, centerX, centerZ, halfExtent, camera, &center, &extent, &cornersInside);
        }
    }
    if (itemList) {
        QuadTreeResultLink* cell = (QuadTreeResultLink*)g_pQuadTree->resultPool->Alloc();
        if (tail)
            tail->next = cell;
        cell->item = this;
        cell->next = 0;
        return cell;
    }
    return tail;
}
