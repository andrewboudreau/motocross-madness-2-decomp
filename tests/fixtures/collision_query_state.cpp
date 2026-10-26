// Executes unchanged production setter and tree-query bodies, compiled in separate
// units. Python first verifies their complete retail bytes and relocation targets.
#include "../../src/krusty2/collision/CollisionShapeTests.h"
#include "../../src/krusty2/bvh/BoundingBoxTreeQuery.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK_LAYOUT(name, expression) typedef char name[(expression) ? 1 : -1]
CHECK_LAYOUT(hit_size, sizeof(QueryHit) == 20 && sizeof(CollisionBoxResult) == 20);
CHECK_LAYOUT(hit_fraction, offsetof(QueryHit, fraction) == offsetof(CollisionBoxResult, fraction));
CHECK_LAYOUT(hit_pointer, offsetof(QueryHit, point) == 4 && offsetof(CollisionBoxResult, point) == 4);
CHECK_LAYOUT(hit_normal, offsetof(QueryHit, normal) == 8 && offsetof(CollisionBoxResult, normal) == 8);
CHECK_LAYOUT(pointer_size, sizeof(g_CollisionBoxResult) == 4);
CHECK_LAYOUT(vector_stride, sizeof(Vec3) == 12 && sizeof(CollisionVec3) == 12);
void SetCollisionBoxResult(CollisionBoxResult* record);

struct GuardedRecord { unsigned int before[4]; CollisionBoxResult hit; unsigned int after[4]; };
static int cases;
static void Check(int condition, int line) {
    if (!condition) { printf("query state failure at %d, case %d\n", line, cases); exit(1); }
}
#define REQUIRE(condition) Check((condition), __LINE__)
static void InitLeaf(QueryTriangleLeaf& leaf, int z, int sign, int scale, int vertexBase) {
    memset(&leaf, 0, sizeof(leaf)); leaf.marker = -1;
    leaf.tri.vertex[0] = vertexBase + (sign > 0 ? 0 : 1);
    leaf.tri.vertex[1] = vertexBase + (sign > 0 ? 1 : 0);
    leaf.tri.vertex[2] = vertexBase + 2;
    leaf.tri.normal.z = float(sign * scale);
    leaf.tri.planeOffset = float(-z * sign * scale);
}
int main() {
    // Setter null and redirection retain the exact caller address. No record is
    // constructed or dereferenced by the production setter itself.
    SetCollisionBoxResult(0); REQUIRE(g_CollisionBoxResult == 0);
    GuardedRecord records[2], wanted, untouched;
    static const float limits[] = {-1, 0, 0.125f, 0.25f, 0.5f, 1, 2};
    for (int x = -8; x <= 8; x += 2) for (int y = -6; y <= 6; y += 2)
    for (int limit = 0; limit < 7; ++limit) for (int direction = 0; direction < 2; ++direction)
    for (int normal = 0; normal < 4; ++normal) for (int mode = 0; mode < 4; ++mode)
    for (int array = 0; array < 2; ++array) for (int translated = 0; translated < 2; ++translated) {
        int sign = (normal & 1) ? -1 : 1, scale = (normal & 2) ? 2 : 1;
        int shift = array ? 12 : 0;
        Vec3 banks[2][6];
        for (int bank = 0; bank < 2; ++bank) for (int layer = 0; layer < 2; ++layer) {
            int base = layer * 3, offset = bank * 12, z = layer ? -1 : 1;
            banks[bank][base] = Vec3(float(offset - 4), -4, float(z));
            banks[bank][base+1] = Vec3(float(offset + 4), -4, float(z));
            banks[bank][base+2] = Vec3(float(offset), 4, float(z));
        }
        Vec3* vertices = banks[array];
        QueryTriangleLeaf nearLeaf, farLeaf;
        InitLeaf(nearLeaf, 1, sign, scale, 0); InitLeaf(farLeaf, -1, sign, scale, 3);
        QueryTreeNode tree;
        memset(&tree, 0, sizeof(tree)); tree.volume = 1;
        tree.center.x = float(shift); tree.halfExtents = Vec3(4, 4, 10);
        tree.child[0] = reinterpret_cast<QueryTreeNode*>(mode == 2 ? &farLeaf : &nearLeaf);
        tree.child[1] = reinterpret_cast<QueryTreeNode*>(mode == 3 ? &nearLeaf : (mode == 2 ? &nearLeaf : &farLeaf));
        QueryTreeNode* root = mode == 0 ? reinterpret_cast<QueryTreeNode*>(&nearLeaf) : &tree;
        Matrix4 matrix; memset(&matrix, 0, sizeof(matrix));
        matrix._11 = matrix._22 = matrix._33 = matrix._44 = 1;
        matrix._41 = translated ? 3.0f : 0.0f; matrix._43 = translated ? 1.0f : 0.0f;
        CollisionVec3 ends[2];
        ends[0].x = ends[1].x = float(x + shift); ends[0].y = ends[1].y = float(y);
        ends[0].z = direction ? -4.0f : 4.0f; ends[1].z = -ends[0].z;
        memset(records, 0xa5, sizeof(records));
        int selected = cases & 1;
        records[selected].hit.fraction = limits[limit];
        records[selected].hit.point = reinterpret_cast<const CollisionVec3*>(0x12345678);
        wanted = records[selected]; untouched = records[1-selected];
        int wantedHit = 0;
        float start = ends[0].z + matrix._43, dz = ends[1].z - ends[0].z;
        int localX = x + (translated ? 3 : 0);
        bool inside = y >= -4 && y <= 4 && 2*localX+y <= 4 && -2*localX+y <= 4;
        int layers[2] = {mode == 2 ? -1 : 1, mode == 3 || mode == 2 ? 1 : -1};
        for (int leaf = 0; leaf < (mode == 0 ? 1 : 2); ++leaf) {
            float t = (float(layers[leaf]) - start) / dz;
            if (inside && dz * float(sign) < 0 && t >= 0 && t <= 1 && t <= wanted.hit.fraction) {
                wantedHit = 1; wanted.hit.fraction = t; wanted.hit.point = ends;
                wanted.hit.normal.x = wanted.hit.normal.y = 0;
                wanted.hit.normal.z = float(sign * scale);
            }
        }
        unsigned char savedBanks[sizeof(banks)], savedTree[sizeof(tree)], savedLeaves[sizeof(nearLeaf)+sizeof(farLeaf)];
        unsigned char savedMatrix[sizeof(matrix)], savedEnds[sizeof(ends)];
        memcpy(savedBanks, banks, sizeof(banks)); memcpy(savedTree, &tree, sizeof(tree));
        memcpy(savedLeaves, &nearLeaf, sizeof(nearLeaf)); memcpy(savedLeaves+sizeof(nearLeaf), &farLeaf, sizeof(farLeaf));
        memcpy(savedMatrix, &matrix, sizeof(matrix)); memcpy(savedEnds, ends, sizeof(ends));
        SetCollisionBoxResult(&records[selected].hit);
        REQUIRE(g_CollisionBoxResult == reinterpret_cast<QueryHit*>(&records[selected].hit));
        int hit = SegmentTreeQueryWithVertices(reinterpret_cast<const Vec3*>(ends), root, &matrix, vertices);
        REQUIRE(hit == wantedHit);
        REQUIRE(!memcmp(&records[selected], &wanted, sizeof(wanted)));
        REQUIRE(!memcmp(&records[1-selected], &untouched, sizeof(untouched)));
        REQUIRE(!memcmp(savedBanks, banks, sizeof(banks)) && !memcmp(savedTree, &tree, sizeof(tree)));
        REQUIRE(!memcmp(savedLeaves, &nearLeaf, sizeof(nearLeaf)) && !memcmp(savedLeaves+sizeof(nearLeaf), &farLeaf, sizeof(farLeaf)));
        REQUIRE(!memcmp(savedMatrix, &matrix, sizeof(matrix)) && !memcmp(savedEnds, ends, sizeof(ends)));
        REQUIRE(g_CollisionBoxResult == reinterpret_cast<QueryHit*>(&records[selected].hit));
        ++cases;
    }
    SetCollisionBoxResult(0); REQUIRE(g_CollisionBoxResult == 0);
    printf("Collision query state: %d native producer/consumer cases passed\n", cases);
    return 0;
}
