// Bring-up definitions (best-guess-unreviewed) for the visibility clipper
// 0x00575a98 (VisibilityQuadTree.cpp's code range 0x0052f080..0x0052fdc0).
//
//  * VisibilityClipper::TestBox (0x0052f570, ret 0x1c) written from the
//    decoded retail function. Retail rounds the screen rectangle with an
//    inline x87 fistp helper (current rounding mode, round-to-nearest-even;
//    0x0052f92a .. 0x0052fa17).  The game build compiles this unit with
//    /QIfist (config/game_build.json bringup_flags), so
//    FistpFloatToIntChecked's cast is a FISTP under the current rounding mode;
//    the six roundings are the unit's only float-to-int conversions.  The
//    range check stays: a box corner near or behind the eye plane (small or
//    negative w) projects outside int, or to an infinity or NaN when w is 0,
//    where retail's 32-bit store gives 0x80000000 and a bare 64-bit store
//    would keep a different low dword.
//    Compared with retail instruction by instruction: the outcodes, the
//    trivial accepts, the shared-bit reject, the projection, the clamp and
//    the viewport offset agree.  The differential run
//    (tools/emu_cases/visibility_testbox.py) found last-bit differences in
//    the corners and the projection under 24-bit precision (333 of 2000
//    runs) from VC6's reassociation of the sums and products; with the
//    split arrays and the parenthesised x scale 0 of 6000 runs differ under
//    53- and 24-bit precision.  Griddraw.h's and D3DIMSoultreeShadow.cpp's views
//    of the clipper forward here (Group2_GridVisibilityTestBox.cpp,
//    Group1_ShadowClipper.cpp), as retail has a single function.
//    Retail's body has the EBP frame of its inline-asm helper, which a
//    pragma keeps for TestBox (docs/GAME_BUILD.md#frame-pointer-units).  EcoSystem.h's view of
//    VisibilityClipper::SphereInFrustum (0x0052fbb0, frameless in retail and
//    defined by src/krusty2/visibility/VisibilityQuadTree.cpp) used to forward
//    from here; it is a link_alias now.
#include <math.h>

#include "visibility/VisibilityQuadTree.h"
#include "../../samples/common/FistpFloatToInt.h"

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
// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
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
    // (pxK[low/high] for column K), in retail's straight-line order.
    // Separate arrays rather than px[4][2]: with one array VC6 reassociates
    // the corner sums as x+z+y, which rounds differently from retail's
    // x+y+z under 24-bit precision (tools/emu_cases/visibility_testbox.py).
    float lowX = center[0] - extent[0];
    float lowY = center[1] - extent[1];
    float lowZ = center[2] - extent[2];
    float highX = center[0] + extent[0];
    float highY = center[1] + extent[1];
    float highZ = center[2] + extent[2];
    float px0[2], px1[2], px2[2], px3[2];
    float py0[2], py1[2], py2[2], py3[2];
    float pz0[2], pz1[2], pz2[2], pz3[2];
    px0[0] = lowX * m[0];
    px1[0] = lowX * m[1];
    px2[0] = lowX * m[2];
    px3[0] = lowX * m[3];
    px0[1] = highX * m[0];
    px1[1] = highX * m[1];
    px2[1] = highX * m[2];
    px3[1] = highX * m[3];
    py0[0] = lowY * m[4];
    py1[0] = lowY * m[5];
    py2[0] = lowY * m[6];
    py3[0] = lowY * m[7];
    py0[1] = highY * m[4];
    py1[1] = highY * m[5];
    py2[1] = highY * m[6];
    py3[1] = highY * m[7];
    pz0[0] = lowZ * m[8];
    pz1[0] = lowZ * m[9];
    pz2[0] = lowZ * m[10];
    pz3[0] = lowZ * m[11];
    pz0[1] = highZ * m[8];
    pz1[1] = highZ * m[9];
    pz2[1] = highZ * m[10];
    pz3[1] = highZ * m[11];

    unsigned int common = 0xffffffff;
    for (i = 0; i < 8; i++) {
        int a = i & 1;
        int b = (i >> 1) & 1;
        int c = i >> 2;
        VisibilityClipPoint* corner = &s_group4BoxCorners[i];
        corner->w = px3[a] + py3[b] + pz3[c] + m[15];
        corner->x = px0[a] + py0[b] + pz0[c] + m[12];
        corner->y = px1[a] + py1[b] + pz1[c] + m[13];
        corner->z = px2[a] + py2[b] + pz2[c] + m[14];
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
        FistpFloatToIntChecked(&screenRect[0], corner->x);
        FistpFloatToIntChecked(&screenRect[1], corner->y);
        screenRect[3] = screenRect[1];
        screenRect[2] = screenRect[0];
        for (i = 1; i < 8; i++) {
            corner = &s_group4BoxCorners[i];
            inverse = 1.0f / corner->w;
            corner->x = corner->x * ((float)width * inverse);
            corner->y = (float)height * corner->y * inverse;
            if ((float)screenRect[0] > corner->x)
                FistpFloatToIntChecked(&screenRect[0], corner->x);
            else if ((float)screenRect[2] < corner->x)
                FistpFloatToIntChecked(&screenRect[2], corner->x);
            if ((float)screenRect[1] > corner->y)
                FistpFloatToIntChecked(&screenRect[1], corner->y);
            else if ((float)screenRect[3] < corner->y)
                FistpFloatToIntChecked(&screenRect[3], corner->y);
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
#pragma optimize("", on)
