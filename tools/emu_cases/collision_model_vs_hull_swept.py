"""CollisionObject::ModelVsHullSwept 0x00437ef0 (tools/emu_diff.py case).

As collision_model_vs_hull with a swept model: the model box is grown by its motion (retail
0x004290d0) and cached at +0x108/+0x114, and each enabled element runs the retail swept hull
test 0x00436af0 over the logging tree-test stub.
"""
from tools.emu_cases.collision_hull_vs_hull import (query_desc, stub_node_position, stub_tree_query, write_hull,
                                                     write_query, write_scratch)
from tools.emu_cases.collision_model_vs_hull import close_hull, model_desc, write_model

RETAIL_VA = 0x00437ef0
SYMBOL = '?ModelVsHullSwept@CollisionObject@@QAEHPAUCollisionModelBody@@PAUCollisionHullBody@@PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x004381cf: 'element test'}


def cases(rng, n):
    for _ in range(n):
        yield (model_desc(rng, swept=1), close_hull(rng), query_desc(rng), rng.randrange(4))


def setup(ctx, case):
    a, b, qd, scratch = case
    ma, hb = write_model(ctx, a), write_hull(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    stub_node_position(ctx)
    return dict(ecx=ctx.alloc(0x100), args=[ma, hb, q])
