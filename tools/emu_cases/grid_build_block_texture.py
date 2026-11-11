"""DrawableGridNodeSharedTextures::BuildBlockTexture 0x0047d780, one block's terrain texture
built from run-length tile rows (tools/emu_diff.py case).

The linked body is samples/render/Grid1NearMisses.cpp. The node's extra (+0x3c) holds the run
table (+4) and the sixteen block records (+0x38, 0x2c bytes each, or null): the record's packed
own bit and index (+0: index -1 or own clear return 0). The texture is `width` pixels wide
(4 to 256, and 12 for a non-power-of-two tile mask); each run is a (tile, cells - 1) byte pair of
4-pixel cells and copies four rows; the runs of a row of cells sum to `width`. Tiles are
width / 4 pixels square: Terrain (+0x34) textures[id] (+0x544) answer LockLevel(tileSize)
(+0x40) with random pixels (padded). The target texture answers LockSurface (+0x34) with its
bits, PixelFormatBytesPerPixel 0x00511970 answers 1 to 4. Every byte written is compared.
"""
import random

RETAIL_VA = 0x0047d780
SYMBOL = '?BuildBlockTexture@DrawableGridNodeSharedTextures@@QAEHHPAVManagedTexture@@@Z'
SOURCE = 'samples/render/Grid1NearMisses.cpp'
FPCWS = (0x027f,)   # integer only
PROBES = {0x0047d8be: 'new tile', 0x0047d8ff: 'one span', 0x0047d982: 'head', 0x0047d9fe: 'whole tile',
          0x0047da53: 'tail', 0x0047dabf: 'next row', 0x0047db45: 'none'}

BPP = 0x00511970


def _row(r, width, style):
    cells = []
    left = width // 4
    while left:
        if style == 'short':
            n = r.randint(1, 3)
        elif style == 'long':
            n = r.choice([width // 16 or 1, width // 4, width // 8 or 1, 3, 5])
        else:
            n = r.choice([1, 2, 3, 4, 5, 7, 8, 9, 16, 31, 64])
        n = max(1, min(n, left, 256))
        cells.append(n)
        left -= n
    return cells


def cases(rng, n):
    for _ in range(n):
        width = rng.choice([4, 8, 16, 32, 64, 64, 128, 256, 12])
        yield dict(seed=rng.getrandbits(32), width=width, bpp=rng.choice([1, 2, 2, 3, 4]),
                   style=rng.choice(['short', 'long', 'mixed', 'mixed']),
                   tiles=rng.choice([1, 2, 5, 40, 256]), block=rng.randrange(16),
                   index=rng.choice([0, 3, 100, 100, -1]), own=rng.random() < 0.92,
                   blocks=rng.random() < 0.95)


def setup(ctx, c):
    r = random.Random(c['seed'])
    bpp, width = c['bpp'], c['width']
    tile_size = width // 4
    runs = bytearray()
    for _ in range(0, width, 4):
        for n in _row(r, width, c['style']):
            runs += bytes([r.randrange(c['tiles']), n - 1])
    index = c['index']
    data = ctx.alloc(len(runs) + 2 * max(index, 0) + 16)
    ctx.write(data + 2 * max(index, 0), runs)

    ctx.stub_at(BPP, 'PixelFormatBytesPerPixel', nargs=1, ret=bpp)

    def lock_level(cx, ecx, args):
        return cx.data['tile_bits'][ecx]
    tile_vt = ctx.vtable([ctx.stub('tile slot %d' % k) for k in range(16)]
                         + [ctx.stub('LockLevel', pop=4, ret=lock_level),
                            ctx.stub('UnlockLevel', pop=4, ret=1)] + [ctx.stub('tile slot 18')])
    terrain = ctx.alloc(0xc90, fill=0x33)
    ctx.data['tile_bits'] = {}
    size = (tile_size + 4) * (tile_size + 4) * bpp
    for k in range(256):
        tex = ctx.alloc(0x30)
        ctx.pack(tex, 'I', tile_vt)
        ctx.data['tile_bits'][tex] = ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size)))
        ctx.pack(terrain + 0x544 + 4 * k, 'I', tex)

    out = ctx.alloc(width * width * bpp + 16, fill=0x77)
    tex_vt = ctx.vtable([ctx.stub('texture slot %d' % k) for k in range(13)]
                        + [ctx.stub('LockSurface', pop=12, ret=out),
                           ctx.stub('UnlockSurface', pop=4, ret=1),
                           ctx.stub('BuildMipLevels', pop=4, ret=1)] + [ctx.stub('texture slot 16')])
    texture = ctx.alloc(0x80, bytes(r.getrandbits(8) for _ in range(0x80)))
    ctx.pack(texture, 'I', tex_vt)
    ctx.pack(texture + 0x14, 'i', width)

    blocks = ctx.alloc(16 * 0x2c, bytes(r.getrandbits(8) for _ in range(16 * 0x2c)))
    packed = (r.getrandbits(16) << 16) | ((index & 0x7fff) << 1) | int(c['own'])
    ctx.pack(blocks + 0x2c * c['block'], 'I', packed)
    extra = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    ctx.pack(extra + 4, 'I', data)
    ctx.pack(extra + 0x38, 'I', blocks if c['blocks'] else 0)
    node = ctx.alloc(0x60, bytes(r.getrandbits(8) for _ in range(0x60)))
    ctx.pack(node + 0x34, 'I', terrain)
    ctx.pack(node + 0x3c, 'I', extra)
    return dict(ecx=node, args=[c['block'], texture])
