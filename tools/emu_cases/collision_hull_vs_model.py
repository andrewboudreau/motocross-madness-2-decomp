"""CollisionObject::HullVsModel 0x00436e50 (tools/emu_diff.py case).

A hull against a model (collision_model_vs_hull's builders): broad phase 0x00424730 as retail
code, then per enabled element the rotate-into-frame / tree test / average / rotate-back
sequence with the out-of-line vector helpers (retail code) and the logging tree-test stub.
The scene-node position calls 0x004fc9a0 are stubbed (their results are dead). Swept hulls
forward to the retail swept body.
"""
from tools.emu_cases.collision_hull_vs_hull import (query_desc, stub_node_position, stub_tree_query, write_hull,
                                                     write_query, write_scratch)
from tools.emu_cases.collision_model_vs_hull import close_hull, model_desc, write_model

RETAIL_VA = 0x00436e50
SYMBOL = '?HullVsModel@CollisionObject@@QAEHPAUCollisionHullBody@@PAUCollisionModelBody@@PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x00437565: 'element tree test', 0x00436e78: 'swept forward'}


def cases(rng, n):
    for _ in range(n):
        yield (close_hull(rng), model_desc(rng), query_desc(rng), rng.randrange(4))


def setup(ctx, case):
    a, b, qd, scratch = case
    ha, mb = write_hull(ctx, a), write_model(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    stub_node_position(ctx)
    return dict(ecx=ctx.alloc(0x100), args=[ha, mb, q])
