// Bring-up definitions (best-guess-unreviewed) for the visibility clipper
// 0x00575a98 (VisibilityQuadTree.cpp's code range 0x0052f080..0x0052fdc0).
//
//  * VisibilityClipper::TestBox (0x0052f570, ret 0x1c) written from the
//    decoded retail function. Retail rounds the screen rectangle with an
//    inline x87 fistp helper (current rounding mode, round-to-nearest);
//    best guess: Group4Vis_RoundNearest reproduces round-half-even without
//    inline assembly.
//  * UnknownEcoVisibilityClipper::SphereInFrustum (0x0052fbb0) is
//    EcoSystem.h's boundary view of VisibilityClipper::SphereInFrustum,
//    which src/krusty2/visibility/VisibilityQuadTree.cpp already defines; the
//    view forwards to it.
#include <math.h>

#include "visibility/VisibilityQuadTree.h"

// The eight clip-space box corners (0x0068abc8, 8 x 0x20 bytes). TestBox
// writes them and then turns the first two floats of each into screen
// coordinates in place. Only TestBox touches them in this unit.
static VisibilityClipPoint s_group4BoxCorners[8];

// Round to nearest, ties to even (the x87 default mode retail's fistp uses).
static int Group4Vis_RoundNearest(float value) {
    double floorValue = floor((double)value);
    double fraction = (double)value - floorValue;
    int result = (int)floorValue;
    if (fraction > 0.5 || (fraction == 0.5 && (result & 1)))
        result++;
    return result;
}

// Outcode of a clip-space point against 0 <= x, y, z <= w:
// x 1/2, y 4/8, z 0x10/0x20 (below / above).
static unsigned int Group4Vis_OutCode(float x, float y, float z, float w) {
    unsigned int code = 0;
    if (x < 0.0f)
        code = 1;
    else if (w - x < 0.0f)
        code = 2;
    if (y < 0.0f)
        code |= 4;
    else if (w - y < 0.0f)
        code |= 8;
    if (z < 0.0f)
        code |= 0x10;
    else if (w - z < 0.0f)
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
        float w = m[3] * center[0] + m[7] * center[1] + m[11] * center[2] + m[15];
        float x = m[0] * center[0] + m[4] * center[1] + m[8] * center[2] + m[12];
        float y = m[1] * center[0] + m[5] * center[1] + m[9] * center[2] + m[13];
        float z = m[2] * center[0] + m[6] * center[1] + m[10] * center[2] + m[14];
        if (Group4Vis_OutCode(x, y, z, w) == 0)
            return 1;
    }

    // Per-axis products of the low/high box faces with the matrix rows.
    float lowX = center[0] - extent[0];
    float lowY = center[1] - extent[1];
    float lowZ = center[2] - extent[2];
    float highX = center[0] + extent[0];
    float highY = center[1] + extent[1];
    float highZ = center[2] + extent[2];
    float px[4][2], py[4][2], pz[4][2];
    for (i = 0; i < 4; i++) {
        px[i][0] = lowX * m[i];
        px[i][1] = highX * m[i];
        py[i][0] = lowY * m[4 + i];
        py[i][1] = highY * m[4 + i];
        pz[i][0] = lowZ * m[8 + i];
        pz[i][1] = highZ * m[8 + i];
    }

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
        screenRect[0] = Group4Vis_RoundNearest(corner->x);
        screenRect[1] = Group4Vis_RoundNearest(corner->y);
        screenRect[3] = screenRect[1];
        screenRect[2] = screenRect[0];
        for (i = 1; i < 8; i++) {
            corner = &s_group4BoxCorners[i];
            inverse = 1.0f / corner->w;
            corner->x = (float)width * inverse * corner->x;
            corner->y = (float)height * corner->y * inverse;
            if ((float)screenRect[0] > corner->x)
                screenRect[0] = Group4Vis_RoundNearest(corner->x);
            else if ((float)screenRect[2] < corner->x)
                screenRect[2] = Group4Vis_RoundNearest(corner->x);
            if ((float)screenRect[1] > corner->y)
                screenRect[1] = Group4Vis_RoundNearest(corner->y);
            else if ((float)screenRect[3] < corner->y)
                screenRect[3] = Group4Vis_RoundNearest(corner->y);
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
