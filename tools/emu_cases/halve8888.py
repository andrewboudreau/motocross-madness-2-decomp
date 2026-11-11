"""Halve8888 0x004cdf10, the 2x2 box filter of 8888 textures (tools/emu_diff.py case).

Blocks with every mix of zero and nonzero alpha (the count switch: 0..4 contributing
pixels) and random channels; strides in pixels. The destination starts filled with 0x77 so
every written byte is compared.
"""
import struct

RETAIL_VA = 0x004cdf10
SYMBOL = '?Halve8888@@YAHPAX0HHHH@Z'
SOURCE = 'samples/render/PixtransNearMisses.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        w = rng.choice([1, 2, 3, 4, 8])
        h = rng.choice([1, 2, 3, 4])
        sstride = 2 * w + rng.choice([0, 0, 1, 3])
        dstride = w + rng.choice([0, 1, 3])
        zero_alpha = rng.choice([0.0, 0.3, 0.7])
        pix = []
        for _ in range(2 * h * sstride + 2):
            p = rng.randrange(1 << 32)
            if rng.random() < zero_alpha:
                p &= 0x00ffffff
            pix.append(p)
        yield (w, h, sstride, dstride, pix)


def setup(ctx, case):
    w, h, sstride, dstride, pix = case
    src = ctx.alloc(4 * len(pix), struct.pack('<%dI' % len(pix), *pix))
    dst = ctx.alloc(4 * dstride * (h + 1) + 8, fill=0x77)
    return dict(args=[dst, src, w, h, dstride, sstride])
