// Candidate: relocation evidence is incomplete; see docs/PHYSICS_VALIDATION.md.
// BoundingBoxTreeBuild.cpp -- D:\aardvark\VC\krusty2\BoundingBoxTreeBuild.cpp (string 0x005682bc).
//
// Builds and loads the collision bounding-box trees: a binary tree of axis-aligned boxes over
// triangles (mesh payload), over points, or over element boxes (model tree).  Every function
// here either passes the file's own __FILE__ string to DebugMalloc / operator delete or sits
// between two that do; see README.md.  The partition and bounds helpers that precede the first
// own xref are in samples/physics/bvh/BoundingBoxTreeSplit.cpp.  Function names are ours.
#include <math.h>
#include <string.h>

#include "bvh/BoundingBoxTreeBuild.h"

// ---------------------------------------------------------------------------------------------
// Building.  The partition and bounds helpers are declared in BoundingBoxTreeBuild.h.

// 0x0042b5f0.  Split plane for a triangle set.  Starts from the axis with the largest centroid
// extent, cut at 4/20 of it, then tries 18 cuts (1/20 .. 18/20 of the extent) on each axis and
// keeps the one whose two halves have the smallest summed box volume.  VC6 note: the grouping
// (extent / 20) * (step + 1) is what keeps retail's fmul 0.05 / fimul order; without it the
// compiler reassociates the product (and folds 0.05 * 4 in the first cut).
void ChooseTriangleSplit(const TreeBuildTriangle* tris, int count, const TreeVec3* verts,
                         float* split, int* axis)
{
    TreeVec3 max = tris[0].centroid;
    TreeVec3 min = tris[0].centroid;
    TreeBuildTriangle* left;
    TreeBuildTriangle* right;
    int leftCount;
    int rightCount;
    float dx;
    float dy;
    float dz;
    float best;
    float cost;
    int bestAxis;
    int bestStep;
    int i;
    int a;

    for (i = 0; i < count; i++) {
        TreeVec3 p = tris[i].centroid;
        if (p.x > max.x) max.x = p.x;
        if (p.y > max.y) max.y = p.y;
        if (p.z > max.z) max.z = p.z;
        if (p.x < min.x) min.x = p.x;
        if (p.y < min.y) min.y = p.y;
        if (p.z < min.z) min.z = p.z;
    }
    best = 0.0f;
    bestAxis = 0;
    dx = max.x - min.x;
    if (dx > best) {
        best = dx;
        bestAxis = 0;
    }
    dy = max.y - min.y;
    if (dy > best) {
        best = dy;
        bestAxis = 1;
    }
    dz = max.z - min.z;
    if (dz > best)
        bestAxis = 2;

    bestStep = 3;
    left = (TreeBuildTriangle*)DebugMalloc(count * sizeof(TreeBuildTriangle), __FILE__, 0x127);
    right = (TreeBuildTriangle*)DebugMalloc(count * sizeof(TreeBuildTriangle), __FILE__, 0x128);
    switch (bestAxis) {
    case 0: *split = (dx / 20.0f) * (bestStep + 1) + min.x; break;
    case 1: *split = (dy / 20.0f) * (bestStep + 1) + min.y; break;
    case 2: *split = (dz / 20.0f) * (bestStep + 1) + min.z; break;
    }
    SplitTriangles(tris, left, right, count, &leftCount, &rightCount, *split, bestAxis);
    best = TriangleBoundsVolume(right, verts, rightCount) + TriangleBoundsVolume(left, verts, leftCount);

    for (i = 0; i < 18; i++) {
        for (a = 0; a < 3; a++) {
            switch (a) {
            case 0: *split = (dx / 20.0f) * (i + 1) + min.x; break;
            case 1: *split = (dy / 20.0f) * (i + 1) + min.y; break;
            case 2: *split = (dz / 20.0f) * (i + 1) + min.z; break;
            }
            SplitTriangles(tris, left, right, count, &leftCount, &rightCount, *split, a);
            cost = TriangleBoundsVolume(right, verts, rightCount) + TriangleBoundsVolume(left, verts, leftCount);
            if (cost < best) {
                bestAxis = a;
                best = cost;
                bestStep = i;
            }
        }
    }
    *axis = bestAxis;
    switch (bestAxis) {
    case 0: *split = (dx / 20.0f) * (bestStep + 1) + min.x; break;
    case 1: *split = (dy / 20.0f) * (bestStep + 1) + min.y; break;
    case 2: *split = (dz / 20.0f) * (bestStep + 1) + min.z; break;
    }
    operator delete(left, __FILE__, 0x158);
    operator delete(right, __FILE__, 0x159);
}

// 0x0042b9d0.  Split plane for a point set: the axis with the largest extent, at the middle of
// the extent.  *axis is left unchanged when every extent is zero.
void ChoosePointSplit(const TreeVec3* points, int count, float* split, int* axis)
{
    TreeVec3 max = points[0];
    TreeVec3 min = points[0];
    float dx;
    float dy;
    float dz;
    float best;
    int i;

    for (i = 1; i < count; i++) {
        if (points[i].x > max.x) max.x = points[i].x;
        else if (points[i].x < min.x) min.x = points[i].x;
        if (points[i].y > max.y) max.y = points[i].y;
        else if (points[i].y < min.y) min.y = points[i].y;
        if (points[i].z > max.z) max.z = points[i].z;
        else if (points[i].z < min.z) min.z = points[i].z;
    }
    best = 0.0f;
    dx = max.x - min.x;
    if (dx > best) {
        best = dx;
        *axis = 0;
    }
    dy = max.y - min.y;
    if (dy > best) {
        best = dy;
        *axis = 1;
    }
    dz = max.z - min.z;
    if (dz > best)
        *axis = 2;
    switch (*axis) {
    case 0: *split = dx * 0.5f + min.x; break;
    case 1: *split = dy * 0.5f + min.y; break;
    case 2: *split = dz * 0.5f + min.z; break;
    }
}

// 0x0042bb60.  Split plane for a box set (boxes = two corners each, centers = their centres):
// the same 18 x 3 search as ChooseTriangleSplit over the box centres, scored by the summed
// corner-bounds volume of the two halves, starting from the middle cut of the largest extent.
// Cuts that leave one side empty are not scored.
void ChooseBoxSplit(const TreeVec3* boxes, const TreeVec3* centers, int count, float* split,
                    int* axis)
{
    TreeVec3 max = boxes[0];
    TreeVec3 min = boxes[0];
    TreeVec3* leftBoxes;
    TreeVec3* rightBoxes;
    TreeVec3* leftCenters;
    TreeVec3* rightCenters;
    int leftCount;
    int rightCount;
    float dx;
    float dy;
    float dz;
    float best;
    float cost;
    int bestAxis;
    int bestStep;
    int i;
    int a;

    for (i = 1; i < 2 * count; i++) {
        if (boxes[i].x > max.x) max.x = boxes[i].x;
        else if (boxes[i].x < min.x) min.x = boxes[i].x;
        if (boxes[i].y > max.y) max.y = boxes[i].y;
        else if (boxes[i].y < min.y) min.y = boxes[i].y;
        if (boxes[i].z > max.z) max.z = boxes[i].z;
        else if (boxes[i].z < min.z) min.z = boxes[i].z;
    }
    best = 0.0f;
    dx = max.x - min.x;
    if (dx > best) {
        best = dx;
        bestAxis = 0;
    }
    dy = max.y - min.y;
    if (dy > best) {
        best = dy;
        bestAxis = 1;
    }
    dz = max.z - min.z;
    if (dz > best)
        bestAxis = 2;

    bestStep = 9;
    leftBoxes = (TreeVec3*)DebugMalloc(count * 2 * sizeof(TreeVec3), __FILE__, 0x1d2);
    rightBoxes = (TreeVec3*)DebugMalloc(count * 2 * sizeof(TreeVec3), __FILE__, 0x1d3);
    leftCenters = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x1d4);
    rightCenters = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x1d5);
    switch (bestAxis) {
    case 0: *split = (dx / 20.0f) * (bestStep + 1) + min.x; break;
    case 1: *split = (dy / 20.0f) * (bestStep + 1) + min.y; break;
    case 2: *split = (dz / 20.0f) * (bestStep + 1) + min.z; break;
    }
    SplitBoxes(boxes, centers, leftBoxes, rightBoxes, leftCenters, rightCenters, count,
               &leftCount, &rightCount, *split, bestAxis);
    best = PointBoundsVolume(leftBoxes, 2 * leftCount) + PointBoundsVolume(rightBoxes, 2 * rightCount);

    for (i = 0; i < 18; i++) {
        for (a = 0; a < 3; a++) {
            switch (a) {
            case 0: *split = (dx / 20.0f) * (i + 1) + min.x; break;
            case 1: *split = (dy / 20.0f) * (i + 1) + min.y; break;
            case 2: *split = (dz / 20.0f) * (i + 1) + min.z; break;
            }
            if (SplitBoxes(boxes, centers, leftBoxes, rightBoxes, leftCenters, rightCenters,
                           count, &leftCount, &rightCount, *split, a)) {
                cost = PointBoundsVolume(leftBoxes, 2 * leftCount) + PointBoundsVolume(rightBoxes, 2 * rightCount);
                if (cost < best) {
                    bestAxis = a;
                    best = cost;
                    bestStep = i;
                }
            }
        }
    }
    *axis = bestAxis;
    switch (bestAxis) {
    case 0: *split = (dx / 20.0f) * (bestStep + 1) + min.x; break;
    case 1: *split = (dy / 20.0f) * (bestStep + 1) + min.y; break;
    case 2: *split = (dz / 20.0f) * (bestStep + 1) + min.z; break;
    }
    operator delete(leftBoxes, __FILE__, 0x205);
    operator delete(rightBoxes, __FILE__, 0x206);
    operator delete(leftCenters, __FILE__, 0x208);
    operator delete(rightCenters, __FILE__, 0x209);
}

// 0x0042bf90.  Fills `node` for `count` triangle records: a leaf for one record, otherwise an
// interior node with the records' bounds, partitioned at ChooseTriangleSplit's plane (in place:
// left half first) and split in half by index when the plane separates nothing.
void BuildTriangleNode(BoxTreeNode* node, TreeBuildTriangle* tris, int count,
                       const TreeVec3* verts)
{
    float split;
    int axis;
    TreeBuildTriangle* left;
    TreeBuildTriangle* right;
    int leftCount;
    int rightCount;
    int toLeft;
    int i;

    if (count == 1) {
        BoxTreeTriangleLeaf* leaf = (BoxTreeTriangleLeaf*)node;
        leaf->marker = -1.0f;
        leaf->vertex[0] = (unsigned short)tris[0].vertex[0];
        leaf->vertex[1] = (unsigned short)tris[0].vertex[1];
        leaf->vertex[2] = (unsigned short)tris[0].vertex[2];
        leaf->normal = tris[0].normal;
        leaf->planeOffset = tris[0].planeOffset;
        return;
    }

    ComputeTriangleBounds(tris, count, verts, &node->center, &node->halfExtents, &node->volume, 0);
    ChooseTriangleSplit(tris, count, verts, &split, &axis);
    leftCount = 0;
    rightCount = 0;
    toLeft = 0;
    left = (TreeBuildTriangle*)DebugMalloc(count * sizeof(TreeBuildTriangle), __FILE__, 0x22a);
    right = (TreeBuildTriangle*)DebugMalloc(count * sizeof(TreeBuildTriangle), __FILE__, 0x22b);
    for (i = 0; i < count; i++) {
        switch (axis) {
        case 0: toLeft = tris[i].centroid.x > split; break;
        case 1: toLeft = tris[i].centroid.y > split; break;
        case 2: toLeft = tris[i].centroid.z > split; break;
        }
        if (toLeft)
            left[leftCount++] = tris[i];
        else
            right[rightCount++] = tris[i];
    }
    for (i = 0; i < leftCount; i++)
        tris[i] = left[i];
    for (i = 0; i < rightCount; i++)
        tris[leftCount + i] = right[i];
    operator delete(left, __FILE__, 0x243);
    operator delete(right, __FILE__, 0x244);

    if (rightCount == 0 || leftCount == 0) {
        leftCount = count / 2;
        rightCount = count - leftCount;
    }
    if (leftCount == 1)
        node->child[0] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeTriangleLeaf), __FILE__, 0x24d);
    else
        node->child[0] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x24f);
    if (rightCount == 1)
        node->child[1] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeTriangleLeaf), __FILE__, 0x253);
    else
        node->child[1] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x255);
    BuildTriangleNode(node->child[0], tris, leftCount, verts);
    BuildTriangleNode(node->child[1], tris + leftCount, rightCount, verts);
}

// 0x0042c260.  Collects the triangles of `model`'s current LOD for the triangle tree.  The model
// is put at the origin with identity axes first (and its matrix restored at the end), so the
// positions come out in model space.  With a `frame`, only triangles whose three corners lie
// in that frame's vertex range are taken, with positions from the mesh's +0x14 array.  Every
// corner gets its own position, then equal positions are merged: the result is *verts
// (*vertCount unique positions) and *indices (three per triangle, *triCount triangles).
// PARTIAL 99.67%: same size and instructions apart from two spots: before the 0x348 allocation
// retail reloads numCorners before remap (VC6 gives the other order), and the remap pass reads
// keep[corners[k]] as [keep + offset] where VC6 gives [offset + keep].  The loop-variable names
// of the merge passes (j outer, k inner, i for the copy) are the ones that put every local in
// retail's stack slot; other choices only move slots.
void GatherModelTriangles(TreeModelSource* model, TreeSceneNode* frame, int* triCount,
                          int* vertCount, int** indices, TreeVec3** verts)
{
    TreeMatrix4 saved;
    TreeModelTriangle tri;
    TreeModelMesh* mesh;
    TreeModelVertex* source;
    TreeVec3* positions;
    TreeVec3* unique;
    int* corners;
    int* remap;
    int* keep;
    int numTris;
    int numCorners;
    int numUnique;
    int numTaken;
    int savedField;
    int inside;
    int first;
    int end;
    int n;
    int i;
    int j;
    int k;
    int t;

    numTris = 0;
    model->GetMatrixIn(0, &saved);
    model->SetAxesIn(0, &kVec3ZAxis, &kVec3YAxis, 0, 1);
    model->SetPositionIn(0, &kVec3Zero);
    savedField = model->field_0x1a4;
    model->field_0x1a4 = 0;
    model->MarkSubtreeDirty();
    model->SelectLod(-1);
    model->field_0x1a4 = savedField;
    model->MarkSubtreeDirty();

    if (frame == 0) {
        for (i = 0; i < model->lods[model->currentLod].meshCount; i++)
            numTris += model->lods[model->currentLod].meshes[i].triangleCount;
    } else {
        for (i = 0; i < model->lods[model->currentLod].meshCount; i++) {
            mesh = &model->lods[model->currentLod].meshes[i];
            first = 0;
            end = 0;
            for (k = 0; k < mesh->partCount; k++) {
                if (mesh->parts[k].frame == frame) {
                    first = ((char*)mesh->parts[k].firstVertex - (char*)mesh->vertices) /
                            sizeof(TreeModelVertex);
                    end = first + mesh->parts[k].vertexCount;
                }
            }
            for (t = 0; t < mesh->triangleCount; t++) {
                inside = 1;
                if (mesh->triangles[t].vertex[0] < first || mesh->triangles[t].vertex[0] >= end)
                    inside = 0;
                if (mesh->triangles[t].vertex[1] < first || mesh->triangles[t].vertex[1] >= end)
                    inside = 0;
                if (mesh->triangles[t].vertex[2] < first || mesh->triangles[t].vertex[2] >= end)
                    inside = 0;
                if (inside)
                    numTris++;
            }
        }
    }

    numCorners = numTris * 3;
    corners = (int*)DebugMalloc(numTris * 3 * sizeof(int), __FILE__, 0x2fc);
    remap = (int*)DebugMalloc(numTris * 3 * sizeof(int), __FILE__, 0x2fd);
    positions = (TreeVec3*)DebugMalloc(numTris * 3 * sizeof(TreeVec3), __FILE__, 0x2fe);

    // One position per corner; remap starts as the identity.
    n = 0;
    numTaken = 0;
    for (i = 0; i < model->lods[model->currentLod].meshCount; i++) {
        mesh = &model->lods[model->currentLod].meshes[i];
        if (frame == 0)
            source = mesh->vertices;
        else
            source = mesh->field_0x14;
        first = 0;
        end = 0;
        if (frame != 0) {
            for (k = 0; k < mesh->partCount; k++) {
                if (mesh->parts[k].frame == frame) {
                    first = ((char*)mesh->parts[k].firstVertex - (char*)mesh->vertices) /
                            sizeof(TreeModelVertex);
                    end = first + mesh->parts[k].vertexCount;
                }
            }
        }
        for (t = 0; t < model->lods[model->currentLod].meshes[i].triangleCount; t++) {
            tri = model->lods[model->currentLod].meshes[i].triangles[t];
            inside = 1;
            if (frame != 0) {
                if (mesh->triangles[t].vertex[0] < first || mesh->triangles[t].vertex[0] >= end)
                    inside = 0;
                if (mesh->triangles[t].vertex[1] < first || mesh->triangles[t].vertex[1] >= end)
                    inside = 0;
                if (mesh->triangles[t].vertex[2] < first || mesh->triangles[t].vertex[2] >= end)
                    inside = 0;
            }
            if (inside) {
                positions[n].x = source[tri.vertex[0]].position.x;
                positions[n].y = source[tri.vertex[0]].position.y;
                positions[n].z = source[tri.vertex[0]].position.z;
                corners[numTaken * 3 + 0] = n;
                remap[n] = n;
                n++;
                positions[n].x = source[tri.vertex[1]].position.x;
                positions[n].y = source[tri.vertex[1]].position.y;
                positions[n].z = source[tri.vertex[1]].position.z;
                corners[numTaken * 3 + 1] = n;
                remap[n] = n;
                n++;
                positions[n].x = source[tri.vertex[2]].position.x;
                positions[n].y = source[tri.vertex[2]].position.y;
                positions[n].z = source[tri.vertex[2]].position.z;
                corners[numTaken * 3 + 2] = n;
                remap[n] = n;
                n++;
                numTaken++;
            }
        }
    }

    // Merge equal positions: a later duplicate is dropped and remapped to the first one.
    keep = (int*)DebugMalloc(numCorners * sizeof(int), __FILE__, 0x348);
    for (i = 0; i < numCorners; i++)
        keep[i] = 1;
    numUnique = 0;
    for (j = 0; j < numCorners; j++) {
        if (keep[j]) {
            for (k = j + 1; k < numCorners; k++) {
                if (keep[k] && positions[j].x == positions[k].x &&
                    positions[j].y == positions[k].y && positions[j].z == positions[k].z) {
                    keep[k] = 0;
                    remap[k] = j;
                }
            }
        }
        if (keep[j])
            numUnique++;
    }

    unique = (TreeVec3*)DebugMalloc(numUnique * sizeof(TreeVec3), __FILE__, 0x361);
    n = 0;
    for (i = 0; i < numCorners; i++) {
        if (keep[i]) {
            unique[n] = positions[i];
            remap[i] = n;
            n++;
        }
    }
    for (k = 0; k < numCorners; k++) {
        if (keep[corners[k]])
            corners[k] = remap[corners[k]];
        else
            corners[k] = remap[remap[corners[k]]];
    }

    operator delete(remap, __FILE__, 0x376);
    operator delete(positions, __FILE__, 0x377);
    operator delete(keep, __FILE__, 0x378);
    *verts = unique;
    *indices = corners;
    *vertCount = numUnique;
    *triCount = numTris;
    model->SetMatrixIn(0, &saved);
}

// 0x0042c8c0.  BuildTriangleMeshTree for a render model: GatherModelTriangles supplies the
// indices and the vertices (returned through `verts`).  The sixth argument is never read.
void BuildModelTriangleTree(BoxTreeNode* root, TreeMatrix4* m, TreeModelSource* model,
                            TreeVec3** verts, TreeSceneNode* frame, int unused)
{
    int* indices;
    int triCount;
    int vertCount;
    TreeVec3* vertices;
    TreeBuildTriangle* tris;
    TreeMatrix4 inverse;
    int i;

    GatherModelTriangles(model, frame, &triCount, &vertCount, &indices, verts);
    vertices = *verts;
    tris = (TreeBuildTriangle*)DebugMalloc(triCount * sizeof(TreeBuildTriangle), __FILE__, 0x394);
    for (i = 0; i < triCount; i++) {
        tris[i].vertex[0] = indices[3 * i];
        tris[i].vertex[1] = indices[3 * i + 1];
        tris[i].vertex[2] = indices[3 * i + 2];
        TriangleNormal(&vertices[indices[3 * i]], &vertices[indices[3 * i + 1]],
                       &vertices[indices[3 * i + 2]], &tris[i].normal, &tris[i].planeOffset);
    }

    memset(m, 0, sizeof(*m));
    m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
    inverse = *m;
    InvertRigid(&inverse);
    for (i = 0; i < vertCount; i++)
        TransformPoint(&vertices[i], vertices[i], &inverse);
    for (i = 0; i < triCount; i++) {
        RotateVector(&tris[i].normal, tris[i].normal, &inverse);
        tris[i].centroid = (vertices[tris[i].vertex[0]] + vertices[tris[i].vertex[1]] +
                            vertices[tris[i].vertex[2]]) * (1.0f / 3.0f);
    }

    BuildTriangleNode(root, tris, triCount, vertices);
    operator delete(indices, __FILE__, 0x3be);
    operator delete(tris, __FILE__, 0x3bf);
}

// 0x0042cc60.  Builds a mesh's triangle tree.  `indices` holds three vertex indices per
// triangle.  The build records get the indices, the face normal and plane offset; then `m` is
// reset to identity, and the vertices and normals go through its inverse (a no-op left from
// the model-space path in BuildModelBoxTree).  The centroids follow, then the tree.  The
// seventh argument (0 at the only call, 0x0043297f) is never read.
void BuildTriangleMeshTree(BoxTreeNode* root, TreeMatrix4* m, TreeVec3* verts,
                           const int* indices, int triCount, int vertCount, int unused)
{
    TreeBuildTriangle* tris;
    TreeMatrix4 inverse;
    int i;

    tris = (TreeBuildTriangle*)DebugMalloc(triCount * sizeof(TreeBuildTriangle), __FILE__, 0x3d0);
    for (i = 0; i < triCount; i++) {
        tris[i].vertex[0] = indices[3 * i];
        tris[i].vertex[1] = indices[3 * i + 1];
        tris[i].vertex[2] = indices[3 * i + 2];
        TriangleNormal(&verts[indices[3 * i]], &verts[indices[3 * i + 1]],
                       &verts[indices[3 * i + 2]], &tris[i].normal, &tris[i].planeOffset);
    }

    memset(m, 0, sizeof(*m));
    m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
    inverse = *m;
    InvertRigid(&inverse);
    for (i = 0; i < vertCount; i++)
        TransformPoint(&verts[i], verts[i], &inverse);
    for (i = 0; i < triCount; i++) {
        RotateVector(&tris[i].normal, tris[i].normal, &inverse);
        tris[i].centroid = (verts[tris[i].vertex[0]] + verts[tris[i].vertex[1]] +
                            verts[tris[i].vertex[2]]) * (1.0f / 3.0f);
    }

    BuildTriangleNode(root, tris, triCount, verts);
    operator delete(tris, __FILE__, 0x3f6);
}

// 0x0042cfa0.  BuildTriangleNode for a point tree: a leaf holds one point.
void BuildPointNode(BoxTreeNode* node, TreeVec3* points, int count)
{
    float split;
    int axis;
    TreeVec3* left;
    TreeVec3* right;
    int leftCount;
    int rightCount;
    int toLeft;
    int i;

    if (count == 1) {
        BoxTreePointLeaf* leaf = (BoxTreePointLeaf*)node;
        leaf->marker = -1.0f;
        leaf->point = points[0];
        return;
    }

    ComputePointBounds(points, count, &node->center, &node->halfExtents, &node->volume, 0);
    ChoosePointSplit(points, count, &split, &axis);
    leftCount = 0;
    rightCount = 0;
    toLeft = 0;
    left = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x40f);
    right = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x410);
    for (i = 0; i < count; i++) {
        switch (axis) {
        case 0: toLeft = points[i].x > split; break;
        case 1: toLeft = points[i].y > split; break;
        case 2: toLeft = points[i].z > split; break;
        }
        if (toLeft)
            left[leftCount++] = points[i];
        else
            right[rightCount++] = points[i];
    }
    for (i = 0; i < leftCount; i++)
        points[i] = left[i];
    for (i = 0; i < rightCount; i++)
        points[leftCount + i] = right[i];
    operator delete(left, __FILE__, 0x428);
    operator delete(right, __FILE__, 0x429);

    if (rightCount == 0 || leftCount == 0) {
        leftCount = count / 2;
        rightCount = count - leftCount;
    }
    if (leftCount == 1)
        node->child[0] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreePointLeaf), __FILE__, 0x432);
    else
        node->child[0] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x434);
    if (rightCount == 1)
        node->child[1] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreePointLeaf), __FILE__, 0x438);
    else
        node->child[1] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x43a);
    BuildPointNode(node->child[0], points, leftCount);
    BuildPointNode(node->child[1], points + leftCount, rightCount);
}

// Vector constants: the four VC6 dynamic initializers at 0x0042d250..0x0042d38b ($E jmp thunk +
// body each) build (0,0,0), (1,0,0), (0,1,0) and (0,0,1) in a stack temporary and copy them into
// 12-byte globals 0x00579680, 0x00579690, 0x005796a0 and 0x00579670 (the shape that opens about
// 73 other retail TUs; see broadphase/Quadtree.cpp).  Here the initializers sit in the middle of
// the file: after BuildPointNode (last line 0x43a) and before 0x0042d390 (first line 0x44f),
// and VC6 emits $E code where the object is defined.  GatherModelTriangles (0x0042c260, line
// 0x2fc) already uses three of them, so they are declared earlier: we take an extern
// declaration in BoundingBoxTreeBuild.h, which makes these definitions external (tier 3; the
// retail image cannot show linkage).  Names are ours.
const TreeVec3 kVec3Zero = TreeVec3(0.0f, 0.0f, 0.0f);
const TreeVec3 kVec3XAxis = TreeVec3(1.0f, 0.0f, 0.0f);
const TreeVec3 kVec3YAxis = TreeVec3(0.0f, 1.0f, 0.0f);
const TreeVec3 kVec3ZAxis = TreeVec3(0.0f, 0.0f, 1.0f);

// 0x0042d390.  Builds a point tree over a render model's vertices: all vertices of the current
// LOD when `frame` is null, otherwise only the vertex ranges that belong to `frame`.  Each point
// goes through the inverse of the model's (or the frame's) GetMatrixIn(0) matrix, then all of
// them through the inverse of `m`.  Only the frame branch reassigns `total` (retail keeps the
// summed count in the frame==0 branch and copies the gathered count in the other one).
// PARTIAL 99.56%: same size; two operand-order spots differ.  In the frame==0 loop retail
// evaluates the y row of the inlined TransformPoint as _22*y, _32*z (every other row here and in
// the frame loop is z-term first), and the frame loop's vertex reads use [offset+vertices] where
// VC6 gives [vertices+offset].  Neither moved with loop-variable, indexing or declaration-order
// variants.
void BuildModelPointTree(BoxTreeNode* root, TreeMatrix4* m, TreeModelSource* model,
                         TreeSceneNode* frame)
{
    TreeVec3* points;
    TreeMatrix4 frameInverse;
    TreeMatrix4 inverse;
    TreeModelMesh* mesh;
    int total;
    int count;
    int first;
    int end;
    int i;
    int j;
    int k;

    model->SelectLod(-1);
    total = 0;
    for (i = 0; i < model->lods[model->currentLod].meshCount; i++)
        total += model->lods[model->currentLod].meshes[i].vertexCount;
    points = (TreeVec3*)DebugMalloc(total * sizeof(TreeVec3), __FILE__, 0x44f);
    count = 0;
    if (frame == 0) {
        model->GetMatrixIn(0, &frameInverse);
        InvertRigid(&frameInverse);
        for (i = 0; i < model->lods[model->currentLod].meshCount; i++) {
            mesh = &model->lods[model->currentLod].meshes[i];
            for (j = 0; j < mesh->vertexCount; j++) {
                points[count].x = mesh->vertices[j].position.x;
                points[count].y = mesh->vertices[j].position.y;
                points[count].z = mesh->vertices[j].position.z;
                TransformPoint(&points[count], points[count], &frameInverse);
                count++;
            }
        }
    } else {
        frame->GetMatrixIn(0, &frameInverse);
        InvertRigid(&frameInverse);
        for (i = 0; i < model->lods[model->currentLod].meshCount; i++) {
            mesh = &model->lods[model->currentLod].meshes[i];
            first = 0;
            end = 0;
            for (k = 0; k < mesh->partCount; k++) {
                if (mesh->parts[k].frame == frame) {
                    first = ((char*)mesh->parts[k].firstVertex - (char*)mesh->vertices) /
                            sizeof(TreeModelVertex);
                    end = first + mesh->parts[k].vertexCount;
                }
            }
            for (j = first; j < end; j++) {
                points[count].x = mesh->vertices[j].position.x;
                points[count].y = mesh->vertices[j].position.y;
                points[count].z = mesh->vertices[j].position.z;
                TransformPoint(&points[count], points[count], &frameInverse);
                count++;
            }
        }
        total = count;
    }

    inverse = *m;
    InvertRigid(&inverse);
    for (i = 0; i < total; i++)
        TransformPoint(&points[i], points[i], &inverse);
    BuildPointNode(root, points, total);
    operator delete(points, __FILE__, 0x48a);
}

// 0x0042d9e0.  BuildTriangleNode for a model (box) tree: element i is the box boxes[2*i],
// boxes[2*i+1] with centre centers[i]; a leaf holds one box.  Retail copies only the
// partitioned centres back, not the box corners, and reads the right-hand centres from index
// leftCount on, so below the root the subtrees work on boxes that were never reordered (both
// kept as decoded).
void BuildBoxNode(BoxTreeNode* node, TreeVec3* boxes, TreeVec3* centers, int count)
{
    float split;
    int axis;
    TreeVec3* leftBoxes;
    TreeVec3* rightBoxes;
    TreeVec3* leftCenters;
    TreeVec3* rightCenters;
    int leftCount;
    int rightCount;
    int i;

    if (count == 1) {
        BoxTreeBoxLeaf* leaf = (BoxTreeBoxLeaf*)node;
        leaf->marker = -1.0f;
        leaf->corner[0] = boxes[0];
        leaf->corner[1] = boxes[1];
        return;
    }

    ComputePointBounds(boxes, 2 * count, &node->center, &node->halfExtents, &node->volume, 0);
    ChooseBoxSplit(boxes, centers, count, &split, &axis);
    leftCount = 0;
    rightCount = 0;
    leftBoxes = (TreeVec3*)DebugMalloc(count * 2 * sizeof(TreeVec3), __FILE__, 0x4a9);
    rightBoxes = (TreeVec3*)DebugMalloc(count * 2 * sizeof(TreeVec3), __FILE__, 0x4aa);
    leftCenters = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x4ac);
    rightCenters = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x4ad);
    SplitBoxes(boxes, centers, leftBoxes, rightBoxes, leftCenters, rightCenters, count,
               &leftCount, &rightCount, split, axis);
    for (i = 0; i < leftCount; i++)
        centers[i] = leftCenters[i];
    for (i = 0; i < rightCount; i++)
        centers[leftCount + i] = rightCenters[leftCount + i];
    operator delete(leftBoxes, __FILE__, 0x4be);
    operator delete(rightBoxes, __FILE__, 0x4bf);
    operator delete(leftCenters, __FILE__, 0x4c0);
    operator delete(rightCenters, __FILE__, 0x4c1);

    if (rightCount == 0 || leftCount == 0) {
        leftCount = count / 2;
        rightCount = count - leftCount;
    }
    if (leftCount == 1)
        node->child[0] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeBoxLeaf), __FILE__, 0x4ca);
    else
        node->child[0] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x4cc);
    if (rightCount == 1)
        node->child[1] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeBoxLeaf), __FILE__, 0x4d0);
    else
        node->child[1] = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x4d2);
    BuildBoxNode(node->child[0], boxes, centers, leftCount);
    BuildBoxNode(node->child[1], boxes + 2 * leftCount, centers + leftCount, rightCount);
}

// 0x0042dc90.  Builds a model's box tree: `boxes` holds count boxes as corner pairs in model
// space.  The centres are taken first, then every corner is moved through the inverse of the
// rigid transform `m` (rotation transposed, translation rotated back and negated) before the
// tree is built over them.
void BuildModelBoxTree(BoxTreeNode* root, const TreeMatrix4* m, TreeVec3* boxes, int count)
{
    TreeVec3* centers;
    TreeMatrix4 inverse;
    int i;

    centers = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x4de);
    for (i = 0; i < count; i++)
        centers[i] = (boxes[2 * i] + boxes[2 * i + 1]) * 0.5f;

    inverse = *m;
    InvertRigid(&inverse);
    for (i = 0; i < 2 * count; i++)
        TransformPoint(&boxes[i], boxes[i], &inverse);

    BuildBoxNode(root, boxes, centers, count);
    operator delete(centers, __FILE__, 0x4ef);
}

// 0x0042de90.  Rotation of `angle` radians about the axis (x, y, z) (normalised here), in the
// row-vector convention, with no translation.  Only the rotation block, the zero column/row
// and _44 are written.
void AxisAngleMatrix(TreeMatrix4* m, float x, float y, float z, float angle)
{
    float length;
    float c;
    float s;
    float t;

    m->_14 = 0.0f;
    m->_24 = 0.0f;
    m->_34 = 0.0f;
    m->_41 = 0.0f;
    m->_42 = 0.0f;
    m->_43 = 0.0f;
    length = (float)sqrt(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;
    c = (float)cos(angle);
    s = (float)sin(angle);
    t = 1.0f - c;
    m->_11 = t * x * x + c;
    m->_12 = t * x * y + s * z;
    m->_13 = t * x * z - s * y;
    m->_21 = t * x * y - s * z;
    m->_22 = t * y * y + c;
    m->_23 = t * y * z + s * x;
    m->_31 = t * x * z + s * y;
    m->_32 = t * y * z - s * x;
    m->_33 = t * z * z + c;
    m->_44 = 1.0f;
}

// ---------------------------------------------------------------------------------------------
// Loading.  A node record is one float (>= 0: interior, the value is the box volume; < 0: leaf)
// followed by the node's fields, each read with its own TreeFile::Read call; interior nodes are
// followed by their two subtrees.

// 0x0042dfa0.  Reads a triangle-tree node and its subtrees.
void ReadTriangleNode(BoxTreeNode** out, TreeFile* file)
{
    float flag;

    file->Read(&flag, 4, 1);
    if (flag >= 0.0f) {
        BoxTreeNode* node = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x52c);
        node->volume = flag;
        file->Read(&node->center.x, 4, 1);
        file->Read(&node->center.y, 4, 1);
        file->Read(&node->center.z, 4, 1);
        file->Read(&node->halfExtents.x, 4, 1);
        file->Read(&node->halfExtents.y, 4, 1);
        file->Read(&node->halfExtents.z, 4, 1);
        ReadTriangleNode(&node->child[0], file);
        ReadTriangleNode(&node->child[1], file);
        *out = node;
    } else {
        BoxTreeTriangleLeaf* leaf =
            (BoxTreeTriangleLeaf*)DebugMalloc(sizeof(BoxTreeTriangleLeaf), __FILE__, 0x548);
        leaf->marker = flag;
        file->Read(&leaf->planeOffset, 4, 1);
        file->Read(&leaf->normal.x, 4, 1);
        file->Read(&leaf->normal.y, 4, 1);
        file->Read(&leaf->normal.z, 4, 1);
        file->Read(&leaf->vertex[0], 2, 1);
        file->Read(&leaf->vertex[1], 2, 1);
        file->Read(&leaf->vertex[2], 2, 1);
        *out = (BoxTreeNode*)leaf;
    }
}

// 0x0042e0f0.  Reads a vertex count, the vertices and then the triangle tree over them.
void ReadTriangleMesh(BoxTreeNode** tree, TreeVec3** verts, TreeFile* file)
{
    int count;
    TreeVec3* v;
    int i;

    file->Read(&count, 4, 1);
    v = (TreeVec3*)DebugMalloc(count * sizeof(TreeVec3), __FILE__, 0x567);
    for (i = 0; i < count; i++) {
        file->Read(&v[i].x, 4, 1);
        file->Read(&v[i].y, 4, 1);
        file->Read(&v[i].z, 4, 1);
    }
    *verts = v;
    ReadTriangleNode(tree, file);
}

// 0x0042e190.  Reads a point-tree node and its subtrees.
void ReadPointNode(BoxTreeNode** out, TreeFile* file)
{
    float flag;

    file->Read(&flag, 4, 1);
    if (flag >= 0.0f) {
        BoxTreeNode* node = (BoxTreeNode*)DebugMalloc(sizeof(BoxTreeNode), __FILE__, 0x582);
        node->volume = flag;
        file->Read(&node->center.x, 4, 1);
        file->Read(&node->center.y, 4, 1);
        file->Read(&node->center.z, 4, 1);
        file->Read(&node->halfExtents.x, 4, 1);
        file->Read(&node->halfExtents.y, 4, 1);
        file->Read(&node->halfExtents.z, 4, 1);
        ReadPointNode(&node->child[0], file);
        ReadPointNode(&node->child[1], file);
        *out = node;
    } else {
        BoxTreePointLeaf* leaf =
            (BoxTreePointLeaf*)DebugMalloc(sizeof(BoxTreePointLeaf), __FILE__, 0x59e);
        leaf->marker = flag;
        file->Read(&leaf->point.x, 4, 1);
        file->Read(&leaf->point.y, 4, 1);
        file->Read(&leaf->point.z, 4, 1);
        *out = (BoxTreeNode*)leaf;
    }
}
