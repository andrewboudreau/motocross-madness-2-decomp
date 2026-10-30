// Group2_GridVisibilityTestBox.cpp -- link bring-up: the visibility clipper's box test,
// retail 0x0052f570 (thiscall, ret 0x1c), as Griddraw.h's GridVisibilityClipper view
// declares it. Written from the retail disassembly (no sample body exists).
//
// Decoded behaviour (tier 2 for the roles, tier 1 for the arithmetic):
//   * `matrix` is a row-major 4x4 (row 3 = translation) taking points to clip space;
//     a point's outcode is 1/2 for x < 0 / x > w, 4/8 for y, 0x10/0x20 for z.
//   * Without screenRect and cornersInside, a centre that is inside accepts at once (1).
//   * The 8 corners centre +- extent are transformed into a 0x20-byte-per-entry .bss
//     array (0x0068abc8: x, y, z, w, then 16 unused bytes). A corner with outcode 0 accepts
//     at once when neither output is requested, else counts as inside. When every corner
//     shares an outside bit the box is rejected: the screen rect (if any) is zeroed and the
//     result is 0.
//   * With screenRect, the corners are projected (x * width / w, y * height / w, stored back
//     into the array) and their rounded bounds become left/top/right/bottom, clamped to
//     [0, width] / [0, height] and offset by the camera's viewport origin.
//   * cornersInside (if any) receives the number of inside corners; the result is 1.
//   * a6 is unused.
#include "../reconstructed/Griddraw.h"
#include <math.h>

namespace {

// Camera members the box test reads (provisional view of the camera at +0x1a0..+0x1ac).
struct TestBoxCameraView {
    unsigned char field_0x000[0x1a0];
    int viewportX;          // +0x1a0 added to left/right
    int viewportY;          // +0x1a4 added to top/bottom
    int viewportWidth;      // +0x1a8 projection scale and right clamp
    int viewportHeight;     // +0x1ac projection scale and bottom clamp
};

struct TestBoxCorner {      // one 0x20-byte entry of the array at 0x0068abc8
    float x, y, z, w;
    float unused[4];
};

// 0x0068abc8: shared scratch for the transformed corners (.bss, no initialiser).
TestBoxCorner s_corners[8];

int OutCode(float x, float y, float z, float w)
{
    int code = 0;
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

// best guess: retail rounds with fistp under the default round-to-nearest mode; ties
// round away from zero here instead of to even.
int RoundToInt(float v)
{
    return (int)floor(v + 0.5f);
}

} // namespace

int GridVisibilityClipper::TestBox(GridCamera* camera, const float* matrix, const float* center,
                                   const float* extent, int* screenRect, int* cornersInside,
                                   int* a6)
{
    const float* m = matrix;
    int insideCount = 0;
    (void)a6;

    if (!screenRect && !cornersInside) {
        // Trivial accept when the centre itself is inside the frustum.
        float w = m[11] * center[2] + m[3] * center[0] + m[7] * center[1] + m[15];
        float x = m[8] * center[2] + m[4] * center[1] + m[0] * center[0] + m[12];
        float y = m[9] * center[2] + m[1] * center[0] + m[5] * center[1] + m[13];
        float z = m[10] * center[2] + m[2] * center[0] + m[6] * center[1] + m[14];
        int code = 0;
        if (x < 0.0)
            code = 1;
        else if (w - x < 0.0)
            code = 2;
        if (y < 0.0)
            code |= 4;
        else if (w - y < 0.0)
            code |= 8;
        if (z < 0.0)
            code |= 0x10;
        else if (w - z < 0.0)
            code |= 0x20;
        if (code == 0)
            return 1;
    }

    // Per-axis products of the low/high box coordinates with the matrix rows.
    float lo[3], hi[3];
    lo[0] = center[0] - extent[0];
    lo[1] = center[1] - extent[1];
    lo[2] = center[2] - extent[2];
    hi[0] = center[0] + extent[0];
    hi[1] = center[1] + extent[1];
    hi[2] = center[2] + extent[2];

    float px[4][2], py[4][2], pz[4][2];   // [output column][low/high]
    for (int column = 0; column < 4; column++) {
        px[column][0] = lo[0] * m[column];
        px[column][1] = hi[0] * m[column];
        py[column][0] = lo[1] * m[4 + column];
        py[column][1] = hi[1] * m[4 + column];
        pz[column][0] = lo[2] * m[8 + column];
        pz[column][1] = hi[2] * m[8 + column];
    }

    int sharedCode = -1;
    for (int i = 0; i < 8; i++) {
        int ix = i & 1;
        int iy = (i >> 1) & 1;
        int iz = i >> 2;
        TestBoxCorner* c = &s_corners[i];
        c->w = px[3][ix] + py[3][iy] + pz[3][iz] + m[15];
        c->x = px[0][ix] + py[0][iy] + pz[0][iz] + m[12];
        c->y = px[1][ix] + py[1][iy] + pz[1][iz] + m[13];
        c->z = px[2][ix] + py[2][iy] + pz[2][iz] + m[14];
        int code = OutCode(c->x, c->y, c->z, c->w);
        if (code == 0) {
            if (!screenRect && !cornersInside)
                return 1;
            insideCount++;
        }
        sharedCode &= code;
    }

    if (sharedCode != 0) {
        if (screenRect) {
            screenRect[0] = 0;
            screenRect[2] = 0;
            screenRect[1] = 0;
            screenRect[3] = 0;
        }
        return 0;
    }

    if (screenRect) {
        TestBoxCameraView* view = (TestBoxCameraView*)camera;
        int* left = &screenRect[0];
        int* top = &screenRect[1];
        int* right = &screenRect[2];
        int* bottom = &screenRect[3];

        float inv = 1.0f / s_corners[0].w;
        s_corners[0].x = (float)view->viewportWidth * inv * s_corners[0].x;
        s_corners[0].y = (float)view->viewportHeight * inv * s_corners[0].y;
        *left = RoundToInt(s_corners[0].x);
        *top = RoundToInt(s_corners[0].y);
        *bottom = *top;
        *right = *left;

        for (int i = 1; i < 8; i++) {
            TestBoxCorner* c = &s_corners[i];
            inv = 1.0f / c->w;
            c->x = (float)view->viewportWidth * inv * c->x;
            c->y = (float)view->viewportHeight * c->y * inv;
            if ((float)*left > c->x)
                *left = RoundToInt(c->x);
            else if ((float)*right < c->x)
                *right = RoundToInt(c->x);
            if ((float)*top > c->y)
                *top = RoundToInt(c->y);
            else if ((float)*bottom < c->y)
                *bottom = RoundToInt(c->y);
        }

        if (*left <= 0)
            *left = 0;
        if (*right > view->viewportWidth)
            *right = view->viewportWidth;
        if (*top <= 0)
            *top = 0;
        if (*bottom > view->viewportHeight)
            *bottom = view->viewportHeight;
        *left += view->viewportX;
        *right += view->viewportX;
        *top += view->viewportY;
        *bottom += view->viewportY;
    }

    if (cornersInside)
        *cornersInside = insideCount;
    return 1;
}
