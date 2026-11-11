// Bring-up definitions (best-guess-unreviewed) for the visibility clipper
// 0x00575a98 (VisibilityQuadTree.cpp's code range 0x0052f080..0x0052fdc0).
//
//  * VisibilityClipper::TestBox (0x0052f570, ret 0x1c) written from the
//    decoded retail function. Retail rounds the screen rectangle with an
//    inline x87 fistp helper (current rounding mode, round-to-nearest-even);
//    X87FloatToInt reproduces it without inline assembly.
//  * UnknownEcoVisibilityClipper::SphereInFrustum (0x0052fbb0) is
//    EcoSystem.h's boundary view of VisibilityClipper::SphereInFrustum,
//    which src/krusty2/visibility/VisibilityQuadTree.cpp already defines; the
//    view forwards to it.
#include <math.h>

#include "visibility/VisibilityQuadTree.h"
#include "X87FloatToInt.h"

// The eight clip-space box corners (0x0068abc8, 8 x 0x20 bytes). TestBox
// writes them and then turns the first two floats of each into screen
// coordinates in place. Only TestBox touches them in this unit.
static VisibilityClipPoint s_group4BoxCorners[8];

// Outcode of a clip-space corner against 0 <= x, y, z <= w:
// x 1/2, y 4/8, z 0x10/0x20 (below / above). Inline in retail, which tests
// y first here and compares with the double 0.0 at 0x005507e8.
static inline unsigned int Group4Vis_OutCode(float x, float y, float z, float w) {
    unsigned int code = 0;
    if (y < 0.0)
        code = 4;
    else if (w - y < 0.0)
        code = 8;
    if (x < 0.0)
        code |= 1;
    else if (w - x < 0.0)
        code |= 2;
    if (z < 0.0)
        code |= 0x10;
    else if (w - z < 0.0)
        code |= 0x20;
    return code;
}

// The viewport origin the screen rectangle is offset by (camera +0x1a0 /
// +0x1a4, inside VisibilityCamera::field_0x17c).
static inline int Group4Vis_ViewportX(const VisibilityCamera* camera) {
    return *(const int*)((const char*)camera + 0x1a0);
}
static inline int Group4Vis_ViewportY(const VisibilityCamera* camera) {
    return *(const int*)((const char*)camera + 0x1a4);
}

// 0x0052f570. Box (center +- extent) against the frustum of `matrix`
// (row-major 4x4, row vector times matrix). Returns 0 when all eight corners
// are outside one plane (the rectangle, when given, is cleared). With
// neither `screenRect` nor `cornersInside`, a centre or any corner inside
// accepts at once. Otherwise `screenRect` receives the clamped screen
// bounds {left, top, right, bottom} and `cornersInside` the number of
// corners inside the frustum. The last argument is unused.
int VisibilityClipper::TestBox(VisibilityCamera* camera, const float* matrix, const float* center,
                               const float* extent, int* screenRect, int* cornersInside, int unused) {
    const float* m = matrix;
    int inside = 0;
    int i;
    if (!screenRect && !cornersInside) {
        // Trivial accept when the centre is inside. Retail sums each row z term
        // first and tests x, then y, then z on the unrounded sums.
        float w = m[11] * center[2] + m[3] * center[0] + m[7] * center[1] + m[15];
        float x = m[8] * center[2] + m[4] * center[1] + m[0] * center[0] + m[12];
        unsigned int code = 0;
        if (x < 0.0)
            code = 1;
        else if (w - x < 0.0)
            code = 2;
        float y = m[9] * center[2] + m[1] * center[0] + m[5] * center[1] + m[13];
        if (y < 0.0)
            code |= 4;
        else if (w - y < 0.0)
            code |= 8;
        float z = m[10] * center[2] + m[2] * center[0] + m[6] * center[1] + m[14];
        if (z < 0.0)
            code |= 0x10;
        else if (w - z < 0.0)
            code |= 0x20;
        if (code == 0)
            return 1;
    }

    // Per-axis products of the low/high box faces with the matrix rows
    // ([column][low/high]), in retail's straight-line order.
    float lowX = center[0] - extent[0];
    float lowY = center[1] - extent[1];
    float lowZ = center[2] - extent[2];
    float highX = center[0] + extent[0];
    float highY = center[1] + extent[1];
    float highZ = center[2] + extent[2];
    float px[4][2], py[4][2], pz[4][2];
    px[0][0] = lowX * m[0];
    px[1][0] = lowX * m[1];
    px[2][0] = lowX * m[2];
    px[3][0] = lowX * m[3];
    px[0][1] = highX * m[0];
    px[1][1] = highX * m[1];
    px[2][1] = highX * m[2];
    px[3][1] = highX * m[3];
    py[0][0] = lowY * m[4];
    py[1][0] = lowY * m[5];
    py[2][0] = lowY * m[6];
    py[3][0] = lowY * m[7];
    py[0][1] = highY * m[4];
    py[1][1] = highY * m[5];
    py[2][1] = highY * m[6];
    py[3][1] = highY * m[7];
    pz[0][0] = lowZ * m[8];
    pz[1][0] = lowZ * m[9];
    pz[2][0] = lowZ * m[10];
    pz[3][0] = lowZ * m[11];
    pz[0][1] = highZ * m[8];
    pz[1][1] = highZ * m[9];
    pz[2][1] = highZ * m[10];
    pz[3][1] = highZ * m[11];

    unsigned int common = 0xffffffff;
    for (i = 0; i < 8; i++) {
        int a = i & 1;
        int b = (i >> 1) & 1;
        int c = i >> 2;
        VisibilityClipPoint* corner = &s_group4BoxCorners[i];
        corner->w = px[3][a] + py[3][b] + pz[3][c] + m[15];
        corner->x = px[0][a] + py[0][b] + pz[0][c] + m[12];
        corner->y = px[1][a] + py[1][b] + pz[1][c] + m[13];
        corner->z = px[2][a] + py[2][b] + pz[2][c] + m[14];
        unsigned int code = Group4Vis_OutCode(corner->x, corner->y, corner->z, corner->w);
        if (code == 0) {
            if (!screenRect && !cornersInside)
                return 1;
            inside++;
        }
        common &= code;
    }

    if (common) {
        if (screenRect) {
            screenRect[0] = 0;
            screenRect[2] = 0;
            screenRect[1] = 0;
            screenRect[3] = 0;
        }
        return 0;
    }

    if (screenRect) {
        unsigned int width = camera->viewportWidth;
        unsigned int height = camera->viewportHeight;
        VisibilityClipPoint* corner = &s_group4BoxCorners[0];
        float inverse = 1.0f / corner->w;
        corner->x = (float)width * inverse * corner->x;
        corner->y = (float)height * inverse * corner->y;
        screenRect[0] = X87FloatToInt(corner->x);
        screenRect[1] = X87FloatToInt(corner->y);
        screenRect[3] = screenRect[1];
        screenRect[2] = screenRect[0];
        for (i = 1; i < 8; i++) {
            corner = &s_group4BoxCorners[i];
            inverse = 1.0f / corner->w;
            corner->x = (float)width * inverse * corner->x;
            corner->y = (float)height * corner->y * inverse;
            if ((float)screenRect[0] > corner->x)
                screenRect[0] = X87FloatToInt(corner->x);
            else if ((float)screenRect[2] < corner->x)
                screenRect[2] = X87FloatToInt(corner->x);
            if ((float)screenRect[1] > corner->y)
                screenRect[1] = X87FloatToInt(corner->y);
            else if ((float)screenRect[3] < corner->y)
                screenRect[3] = X87FloatToInt(corner->y);
        }
        // Clamp to the viewport (signed compares, as retail) and offset by
        // its origin.
        int left = screenRect[0] > 0 ? screenRect[0] : 0;
        int right = screenRect[2] < (int)width ? screenRect[2] : (int)width;
        int top = screenRect[1] > 0 ? screenRect[1] : 0;
        int bottom = screenRect[3] < (int)height ? screenRect[3] : (int)height;
        screenRect[0] = Group4Vis_ViewportX(camera) + left;
        screenRect[2] = Group4Vis_ViewportX(camera) + right;
        screenRect[1] = Group4Vis_ViewportY(camera) + top;
        screenRect[3] = Group4Vis_ViewportY(camera) + bottom;
    }
    if (cornersInside)
        *cornersInside = inside;
    return 1;
}

// EcoSystem.h's boundary view of the same clipper (class keyword and
// parameter types as declared there, so the mangled name matches its calls).
class UnknownEcoCamera;
struct Vector3;
class UnknownEcoVisibilityClipper {
public:
    int SphereInFrustum(const UnknownEcoCamera* camera, const void* matrix, const Vector3* center,
                        float radius, int* fullyInside);
};

// 0x0052fbb0: the EcoSystem's camera is the view's Camera record that
// VisibilityCamera describes; the matrix and centre have the same layouts.
int UnknownEcoVisibilityClipper::SphereInFrustum(const UnknownEcoCamera* camera, const void* matrix,
                                                 const Vector3* center, float radius,
                                                 int* fullyInside) {
    return reinterpret_cast<VisibilityClipper*>(this)->SphereInFrustum(
        reinterpret_cast<const VisibilityCamera*>(camera), static_cast<const VisibilityMatrix*>(matrix),
        reinterpret_cast<const VisibilityBoxVec*>(center), radius, fullyInside);
}
