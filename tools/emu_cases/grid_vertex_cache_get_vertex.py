"""GridVertexCache::GetVertex 0x0047de90 (tools/emu_diff.py case).

Linked from samples/render/GriddrawNearMisses.cpp. The cache (layout in
src/reconstructed/Griddraw.h: node +0, stamp +4, slots +8, vertices +0x910, count +0x2d30 and
the latched parameters up to +0x2d70) is a fresh heap copy with random slot stamps below,
at and above the frame stamp, a count of 0 (the first vertex latches the node) or a few, and
random latched values. The node has a random grid position, shift, relief/height scales
(negative relief flips the normals), level 0/1/2, a 17 x 17 cell block (random heights,
normals, base height at +0xd4c) and draw data with texture origin/step and the b2 normal
flag; the terrain gives the cell size, lowestQualityOverride (+0xcb0) and the unlit colour
and specular bits (+0xc9c/+0xca0). g_gridOriginX/Z and g_gridDrawNormalScale are retail's
globals, set per run. Each run looks up one vertex at a random (x, z).
"""
import random
import struct

RETAIL_VA = 0x0047de90
SYMBOL = '?GetVertex@GridVertexCache@@QAEHHH@Z'
SOURCE = 'samples/render/GriddrawNearMisses.cpp'
NORMAL_SCALE = 0x0056c204
ORIGIN_X = 0x006754dc
ORIGIN_Z = 0x0067a688


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    cells = b''.join(struct.pack('<BBHbbbB', r.randrange(256), r.randrange(256), r.randrange(65536),
                                 r.randrange(-127, 128), r.randrange(-127, 128), r.randrange(-127, 128),
                                 r.randrange(256)) for _ in range(17 * 17))
    block = ctx.alloc(0xd50, cells + bytes(0xd4c - len(cells)) +
                      struct.pack('<i', r.choice([0, 1000, 30000, r.randrange(-5000, 70000)])))
    terrain = ctx.alloc(0xd00, fill=0x3c)
    ctx.pack(terrain + 0x40, 'f', r.choice([1.0, 2.5, r.uniform(0.1, 10)]))
    ctx.pack(terrain + 0xc9c, 'II', r.randrange(1 << 32), r.randrange(1 << 32))
    ctx.pack(terrain + 0xcb0, 'i', r.choice([0, 0, 1]))
    data = ctx.alloc(0x180, fill=0x2d)
    ctx.pack(data + 0x121, 'B', r.randrange(256))
    ctx.pack(data + 0x158, '3f', r.uniform(-2, 2), r.uniform(-2, 2), r.choice([1 / 16.0, 0.25, r.uniform(0, 1)]))
    node = ctx.alloc(0x60, fill=0x4e)
    ctx.pack(node + 0x0c, 'I', block)
    ctx.pack(node + 0x10, '2f', r.choice([1.0, -1.0, 0.5, r.uniform(-3, 3)]), r.choice([1.0, 0.01, r.uniform(0, 2)]))
    ctx.pack(node + 0x24, 'HHBB', r.randrange(4096), r.randrange(4096), r.choice([0, 1, 2]), r.choice([0, 4, 8]))
    ctx.pack(node + 0x34, 'II', terrain, data)
    ctx.pack(NORMAL_SCALE, 'f', r.choice([1 / 16.0, r.uniform(0, 1)]))
    ctx.pack(ORIGIN_X, 'i', r.randrange(-16, 17))
    ctx.pack(ORIGIN_Z, 'i', r.randrange(-16, 17))

    frame = r.randrange(1, 1000)
    count = r.choice([0, 0, r.randrange(1, 40)])
    cache = ctx.alloc(0x2d74, fill=0x00)
    slots = []
    for _ in range(17 * 17):
        slots += [r.choice([frame - 1, frame - 1, frame, frame + 1, 0]), r.randrange(count + 1)]
    ctx.write(cache + 8, struct.pack('<%dI' % len(slots), *[s & 0xffffffff for s in slots]))
    ctx.pack(cache, 'II', node, frame)
    ctx.pack(cache + 0x2d30, '4i', count, r.randrange(4096), r.randrange(4096), r.choice([0, 4]))
    ctx.pack(cache + 0x2d40, 'I', block)
    ctx.write(cache + 0x2d44, struct.pack('<12f', *[r.uniform(-5, 5) for _ in range(12)]))
    ctx.pack(cache + 0x2d5c, '2f', r.uniform(-1000, 0), r.uniform(0, 1000))
    ctx.pack(cache + 0x2d64, 'i', r.choice([0, 1]))
    return dict(ecx=cache, args=[r.randrange(17), r.randrange(17)])
