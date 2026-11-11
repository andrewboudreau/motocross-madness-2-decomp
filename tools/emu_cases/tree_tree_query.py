"""TreeTreeQueryNodes 0x004280e0, the two-tree walk of the box-tree query (tools/emu_diff.py case).

Tree A holds segment leaves in A's frame, tree B triangle leaves (indices into the vertex
array at 0x00578f4c) in B's frame; interior nodes carry their children's bounding box and its
volume. The relative frames (s_relativeAB, s_relativeBA), the motion matrix and the leaf mode
(s_mode 0, 1 or 2) live at their retail addresses, and the hit record pointer
(0x00579058) points at a fresh record. The walk's callees (LeafPairQuery, LeafNodeQuery,
SweptBoxOverlap and below) run as retail code in both bodies, so differences come from the
walk itself: its inline box-overlap SAT, the size comparison and the recursion order. Every
BISECT_EVERY-th pair of trees is slid apart to the two binary32 offsets where retail's outcome
changes (bisected against retail at 53-bit precision).
"""
from tools.emu_cases.segment_triangle_query import (
    HIT_RECORD_PTR, MODE, MOTION, RELATIVE_AB, RELATIVE_BA, SCRATCH_COUNT, STATIC_BINDINGS,
    apply, matrix, retail_flip, rotation, triangle, unit, write_matrix, write_vertices)

RETAIL_VA = 0x004280e0
SYMBOL = '?TreeTreeQueryNodes@@YAHPAUQueryTreeNode@@0H@Z'
SOURCE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'
BINDINGS = dict(STATIC_BINDINGS)
PROBES = {0x004275f0: 'LeafPairQuery', 0x004269e0: 'LeafNodeQuery', 0x00424730: 'SweptBoxOverlap'}
BISECT_EVERY = 3


def _transpose(r):
    return [[r[j][i] for j in range(3)] for i in range(3)]


def _leaf_box(points):
    lo = [min(p[i] for p in points) for i in range(3)]
    hi = [max(p[i] for p in points) for i in range(3)]
    return lo, hi


def _tree(rng, leaves):
    """leaves: list of (payload, lo, hi). Returns a nested ('node', lo, hi, a, b) / ('leaf', payload)."""
    if len(leaves) == 1:
        return ('leaf', leaves[0][0], leaves[0][1], leaves[0][2])
    axis = rng.randrange(3)
    leaves = sorted(leaves, key=lambda l: l[1][axis] + l[2][axis])
    k = rng.randrange(1, len(leaves))
    a, b = _tree(rng, leaves[:k]), _tree(rng, leaves[k:])
    lo = [min(a[2][i], b[2][i]) for i in range(3)]
    hi = [max(a[3][i], b[3][i]) for i in range(3)]
    return ('node', None, lo, hi, a, b)


def cases(rng, n):
    for _ in range(n):  # noqa: B007 (the index picks the bisected cases)
        spread = rng.choice([3.0, 8.0, 20.0])
        segs = []
        for _ in range(rng.randrange(1, 7)):
            p = [rng.uniform(-spread, spread) for _ in range(3)]
            q = [p[i] + rng.uniform(-4, 4) for i in range(3)]
            lo, hi = _leaf_box([p, q])
            segs.append(((p, q), lo, hi))
        verts, tris = [], []
        for _ in range(rng.randrange(1, 7)):
            c = [rng.uniform(-spread, spread) for _ in range(3)]
            vs, nrm, plane = triangle(rng, scale=rng.uniform(1, 6), center=c)
            base = len(verts)
            verts += vs
            lo, hi = _leaf_box(vs)
            tris.append((((base, base + 1, base + 2), nrm, plane), lo, hi))
        rot = rotation(rng)
        trans = [rng.uniform(-3, 3) for _ in range(3)]
        mrot = rotation(rng, rng.choice([0, 0, 2]))
        if rng.random() < 0.5:
            mrot = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]
        mtrans = [rng.uniform(-2, 2) for _ in range(3)]
        base = (_tree(rng, segs), _tree(rng, tris), verts, rot, trans, mrot, mtrans,
                rng.choice([0, 1, 1, 2, 2]), rng.choice([0, 0, 1]))
        if BISECT_EVERY and _ % BISECT_EVERY == 0:
            # Slide tree B away along a random direction; retail's outcome (result, hit record,
            # axes) changes somewhere on the way: run the two binary32 offsets around it.
            direction = unit([rng.uniform(-1, 1) for _ in range(3)])

            def make(x, base=base, direction=direction):
                moved = [base[4][i] + x * direction[i] for i in range(3)]
                return base[:4] + (moved,) + base[5:]
            flip = retail_flip(RETAIL_VA, setup, make, 0.0, 4 * spread + 20)
            if flip:
                yield make(flip[0])
                yield make(flip[1])
                continue
        yield base


def _write_tree(ctx, t, kind):
    if t[0] == 'leaf':
        payload = t[1]
        leaf = ctx.alloc(0x1c)
        if kind == 'seg':
            ctx.pack(leaf, 'f6f', -1.0, *payload[0], *payload[1])
        else:
            idx, nrm, plane = payload
            ctx.pack(leaf, 'f3H2x4f', -1.0, *idx, *nrm, plane)
        return leaf
    _, _, lo, hi, a, b = t
    pa, pb = _write_tree(ctx, a, kind), _write_tree(ctx, b, kind)
    half = [(hi[i] - lo[i]) / 2 for i in range(3)]
    center = [(hi[i] + lo[i]) / 2 for i in range(3)]
    volume = 8 * half[0] * half[1] * half[2]
    node = ctx.alloc(0x24)
    ctx.pack(node, '7f2I', volume, *center, *half, pa, pb)
    return node


def setup(ctx, case):
    ta, tb, verts, rot, trans, mrot, mtrans, mode, use_motion = case
    write_vertices(ctx, verts)
    a = _write_tree(ctx, ta, 'seg')
    b = _write_tree(ctx, tb, 'tri')
    # B -> A and A -> B.
    write_matrix(ctx, matrix(rot, trans), RELATIVE_AB)
    rt = _transpose(rot)
    inv_t = [-x for x in apply(trans, rt, [0.0, 0.0, 0.0])]
    write_matrix(ctx, matrix(rt, inv_t), RELATIVE_BA)
    write_matrix(ctx, matrix(mrot, mtrans), MOTION)
    ctx.pack(MODE, 'i', mode)
    record = ctx.alloc(0x40)
    ctx.pack(record, 'f', 1.0)
    ctx.pack(record + 0x24, 'f', 1.0)
    ctx.pack(HIT_RECORD_PTR, 'I', record)
    ctx.pack(SCRATCH_COUNT, 'i', 0)
    return dict(args=[a, b, use_motion])
