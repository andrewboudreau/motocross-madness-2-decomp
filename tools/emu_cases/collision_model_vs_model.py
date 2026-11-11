"""CollisionObject::ModelVsModel 0x00438280 (tools/emu_diff.py case).

Two models from collision_model_vs_hull's builder; every element of b is tested against the
whole of a with the retail ModelVsHull 0x00437c20 (and its element tests over the logging
tree-test stub), and the contacts are averaged. Swept a forwards to the retail swept body.
"""
from tools.emu_cases.collision_hull_vs_hull import (query_desc, stub_node_position, stub_tree_query, write_query,
                                                     write_scratch)
from tools.emu_cases.collision_model_vs_hull import model_desc, write_model

RETAIL_VA = 0x00438280
SYMBOL = '?ModelVsModel@CollisionObject@@QAEHPAUCollisionModelBody@@0PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x00438493: 'element test', 0x004382a1: 'swept forward'}


def cases(rng, n):
    for _ in range(n):
        yield (model_desc(rng, count=1 + rng.randrange(3)), model_desc(rng, count=1 + rng.randrange(3)),
               query_desc(rng), rng.randrange(4))


def setup(ctx, case):
    a, b, qd, scratch = case
    ma, mb = write_model(ctx, a), write_model(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    stub_node_position(ctx)
    return dict(ecx=ctx.alloc(0x100), args=[ma, mb, q])
