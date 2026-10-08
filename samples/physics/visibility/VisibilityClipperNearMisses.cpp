// Near misses of the VisibilityClipper methods (src/krusty2/visibility/VisibilityQuadTree.cpp,
// TU VisibilityQuadTree.cpp).  Status under VC6 SP3, vc6_o2_ml, every relocation bound by
// src/krusty2/visibility/VisibilityQuadTree.bindings.json:
// - ProjectVertices 0x0052f190: 382/418, same size and frame; only the x87 load order inside
//   the twelve products differs (retail loads the vertex operand first in most of them).  The
//   written factor order, the matrix spelling (m[r][c], flat, named), indexing versus pointer
//   increments, the loop form and the statement form were tried without effect.
// - SphereInFrustum 0x0052fbb0: 517/526, same size and code; retail keeps depth in the camera
//   argument's slot and the near depth in the centre argument's slot, VC6 swaps the two (and
//   so the x high/low values that reuse them).  Declaration order and dropping the named
//   temporaries change nothing; neither do a named `farDepth`, `nearDepth`/`depth` declared
//   at their use, the lows assigned before their tests, an explicit `crosses = 0` else arm,
//   `depth + -radius` or the second test's operands swapped (docs/NEAR_MISS_INDEX.md, class a).
#include "visibility/VisibilityQuadTree.cpp"

// 0x0052f190 (ret 0x18).  ProjectPoint over `count` vertices (0x20-byte records): the
// viewport-scaled projections go to `out`, the outcodes to `codes` when it is given.
void VisibilityClipper::ProjectVertices(const VisibilityCamera* camera, const VisibilityMatrix* m,
                                        int count, const VisibilityVertex* v,
                                        VisibilityBoxVec* out, int* codes)
{
    for (int i = 0; i < count; i++) {
        VisibilityClipPoint c;
        int code = 0;
        c.w = v->x * m->m[0][3];
        c.w += v->y * m->m[1][3];
        c.w += v->z * m->m[2][3];
        c.w += m->m[3][3];
        float inv = 1.0f / c.w;
        c.x = v->x * m->m[0][0];
        c.x += v->z * m->m[2][0];
        c.x += v->y * m->m[1][0];
        c.x += m->m[3][0];
        if (c.x < 0.0)
            code = 1;
        else if (c.w - c.x < 0.0)
            code = 2;
        c.y = v->z * m->m[2][1];
        c.y += v->y * m->m[1][1];
        c.y += v->x * m->m[0][1];
        c.y += m->m[3][1];
        if (c.y < 0.0)
            code |= 4;
        else if (c.w - c.y < 0.0)
            code |= 8;
        c.z = v->y * m->m[1][2];
        c.z += v->x * m->m[0][2];
        c.z += v->z * m->m[2][2];
        c.z += m->m[3][2];
        if (c.z < 0.0)
            code |= 0x10;
        else if (c.w - c.z < 0.0)
            code |= 0x20;
        out->x = camera->viewportWidth * c.x * inv;
        out->y = camera->viewportHeight * c.y * inv;
        out->z = camera->viewportHeight * c.z * inv;
        if (codes)
            *codes++ = code;
        v++;
        out++;
    }
}

// 0x0052fbb0 (ret 0x14).  Sphere against the view frustum.  The camera supplies the depth
// axis (+0xb4/+0xc4/+0xd4/+0xe4, a column of matrixB), the near/far limits (+0x1bc / +0x1c0)
// and the side-plane scale (matrixC column 0); `m` gives x', y' and w.  Returns 0 when the
// sphere is outside; `fullyInside`, when given, receives 1 only if the sphere crosses no plane.
int VisibilityClipper::SphereInFrustum(const VisibilityCamera* camera, const VisibilityMatrix* m,
                                       const VisibilityBoxVec* center, float radius,
                                       int* fullyInside)
{
    int crosses = 0;
    float depth, nearDepth, spread, xHigh, xLow, yHigh, yLow;
    VisibilityClipPoint c;
    depth = center->y * camera->matrixB[1][0];
    depth += center->z * camera->matrixB[2][0];
    depth += center->x * camera->matrixB[0][0];
    depth += camera->matrixB[3][0];
    nearDepth = depth - radius;
    if (nearDepth > camera->farPlane || depth + radius < camera->nearPlane) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    if (depth + radius > camera->farPlane || nearDepth < camera->nearPlane)
        crosses = 1;
    spread = (camera->matrixC[1][0] + camera->matrixC[0][0]) * radius;
    spread += depth * camera->matrixC[2][0];
    spread += camera->matrixC[3][0];
    c.w = center->y * m->m[1][3];
    c.w += center->z * m->m[2][3];
    c.w += center->x * m->m[0][3];
    c.w += m->m[3][3];
    c.x = center->y * m->m[1][0];
    c.x += center->z * m->m[2][0];
    c.x += center->x * m->m[0][0];
    c.x += m->m[3][0];
    xHigh = c.x + spread;
    if (xHigh < 0.0f || (xLow = c.x - spread) > c.w) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    c.y = center->y * m->m[1][1];
    c.y += center->z * m->m[2][1];
    c.y += center->x * m->m[0][1];
    c.y += m->m[3][1];
    yHigh = c.y + spread;
    if (yHigh < 0.0f || (yLow = c.y - spread) > c.w) {
        if (fullyInside)
            *fullyInside = 0;
        return 0;
    }
    if (!fullyInside)
        return 1;
    if (!crosses && !(xLow < 0.0f) && !(xHigh > c.w) && !(yLow < 0.0f) && !(yHigh > c.w))
        *fullyInside = 1;
    else
        *fullyInside = 0;
    return 1;
}
