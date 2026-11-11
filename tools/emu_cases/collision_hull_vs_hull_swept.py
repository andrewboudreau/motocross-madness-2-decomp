"""CollisionObject::HullVsHullSwept 0x00436af0 (tools/emu_diff.py case).

The swept hull pair of collision_hull_vs_hull: random rigid frames, a previous body frame near
the current one, random query vectors and stale scratch points. The relative frame 0x00432260
and the vector helpers run as retail code; the oriented-box tree test 0x00428950 is the same
logging stub (it records the relative frame it is handed, so the frame's contents are compared).
"""
from tools.emu_cases.collision_hull_vs_hull import (hull_desc, query_desc, stub_tree_query, write_hull,
                                                     write_query, write_scratch)

RETAIL_VA = 0x00436af0
SYMBOL = '?HullVsHullSwept@CollisionObject@@QAEHPAUCollisionHullBody@@0PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'


def cases(rng, n):
    for _ in range(n):
        yield (hull_desc(rng, swept=1), hull_desc(rng), query_desc(rng), rng.randrange(7))


def setup(ctx, case):
    a, b, qd, scratch = case
    ha, hb = write_hull(ctx, a), write_hull(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    return dict(ecx=ctx.alloc(0x100), args=[ha, hb, q])
