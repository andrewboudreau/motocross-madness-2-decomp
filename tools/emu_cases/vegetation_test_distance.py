"""Vegetation::TestDistance 0x00456890 (tools/emu_diff.py case).

The linked body is src/bringup/Group5_Vegetation.cpp. Offsets from the retail body: the
vegetation's quantized x/z (+0x0c/+0x10, unsigned shorts) and definition index (+0x12 byte);
g_EcoSystem 0x0059aebc with +0x18 render context (+0x08 camera, position x/z at
+0x170/+0x178), +0x58 definition table (definition +0x1fc blendLods), +0x5a8 units per
coordinate and +0x5c0 detail level; the detail band table at 0x0059af14 (0x20-byte entries,
+0x00 geometry distance, +0x04 fade start, ints). Both globals are written at their retail
addresses.

The x/z offsets sit on the band distance (equal, one ulp either side, far or negative) so the
one-sided `>` tests and their NaN/equal arms run. FastSqrt 0x00460b50 is stubbed (it reads a
.bss table that retail fills at start-up): the stub notes its binary32 argument and returns
the binary32 root, the band distance itself, the fade start, or a value next to them, so the
`> outer` and `< inner` tests sit on their boundaries. inner == outer (0/0 or x/0 into __ftol)
occurs too; __ftol runs as retail code.
"""
import math
import struct

RETAIL_VA = 0x00456890
SYMBOL = '?TestDistance@Vegetation@@QAEXPAH0@Z'
SOURCE = 'src/bringup/Group5_Vegetation.cpp'
PROBES = {0x00456952: 'sqrt', 0x00456969: 'far', 0x0045698b: 'blend', 0x004569ac: 'fade',
          0x004569d7: 'no blend'}
G_ECOSYSTEM = 0x0059aebc
G_BANDS = 0x0059af14


def _f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def _next(x, d):
    bits = struct.unpack('<i', struct.pack('<f', x))[0]
    if x == 0:
        return _f32(d * 1e-45)
    bits += d if x > 0 else -d
    return struct.unpack('<f', struct.pack('<i', bits))[0]


def cases(rng, n):
    for _ in range(n):
        outer = rng.choice([50, 100, 200, 400, rng.randrange(1, 1000)])
        inner = rng.choice([0, outer // 2, outer, rng.randrange(0, outer + 1)])
        yield dict(outer=outer, inner=inner, level=rng.randrange(4), unit=rng.choice([1.0, 0.5, rng.uniform(0.1, 4)]),
                   q=(rng.randrange(65536), rng.randrange(65536)), def_index=rng.randrange(256),
                   offset=[rng.choice(['eq', 'below', 'above', 'small', 'small', 'neg', 'neg', 'far']) for _ in range(2)],
                   sqrt=rng.choice(['root', 'root', 'outer', 'outer-', 'outer+', 'inner', 'inner-', 'inner+']),
                   blend=rng.choice([0, 1, 7]))


def setup(ctx, c):
    outer, inner = float(c['outer']), float(c['inner'])
    eco = ctx.alloc(0x600, fill=0x21)
    veg = ctx.alloc(0x20, fill=0x31)
    ctx.pack(veg + 0x0c, 'H', c['q'][0])
    ctx.pack(veg + 0x10, 'H', c['q'][1])
    ctx.pack(veg + 0x12, 'B', c['def_index'])
    definition = ctx.alloc(0x200, fill=0x41)
    ctx.pack(definition + 0x1fc, 'i', c['blend'])
    ctx.pack(eco + 0x58 + 4 * c['def_index'], 'I', definition)
    ctx.pack(eco + 0x5a8, 'f', c['unit'])
    ctx.pack(eco + 0x5c0, 'i', c['level'])
    target = ctx.alloc(0x20)
    camera = ctx.alloc(0x180, fill=0x51)
    ctx.pack(target + 8, 'I', camera)
    ctx.pack(eco + 0x18, 'I', target)
    unit = _f32(c['unit'])
    pos = []
    for q, kind in zip(c['q'], c['offset']):
        base = _f32(q * unit)
        want = {'eq': outer, 'below': _next(outer, -1), 'above': _next(outer, 1), 'small': inner * 0.5,
                'neg': -outer * 1.5, 'far': outer * 3}[kind]
        pos.append(_f32(base + want))
    ctx.pack(camera + 0x170, 'f', pos[0])
    ctx.pack(camera + 0x178, 'f', pos[1])
    bands = ctx.alloc(0x80, fill=0x61)
    ctx.pack(bands + 0x20 * c['level'], 'ii', c['outer'], c['inner'])
    ctx.pack(G_ECOSYSTEM, 'I', eco)
    ctx.pack(G_BANDS, 'I', bands)

    def fast_sqrt(cx, ecx, a):
        cx.note('FastSqrt', a[0])
        value = struct.unpack('<f', struct.pack('<I', a[0]))[0]
        root = _f32(math.sqrt(value)) if value >= 0 else float('nan')
        result = {'root': root, 'outer': outer, 'outer-': _next(outer, -1), 'outer+': _next(outer, 1),
                  'inner': inner, 'inner-': _next(inner, -1), 'inner+': _next(inner, 1)}[c['sqrt']]
        return 0, result

    ctx.stub_at(0x00460b50, 'FastSqrt', nargs=1, ret=fast_sqrt, ret_float='f', log=False)
    out = ctx.alloc(8, fill=0x77)
    return dict(ecx=veg, args=[out, out + 4], ret_eax=False)
