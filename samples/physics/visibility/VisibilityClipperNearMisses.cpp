// Near miss of the VisibilityClipper methods (src/krusty2/visibility/VisibilityQuadTree.cpp,
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
