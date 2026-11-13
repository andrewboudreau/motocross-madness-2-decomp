"""ProjectedShadow::Present 0x004dbd30, the 2x2 box-filter of the shadow height map
(tools/emu_diff.py case, integer mode).

The linked body is samples/physics/shadow/ProjectedShadowNearMisses.cpp. The texture's
LockPixels (vtable +0x34, ret 0xc) answers a random 16-bit height map, UnlockPixels (+0x38,
ret 4) is logged. The dirty rectangle is random inside the texture, sometimes empty in x or y
(left > right, top > bottom: the span loops and the row loop never run) and with odd and even
edges, so the halved rectangle and the cleared tail (oldRight - right) and rows (up to the old
bottom) vary. The map starts on 4- or 2-byte alignment, which picks the leading short of the
Duff's-device fill. For formatA 9 the texels are drawn from colorKeyLow, shadowPixel and random
values, so every one of the five sum compares (key*4, alpha*4, the two mixes, else) is taken.
The early returns (surfaceFormat 0x613, downsampleOnPresent zero) are covered too. Compared: every byte
of the map and the clip rectangle, heightMap and the lock/unlock calls.
"""
import struct

RETAIL_VA = 0x004dbd30
SYMBOL = '?Present@ProjectedShadow@@QAEXXZ'
SOURCE = 'samples/physics/shadow/ProjectedShadowNearMisses.cpp'
PROBES = {0x004dbdcc: 'format 9', 0x004dbec0: 'sum == key*4', 0x004dbecf: 'sum == alpha*4',
          0x004dbede: 'mix A', 0x004dbef2: 'mix C', 0x004dbf46: 'fill table 1',
          0x004dbfd7: 'average row', 0x004dc067: 'fill table 2', 0x004dc0e7: 'clear rows'}


def cases(rng, n):
    for _ in range(n):
        shift = rng.choice([2, 3, 4, 5, 6, 7])
        size = 1 << shift
        l, r = sorted(rng.randrange(size) for _ in range(2))
        t, b = sorted(rng.randrange(size) for _ in range(2))
        if rng.random() < 0.1:
            l, r = r, l
        if rng.random() < 0.1:
            t, b = b, t
        if rng.random() < 0.15:
            l, t, r, b = 0, 0, size - 1, size - 1
        yield dict(seed=rng.getrandbits(32), size=size, rect=(l, t, r, b),
                   format=rng.choice([9, 9, 9, 5, 1, 0xc]),
                   surface=rng.choice([0x115c] * 12 + [0x613]),
                   enabled=int(rng.random() > 0.05),
                   key=rng.choice([0, 0xffff, rng.randrange(0x10000)]),
                   pixel=rng.choice([0x8000, 0x0f00, rng.randrange(0x10000)]),
                   pair=rng.getrandbits(32), align=rng.choice([0, 0, 2]))


def setup(ctx, c):
    rng = ctx.rng
    size = c['size']
    key, pixel = c['key'], c['pixel']
    cells = 4 * size * size + 64
    pool = [key, pixel, key, pixel, rng.randrange(0x10000)]
    words = [rng.choice(pool) if c['format'] == 9 and rng.random() < 0.85 else rng.randrange(0x10000)
             for _ in range(cells)]
    base = ctx.alloc(2 * cells + 16)
    hm = base + c['align']
    ctx.write(hm, struct.pack('<%dH' % cells, *words))

    lock = ctx.stub('Texture::LockPixels', pop=0xc, ret=hm)
    unlock = ctx.stub('Texture::UnlockPixels', pop=4)
    vt = ctx.vtable([ctx.stub('Texture::vslot%d' % i) for i in range(13)] + [lock, unlock])
    texture = ctx.alloc(0x80, fill=0x5a)
    ctx.pack(texture, 'I', vt)
    ctx.pack(texture + 0x14, 'i', size)

    this = ctx.alloc(0x134, fill=0x11)
    ctx.pack(this + 0x54, 'I', texture)
    ctx.pack(this + 0x64, 'I', 0)
    ctx.pack(this + 0x68, '4i', *c['rect'])
    ctx.pack(this + 0x94, 'H', pixel)
    ctx.pack(this + 0x98, 'i', c['enabled'])
    ctx.pack(this + 0x9c, 'i', c['format'])
    ctx.pack(this + 0xb0, 'i', c['surface'])
    ctx.pack(this + 0xb4, 'HHI', key, 0xcccc, c['pair'])
    return dict(ecx=this, args=[], ret_eax=False)   # void
