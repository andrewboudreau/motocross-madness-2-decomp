"""BoxCapsuleOverlap 0x00425900, box against a capsule moved by a rigid transform (tools/emu_diff.py case).

Boxes and capsules of similar size near contact. Every BISECT_EVERY-th geometry is run twice
with the two adjacent binary32 radii where retail's answer flips (found by bisecting the
radius against the retail body at 53-bit precision), so a rounding or comparison difference
in the binding axis test shows up; the others use random radii around a sampled gap.
Axis-aligned segments give the zero components the normalisations special-case. The helpers
(FastInvSqrt, DotProduct, the vector operators) run as retail code. Only EAX is observable.
"""
import math

from tools.emu_cases.segment_triangle_query import apply, matrix, retail_flip, rotation, write_matrix

RETAIL_VA = 0x00425900
SYMBOL = '?BoxCapsuleOverlap@@YAHPBUVec3@@00MMPBUMatrix4@@@Z'
SOURCE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'
# The last separating-axis test (yz corner direction) and its pass.
PROBES = {0x004269cd: 'returns 1'}
BISECT_EVERY = 2


def _segment_box_distance(center, half, p0, p1):
    """Approximate distance from a segment to a box (sampling), for placing near contact."""
    best = float('inf')
    for k in range(65):
        s = k / 64
        p = [p0[i] + s * (p1[i] - p0[i]) for i in range(3)]
        d = [max(abs(p[i] - center[i]) - half[i], 0.0) for i in range(3)]
        best = min(best, math.sqrt(sum(x * x for x in d)))
    return best


def cases(rng, n):
    for _ in range(n):
        center = [rng.uniform(-20, 20) for _ in range(3)]
        half = [rng.choice([rng.uniform(0.1, 5), 1.0, 2.0]) for _ in range(3)]
        mid = [center[i] + rng.uniform(-8, 8) for i in range(3)]
        kind = rng.randrange(4)
        if kind == 0:
            d = [0.0, 0.0, 0.0]
            d[rng.randrange(3)] = rng.choice([-1, 1]) * rng.uniform(0.5, 6)
        else:
            d = [rng.uniform(-4, 4) for _ in range(3)]
        p0 = [mid[i] - d[i] for i in range(3)]
        p1 = [mid[i] + d[i] for i in range(3)]
        gap = _segment_box_distance(center, half, p0, p1)
        radius = rng.choice([gap * (1 + rng.choice([1e-6, -1e-6, 1e-3, -1e-3])),
                             gap * rng.uniform(0.5, 1.5), rng.uniform(0, 3), 0.0])
        rot, trans = rotation(rng), [rng.uniform(-30, 30) for _ in range(3)]
        e0, e1 = apply(p0, rot, trans), apply(p1, rot, trans)
        m = matrix(rot, trans)
        if BISECT_EVERY and _ % BISECT_EVERY == 0:
            flip = retail_flip(RETAIL_VA, setup, lambda r: (center, half, e0, e1, r, m), 0.0, 2 * gap + 10)
            if flip:
                yield (center, half, e0, e1, flip[0], m)
                yield (center, half, e0, e1, flip[1], m)
                continue
        yield (center, half, e0, e1, radius, m)


def setup(ctx, case):
    center, half, e0, e1, radius, m = case
    c = ctx.alloc(12)
    ctx.pack(c, '3f', *center)
    h = ctx.alloc(12)
    ctx.pack(h, '3f', *half)
    ends = ctx.alloc(24)
    ctx.pack(ends, '6f', *e0, *e1)
    xf = write_matrix(ctx, m)
    return dict(args=[c, h, ends, float(radius), float(radius * radius), xf])
