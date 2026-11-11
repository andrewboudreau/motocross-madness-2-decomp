"""MeanTextureChannelDifference 0x004cb330, mean channel difference of two textures (tools/emu_diff.py case).

Two TextureMaps (vtable slots 13 LockSurface and 14 UnlockSurface stubbed): 555 (format
0x22b) and 565 surfaces, pitches with and without row padding, identical, random and
near-identical pixels, and either lock failing (the -1 returns and the single unlock).
"""
import struct

RETAIL_VA = 0x004cb330
SYMBOL = '?MeanTextureChannelDifference@@YAMPAVTextureMap@@0@Z'
SOURCE = 'samples/render/TextureChannelDifference.cpp'


def cases(rng, n):
    for _ in range(n):
        w = rng.choice([1, 2, 3, 7, 16])
        h = rng.choice([1, 2, 5, 9])
        fmt = rng.choice([0x22b, 0x22b, 0x22c, 0x0])
        pitches = [2 * w + 2 * rng.choice([0, 0, 1, 5]) for _ in range(2)]
        kind = rng.randrange(4)
        a = [rng.randrange(0x10000) for _ in range(h * pitches[0] // 2)]
        if kind == 0:
            b = [rng.randrange(0x10000) for _ in range(h * pitches[1] // 2)]
        else:
            b = [a[(i % (pitches[1] // 2)) + (i // (pitches[1] // 2)) * (pitches[0] // 2)]
                 if i % (pitches[1] // 2) < w else 0 for i in range(h * pitches[1] // 2)]
            if kind == 2:
                b = [p ^ (1 << rng.randrange(16)) if rng.random() < 0.3 else p for p in b]
        fail = rng.choice([None] * 8 + [0, 1])
        yield (w, h, fmt, pitches, a, b, fail)


def setup(ctx, case):
    w, h, fmt, pitches, a, b, fail = case
    surfaces = {}

    def lock(c, ecx, args):
        index, buffer, pitch = surfaces[ecx]
        if index == fail:
            return 0
        c.pack(args[1], 'i', pitch)
        return buffer

    lock_stub = ctx.stub('TextureMap::LockSurface', pop=12, ret=lock)
    unlock_stub = ctx.stub('TextureMap::UnlockSurface', pop=4)
    other = ctx.stub('TextureMap::other', pop=0)
    vt = ctx.vtable([other] * 13 + [lock_stub, unlock_stub] + [other] * 4)
    maps = []
    for index, (pix, pitch) in enumerate(((a, pitches[0]), (b, pitches[1]))):
        buffer = ctx.alloc(2 * len(pix) + 4, struct.pack('<%dH' % len(pix), *pix))
        tm = ctx.alloc(0x80)
        ctx.pack(tm, 'I', vt)
        ctx.pack(tm + 0x14, 'iiii', w if index == 0 else w + 3, h if index == 0 else h + 1, 0,
                 fmt if index == 0 else 0x22c)
        surfaces[tm] = (index, buffer, pitch)
        maps.append(tm)
    return dict(args=maps, ret_float=True, ret_eax=False)
