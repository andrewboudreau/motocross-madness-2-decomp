"""EcoSystem::GenerateObjects 0x00459ce0, the Auto placement loop (tools/emu_diff.py case).

The linked body is src/bringup/Group4_EcoSystem.cpp. Offsets from the retail body: the
ecosystem's +0x34 TotalObjects, +0x38 vegetation array (0x1c-byte records), +0x44 ground
terrain, +0x48 texture manager, +0x58 definition table (256), +0x4dc probability TGA name,
+0x55c placed count; a definition's +0x100 TGA name, +0x198/+0x19c slope mean/deviation,
+0x1b0/+0x1b4 altitude mean/deviation, +0x200 PercentBias; a TGA's +0x0e width, +0x10 height
(shorts), +0x12 bits per pixel, +0x14 pixels. Globals at their retail addresses:
g_collisionQuadTree 0x0068aba4 (+0x50/+0x54 extents), g_TrackGame 0x0056e26c (vtable +0x50
GetRegistryInt, the EcoGen mode: 0, 3, 4, 5 or 6), g_SharedNormalDistribution 0x0056ece0,
g_ResourceManager 0x00572b44, g_MemTagStack 0x0056df04.

Each case has 1..4 definitions (some with their own 32/24/16-bit TGA), an optional global
TGA with mostly nonzero green, a small TotalObjects so the loop ends by placing (the
10000-failure exit is not reached). Stubbed: rand 0x00534c1b (per-run random sequence),
PlaceAuthoredObjects 0x00459b40, LoadFile 0x005125c0 / FreeFile 0x00512dd0 (names and
pointers noted), GetHeightRange 0x00508970, the definition's RandomParameter 0x00455f50,
HeightForParameter 0x00455f90, ParameterForHeight 0x00455f60 and RadiusForParameter
0x00455ff0 (deterministic in their argument), ComputeCode 0x004dc8c0 (the four bounds noted),
IsValidCode 0x004ddd90, QueryGround 0x00507c10 (notes the probe, writes its height and the
normal), NormalDistribution::Lookup 0x004b0070 (arguments noted, random weight),
Vegetation::Place 0x004567e0 (probe noted), MemTagStack::Push 0x004a2d00 (tag text noted) and
QuadTree::Insert 0x004dcac0. __ftol and acos run as retail code.
"""
import random
import struct

RETAIL_VA = 0x00459ce0
SYMBOL = '?GenerateObjects@EcoSystem@@QAEHH@Z'
SOURCE = 'src/bringup/Group4_EcoSystem.cpp'
QUADTREE = 0x0068aba4
TRACK_GAME = 0x0056e26c
NORMAL_DIST = 0x0056ece0
RESOURCES = 0x00572b44
MEM_TAG_STACK = 0x0056df04
PROBES = {0x0045a19b: 'mode 5 outer', 0x0045a1f4: 'mode 6 outer', 0x0045a333: 'code check',
          0x0045a369: 'global tga', 0x0045a4bc: 'definition tga 32', 0x0045a4dd: 'definition tga 24',
          0x0045a4fd: 'definition tga 16', 0x0045a62f: 'cluster test', 0x0045a8ca: 'place', 0x0045a7ee: 'cluster place',
          0x0045a95f: 'free global'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), mode=rng.choice([0, 0, 3, 4, 5, 6]),
                   ndefs=rng.choice([1, 1, 2, 3, 4]), global_tga=rng.random() < 0.5,
                   total=rng.choice([1, 2, 4, 6, 9, 17]), placed=rng.choice([0, 0, 1]),
                   check=rng.choice([0, 1]))


def _tga(ctx, r, bpp):
    width, height = r.randrange(3, 12), r.randrange(3, 12)
    size = width * height * (bpp // 8)
    pad = 0x40
    bits = ctx.alloc(size + 2 * pad, fill=0x80) + pad
    data = bytearray(size)
    for k in range(width * height):
        if bpp == 16:
            green = r.choice([0, 31, r.randrange(1, 32), r.randrange(32), r.randrange(32), r.randrange(32), r.randrange(32), r.randrange(32)])
            struct.pack_into('<H', data, 2 * k, (green << 5) | r.randrange(0x10000) & ~0x3e0)
        else:
            px = bpp // 8
            for b in range(px):
                data[px * k + b] = r.randrange(256)
            data[px * k + 1] = r.choice([0, 255, r.randrange(1, 256), r.randrange(256), r.randrange(256), r.randrange(256), r.randrange(256), r.randrange(256)])
    ctx.write(bits, bytes(data))
    tga = ctx.alloc(0x20, fill=0x13)
    ctx.pack(tga + 0xe, 'hhB', width, height, bpp)
    ctx.pack(tga + 0x14, 'I', bits)
    return tga


def setup(ctx, c):
    r = random.Random(c['seed'])
    eco = ctx.alloc(0x600)
    total = c['total']
    vegetation = ctx.alloc(0x1c * (total + 4), fill=0x5c)
    ctx.pack(eco + 0x34, 'iI', total, vegetation)
    ctx.pack(eco + 0x44, 'II', ctx.alloc(0x40), ctx.alloc(0x40))
    ctx.pack(eco + 0x55c, 'i', c['placed'])
    tgas = {}
    if c['global_tga']:
        ctx.write(eco + 0x4dc, b'global.tga\0')
        tgas[b'global.tga'] = _tga(ctx, r, r.choice([32, 24, 16]))
    slots = sorted(r.sample(range(256), c['ndefs']))
    bias = [r.uniform(5, 60) for _ in slots]
    for n, slot in enumerate(slots):
        d = ctx.alloc(0x210, fill=0x2d)
        name = b''
        if r.random() < 0.6:
            name = b'def%d.tga' % n
            tgas[name] = _tga(ctx, r, r.choice([32, 24, 16]))
        ctx.write(d + 0x100, name + b'\0')
        ctx.pack(d + 0x198, '2f', r.uniform(0, 1), r.uniform(0.1, 1))
        ctx.pack(d + 0x1b0, '2f', r.uniform(0, 1), r.uniform(0.1, 1))
        ctx.pack(d + 0x200, 'f', bias[n] * (100 / sum(bias)) if r.random() < 0.8 else bias[n])
        ctx.pack(eco + 0x58 + 4 * slot, 'I', d)

    quadtree = ctx.alloc(0x60, fill=0x71)
    extent = r.choice([1000.0, 1024.0, r.uniform(100, 3000)])
    ctx.pack(quadtree + 0x50, '2f', extent, r.choice([extent, r.uniform(100, 3000)]))
    ctx.pack(QUADTREE, 'I', quadtree)
    ctx.pack(NORMAL_DIST, 'I', ctx.alloc(0x40))
    ctx.pack(RESOURCES, 'I', ctx.alloc(0x40))
    ctx.pack(MEM_TAG_STACK, 'I', ctx.alloc(0x40))

    def text(cx, p):
        return cx.read(p, 16).split(b'\0')[0]

    def registry(cx, ecx, a):
        cx.note('GetRegistryInt', text(cx, a[0]), a[1])
        return c['mode']
    game = ctx.alloc(0x40)
    ctx.pack(game, 'I', ctx.vtable([ctx.stub('vslot%d' % i) if i != 20 else
                                    ctx.stub('GetRegistryInt', pop=8, ret=registry, log=False)
                                    for i in range(24)]))
    ctx.pack(TRACK_GAME, 'I', game)

    def load_file(cx, ecx, a):
        name = text(cx, a[0])
        cx.note('LoadFile', name, a[1], a[2])
        return tgas.get(name, 0)

    def height_range(cx, ecx, a):
        low = cx.rng.uniform(-50, 50)
        cx.pack(a[0], 'f', low)
        cx.pack(a[1], 'f', low + cx.rng.choice([0.0, cx.rng.uniform(1, 200)]))

    def f32(x):
        return struct.unpack('<f', struct.pack('<f', x))[0]

    def as_float(bits):
        return struct.unpack('<f', struct.pack('<I', bits))[0]

    def query_ground(cx, ecx, a):           # (probe, normal or null, 0, 0)
        cx.note('QueryGround', cx.read(a[0], 12).hex(), bool(a[1]), a[2], a[3])
        cx.pack(a[0] + 4, 'f', cx.rng.uniform(-60, 260))
        if a[1]:
            y = cx.rng.choice([1.0, cx.rng.uniform(0, 1), cx.rng.uniform(-0.2, 1)])
            cx.pack(a[1], '3f', cx.rng.uniform(-1, 1), y, cx.rng.uniform(-1, 1))

    def code(cx, ecx, a):
        cx.note('ComputeCode', *a)
        return (a[0] ^ a[3]) & 0xffff

    ctx.stub_at(0x00534c1b, 'rand', ret=lambda cx, e, a: cx.rng.randrange(0x8000), log=False)
    ctx.stub_at(0x00459b40, 'PlaceAuthoredObjects')
    ctx.stub_at(0x005125c0, 'LoadFile', nargs=3, ret=load_file, log=False)
    ctx.stub_at(0x00512dd0, 'FreeFile', nargs=1, ret=lambda cx, e, a: cx.note('FreeFile', a[0]), log=False)
    ctx.stub_at(0x00508970, 'GetHeightRange', pop=8, ret=height_range)
    ctx.stub_at(0x00455f50, 'RandomParameter', ret=lambda cx, e, a: cx.rng.randrange(256))
    # The byte parameters are pushed as dwords whose upper bytes are stale stack: mask them.
    def by_parameter(name, base, step):
        def ret(cx, e, a):
            cx.note(name, e, a[0] & 0xff)
            return 0, f32(base + (a[0] & 0xff) * step)
        return ret
    ctx.stub_at(0x00455f90, 'HeightForParameter', pop=4, ret_float='f',
                ret=by_parameter('HeightForParameter', 0.5, 0.031), log=False)
    ctx.stub_at(0x00455f60, 'ParameterForHeight', pop=4,
                ret=lambda cx, e, a: int(as_float(a[0]) * 37) & 0xff | 0x1234500)
    ctx.stub_at(0x00455ff0, 'RadiusForParameter', pop=4, ret_float='f',
                ret=by_parameter('RadiusForParameter', 0.25, 0.0173), log=False)
    ctx.stub_at(0x004dc8c0, 'ComputeCode', pop=16, ret=code, log=False)
    ctx.stub_at(0x004ddd90, 'IsValidCode', pop=4, ret=lambda cx, e, a: int(cx.rng.random() < 0.8))
    ctx.stub_at(0x00507c10, 'QueryGround', pop=16, ret=query_ground, log=False)
    ctx.stub_at(0x004b0070, 'Lookup', pop=12, ret_float='f',
                ret=lambda cx, e, a: (0, f32(cx.rng.choice([0.0, cx.rng.uniform(0.3, 4), cx.rng.uniform(1, 4), cx.rng.uniform(2, 6)]))))
    ctx.stub_at(0x004567e0, 'Place', pop=20,
                ret=lambda cx, e, a: cx.note('Place', e, a[0], a[1], cx.read(a[2], 12).hex(), a[3] & 0xff, a[4] & 0xff),
                log=False)
    ctx.stub_at(0x004a2d00, 'MemTagPush', pop=4, ret=lambda cx, e, a: cx.note('Push', e, text(cx, a[0])), log=False)
    ctx.stub_at(0x004dcac0, 'Insert', pop=16)
    return dict(ecx=eco, args=[c['check']])
