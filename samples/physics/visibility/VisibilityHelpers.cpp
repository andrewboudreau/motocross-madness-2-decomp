// VisibilityHelpers.cpp -- free functions in the VisibilityQuadTree.cpp bracket
// (0x0052d22a..0x005300f8) that have no __FILE__ reference and no class owner.
// Ownership is UNCERTAIN: they sit between VisibilityQuadTree methods, so they are probably
// file-local helpers of VisibilityQuadTree.cpp, but the promotion rule (own __FILE__ string, or a
// method of a class that has one) is not met, so they stay here as samples.
#include "visibility/VisibilityQuadTree.h"

struct VisVec3 {
    float x, y, z;
};
struct VisMatrix {
    float m[4][4];
};

// 0x0052f0a0 (ret 0x10).  Rotates `count` vectors by the upper 3x3 of the matrix (row-vector
// convention: x' = x*m[0][0] + y*m[1][0] + z*m[2][0]); the translation row is not applied.
// Accumulating `t += ...` statements keep retail's x, z, y term order; a single sum expression
// is re-ordered by VC6 (the term order of a + b + c does not follow the source).
void __stdcall VisTransformVectors(const VisVec3* src, VisVec3* dst, const VisMatrix* mat, int count)
{
    for (int i = 0; i < count; i++) {
        VisVec3 v = src[i];
        float t = v.x * mat->m[0][0];
        t += v.z * mat->m[2][0];
        t += v.y * mat->m[1][0];
        dst[i].x = t;
        t = v.x * mat->m[0][1];
        t += v.z * mat->m[2][1];
        t += v.y * mat->m[1][1];
        dst[i].y = t;
        t = v.x * mat->m[0][2];
        t += v.z * mat->m[2][2];
        t += v.y * mat->m[1][2];
        dst[i].z = t;
    }
}

// 0x0052f140 (ret 0xc).  Compares a dot product against a threshold derived from the
// camera record's float at +0x16c (10.0f at 0x00550780 and 0.00461538f at 0x00558e14).
int __stdcall VisTestDot(const VisibilityCamera* camera, const VisVec3* a, const VisVec3* b)
{
    float dot = a->y * b->y;
    dot += a->x * b->x;
    dot += a->z * b->z;
    if (dot > (camera->field_0x16c - 10.0f) * 0.00461538f)
        return 0;
    return 1;
}

// 0x0052f340 (ret 0x14).  Projects a world point with the camera's first matrix, writes the
// outcode (x: 1 / 2 for x' < 0 / x' > w, y: 4 / 8, z: 0x10 / 0x20) and, when `screen` is given,
// the viewport-scaled coordinates.  The first argument is not read (tier 1 decoded).
struct VisVec4 {
    float x, y, z, w;
};
struct VisClipPoint {   // frame-size probe: retail reserves 0x20 bytes for the clip-space locals
    float x, y, z, w;
    float field_0x10[4];
};

int __stdcall VisProjectPoint(int unused, const VisibilityCamera* camera, const VisVec3* p,
                              VisVec3* screen, unsigned int* outCode)
{
    unsigned int code = 0;
    VisClipPoint c;
    c.w = p->z * camera->viewProjection[2][3];
    c.w += p->y * camera->viewProjection[1][3];
    c.w += p->x * camera->viewProjection[0][3];
    c.w += camera->viewProjection[3][3];
    c.x = p->z * camera->viewProjection[2][0];
    c.x += p->y * camera->viewProjection[1][0];
    c.x += p->x * camera->viewProjection[0][0];
    c.x += camera->viewProjection[3][0];
    if (c.x < 0.0)
        code = 1;
    else if (c.w - c.x < 0.0)
        code = 2;
    c.y = p->z * camera->viewProjection[2][1];
    c.y += p->y * camera->viewProjection[1][1];
    c.y += p->x * camera->viewProjection[0][1];
    c.y += camera->viewProjection[3][1];
    if (c.y < 0.0)
        code |= 4;
    else if (c.w - c.y < 0.0)
        code |= 8;
    c.z = p->z * camera->viewProjection[2][2];
    c.z += p->y * camera->viewProjection[1][2];
    c.z += p->x * camera->viewProjection[0][2];
    c.z += camera->viewProjection[3][2];
    if (c.z < 0.0)
        code |= 0x10;
    else if (c.w - c.z < 0.0)
        code |= 0x20;
    if (screen) {
        float inv = 1.0f / c.w;
        screen->x = camera->viewportWidth * inv * c.x;
        screen->y = camera->viewportHeight * inv * c.y;
        screen->z = camera->viewportHeight * inv * c.z;
    }
    if (outCode)
        *outCode = code;
    return code == 0;
}


// 0x0052fac0 (ret 0xc).  View-space polygon cull: points are 0x10-byte records (x, y, z, outcode).
// Returns 0 when every point has z below the camera's +0x1bc, or when the points' outcodes
// (|y| against z * +0x1b8, |x| against z) share a bit, i.e. the polygon is entirely outside one
// frustum side; the second test is skipped when the camera's +0x16c (a field of view in degrees,
// compared with 90.0) is above 90.
struct VisCullPoint {
    float x, y, z;
    unsigned int code;
};
int __stdcall VisCullPolygon(const VisibilityCamera* camera, VisCullPoint* points, int count)
{
    int i;
    int allBehind = 1;
    for (i = 0; i < count; i++) {
        if (points[i].z < camera->field_0x1bc && allBehind)
            allBehind = 1;
        else
            allBehind = 0;
    }
    if (allBehind)
        return 0;
    if (camera->field_0x16c <= 90.0) { /* 0x00558e18 */
    for (i = 0; i < count; i++) {
        unsigned int code = 0;
        float slope = points[i].z * camera->field_0x1b8;
        if (slope < points[i].y)
            code = 4;
        else if (-slope > points[i].y)
            code = 8;
        if (points[i].x > points[i].z)
            code |= 2;
        else if (-points[i].z > points[i].x)
            code |= 1;
        points[i].code = code;
    }
    unsigned int common = 0xffffffff;
    for (i = 0; i < count; i++)
        common &= points[i].code;
    if (common)
        return 0;
    }
    return 1;
}

struct VisCullVertex {
    VisVec3 position;
    unsigned int code;
};
// 0x0052f4d0 (ret 0x18).  Gathers four vertices by index (12-byte stride) into a 4-record
// polygon and runs VisCullPolygon on it.  The call below goes to the same address (0x0052fac0).
int __stdcall VisCullQuad(const VisibilityCamera* camera, const VisVec3* points, int i0, int i1,
                          int i2, int i3)
{
    VisCullVertex quad[4];
    quad[0].position = points[i0];
    quad[1].position = points[i1];
    quad[2].position = points[i2];
    quad[3].position = points[i3];
    return VisCullPolygon(camera, (VisCullPoint*)quad, 4);
}

// 0x0052fbb0 (ret 0x14).  Sphere against the view frustum.  `camera` supplies the depth axis
// (+0xb4/+0xc4/+0xd4/+0xe4, a column of the second matrix), the near/far limits (+0x1bc /
// +0x1c0) and the side-plane scale (+0x12c/+0x13c/+0x14c/+0x15c); `matrix` is the matrix whose
// first two columns and last column give x', y' and w.  Returns 0 when the sphere is outside;
// `fullyInside`, when given, receives 1 only if the sphere crosses no plane.  Tier 3 reading of
// the constants, decoded from the instructions.
int __stdcall VisSphereInFrustum(const VisibilityCamera* camera, const VisMatrix* matrix,
                                 const VisVec3* center, float radius, int* fullyInside)
{
    int crosses = 0;
    float depth, nearDepth, spread, w, x, xHigh, xLow, y, yHigh, yLow;
    depth = center->y * camera->matrixB[1][0];
    depth += center->z * camera->matrixB[2][0];
    depth += center->x * camera->matrixB[0][0];
    depth += camera->matrixB[3][0];
    nearDepth = depth - radius;
    if (nearDepth > camera->field_0x1c0 || depth + radius < camera->field_0x1bc) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    if (depth + radius > camera->field_0x1c0 || nearDepth < camera->field_0x1bc)
        crosses = 1;
    spread = (camera->matrixC[1][0] + camera->matrixC[0][0]) * radius;
    spread += depth * camera->matrixC[2][0];
    spread += camera->matrixC[3][0];
    w = center->y * matrix->m[1][3];
    w += center->z * matrix->m[2][3];
    w += center->x * matrix->m[0][3];
    w += matrix->m[3][3];
    x = center->y * matrix->m[1][0];
    x += center->z * matrix->m[2][0];
    x += center->x * matrix->m[0][0];
    x += matrix->m[3][0];
    xHigh = x + spread;
    if (xHigh < 0.0f || (xLow = x - spread) > w) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    y = center->y * matrix->m[1][1];
    y += center->z * matrix->m[2][1];
    y += center->x * matrix->m[0][1];
    y += matrix->m[3][1];
    yHigh = y + spread;
    if (yHigh < 0.0f || (yLow = y - spread) > w) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    if (!fullyInside)
        return 1;
    if (!crosses && !(xLow < 0.0f) && !(xHigh > w) && !(yLow < 0.0f) && !(yHigh > w))
        *fullyInside = 1;
    else
        *fullyInside = 0;
    return 1;
}
