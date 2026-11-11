"""TerrainShutdownObject::LookupCell 0x00484d70, grid leaf lookup (tools/emu_diff.py case).

Linked from src/bringup/Group6_TerrainShadow.cpp. A random sparse grid tree (1 to 3 levels):
child tables of 256 slots (+0x04) at a random density, per-node shift (+0x29) and leaves
whose 16-bit grid position (+0x24/+0x26) matches the queried cell, a neighbour or garbage.
Queries are cells inside the tree, negative and past the 16-bit range (so the unsigned
compare decides). Retail indexes the row with g_gridRow16 (0x0056c2ac, row * 16).
"""
import random
import struct

RETAIL_VA = 0x00484d70
SYMBOL = '?LookupCell@TerrainShutdownObject@@QAEPAUTerrainCell@@HH@Z'
SOURCE = 'src/bringup/Group6_TerrainShadow.cpp'
PROBES = {0x00484da7: 'descend', 0x00484dc2: 'miss', 0x00484dc4: 'return'}


def cases(rng, n):
    for _ in range(n):
        yield (rng.randrange(1 << 30), rng.choice([1, 2, 3]), rng.choice([0.2, 0.6, 1.0]))


def setup(ctx, case):
    seed, levels, density = case
    r = random.Random(seed)
    leaves = []

    def node(level, base_x, base_z):
        at = ctx.alloc(0x30, fill=0x5c)
        shift = 4 * (levels - level - 1)
        ctx.pack(at + 0x28, 'BB', level, shift)
        if level == levels - 1 and r.random() < 0.9:
            ctx.pack(at + 4, 'I', 0)
            return at
        slots = [0] * 256
        for i in range(256):
            if r.random() < (density if levels == 1 else 0.12 if level == 0 else 0.06):
                cz, cx = divmod(i, 16)
                x, z = base_x + (cx << shift), base_z + (cz << shift)
                if level + 1 >= levels:
                    gx, gz = r.choice([(x, z), (x, z), (x, z), (x + 1, z), (x, z + 1), (r.randrange(65536), z)])
                    slots[i] = ctx.alloc(0x30, b'\x5c' * 4 + bytes(4) + b'\x5c' * 0x1c +
                                         struct.pack('<HH', gx & 0xffff, gz & 0xffff) + b'\x5c' * 8)
                    leaves.append((x, z))
                else:
                    slots[i] = node(level + 1, x, z)
        ctx.pack(at + 4, 'I', ctx.alloc(0x400, struct.pack('<256I', *slots)))
        return at

    root = node(0, 0, 0)
    span = 16 ** levels
    kind = r.randrange(8)
    if kind < 2 and leaves:
        x, z = r.choice(leaves)
    elif kind >= 5 and leaves:
        x, z = r.choice(leaves)
        x, z = r.choice([(x + 0x10000, z), (x, z + 0x10000), (x - 0x10000, z), (x, z - 0x20000)])
    elif kind == 2:
        x, z = r.randrange(span), r.randrange(span)
    elif kind == 3:
        x, z = r.choice([-1, -16, 0x10000, 0x10005, -0x10000]), r.randrange(span)
    else:
        x, z = r.randrange(-span, 2 * span), r.randrange(-0x20000, 0x20000)
    if kind in (2, 3, 4) and r.random() < 0.5:
        x, z = z, x
    return dict(ecx=root, args=[x, z])
