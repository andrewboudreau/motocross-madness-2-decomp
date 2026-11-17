"""Halve8 0x004cee30, the 2x2 box filter of palettized textures (tools/emu_diff.py case).

A palette (colours at +0x10, the 555-to-index table at +0x710, read through the retail
accessors 0x004de270 and 0x004de280) with or without a magenta (ff 00 ff) key entry, so both
the keyed loop (blocks with 0..4 key pixels: the < 2 branch writes the key, 2..4 average with
rounding) and the plain rounded 4-pixel average run; random and extreme (0/ff) colours; a NULL
palette (returns 0, writes nothing). Widths and heights 1..8 with padded strides; the
destination starts filled with 0x77 so every written byte is compared.
"""

RETAIL_VA = 0x004cee30
SYMBOL = '?Halve8@@YAHPAX0HHHHPAUTextureColorMapperView@@@Z'
SOURCE = 'samples/render/PixtransNearMisses.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        w = rng.choice([1, 2, 3, 4, 5, 8])
        h = rng.choice([1, 2, 3, 5, 8])
        yield dict(w=w, h=h, sstride=2 * w + rng.choice([0, 0, 1, 3]),
                   dstride=w + rng.choice([0, 1, 3]),
                   palette=rng.choice(['keyed', 'keyed', 'plain', 'plain', 'none']),
                   colors=rng.choice(['random', 'extreme']),
                   keyrate=rng.choice([0.0, 0.25, 0.5, 0.9]),
                   seed=rng.getrandbits(32))


def setup(ctx, c):
    rng = ctx.rng
    colors = bytearray()
    for _ in range(256):
        if c['colors'] == 'extreme':
            colors += bytes(rng.choice([0, 0xff]) for _ in range(3))
        else:
            colors += bytes(rng.getrandbits(8) for _ in range(3))
    for i in range(256):   # no accidental key entries
        if colors[3 * i:3 * i + 3] == b'\xff\x00\xff':
            colors[3 * i + 1] = 1
    key = rng.randrange(256)
    if c['palette'] == 'keyed':
        colors[3 * key:3 * key + 3] = b'\xff\x00\xff'
    n = c['sstride'] * 2 * c['h'] + 4
    src = bytes(key if c['palette'] == 'keyed' and rng.random() < c['keyrate']
                else rng.randrange(256) for _ in range(n))
    source = ctx.alloc(n, src)
    palette = 0
    if c['palette'] != 'none':
        palette = ctx.alloc(0x710 + 0x8000)
        ctx.write(palette + 0x10, bytes(colors))
        ctx.write(palette + 0x710, bytes(rng.getrandbits(8) for _ in range(0x8000)))
    dst = ctx.alloc(c['dstride'] * (c['h'] + 1) + 8, fill=0x77)
    return dict(args=[dst, source, c['w'], c['h'], c['dstride'], c['sstride'], palette])
