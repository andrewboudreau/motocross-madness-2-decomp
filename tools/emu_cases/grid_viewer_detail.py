"""DrawableGridNode::ComputeViewerDetail 0x004815e0, the node's error limits (tools/emu_diff.py case).

The viewer left of, right of and inside the node's x and z ranges (the far-corner choice
inside), below, above and within its height range; coarse calls (distance only) and detail
updates with the draw distance off, the viewer straight above the node (near distance 0),
degenerate nodes (far distance 0) and error products past 65535 (the clamp). Offsets from
retail's loads: node terrain +0x34, data +0x38, heights +0x18/+0x1c; terrain viewer
+0x60..+0x68, draw distance +0x6c, ratio +0x74, update count +0x98; data grid bounds
+0x164..+0x170, inverse error scale +0x174, error limits +0x122/+0x124 and their previous
values +0x126/+0x128. FastSqrt and __ftol run as retail code. The far-0 arm (0x0048183f)
is unreachable (the far distance is never below the near one), as is the >0xffff clamp of the
short lower limit (0x004818c3).
"""
RETAIL_VA = 0x004815e0
SYMBOL = '?ComputeViewerDetail@DrawableGridNode@@QAEMH@Z'
SOURCE = 'samples/render/GriddrawNearMisses.cpp'
PROBES = {0x004817fd: 'draw distance off', 0x0048181c: 'near 0', 0x00481941: 'lower clamp',
          0x004818d4: 'error limits'}


def cases(rng, n):
    for _ in range(n):
        size = rng.choice([16.0, 64.0, 512.0, 0.0])
        x0, z0 = rng.uniform(-2000, 2000), rng.uniform(-2000, 2000)
        y0 = rng.uniform(-50, 100)
        h = rng.choice([0.0, rng.uniform(0, 80)])

        def around(lo, extent):
            return rng.choice([lo - rng.uniform(0, 3 * extent + 10), lo + rng.uniform(0, extent),
                               lo + extent + rng.uniform(0, 3 * extent + 10), lo, lo + extent,
                               lo + extent / 2])
        viewer = (around(x0, size), around(y0, h), around(z0, size))
        coarse = rng.choice([0, 0, 0, 1])
        draw = rng.choice([1, 1, 1, 0])
        scale = rng.choice([rng.uniform(0.001, 0.5), rng.uniform(1, 400)])
        ratio = rng.choice([1.0, rng.uniform(0.2, 3.0)])
        limits = [rng.randrange(0x10000) for _ in range(2)]
        yield (x0, z0, size, y0, h, viewer, coarse, draw, scale, ratio, limits)


def setup(ctx, case):
    x0, z0, size, y0, h, viewer, coarse, draw, scale, ratio, limits = case
    terrain = ctx.alloc(0x100)
    ctx.pack(terrain + 0x60, '3fi', *viewer, draw)
    ctx.pack(terrain + 0x74, 'f', ratio)
    ctx.pack(terrain + 0x98, 'i', 41)
    data = ctx.alloc(0x180)
    ctx.pack(data + 0x122, 'HHHH', limits[0], limits[1], 0x7777, 0x7777)
    ctx.pack(data + 0x164, '5f', x0, z0, x0 + size, z0 + size, scale)
    node = ctx.alloc(0x40)
    ctx.pack(node + 0x18, 'ff', y0, y0 + h)
    ctx.pack(node + 0x34, 'II', terrain, data)
    return dict(ecx=node, args=[coarse], ret_float=True, ret_eax=False)
