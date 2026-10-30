// Group0_GatherModelTriangles.cpp -- bring-up body for GatherModelTriangles
// (0x0042c260), the one BoundingBoxTreeBuild.cpp function still missing from
// src/reconstructed/BoundingBoxTreeBuild.cpp. Extracted unchanged from
// samples/physics/bvh/BoundingBoxTreeBuildNearMisses.cpp (that sample reports
// the same size and instructions as retail apart from two register-allocation
// spots), with __FILE__ replaced by the retail string at 0x005682bc. The
// includes are BoundingBoxTreeBuild.cpp's own. The scene-node calls go
// through Group0_SoultreeBridge: BoundingBoxTreeBuild.h's TreeSceneNode
// stand-in methods are declared only, the SoultreeObject ones are defined.
#include <math.h>
#include <string.h>

#include "BoundingBoxTreeBuild.h"
#include "TextureMap.h"
#include "MatrixUtil.h"
#include "D3DIMSoulTree.h"
#include "Group0_SoultreeBridge.h"

static const char kBoundingBoxTreeBuildFile[] = "D:\\aardvark\\VC\\krusty2\\BoundingBoxTreeBuild.cpp";

// 0x0042c260. Collects the triangles of `model`'s current LOD for the
// triangle tree (model space; only the triangles inside `frame`'s vertex range
// when a frame is given), merges equal positions and returns the unique
// positions and three indices per triangle.
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
    reinterpret_cast<SoultreeObject*>(model)->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&saved));
    Group0_SoultreeSetAxesIn(model, 0, &kVec3ZAxis.x, &kVec3YAxis.x, 0, 1);
    Group0_SoultreeSetPositionIn(model, 0, &kVec3Zero.x);
    savedField = model->field_0x1a4;
    model->field_0x1a4 = 0;
    Group0_SoultreeMarkSubtreeDirty(model);
    reinterpret_cast<D3DIMSoultreeObject*>(model)->TransformVertexGroups(-1);
    model->field_0x1a4 = savedField;
    Group0_SoultreeMarkSubtreeDirty(model);

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
    corners = (int*)DebugMalloc(numTris * 3 * sizeof(int), kBoundingBoxTreeBuildFile, 0x2fc);
    remap = (int*)DebugMalloc(numTris * 3 * sizeof(int), kBoundingBoxTreeBuildFile, 0x2fd);
    positions = (TreeVec3*)DebugMalloc(numTris * 3 * sizeof(TreeVec3), kBoundingBoxTreeBuildFile, 0x2fe);

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
    keep = (int*)DebugMalloc(numCorners * sizeof(int), kBoundingBoxTreeBuildFile, 0x348);
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

    unique = (TreeVec3*)DebugMalloc(numUnique * sizeof(TreeVec3), kBoundingBoxTreeBuildFile, 0x361);
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

    DebugFree(remap, kBoundingBoxTreeBuildFile, 0x376);
    DebugFree(positions, kBoundingBoxTreeBuildFile, 0x377);
    DebugFree(keep, kBoundingBoxTreeBuildFile, 0x378);
    *verts = unique;
    *indices = corners;
    *vertCount = numUnique;
    *triCount = numTris;
    Group0_SoultreeSetMatrixIn(model, 0, &saved);
}
