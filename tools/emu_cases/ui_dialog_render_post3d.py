"""UIDialog::RenderPost3D 0x0046f320, where a closing dialog marks itself for release
(tools/emu_diff.py case).

The linked body is src/reconstructed/GameUi.cpp. A dialog that is not paused (bit 2 of +0x25)
runs GameObject::RenderPost3D 0x004693a0 (a stub here). A paused dialog with a screen grab
(+0x7f44) and post-3D controls (+0x7f18) draws the grab through its background (+0x110,
DrawImage 0x00404480 with the grab region +0x7f48 and copy frames +0x7f4c) or, without
one, through its render context's (+0x18) slot 3 Blit. Then a closing dialog (+0x7f50) whose
closeFrame (+0x7f54) differs from the render context's frameCount (+0x1c) calls its own
slot 26, MarkPendingRelease. The frame count equals closeFrame, is one past it or is random.
"""
import random

RETAIL_VA = 0x0046f320
SYMBOL = '?RenderPost3D@UIDialog@@UAEHXZ'
SOURCE = 'src/reconstructed/GameUi.cpp'
FPCWS = (0x027f,)
GAME_OBJECT_RENDER_POST3D, DRAW_IMAGE = 0x004693a0, 0x00404480
PROBES = {0x0046f329: 'not paused', 0x0046f36f: 'DrawImage', 0x0046f38a: 'Blit',
          0x0046f3a5: 'mark pending release'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32), paused=rng.random() < 0.5,
                   grab=rng.random() < 0.8, post=rng.random() < 0.8, background=rng.random() < 0.5,
                   closing=rng.choice([0, 1, 1, rng.getrandbits(32)]), frame=rng.choice(['same', 'next', 'random']))


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.stub_at(GAME_OBJECT_RENDER_POST3D, 'GameObject::RenderPost3D', ret=r.getrandbits(32))
    ctx.stub_at(DRAW_IMAGE, 'UIBackground::DrawImage', pop=0x20, ret=r.getrandbits(32))
    blit = ctx.stub('RenderTarget::Blit', pop=0x10, ret=r.getrandbits(32))
    mark = ctx.stub('UIDialog slot 26')
    target = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    ctx.pack(target, 'I', ctx.vtable([0, 0, 0, blit, 0, 0, 0, 0]))
    dlg = ctx.alloc(0x7f58, bytes(r.getrandbits(8) for _ in range(0x7f58)))
    ctx.pack(dlg, 'I', ctx.vtable([0] * 26 + [mark] + [0] * 8))
    flags = r.getrandbits(8) & ~4 | (4 if c['paused'] else 0)
    ctx.pack(dlg + 0x25, 'B', flags)
    ctx.pack(dlg + 0x18, 'I', target)
    ctx.pack(dlg + 0x110, 'I', ctx.alloc(0x40, fill=0x42) if c['background'] else 0)
    ctx.pack(dlg + 0x7f18, 'I', r.getrandbits(32) | 1 if c['post'] else 0)
    ctx.pack(dlg + 0x7f44, 'I', ctx.alloc(0x40, fill=0x47) if c['grab'] else 0)
    close_frame = r.getrandbits(32)
    count = {'same': close_frame, 'next': (close_frame + 1) & 0xffffffff, 'random': r.getrandbits(32)}[c['frame']]
    ctx.pack(dlg + 0x7f50, 'II', c['closing'], close_frame)
    ctx.pack(target + 0x1c, 'I', count)
    return dict(ecx=dlg, args=[])
