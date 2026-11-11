"""Convert555To8Fast 0x004d0d40, 555 pixels to palette indices (tools/emu_diff.py case).

Widths that are multiples of eight (the only ones the caller 0x004d0c40 passes), heights 1..6,
destination strides at least the width, random pixels with the top bit set or clear (the
table index is the low 15 bits) and a random 32 KiB index table. Retail's unrolled assembly
preloads four bytes past the source end, so the source buffer gets padding.
"""
import struct

RETAIL_VA = 0x004d0d40
SYMBOL = '?Convert555To8Fast@@YAXPAX0HHHPAE@Z'
SOURCE = 'src/bringup/Group0_Pixtrans.cpp'
FPCWS = (0x027f,)   # integer only


def cases(rng, n):
    for _ in range(n):
        w = 8 * rng.choice([1, 2, 3, 5])
        h = rng.choice([1, 2, 3, 6])
        dstride = w + rng.choice([0, 0, 1, 7])
        yield (w, h, dstride, rng.randrange(1 << 30))


def setup(ctx, case):
    w, h, dstride, seed = case
    rng = ctx.rng
    pix = [rng.randrange(0x10000) for _ in range(w * h)]
    src = ctx.alloc(2 * len(pix) + 16, struct.pack('<%dH' % len(pix), *pix) + bytes(16))
    table = ctx.alloc(0x8000, bytes(rng.randrange(256) for _ in range(0x8000)))
    dst = ctx.alloc(dstride * h + 8, fill=0x77)
    return dict(args=[src, dst, dstride, w, h, table], ret_eax=False)
