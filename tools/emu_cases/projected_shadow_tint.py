"""ProjectedShadow::TintCasterVertices 0x004dc2b0 (tools/emu_diff.py case).

Casters with and without a mesh, each with a few parts; every part's projected vertices
(vertexBuffer, 12 bytes each) sit on and around texel centres (k + 0.5 exactly, one float ulp
either side, random fractions) and around the texture edge, so `fistp(v - 0.5f)` lands on
both sides of each boundary and of the 0 <= x, y <= size test. The height map holds
colorKeyLow in about half the cells. Compared: the colour words the function paints
(+0x10 of each 0x20-byte caster vertex) and the PartCount/GetPart call sequence.
"""
import struct

RETAIL_VA = 0x004dc2b0
SYMBOL = '?TintCasterVertices@ProjectedShadow@@QAEXXZ'
SOURCE = 'samples/physics/shadow/ProjectedShadowNearMisses.cpp'
PROBES = {0x004dc351: 'x fistp', 0x004dc3b4: 'tint store'}

PART_COUNT = 0x00445030   # ShadowCaster::PartCount(int), thiscall ret 4
GET_PART = 0x004433f0     # ShadowCaster::GetPart(...), thiscall ret 0x1c


def bump(x, ulps):
    """x rounded to binary32, moved by ``ulps`` units in the last place."""
    bits = struct.unpack('<i', struct.pack('<f', x))[0]
    if x < 0:
        ulps = -ulps
    return struct.unpack('<f', struct.pack('<i', bits + ulps))[0]


def coord(rng, size):
    k = rng.randrange(-2, size + 3)
    kind = rng.randrange(10)
    if kind < 3:
        return k + 0.5
    if kind < 5:
        return bump(k + 0.5, rng.choice([-1, 1, -2, 2]))
    if kind == 5:
        return float(k)
    if kind == 6:
        return rng.choice([0.5, -0.5, 1e-30, -1e-30, 0.49999997, size + 0.5, size + 0.49999, 1.5,
                           3e9, -3e9, float('inf'), float('nan')])
    return k + rng.random()


def cases(rng, n):
    for _ in range(n):
        shift = rng.choice([7, 7, 6, 4, 3])
        size = rng.choice([1 << shift, (1 << shift) - 1, rng.randrange(1, (1 << shift) + 1)])
        casters = []
        for _ in range(rng.choice([0, 1, 1, 2, 3])):
            mesh = rng.random() < 0.85
            parts = [rng.choice([0, 1, 3, 8, 20]) for _ in range(rng.choice([0, 1, 1, 2, 3]))]
            casters.append((mesh, parts))
        yield (shift, size, casters, rng.randrange(1 << 16), rng.getrandbits(32), rng.randrange(1 << 30))


def setup(ctx, case):
    shift, size, casters, key, color, seed = case
    rng = ctx.rng
    total = sum(sum(p) for mesh, p in casters if mesh) + 1
    vb = ctx.alloc(12 * total)
    for i in range(total):
        ctx.pack(vb + 12 * i, '3f', coord(rng, size), coord(rng, size), rng.uniform(-1, 1))
    cells = ((size + 1) << shift) + size + 2
    hm = ctx.alloc(2 * cells)
    heights = [key if rng.random() < 0.5 else rng.randrange(1 << 16) for _ in range(cells)]
    ctx.write(hm, struct.pack('<%dH' % cells, *heights))
    texture = ctx.alloc(0x80, fill=0x5a)
    ctx.pack(texture + 0x14, 'i', size)

    parts_of = {}
    part_data = {}
    array = ctx.alloc(4 * max(len(casters), 1))
    for ci, (mesh, parts) in enumerate(casters):
        c = ctx.alloc(0x280, fill=0x3c)
        ctx.pack(c + 0x1a4, 'I', ctx.alloc(4) if mesh else 0)
        ctx.pack(array + 4 * ci, 'I', c)
        parts_of[c] = len(parts)
        for pi, count in enumerate(parts):
            verts = ctx.alloc(0x20 * max(count, 1), fill=0xa5)
            part_data[(c, pi)] = (verts, count)

    def part_count(ctx, ecx, a):
        return parts_of[ecx]

    def get_part(ctx, ecx, a):
        verts, count = part_data[(ecx, a[0])]
        ctx.pack(a[1], 'I', verts)
        ctx.pack(a[2], 'i', count)
        ctx.pack(a[3], 'I', 0x0badd00d)
        ctx.pack(a[4], 'i', 0)
    ctx.stub_at(PART_COUNT, 'PartCount', pop=4, ret=part_count)
    ctx.stub_at(GET_PART, 'GetPart', pop=0x1c, ret=get_part)

    this = ctx.alloc(0x134, fill=0x11)
    ctx.pack(this + 0x2c, 'Ii', array, len(casters))
    ctx.pack(this + 0x54, 'I', texture)
    ctx.pack(this + 0x64, 'I', hm)
    ctx.pack(this + 0x88, 'i', shift)
    ctx.pack(this + 0xa8, 'II', vb, color)
    ctx.pack(this + 0xb4, 'H', key)
    return dict(ecx=this, args=[], ret_eax=False)   # void
