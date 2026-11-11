"""RenderTarget::AdvanceFrame 0x004e8cc0, the frame counter UIDialog's close waits on
(tools/emu_diff.py case).

The linked body is src/reconstructed/RenderTarget.cpp. Unless g_TrackGame 0x0056e26c's
display (+0xc) freezes the frame index (+0x6c), frameIndex (+0x18) steps and wraps to 0 at
frameModulus (+0x14); frameCount (+0x1c) always steps. Game::Tick calls it once per frame
right after the root's ReleaseDetached (retail 0x00468844), so a dialog whose EndDialog
recorded frameCount is marked for release by the next frame's RenderPost3D. The index sits
at, below or past the modulus (which can be 0 or negative) and the count can wrap.
"""
import random

RETAIL_VA = 0x004e8cc0
SYMBOL = '?AdvanceFrame@RenderTarget@@QAEXXZ'
SOURCE = 'src/reconstructed/RenderTarget.cpp'
FPCWS = (0x027f,)
TRACK_GAME = 0x0056e26c
PROBES = {0x004e8ccf: 'index steps', 0x004e8cdd: 'index wraps'}


def cases(rng, n):
    for _ in range(n):
        modulus = rng.choice([1, 2, 3, 60, 0, -1, rng.getrandbits(32)])
        index = rng.choice([modulus - 1, modulus, 0, modulus + 1, rng.getrandbits(32)])
        yield dict(seed=rng.getrandbits(32), freeze=rng.choice([0, 0, 0, 1, rng.getrandbits(32)]),
                   modulus=modulus & 0xffffffff, index=index & 0xffffffff,
                   count=rng.choice([0, 0xffffffff, 0x7fffffff, rng.getrandbits(32)]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    display = ctx.alloc(0x80, bytes(r.getrandbits(8) for _ in range(0x80)))
    ctx.pack(display + 0x6c, 'I', c['freeze'])
    game = ctx.alloc(0x20, bytes(r.getrandbits(8) for _ in range(0x20)))
    ctx.pack(game + 0xc, 'I', display)
    ctx.pack(TRACK_GAME, 'I', game)
    target = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    ctx.pack(target + 0x14, 'III', c['modulus'], c['index'], c['count'])
    return dict(ecx=target, args=[], ret_eax=False)    # void
