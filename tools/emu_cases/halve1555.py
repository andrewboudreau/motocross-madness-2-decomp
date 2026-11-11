"""Halve1555 0x004ce420, the 2x2 box filter of 1555 textures (tools/emu_diff.py case).

Blocks with every mix of opaque (bit 15) and transparent pixels, so the < 2 opaque branch
(written as 0) and the rounded 2-, 3- and 4-pixel averages are all taken; random, all-ones and
all-zero channels; widths and heights 1..8 (odd ones included) and padded strides in pixels.
The destination starts filled with 0x77 so every written byte is compared.
"""
import struct

RETAIL_VA = 0x004ce420
SYMBOL = '?Halve1555@@YAHPAX0HHHH@Z'
SOURCE = 'samples/render/PixtransNearMisses.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        w = rng.choice([1, 2, 3, 4, 5, 8])
        h = rng.choice([1, 2, 3, 5, 8])
        sstride = 2 * w + rng.choice([0, 0, 1, 3])
        dstride = w + rng.choice([0, 1, 3])
        opaque = rng.choice([0.2, 0.5, 0.8, 1.0])
        kind = rng.randrange(3)
        pix = []
        for _ in range(2 * h * sstride + 2):
            if kind == 1:
                p = rng.choice([0x7fff, 0x0000, 0x7c00, 0x03e0, 0x001f])
            elif kind == 2:
                p = rng.choice([0x7fff, 0x0000])
            else:
                p = rng.randrange(0x8000)
            if rng.random() < opaque:
                p |= 0x8000
            pix.append(p)
        yield (w, h, sstride, dstride, pix)


def setup(ctx, case):
    w, h, sstride, dstride, pix = case
    src = ctx.alloc(2 * len(pix), struct.pack('<%dH' % len(pix), *pix))
    dst = ctx.alloc(2 * dstride * (h + 1) + 8, fill=0x77)
    return dict(args=[dst, src, w, h, dstride, sstride])
