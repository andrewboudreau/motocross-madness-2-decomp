"""TextureHasColor 0x0047b490, the menu texture colour scan (tools/emu_diff.py case).

565 (0x235), 555 (0x22b) and other formats, keys that map to pixel 0 (no scan), keys
present in the first, a middle or the last row or absent, and lock (slot 13) or unlock
(slot 14) failing. Rows are `textureWidth` pixels apart, as retail steps them.
"""
import struct

RETAIL_VA = 0x0047b490
SYMBOL = '?TextureHasColor@@YAHPAVTextureMap@@H@Z'
SOURCE = 'samples/ui/TextureHasColor.cpp'
FPCWS = (0x027f,)   # integer only


def pixel_of(fmt, key):
    if fmt == 0x235:
        return ((key >> 8) & 0xf800) | ((key >> 5) & 0x7e0) | ((key >> 3) & 0x1f)
    return ((key >> 9) & 0x7c00) | ((key >> 6) & 0x3e0) | ((key >> 3) & 0x1f)


def cases(rng, n):
    for _ in range(n):
        w = rng.choice([1, 2, 5, 16])
        h = rng.choice([1, 2, 4, 7])
        fmt = rng.choice([0x235, 0x235, 0x22b, 0x22b, 0x22c])
        key = rng.choice([rng.randrange(1 << 24), 0x0000ff, 0xff00ff, 0x000007, 0, rng.randrange(1 << 32)])
        pix = [rng.randrange(0x10000) for _ in range(w * h)]
        where = rng.choice(['none', 'first', 'last', 'any'])
        target = pixel_of(fmt, key)
        if where == 'first':
            pix[rng.randrange(w)] = target
        elif where == 'last':
            pix[w * (h - 1) + rng.randrange(w)] = target
        elif where == 'any':
            pix[rng.randrange(w * h)] = target
        fail = rng.choice([None] * 6 + ['lock', 'unlock'])
        yield (w, h, fmt, key, pix, fail)


def setup(ctx, case):
    w, h, fmt, key, pix, fail = case
    buffer = ctx.alloc(2 * len(pix) + 4, struct.pack('<%dH' % len(pix), *pix))
    lock = ctx.stub('TextureMap::LockSurface', pop=12, ret=0 if fail == 'lock' else buffer)
    unlock = ctx.stub('TextureMap::UnlockSurface', pop=4, ret=0 if fail == 'unlock' else 1)
    other = ctx.stub('TextureMap::other', pop=0)
    vt = ctx.vtable([other] * 13 + [lock, unlock] + [other] * 4)
    tm = ctx.alloc(0x80)
    ctx.pack(tm, 'I', vt)
    ctx.pack(tm + 0x14, 'iiii', w, h, 0, fmt)
    return dict(args=[tm, key])
