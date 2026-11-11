"""SegmentTouchesObject 0x00439820 (tools/emu_diff.py case).

A segment against a collision object: the bounding-sphere pre-test (retail vector operators,
dot and length), then for a hull (shape 0) or a model (shape 1, model bounds test 0x004253b0
as retail code) the segment tree query 0x00429e60. That query is stubbed: it records the
inverse frame it is handed (contents) and, in about half of the runs, writes a hit fraction
and normal into the installed box result (0x00579058) and reports a hit. The hit point and
the normal rotated back to world space are compared. Objects are enabled (flag bit at +0x31)
in most runs; shape 2 and 3 cover the fall-through.
"""
from tools.emu_cases.collision_hull_vs_hull import BOX_RESULT, vec, write_hull
from tools.emu_cases.collision_model_vs_hull import close_hull, model_desc, write_model

RETAIL_VA = 0x00439820
SYMBOL = '?SegmentTouchesObject@@YAHPBUCollisionVec3@@PAVCollisionObject@@PAU1@2@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
SEGMENT_TREE_QUERY = 0x00429e60
PROBES = {0x0043995b: 'model', 0x00439b85: 'hull', 0x00439a60: 'model element query', 0x00439cb8: 'hull query'}


def cases(rng, n):
    for _ in range(n):
        shape = rng.choice([0, 0, 1, 1, 2, 3])
        desc = close_hull(rng) if shape == 0 else model_desc(rng)
        ends = [vec(rng, 6), vec(rng, 6)]
        if rng.random() < 0.15:
            ends[1] = list(ends[0])                      # degenerate segment
        yield (shape, desc, ends, vec(rng, 2), rng.choice([0.5, 3.0, 8.0, 100.0]),
               rng.random() < 0.9)


def setup(ctx, case):
    shape, desc, ends, center, radius, enabled = case
    payload = write_hull(ctx, desc) if shape == 0 else write_model(ctx, desc)
    obj = ctx.alloc(0x100, fill=0x5e)
    ctx.pack(obj + 0x30, 'I', 0x100 if enabled else 0)
    ctx.pack(obj + 0x40, '3ffiI', *center, radius, shape, payload)
    seg = ctx.alloc(24)
    ctx.pack(seg, '6f', *ends[0], *ends[1])
    point = ctx.alloc(12, fill=0x77)
    normal = ctx.alloc(12, fill=0x77)

    def query(c, ecx, a):
        c.note(c.read(a[0], 24).hex(), c.read(a[2], 64).hex(), a[1], a[3])
        if c.rng.random() < 0.5:
            return 0
        rec = c.unpack(BOX_RESULT, 'I')[0]
        c.pack(rec, 'f', c.rng.random())
        c.pack(rec + 8, '3f', *vec(c.rng, 1))
        return 1
    ctx.stub_at(SEGMENT_TREE_QUERY, 'SegmentTreeQuery', nargs=4, ret=query)
    return dict(args=[seg, obj, point, normal])
