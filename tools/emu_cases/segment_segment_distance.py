"""SegmentSegmentDistance 0x0042a640, the box-tree query's segment/segment distance (tools/emu_diff.py case).

Random segments, parallel and nearly parallel pairs (the zero-normal early return), segments
whose closest points sit on an end, crossings inside both segments, and unit-length
directions (the `sq == 1` shortcuts of the normalise and length helpers). The helpers
(FastInvSqrt, FastSqrt, the vector operators) run as retail code in both bodies. The result
is read from ST0.
"""
RETAIL_VA = 0x0042a640
SYMBOL = '?SegmentSegmentDistance@@YAMPBUVec3@@000@Z'
SOURCE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'


def _vec(rng, scale):
    return [rng.uniform(-scale, scale) for _ in range(3)]


def cases(rng, n):
    axes = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]
    for _ in range(n):
        kind = rng.randrange(6)
        p0, p1 = _vec(rng, 20), _vec(rng, 20)
        d0, d1 = _vec(rng, 10), _vec(rng, 10)
        if kind == 1:                        # parallel (zero cross product)
            k = rng.choice([1.0, -2.0, 0.5])
            d1 = [k * x for x in d0]
        elif kind == 2:                      # nearly parallel
            d1 = [x + rng.uniform(-1e-3, 1e-3) for x in d0]
        elif kind == 3:                      # unit axes: the |v|^2 == 1 shortcuts
            d0, d1 = rng.sample(axes, 2)
            d0 = [x * rng.choice([1, -1, 3]) for x in d0]
        elif kind == 4:                      # crossing inside both segments
            s, t = rng.random(), rng.random()
            q = _vec(rng, 5)
            p0 = [q[i] - s * d0[i] for i in range(3)]
            p1 = [q[i] - t * d1[i] + rng.choice([0.0, 0.1, -1.0]) * (i == 1) for i in range(3)]
        elif kind == 5:                      # small integers, exact ties
            p0, p1 = [float(rng.randrange(-3, 4)) for _ in range(3)], [float(rng.randrange(-3, 4)) for _ in range(3)]
            d0, d1 = [float(rng.randrange(-2, 3)) for _ in range(3)], [float(rng.randrange(-2, 3)) for _ in range(3)]
        yield (p0, d0, p1, d1)


def setup(ctx, case):
    ptrs = []
    for v in case:
        a = ctx.alloc(12)
        ctx.pack(a, '3f', *v)
        ptrs.append(a)
    return dict(args=ptrs, ret_float=True, ret_eax=False)
