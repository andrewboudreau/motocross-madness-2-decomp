"""SegmentTriangleQuery 0x00427c10, a moved segment against a triangle leaf (tools/emu_diff.py case).

The segment is aimed at a point inside, on the edge of or outside a random triangle with a
random hit fraction (some outside [0, 1]), under identity, permuted and random rigid
transforms, with the sweep record's offset nonzero. The scratch normal list starts empty,
holding the triangle's normal or holding others, so the deduplication loop runs both ways.
PointInTriangle, Vec3Equal and the vector operators run as retail code in both bodies; the
static vertex array pointer and the record live at their retail addresses.

Fixed difference (2026-11-11): the natural TransformPointInline for segment[1] summed the y
row (x + z) + y where retail sums (y + z) + x; at 24-bit precision 26 of 400 cases differed
in the hit fraction (TransformPointYZX in the sample now gives retail's order).

0x00579668 is never written in retail (.bss, so always 0), so by default only its clear arm
runs. With SET_FLAG = True a quarter of the cases take the set-flag arm. Its push dot summed
(z + y) + x where retail sums (x + y) + z (DotProduct, before 2026-11-12): 15 of 2400 runs
differed at 24-bit precision. QueryDotZKept now gives retail's order and stack shape (strict
374 -> 424), and the test runs the case with SET_FLAG = True.

The module also holds the input builders the other box-tree query cases import.
"""
import math

VERTICES_PTR = 0x00578f4c      # s_vertices: Vec3* of the tree being queried
AXES = 0x00578f50              # s_axes: Vec3[15] scratch
HIT_RECORD_PTR = 0x00579058    # g_CollisionBoxResult
SCRATCH_COUNT = 0x00579014     # g_CollisionScratchCount
SCRATCH_POINTS = 0x00579068    # g_CollisionScratchPoints
FLAG_579668 = 0x00579668       # read only (always 0 in retail)
MODE = 0x0057905c              # s_mode
MOTION = 0x00579018            # s_motion (Matrix4)
RELATIVE_AB = 0x00578ef0       # s_relativeAB
RELATIVE_BA = 0x00578eb0       # s_relativeBA

# The unit's file statics are storage-class-static in the object; bind them to retail so both
# bodies read and write the same storage.
STATIC_BINDINGS = {'_s_vertices': VERTICES_PTR, '_s_axes': AXES, '_s_mode': MODE, '_s_motion': MOTION,
                   '_s_relativeAB': RELATIVE_AB, '_s_relativeBA': RELATIVE_BA}


def rotation(rng, kind=None):
    """3x3 rotation rows: identity, an axis permutation or a random rotation."""
    kind = kind if kind is not None else rng.randrange(4)
    if kind == 0:
        return [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]
    if kind == 1:
        rows = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]
        rng.shuffle(rows)
        return [[x * s for x in r] for r, s in zip(rows, (1, rng.choice([1, -1]), 1))]
    a, b, c = (rng.uniform(-math.pi, math.pi) for _ in range(3))
    ca, sa, cb, sb, cc, sc = math.cos(a), math.sin(a), math.cos(b), math.sin(b), math.cos(c), math.sin(c)
    rz = [[ca, sa, 0], [-sa, ca, 0], [0, 0, 1]]
    ry = [[cb, 0, -sb], [0, 1, 0], [sb, 0, cb]]
    rx = [[1, 0, 0], [0, cc, sc], [0, -sc, cc]]
    return mul3(mul3(rz, ry), rx)


def mul3(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]


def matrix(rot, trans):
    """Matrix4 as 16 floats (row-vector convention)."""
    return rot[0] + [0.0] + rot[1] + [0.0] + rot[2] + [0.0] + list(trans) + [1.0]


def apply(v, rot, trans):
    return [sum(v[k] * rot[k][j] for k in range(3)) + trans[j] for j in range(3)]


def unapply(w, rot, trans):
    d = [w[j] - trans[j] for j in range(3)]
    return [sum(d[j] * rot[i][j] for j in range(3)) for i in range(3)]


def sub(a, b):
    return [a[i] - b[i] for i in range(3)]


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def dot(a, b):
    return sum(a[i] * b[i] for i in range(3))


def unit(v):
    n = math.sqrt(dot(v, v)) or 1.0
    return [x / n for x in v]


def triangle(rng, scale=10.0, center=None):
    """Three vertices, the unit face normal (either winding) and the plane offset."""
    center = center or [0.0, 0.0, 0.0]
    while True:
        vs = [[center[i] + rng.uniform(-scale, scale) for i in range(3)] for _ in range(3)]
        n = cross(sub(vs[1], vs[0]), sub(vs[2], vs[0]))
        if dot(n, n) > 1e-3:
            break
    n = unit(n)
    if rng.random() < 0.5:
        n = [-x for x in n]
    return vs, n, -dot(n, vs[0])


def write_triangle(ctx, verts_addr_index, n, plane, at=None):
    """A 0x18-byte QueryTriangle (three ushort indices, pad, normal, planeOffset)."""
    tri = at if at is not None else ctx.alloc(0x18)
    ctx.pack(tri, '3H2x4f', *verts_addr_index, *n, plane)
    return tri


def write_vertices(ctx, verts):
    arr = ctx.alloc(12 * len(verts))
    for i, v in enumerate(verts):
        ctx.pack(arr + 12 * i, '3f', *v)
    ctx.pack(VERTICES_PTR, 'I', arr)
    return arr


def write_matrix(ctx, m, at=None):
    at = at if at is not None else ctx.alloc(64)
    ctx.pack(at, '16f', *m)
    return at


RETAIL_VA = 0x00427c10
SYMBOL = '?SegmentTriangleQuery@@YAHPBUQueryTriangle@@PBUVec3@@PBUMatrix4@@@Z'
SOURCE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'
PROBES = {0x00427f53: 'PointInTriangle call', 0x00427fa6: 'dedup loop', 0x00427fbf: 'normal appended'}
SET_FLAG = False


def cases(rng, n):
    for _ in range(n):
        verts, nrm, plane = triangle(rng)
        bary = [rng.random() for _ in range(3)]
        where = rng.randrange(4)
        if where == 1:                 # on an edge
            bary[rng.randrange(3)] = 0.0
        elif where == 2:               # outside
            bary[rng.randrange(3)] = -rng.uniform(0.05, 1.0)
        tot = sum(bary) or 1.0
        q = [sum(bary[k] * verts[k][i] for k in range(3)) / tot for i in range(3)]
        d = [rng.uniform(-5, 5) for _ in range(3)]
        if dot(d, nrm) <= 0:
            d = [-x for x in d]
        if rng.random() < 0.1:
            d = [-x for x in d]        # moving away: rejected by the facing test
        t = rng.choice([rng.random(), rng.random(), 0.0, 1.0, -0.2, 1.3])
        start = [q[i] - t * d[i] for i in range(3)]
        offset = [rng.choice([0.0, rng.uniform(-1, 1)]) for _ in range(3)]
        rot, trans = rotation(rng), [rng.uniform(-30, 30) for _ in range(3)]
        seg0 = unapply([start[i] + offset[i] for i in range(3)], rot, trans)
        seg1 = unapply([start[i] + d[i] for i in range(3)], rot, trans)
        flag = rng.choice([0, 0, 0, 1]) if SET_FLAG else 0
        scratch = rng.choice(['empty', 'same', 'other'])
        yield (verts, nrm, plane, seg0, seg1, matrix(rot, trans), offset, flag, scratch,
               rng.randrange(0, 3))


def setup(ctx, case):
    verts, nrm, plane, seg0, seg1, m, offset, flag, scratch, count = case
    write_vertices(ctx, verts)
    tri = write_triangle(ctx, (0, 1, 2), nrm, plane)
    seg = ctx.alloc(24)
    ctx.pack(seg, '6f', *seg0, *seg1)
    mat = write_matrix(ctx, m)
    record = ctx.alloc(0x40, fill=0x11)
    ctx.pack(record, '3f', *offset)
    ctx.pack(record + 0x18, '3f4xi', 0.5, -0.25, 2.0, count)
    ctx.pack(record + 0x24, 'f', ctx.rng.choice([1.0, 0.75]))
    ctx.pack(HIT_RECORD_PTR, 'I', record)
    ctx.pack(FLAG_579668, 'i', flag)
    points = []
    if scratch == 'same':
        points = [[0.0, 1.0, 0.0], list(nrm)]
    elif scratch == 'other':
        points = [[0.0, 1.0, 0.0], [1.0, 0.0, 0.0]]
    for i, p in enumerate(points):
        ctx.pack(SCRATCH_POINTS + 12 * i, '3f', *p)
    ctx.pack(SCRATCH_COUNT, 'i', len(points))
    return dict(args=[tri, seg, mat])


# ---------------------------------------------------------------- boundary search
_RETAIL_EMU = []


def retail_emulator():
    """A shared Emulator over the retail image only (None without the private exe)."""
    if not _RETAIL_EMU:
        import os
        from pathlib import Path
        from tools import emu_diff
        path = Path(os.environ.get('MCM2_EXE') or emu_diff.DEFAULT_RETAIL)
        emu = None
        if path.is_file():
            try:
                from mcm2tool.pe import PEImage
                import unicorn  # noqa: F401
                emu = emu_diff.Emulator(PEImage(path), None)
            except ImportError:
                emu = None
        _RETAIL_EMU.append(emu)
    return _RETAIL_EMU[0]


def retail_flip(va, setup, make_case, lo, hi, steps=40, fpcw=0x027f):
    """Bisect a scalar between lo and hi for where retail's EAX changes.

    ``make_case(x)`` builds the case for parameter x. Returns (below, above), the two
    binary32 parameters around the flip, or None when retail returns the same at both ends
    (or the exe is unavailable).
    """
    import struct as _s
    emu = retail_emulator()
    if emu is None:
        return None

    def f32(x):
        return _s.unpack('<f', _s.pack('<f', x))[0]

    def eax(x):
        r = emu.run(va, setup, make_case(f32(x)), fpcw, ('bisect',))
        return r.outcome
    lo, hi = f32(lo), f32(hi)
    a, b = eax(lo), eax(hi)
    if a == b:
        return None
    for _ in range(steps):
        mid = f32((lo + hi) / 2)
        if mid in (lo, hi):
            break
        if eax(mid) == a:
            lo = mid
        else:
            hi = mid
    return lo, hi
