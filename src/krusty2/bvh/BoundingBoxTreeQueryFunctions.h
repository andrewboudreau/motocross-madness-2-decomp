#pragma once

// Query entry points, shared without importing the query TU's
// inline arithmetic helpers or per-TU vector constants into collision callers.
struct QueryTreeNode;
struct Vec3;
struct Matrix4;
void FreeBoxTree(QueryTreeNode* node); // 0x0042a160, recursive postorder destruction

// The final matrix is optional motion input, not a result buffer. Vertices are
// the mutable vertex array addressed by triangle-leaf indices and retained by
// the query; a nonzero mode selects leaf handling.
int TreeTreeQuery(QueryTreeNode* a, QueryTreeNode* b, const Matrix4* xfA, const Matrix4* xfB,
                  int mode, Vec3* vertices, const Matrix4* motion);                // 0x00428950
int TreeBoxOverlap(QueryTreeNode* a, QueryTreeNode* b, const Matrix4* xfA,
                   const Matrix4* xfB);                                            // 0x00428c20
int SphereTreeQuery(const Vec3* center, float radius, float radiusSq, QueryTreeNode* node,
                    const Matrix4* xf, int triangles);                             // 0x00429570
int SphereTreeQueryWithVertices(const Vec3* center, float radius, float radiusSq,
                                QueryTreeNode* node, const Matrix4* xf, int triangles,
                                Vec3* vertices);                                   // 0x00429540
int CapsuleTreeQueryWithVertices(const Vec3* ends, float radius, float radiusSq,
                                 QueryTreeNode* node, const Matrix4* xf, int triangles,
                                 Vec3* vertices);                                  // 0x00429890

int SweptBoxOverlap(Vec3 aCenter, Vec3 aHalf, const Vec3* bCenter, const Vec3* bHalf,
                    const Matrix4* rel, const Matrix4* xfA);                       // 0x00424730
int BoxSphereOverlap(const Vec3* center, const Vec3* halfExtents, Vec3 sphereCenter,
                     float radius, float radiusSq, const Matrix4* xf);             // 0x00425750
// The two pointer arguments supply the existing bounds and receive their union
// with the transformed by-value box. They are not uninitialized output storage.
void GrowBoxByTransformedBox(Vec3* outCenter, Vec3* outHalfExtents, Vec3 center,
                             Vec3 halfExtents, const Matrix4* m);                  // 0x004290d0
void Vec3TransformNormal(Vec3* out, Vec3 v, const Matrix4* m);                     // 0x0042a450
void Vec3TransformPoint(Vec3* out, Vec3 v, const Matrix4* m);                      // 0x0042a510
