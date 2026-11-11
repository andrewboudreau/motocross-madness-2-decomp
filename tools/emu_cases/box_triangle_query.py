"""BoxTriangleQuery 0x00426be0, the 13-axis box/triangle separating-axis test (tools/emu_diff.py case).

A random triangle in the query's vertex array (0x00578f4c) moved by a rigid transform, and a
box whose centre slides from the triangle's centroid along a random direction. Every
BISECT_EVERY-th geometry runs at the two adjacent binary32 slide distances where retail's
answer flips (bisected against the retail body at 53-bit precision), so the binding axis test
sits exactly on its boundary; the rest use random distances. Degenerate edges (a repeated
vertex) and axis-aligned triangles give zero cross products. The axes are written to the
unit's static s_axes (0x00578f50), which both bodies share; CrossProduct, DotProduct, the
normalisation and the vector operators run as retail code.

Known open difference: about 1 in 300 boundary cases at 0x027f only (none at 0x007f), a
1-ulp contact flip at axis 4. Decoded: retail keeps `p2 = p0 - Dot(e1)` on the x87 stack
and compares it unrounded in the first QueryMin, where VC6 here stores and rounds it; that
this is the cause of the flip is inference. Inline p1, an if-form min, assignment in the
call and max before min left it unchanged. The test runs this case at n=60 (0 seen).
"""
from tools.emu_cases.segment_triangle_query import (
    STATIC_BINDINGS, apply, matrix, retail_flip, rotation, triangle, unit, write_matrix,
    write_triangle, write_vertices)

RETAIL_VA = 0x00426be0
SYMBOL = '?BoxTriangleQuery@@YAHPBUVec3@@0PBUQueryTriangle@@PBUMatrix4@@@Z'
SOURCE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'
BINDINGS = dict(STATIC_BINDINGS)
PROBES = {0x0042759f: 'returns 1'}
BISECT_EVERY = 2


def cases(rng, n):
    for k in range(n):
        verts, nrm, plane = triangle(rng, scale=rng.choice([1.0, 5.0, 20.0]))
        kind = rng.randrange(6)
        if kind == 0:
            verts[2] = list(verts[1])                     # degenerate edge
        elif kind == 1:
            for v in verts:
                v[1] = verts[0][1]                        # in a y plane
            nrm = [0.0, rng.choice([1.0, -1.0]), 0.0]
        rot, trans = rotation(rng), [rng.uniform(-30, 30) for _ in range(3)]
        m = matrix(rot, trans)
        world = [apply(v, rot, trans) for v in verts]
        centroid = [sum(v[i] for v in world) / 3 for i in range(3)]
        half = [rng.choice([rng.uniform(0.1, 6), 1.0, 0.5]) for _ in range(3)]
        direction = unit([rng.uniform(-1, 1) for _ in range(3)])
        reach = 40.0

        def make(x, verts=verts, nrm=nrm, plane=plane, m=m, half=half, centroid=centroid, direction=direction):
            return (verts, nrm, plane, m, [centroid[i] + x * direction[i] for i in range(3)], half)
        if BISECT_EVERY and k % BISECT_EVERY == 0:
            flip = retail_flip(RETAIL_VA, setup, make, 0.0, reach)
            if flip:
                yield make(flip[0])
                yield make(flip[1])
                continue
        yield make(rng.choice([rng.uniform(0, 10), rng.uniform(0, reach)]))


def setup(ctx, case):
    verts, nrm, plane, m, center, half = case
    write_vertices(ctx, verts)
    tri = write_triangle(ctx, (0, 1, 2), nrm, plane)
    c = ctx.alloc(12)
    ctx.pack(c, '3f', *center)
    h = ctx.alloc(12)
    ctx.pack(h, '3f', *half)
    return dict(args=[c, h, tri, write_matrix(ctx, m)])
