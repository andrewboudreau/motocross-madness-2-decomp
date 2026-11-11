"""DitherConvert 0x004cf2a0, the Floyd-Steinberg texture ditherer (tools/emu_diff.py case).

555 (0x22b), 565 (0x235) and 24-bit (0x378) sources read by retail's ReadRow helpers, widths
1..9 (odd ones included) and heights 1..5, padded strides, any mix of 24-bit, 565, 555 and
8-bit outputs (8-bit only with a palette), with and without a palette (random colours and a
random 555-to-index table, or, for 555 sources, the few colours the image uses with their
555 codes indexed, so quantized levels equal a palette entry and the snap is taken).
The two error rows (0x00689a6c capacity, 0x00689a70/74 rows) start empty, large enough or too
small, so the grow path (DebugFree 0x004a2e60, DebugMalloc 0x004a2e20, noted by size and line;
a NULL result returns 0) runs. Outputs start filled with 0x77, so every written byte, the error
rows included, is compared.
"""
import struct

RETAIL_VA = 0x004cf2a0
SYMBOL = '?DitherConvert@@YAHPAXHHHHH0000PAUUnknownTexturePalette@@@Z'
SOURCE = 'samples/render/PixtransNearMisses.cpp'
FPCWS = (0x027f,)   # integer only
PROBES = {0x004cf31a: 'grow', 0x004cf629: 'snap', 0x004cf353: '555', 0x004cf369: '565', 0x004cf370: '24'}

CAPACITY, ROWS = 0x00689a6c, 0x00689a70
MALLOC, FREE = 0x004a2e20, 0x004a2e60
FORMATS = {0x22b: 2, 0x235: 2, 0x378: 3}


def cases(rng, n):
    for _ in range(n):
        fmt = rng.choice(list(FORMATS))
        w = rng.choice([1, 2, 3, 4, 5, 7, 9])
        h = rng.choice([1, 2, 3, 5])
        sstride = w + rng.choice([0, 0, 1, 3])
        dstride = w + rng.choice([0, 0, 2])
        palette = rng.choice([None, 'random', 'exact'] if fmt == 0x22b else [None, 'random'])
        outs = [rng.random() < 0.6 for _ in range(3)] + [palette is not None and rng.random() < 0.7]
        rows = rng.choice(['empty', 'big', 'small'])
        yield dict(seed=rng.getrandbits(32), fmt=fmt, w=w, h=h, sstride=sstride, dstride=dstride,
                   palette=palette, outs=outs, rows=rows, fail=rng.random() < 0.05,
                   pixels=rng.choice(['random', 'random', 'extreme']))


def setup(ctx, c):
    rng = ctx.rng
    w, h, fmt = c['w'], c['h'], c['fmt']
    bpp = FORMATS[fmt]
    n = c['sstride'] * h * bpp + 16
    codes = [rng.randrange(0x10000) for _ in range(rng.choice([1, 3, 8]))]
    if c['palette'] == 'exact':
        src = b''.join(struct.pack('<H', rng.choice(codes)) for _ in range(n // 2))
    elif c['pixels'] == 'extreme':
        src = bytes(rng.choice([0, 0xff, 0x7f, 0x80]) for _ in range(n))
    else:
        src = bytes(rng.getrandbits(8) for _ in range(n))
    source = ctx.alloc(n, src)

    def malloc(cx, ecx, args):
        cx.note('malloc', args[0], args[2])
        if c['fail']:
            return 0
        return cx.alloc(args[0] + 16, fill=0x5a)

    def free(cx, ecx, args):
        cx.note('free', args[0], args[2])
        return 0
    ctx.stub_at(MALLOC, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(FREE, 'DebugFree', nargs=3, ret=free, log=False)
    if c['rows'] == 'empty':
        ctx.pack(CAPACITY, 'iII', 0, 0, 0)
    else:
        cap = w + 3 if c['rows'] == 'big' else max(w - 1, 0)
        rows = ctx.alloc(cap * 24 + 16, fill=0x33)
        ctx.pack(CAPACITY, 'iII', cap, rows, rows + cap * 12)
    palette = 0
    if c['palette']:
        palette = ctx.alloc(0x710 + 0x8000)
        colors = bytearray(rng.getrandbits(8) for _ in range(768))
        index = bytearray(rng.getrandbits(8) for _ in range(0x8000))
        if c['palette'] == 'exact':
            for k, code in enumerate(codes):
                at = rng.randrange(256)
                colors[3 * at:3 * at + 3] = bytes(((code >> 7) & 0xf8, (code >> 2) & 0xf8, (code << 3) & 0xf8))
                index[code & 0x7fff] = at
        ctx.write(palette + 0x10, bytes(colors))
        ctx.write(palette + 0x710, bytes(index))
    outs = []
    for k, size in enumerate((3, 2, 2, 1)):
        if c['outs'][k]:
            outs.append(ctx.alloc(c['dstride'] * h * size + 8, fill=0x77))
        else:
            outs.append(0)
    return dict(args=[source, fmt, w, h, c['sstride'], c['dstride']] + outs + [palette])
