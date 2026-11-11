"""CollisionObject::ModelVsHull 0x00437c20 (tools/emu_diff.py case; model helpers for the model cases).

A model (an array of 1..4 hulls with enable flags and shared bounds) against a hull. Frames are
kept within a few units of each other and the boxes are large enough that the retail broad
phase 0x00424730 (run as retail code, like the matrix product, the box grower 0x004290d0 and
the element tests 0x00436720/0x00436af0) passes in most runs. The oriented-box tree test
0x00428950 under the element tests is the logging stub of collision_hull_vs_hull, so contacts
and counts vary per element and the averaging is exercised. Swept models forward to the retail
swept body.
"""
from tools.emu_cases.collision_hull_vs_hull import (HULL_SIZE, hull_desc, near, query_desc, rigid,
                                                     stub_node_position, stub_tree_query, vec, write_hull,
                                                     write_query, write_scratch)

RETAIL_VA = 0x00437c20
SYMBOL = '?ModelVsHull@CollisionObject@@QAEHPAUCollisionModelBody@@PAUCollisionHullBody@@PAUCollisionSweepQuery@@@Z'
SOURCE = 'src/krusty2/collision/CollisionObject.cpp'
PROBES = {0x00437e41: 'element test', 0x00437c41: 'swept forward'}


def close_hull(rng, swept=None):
    d = hull_desc(rng, swept=swept, box=(vec(rng, 1), [rng.uniform(0.5, 6) for _ in range(3)]))
    for key in ('world', 'body'):
        d[key] = rigid(rng, spread=3.0)
    d['prev'] = near(rng, d['body'], 0.5)
    return d


def model_desc(rng, swept=None, count=None):
    count = rng.randrange(1, 5) if count is None else count
    body = rigid(rng, spread=3.0)
    return dict(swept=rng.randrange(2) if swept is None else swept,
                enabled=[rng.choice([1, 1, 1, 0]) for _ in range(count)],
                elements=[close_hull(rng, swept=rng.randrange(2)) for _ in range(count)],
                center=vec(rng, 1), half=[rng.uniform(1, 8) for _ in range(3)],
                prev_center=vec(rng, 1), prev_half=[rng.uniform(1, 8) for _ in range(3)],
                prev=near(rng, body, 0.5), body=body, motion=near(rng, rigid(rng, spread=0.0), 0.3))


def write_model(ctx, d):
    n = len(d['elements'])
    m = ctx.alloc(0x120, fill=0x5d)
    enabled = ctx.alloc(4 * n)
    ctx.pack(enabled, '%di' % n, *d['enabled'])
    highlight = ctx.alloc(4 * n)
    elements = ctx.alloc(HULL_SIZE * n)
    for i, e in enumerate(d['elements']):
        h = write_hull(ctx, e)
        ctx.write(elements + HULL_SIZE * i, ctx.read(h, HULL_SIZE))
    ctx.pack(m, 'iIIiiI', n, enabled, highlight, d['swept'], 0, elements)
    ctx.pack(m + 0x18, '12f', *d['center'], *d['half'], *d['prev_center'], *d['prev_half'])
    ctx.pack(m + 0x48, '16f', *d['prev'])
    ctx.pack(m + 0x88, '16f', *d['body'])
    ctx.pack(m + 0xc8, '16f', *d['motion'])
    return m


def cases(rng, n):
    for _ in range(n):
        yield (model_desc(rng), close_hull(rng), query_desc(rng), rng.randrange(4))


def setup(ctx, case):
    a, b, qd, scratch = case
    ma, hb = write_model(ctx, a), write_hull(ctx, b)
    q = write_query(ctx, qd)
    write_scratch(ctx, ctx.rng, scratch)
    stub_tree_query(ctx, q)
    stub_node_position(ctx)
    return dict(ecx=ctx.alloc(0x100), args=[ma, hb, q])
