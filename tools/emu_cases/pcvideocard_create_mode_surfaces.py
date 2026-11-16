"""PCVideoCard::CreateModeSurfaces 0x004ca520 (tools/emu_diff.py case).

The surfaces for the current display mode. Full screen first checks the video memory
budget: the whole chain (w * h * bpp / 8 per buffer, signed division) must fit in the
budget (+0x74) less one 16-bit frame (w * h * 2); then CreateFlipChain (0x004c9f30) gets
bufferCount - 1 back buffers. Windowed goes to CreateSystemRenderSurface (0x004ca130).
The linked body is samples/render/ModeSurfaces.cpp.

Layout (src/reconstructed/PCVideoCard.h, DisplayMode.h): +0x0c the current mode index,
+0x10 the 36-byte mode table (+0 width, +4 height, +8 bit depth, +0x10 buffer count), +0x74
the video memory budget. Both creators are logged stubs with case-chosen results.
Budgets around the threshold, negative budgets (the case where retail quits) and products
that overflow 32 bits are all covered.
"""
import struct

RETAIL_VA = 0x004ca520
SYMBOL = '?CreateModeSurfaces@PCVideoCard@@QAEHH@Z'
SOURCE = 'samples/render/ModeSurfaces.cpp'
FLIP_CHAIN, SYSTEM_SURFACE = 0x004c9f30, 0x004ca130
PROBES = {0x004ca569: 'budget fits', 0x004ca58b: 'failure', 0x004ca580: 'windowed'}

MODES = [(640, 480, 16), (800, 600, 16), (1024, 768, 16), (640, 480, 32), (1280, 1024, 32),
         (1600, 1200, 32), (320, 200, 8), (512, 384, 24)]


def cases(rng, n):
    for _ in range(n):
        modes = [rng.choice(MODES) + (rng.choice([1, 2, 2, 3, 0, -1]),) for _ in range(4)]
        current = rng.randrange(4)
        w, h, bpp, buffers = modes[current]
        need = w * h * bpp // 8 * buffers + w * h * 2
        if rng.random() < 0.3:
            w, h = rng.choice([(46341, 46341), (65536, 32768), (0x7fff, 0x7fff)])
            modes[current] = (w, h, bpp, buffers)
        budget = rng.choice([need, need - 1, need + 1, need * 2, 0, -1, -rng.randrange(1 << 30),
                             rng.randrange(1 << 31), 4 << 20, 8 << 20])
        yield dict(modes=modes, current=current, budget=max(min(budget, 0x7fffffff), -0x80000000),
                   full=rng.choice([0, 1, 1, 2]), flip=rng.choice([0, 1, 7]), system=rng.choice([0, 1, 5]))


def setup(ctx, c):
    ctx.stub_at(FLIP_CHAIN, 'CreateFlipChain', pop=4, nargs=1, ret=lambda cx, ecx, a: c['flip'])
    ctx.stub_at(SYSTEM_SURFACE, 'CreateSystemRenderSurface', pop=4, nargs=1, ret=lambda cx, ecx, a: c['system'])
    table = ctx.alloc(36 * 4, b''.join(struct.pack('<5i16x', w, h, bpp, 60, buffers)
                                        for w, h, bpp, buffers in c['modes']))
    card = ctx.alloc(0x100, fill=0x5a)
    ctx.pack(card + 0x0c, 'iI', c['current'], table)
    ctx.pack(card + 0x74, 'i', c['budget'])
    return dict(ecx=card, args=[c['full']], ret_eax=True)
