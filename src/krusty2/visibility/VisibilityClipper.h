#ifndef VISIBILITY_CLIPPER_H
#define VISIBILITY_CLIPPER_H

struct VisibilityCamera;
struct VisibilityBoxVec;
struct VisibilityMatrix;
struct VisibilityVertex;
struct VisibilityCullPoint;

// Tier 3.  The object at 0x00575a98: every caller loads ecx from that pointer and the
// methods are thiscall (ret N), but none of them reads its fields; CullQuad keeps ecx intact
// to pass it on to CullPolygon.  The methods lie in this unit's code between the second empty
// static (0x0052f080) and the Math3D.h vector initializers (0x0052fdc0), so they are
// VisibilityQuadTree.cpp's (strong inference: the unit's code range).
//  * TestBox 0x0052f570 (ret 0x1c) takes the camera, a matrix (camera + 0xec), a box centre
//    and half extents (3 floats each), an optional screen rect (always 0 here) and returns
//    the number of box corners inside the frustum through its sixth argument.  The result is
//    0 when all 8 corners fail one plane.  It rounds with inline x87 code (excluded).
class VisibilityClipper {
public:
    int TestBox(VisibilityCamera* camera, const float* matrix, const float* center,
                const float* extent, int* screenRect, int* cornersInside, int unused);
    void TransformVectors(const VisibilityBoxVec* src, VisibilityBoxVec* dst,
                          const VisibilityMatrix* m, int count);               // 0x0052f0a0
    int TestDot(const VisibilityCamera* camera, const VisibilityBoxVec* a,
                const VisibilityBoxVec* b);                                    // 0x0052f140
    void ProjectVertices(const VisibilityCamera* camera, const VisibilityMatrix* m, int count,
                         const VisibilityVertex* vertices, VisibilityBoxVec* out,
                         int* codes);                                          // 0x0052f190
    int ProjectPoint(const VisibilityCamera* camera, const VisibilityMatrix* m,
                     const VisibilityBoxVec* p, VisibilityBoxVec* screen,
                     unsigned int* outCode);                                   // 0x0052f340
    int CullQuad(const VisibilityCamera* camera, const VisibilityBoxVec* points, int i0,
                 int i1, int i2, int i3);                                      // 0x0052f4d0
    int CullPolygon(const VisibilityCamera* camera, VisibilityCullPoint* points,
                    int count);                                                // 0x0052fac0
    int SphereInFrustum(const VisibilityCamera* camera, const VisibilityMatrix* m,
                        const VisibilityBoxVec* center, float radius,
                        int* fullyInside);                                     // 0x0052fbb0
};

#endif
