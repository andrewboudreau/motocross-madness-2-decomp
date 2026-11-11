"""Halve555 0x004ce5f0, the 2x2 box filter of 555 textures (tools/emu_diff.py case).

All three paths: filter bit 0 (packed sums; the inner loop counts `height` pixels as retail
does), bit 1 (keyed average, with pixels equal to the key mixed in, and blocks that are all
key) and the plain rounded average. The destination starts filled with 0x77 so every written
byte is compared. ``halve565`` reuses this module with the other entry point.
"""
import struct

RETAIL_VA = 0x004ce5f0
SYMBOL = '?Halve555@@YAHPAX0HHHHHG@Z'
SOURCE = 'src/bringup/Group0_Pixtrans.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        w = rng.choice([1, 2, 3, 4, 8])
        h = rng.choice([1, 2, 3, 4, 8])
        sstride = 2 * max(w, h) + rng.choice([0, 0, 2, 5])
        dstride = max(w, h) + rng.choice([0, 1, 3])
        flt = rng.choice([0, 1, 2, 3, 4])
        key = rng.choice([0, 0x7c1f, 0xf81f, rng.randrange(0x10000)])
        rows = 2 * h
        kind = rng.randrange(3)
        pix = []
        for _ in range(rows * sstride + 4):
            if kind == 1 and rng.random() < 0.4:
                pix.append(key)
            elif kind == 2:
                pix.append(rng.choice([0, 0xffff, 0x7fff, key]))
            else:
                pix.append(rng.randrange(0x10000))
        yield (w, h, sstride, dstride, flt, key, pix)


def setup(ctx, case):
    w, h, sstride, dstride, flt, key, pix = case
    src = ctx.alloc(2 * len(pix), struct.pack('<%dH' % len(pix), *pix))
    dst = ctx.alloc(2 * dstride * (h + 1) + 8, fill=0x77)
    return dict(args=[dst, src, w, h, dstride, sstride, flt, key])
