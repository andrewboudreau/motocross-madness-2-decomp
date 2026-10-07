// Near-miss BoundingBoxTreeBuild.cpp candidates, kept out of src/reconstructed until they
// match. The canonical file is included first so the types, constants and helpers are the same.

#include "../../../src/reconstructed/BoundingBoxTreeBuild.cpp"

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

    DebugFree(remap, __FILE__, 0x376);
    DebugFree(positions, __FILE__, 0x377);
    DebugFree(keep, __FILE__, 0x378);
    *verts = unique;
    *indices = corners;
    *vertCount = numUnique;
    *triCount = numTris;
    model->SetMatrixIn(0, &saved);
}

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
    DebugFree(points, __FILE__, 0x48a);
}
