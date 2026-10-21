// Native boundary checks execute unchanged production helper bodies. Python checks
// both their production and extracted COFF extents against the complete retail code.
#include <stddef.h>
#define CHECK_LAYOUT(name, expression) typedef char name[(expression) ? 1 : -1]
#ifdef QUERY_LAYOUT
#include "../../src/krusty2/bvh/BoundingBoxTreeQuery.h"
CHECK_LAYOUT(node_size, sizeof(QueryTreeNode) == 0x24);
CHECK_LAYOUT(node_center, offsetof(QueryTreeNode, center) == 4);
CHECK_LAYOUT(node_half, offsetof(QueryTreeNode, halfExtents) == 0x10);
CHECK_LAYOUT(node_children, offsetof(QueryTreeNode, child) == 0x1c);
CHECK_LAYOUT(point_leaf, sizeof(QueryPointLeaf) == 0x10);
CHECK_LAYOUT(segment_leaf, sizeof(QuerySegmentLeaf) == 0x1c);
CHECK_LAYOUT(vector_size, sizeof(Vec3) == 12);
CHECK_LAYOUT(matrix_size, sizeof(Matrix4) == 64);
#else
#include "../../src/krusty2/collision/CollisionShapeTests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
CHECK_LAYOUT(bounds_size, sizeof(CollisionBoxBounds) == 0x24);
CHECK_LAYOUT(bounds_center, offsetof(CollisionBoxBounds, center) == 4);
CHECK_LAYOUT(bounds_half, offsetof(CollisionBoxBounds, halfExtents) == 0x10);
CHECK_LAYOUT(bounds_children, offsetof(CollisionBoxBounds, child) == 0x1c);
CHECK_LAYOUT(hull_size, sizeof(CollisionHullBody) == 0x198);
CHECK_LAYOUT(hull_motion, offsetof(CollisionHullBody, motionTransform) == 8);
CHECK_LAYOUT(hull_relative, offsetof(CollisionHullBody, relativeFrame) == 0x148);
CHECK_LAYOUT(hull_tree, offsetof(CollisionHullBody, triangleTree) == 0x188);
CHECK_LAYOUT(hull_points, offsetof(CollisionHullBody, pointTree) == 0x18c);
CHECK_LAYOUT(hull_vertices, offsetof(CollisionHullBody, vertices) == 0x190);
CHECK_LAYOUT(vector_size, sizeof(CollisionVec3) == sizeof(Vec3) && sizeof(Vec3) == 12);
CHECK_LAYOUT(vector_x, offsetof(CollisionVec3, x) == offsetof(Vec3, x));
CHECK_LAYOUT(vector_y, offsetof(CollisionVec3, y) == offsetof(Vec3, y));
CHECK_LAYOUT(vector_z, offsetof(CollisionVec3, z) == offsetof(Vec3, z));
CHECK_LAYOUT(matrix_size, sizeof(Matrix4) == 64);

static unsigned int actual[32], expected[32];
static int cases;
static void Check(int condition, int line) {
    if (!condition) { printf("collision query boundary failure %d, case %d\n", line, cases); exit(1); }
}
#define REQUIRE(condition) Check((condition), __LINE__)
static unsigned int Next(unsigned int& s) { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
static void Reset(unsigned int seed) {
    static const unsigned int edge[] = {0, 0x80000000, 1, 0x807fffff, 0x3f800000,
        0xbf800000, 0x7f800000, 0xff800000, 0x7fc12345, 0xffc54321};
    for (int i = 0; i < 32; ++i) {
        unsigned int bits = Next(seed);
        actual[i] = i < 10 ? edge[(i + seed) % 10] : ((bits & 0x807fffff) | ((110 + bits % 30) << 23));
    }
    memcpy(expected, actual, sizeof(actual));
}
static void Compare() { REQUIRE(!memcmp(actual, expected, sizeof(actual))); ++cases; }
static CollisionVec3 Negated(const CollisionVec3& v) {
    CollisionVec3 result; result.x = -v.x; result.y = -v.y; result.z = -v.z; return result;
}
static void Matrix(Matrix4& m, int kind) {
    memset(&m, 0, sizeof(m));
    for (int axis = 0; axis < 3; ++axis)
        m.m[axis][(axis + kind) % 3] = (kind & 1) ? -1.0f : 1.0f;
    m._41 = 3; m._42 = -5; m._43 = 7; m._44 = 1;
}
static void Transforms() {
    static const int offsets[] = {2, 3, 4, 5, 6, 12};
    for (unsigned int seed = 1; seed <= 256; ++seed) {
        Matrix4 matrix; Matrix(matrix, seed % 4);
        for (int at = 0; at < 6; ++at) for (int point = 0; point < 2; ++point)
        for (int negate = 0; negate < 2; ++negate) {
            Reset(seed);
            CollisionVec3* input = reinterpret_cast<CollisionVec3*>(actual + 4);
            CollisionVec3* output = reinterpret_cast<CollisionVec3*>(actual + offsets[at]);
            Vec3 value; memcpy(&value, expected + 4, sizeof(value));
            if (negate) { value.x = -value.x; value.y = -value.y; value.z = -value.z; }
            if (point) {
                Vec3TransformPoint(reinterpret_cast<Vec3*>(expected + offsets[at]), value, &matrix);
                if (negate) Vec3TransformPoint(reinterpret_cast<Vec3*>(output), reinterpret_cast<const Vec3&>(Negated(*input)), &matrix);
                else Vec3TransformPoint(reinterpret_cast<Vec3*>(output), reinterpret_cast<const Vec3&>(*input), &matrix);
            } else {
                Vec3TransformNormal(reinterpret_cast<Vec3*>(expected + offsets[at]), value, &matrix);
                if (negate) Vec3TransformNormal(reinterpret_cast<Vec3*>(output), reinterpret_cast<const Vec3&>(Negated(*input)), &matrix);
                else Vec3TransformNormal(reinterpret_cast<Vec3*>(output), reinterpret_cast<const Vec3&>(*input), &matrix);
            }
            Compare();
        }
    }
    // Independent integer-valued result checks distinguish directions from points.
    for (seed = 0; seed < 64; ++seed) for (int point = 0; point < 2; ++point) {
        Matrix4 matrix; Matrix(matrix, seed % 4);
        CollisionVec3 value; value.x = float(int(seed) - 30); value.y = float(20 - int(seed)); value.z = float(seed % 7);
        float result[3];
        for (int axis = 0; axis < 3; ++axis)
            result[axis] = value.x * matrix.m[0][axis] + value.y * matrix.m[1][axis]
                + value.z * matrix.m[2][axis] + (point ? matrix.m[3][axis] : 0.0f);
        if (point) Vec3TransformPoint(reinterpret_cast<Vec3*>(&value), reinterpret_cast<const Vec3&>(value), &matrix);
        else Vec3TransformNormal(reinterpret_cast<Vec3*>(&value), reinterpret_cast<const Vec3&>(value), &matrix);
        REQUIRE(value.x == result[0] && value.y == result[1] && value.z == result[2]); ++cases;
    }
}
static void Bounds() {
    for (unsigned int seed = 1; seed <= 256; ++seed) for (int overlap = 0; overlap < 4; ++overlap)
    for (int input = 0; input < 3; ++input) {
        Reset(seed); Matrix4 matrix; Matrix(matrix, seed % 4);
        int half = overlap == 0 ? 12 : 4 + overlap - 1;
        int centerSource = input == 0 ? 20 : 4;
        int halfSource = input == 2 ? half : 24;
        Vec3 center, extents;
        memcpy(&center, expected + centerSource, 12); memcpy(&extents, expected + halfSource, 12);
        GrowBoxByTransformedBox(reinterpret_cast<Vec3*>(expected + 4), reinterpret_cast<Vec3*>(expected + half), center, extents, &matrix);
        GrowBoxByTransformedBox(reinterpret_cast<Vec3*>(actual + 4), reinterpret_cast<Vec3*>(actual + half),
            reinterpret_cast<const Vec3&>(*reinterpret_cast<CollisionVec3*>(actual + centerSource)),
            reinterpret_cast<const Vec3&>(*reinterpret_cast<CollisionVec3*>(actual + halfSource)), &matrix);
        Compare();
    }
    // Independent union-of-eight-corners oracle: the existing output box matters.
    for (seed = 0; seed < 64; ++seed) for (int kind = 0; kind < 4; ++kind) {
        Matrix4 matrix; Matrix(matrix, kind);
        CollisionVec3 center, half, source, extent;
        center.x = float(seed % 5); center.y = -2; center.z = 4;
        half.x = 2; half.y = 3; half.z = 1;
        source.x = float(int(seed) - 32); source.y = 5; source.z = -3;
        extent.x = 1; extent.y = float(seed % 4); extent.z = 2;
        float lo[3] = {center.x-half.x, center.y-half.y, center.z-half.z};
        float hi[3] = {center.x+half.x, center.y+half.y, center.z+half.z};
        for (int corner = 0; corner < 8; ++corner) {
            float p[3] = {source.x + ((corner & 1) ? extent.x : -extent.x),
                source.y + ((corner & 2) ? extent.y : -extent.y), source.z + ((corner & 4) ? extent.z : -extent.z)};
            for (int axis = 0; axis < 3; ++axis) {
                float v = p[0]*matrix.m[0][axis] + p[1]*matrix.m[1][axis] + p[2]*matrix.m[2][axis] + matrix.m[3][axis];
                if (v < lo[axis]) lo[axis] = v; if (v > hi[axis]) hi[axis] = v;
            }
        }
        GrowBoxByTransformedBox(reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&half),
            reinterpret_cast<const Vec3&>(source), reinterpret_cast<const Vec3&>(extent), &matrix);
        for (int axis = 0; axis < 3; ++axis) {
            REQUIRE((&center.x)[axis] == (lo[axis]+hi[axis])*0.5f);
            REQUIRE((&half.x)[axis] == (hi[axis]-lo[axis])*0.5f);
        }
        ++cases;
    }
}
static void Spheres() {
    Matrix4 matrix; Matrix(matrix, 0); matrix._41 = matrix._42 = matrix._43 = 0;
    for (int x = -8; x <= 8; ++x) for (int y = -5; y <= 5; ++y) for (int r = 0; r < 5; ++r) {
        CollisionVec3 center, half, sphere;
        center.x = center.y = center.z = 0; half.x = 3; half.y = 2; half.z = 1;
        sphere.x = float(x); sphere.y = float(y); sphere.z = 2;
        int dx = abs(x)-3, dy = abs(y)-2; if (dx < 0) dx = 0; if (dy < 0) dy = 0;
        int wanted = r*r > dx*dx + dy*dy + 1; // Retail excludes tangency.
        int result = BoxSphereOverlap(reinterpret_cast<const Vec3*>(&center), reinterpret_cast<const Vec3*>(&half),
            reinterpret_cast<const Vec3&>(sphere), float(r), float(r*r), &matrix);
        REQUIRE(result == wanted); ++cases;
    }
}
int main() {
    Transforms(); Bounds(); Spheres();
    printf("Collision query boundaries: %d native cases passed\n", cases); return 0;
}
#endif
