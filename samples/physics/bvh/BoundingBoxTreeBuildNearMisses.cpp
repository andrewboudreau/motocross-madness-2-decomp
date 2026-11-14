// Near-miss BoundingBoxTreeBuild.cpp candidates, kept out of src/reconstructed until they
// match. The canonical file is included first so the types, constants and helpers are the same.

#include "../../../src/reconstructed/BoundingBoxTreeBuild.cpp"

// 0x0042c260.  Collects the triangles of `model`'s current LOD for the triangle tree.  The model
// is put at the origin with identity axes first (and its matrix restored at the end), so the
// positions come out in model space.  With a `frame`, only triangles whose three corners lie
// in that frame's vertex range are taken, with positions from the mesh's +0x14 array.  Every
// corner gets its own position, then equal positions are merged: the result is *verts
// (*vertCount unique positions) and *indices (three per triangle, *triCount triangles).
// PARTIAL 1626/1627: same size and instructions apart from one ModRM/SIB byte: the remap pass
// reads keep[corners[k]] as [keep + offset] where VC6 gives [offset + keep].  The loop-variable
// names of the merge passes (j outer, k inner, i for the copy) are the ones that put every local
// in retail's stack slot; other choices only move slots.
// Round 9: `remap = 0;` at the top of the function is what gives retail's reload order before
// the 0x348 allocation (numCorners, then remap; without it VC6 reloads remap first, 1622).
// Clearing any other single local there is inert or worse (keep 1614, k/i 1622, the rest
// 1618), and clearing keep as well, or numCorners before remap, loses it again (1622).  With it,
// for the remaining byte: any second local cleared after remap, the remap pass on i/j/n/t/
// first/end/inside/numTaken, a while loop, keep declared before remap, `!= 0`, `*(keep + c)`,
// `corners[k][keep]`, `*&keep[...]` and an `int* slot = &keep[corners[k]]` local are all 1626;
// `== 1` is 1624, `> 0` 1625, the negated test 1609, `n = remap[...]; if (!keep[...]) n =
// remap[n];` 1481, a ternary 1472 and a descending loop 1378.
// The older sessions below were measured without `remap = 0;`.
// Session 14: for the remap read, *(keep + c), corners[k][keep], != 0, a corner local (t, n, i
// or first), a ternary and the negated test leave [offset + keep] or reshape the pass (57+ lines).
// For the reload order, numCorners set just before the 0x348 allocation (from numTris, n or
// numTaken) or numTris * 3 as the size reshape the function (300+); declaration order does not
// matter.  Both spots look like register-allocation choices with no source lever found.
// Session 15: walking the corners with a pointer local (`int* corner = corners; ...
// keep[*corner]`) in the remap pass keeps [offset + keep] and reshapes the pass (1370/1623).
// Session 22: `numCorners = numTris * 3` moved after the corners, remap or positions allocation or
// after `n = 0` is worse (1591, 1568, 1523, 401).
// Remap-pass index casts ((unsigned), (unsigned long), (long), (int)) and keep/remap declared as
// unsigned int*, long* or unsigned long* are inert (1622); (unsigned short) is 1467.  An inline
// remap helper is 1622 as a for loop, 1470 with a corner local and 1370 as a pointer walk.
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

    remap = 0;
    numTris = 0;
    reinterpret_cast<SoultreeObject*>(model)->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&saved));
    model->SetAxesIn(0, &kVec3ZAxis, &kVec3YAxis, 0, 1);
    model->SetPositionIn(0, &kVec3Zero);
    savedField = model->useLighting;
    model->useLighting = 0;
    model->MarkSubtreeDirty();
    reinterpret_cast<D3DIMSoultreeObject*>(model)->TransformVertexGroups(-1);
    model->useLighting = savedField;
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
            source = mesh->drawnVertices;
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

    DebugFree(remap, __FILE__, 0x376);
    DebugFree(positions, __FILE__, 0x377);
    DebugFree(keep, __FILE__, 0x378);
    *verts = unique;
    *indices = corners;
    *vertCount = numUnique;
    *triCount = numTris;
    model->SetMatrixIn(0, &saved);
}
