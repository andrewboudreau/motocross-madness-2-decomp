"""GUIManager::CopyScreenToTexture 0x00486170 (tools/emu_diff.py case, call-record/replay mode).

The menu screen grab: hides the cursor and tool tips, renders and presents a frame,
locks the display's render surface (else the primary), copies `rect` (or the whole
surface) into a DebugMalloc buffer, optionally dims it (1 byte: every other pixel
cleared in a checkerboard; 2 bytes: halved under the 555/565 mask; 3 and 4 bytes: the
first three channels halved), and builds a PCTextureMap from it. The linked body is
samples/ui/GuiScreenTexture.cpp.

Modelled: PixelFormatBytesPerPixel (0x00511970, case-chosen 1..4), the surface's Lock
(+0x64, COM: height, width, pitch and real pixels) and Unlock (+0x80), DebugMalloc (a
real buffer), new and the PCTextureMap constructor, and the texture's CreateSurface (slot
4), which notes the captured pixels: the buffer is the function's only output. The class
name strings for Disable/EnableByClassName (0x00468dd0, 0x00468f10) and the source file
names are literals at different addresses in the two images, so those calls are noted
by text. Allocation never fails here: both bodies write through the results unchecked.

Layout (src/reconstructed/GUIManager.h, TrackGame.h): GUIManager +0x3c background,
+0xd8 the GUI texture manager; g_TrackGame +0x0c display (+0x19c primary, +0x1a8 render
surface), +0x10 render target (+0x28 pixel format), +0x34 child root, vtable +0x20
RenderFrame and +0x24 PresentFrame.
"""
import struct

from tools.emu_cases import bike_race_create as base

RETAIL_VA = 0x00486170
SYMBOL = '?CopyScreenToTexture@GUIManager@@'
SOURCE = 'samples/ui/GuiScreenTexture.cpp'
BYTES_PER_PIXEL, RELEASE_DC, DISABLE, ENABLE, ADVANCE = 0x00511970, 0x00404c80, 0x00468dd0, 0x00468f10, 0x004e8cc0
MALLOC, FREE, NEW, TEXTURE_CTOR = 0x004a2e20, 0x004a2e60, 0x004a3010, 0x004c5f00
TRACK_GAME = 0x0056e26c
PROBES = {0x004861b9: 'release DC', 0x0048625a: 'primary surface', 0x0048628e: 'rect',
          0x004862ea: 'copy', 0x004864d4: 'lock failed', 0x00486397: '1-byte dim', 0x004863dd: '2-byte dim',
          0x004863fd: '3-byte dim', 0x00486432: '4-byte dim'}

FAKE = 0x22000000
LOCK, UNLOCK = FAKE + 0x64, FAKE + 0x80


def _st(ctx):
    return ctx.data['case']


def _class_call(name):
    def call(ctx, rec):
        ctx.note(name, base._cstr(ctx, rec.args[0]))
        return rec.rng.getrandbits(32)
    return call


def _lock(ctx, rec):
    # Lock(this, rect, desc, flags, event)
    c = _st(ctx)
    ctx.note('lock size', struct.unpack('<I', ctx.read(rec.args[2], 4))[0])
    if c['lock'] == 0:
        ctx.pack(rec.args[2] + 0x08, 'IIi', c['h'], c['w'], c['pitch'])
        ctx.pack(rec.args[2] + 0x24, 'I', ctx.data['pixels'])
    return c['lock']


def _malloc(ctx, rec):
    ctx.note('malloc', rec.args[0], rec.args[2])
    return ctx.alloc(rec.args[0] + 0x10, fill=0xcd)


def _free(ctx, rec):
    ctx.note('free', rec.args[0], rec.args[2])
    return 0


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return ctx.alloc(rec.args[0] + 0x10, fill=0xcd)


def _texture_ctor(ctx, rec):
    ctx.pack(rec.ecx, 'I', ctx.data['texture_vt'])
    return rec.ecx


def _create_surface(ctx, rec):
    # CreateSurface(bits, width, height, stride, minimum, format, format, ...)
    w, h = rec.args[1], rec.args[2]
    ctx.note('bits', ctx.read(rec.args[0], w * h * _st(ctx)['bytes']).hex())
    return rec.rng.choice([0, 1])


REPLAY = dict(
    returns={BYTES_PER_PIXEL: lambda ctx, rec: _st(ctx)['bytes'], DISABLE: _class_call('disable'),
             ENABLE: _class_call('enable'), MALLOC: _malloc, FREE: _free, NEW: _new, TEXTURE_CTOR: _texture_ctor},
    pops={BYTES_PER_PIXEL: 0, RELEASE_DC: 0, DISABLE: 4, ENABLE: 4, ADVANCE: 0, MALLOC: 0, FREE: 0, NEW: 0,
          TEXTURE_CTOR: 8},
    args={BYTES_PER_PIXEL: 1, RELEASE_DC: 0, DISABLE: 1, ENABLE: 1, ADVANCE: 0, MALLOC: 3, FREE: 3, NEW: 3,
          TEXTURE_CTOR: 2},
    ecx={BYTES_PER_PIXEL: False, MALLOC: False, FREE: False, NEW: False, LOCK: False, UNLOCK: False},
    unlogged=(DISABLE, ENABLE, MALLOC, FREE, NEW))


def cases(rng, n):
    for _ in range(n):
        bpp = rng.choice([1, 2, 2, 3, 4])
        w, h = rng.randint(1, 40), rng.randint(1, 30)
        rect = None
        if rng.random() < 0.6:
            left, top = rng.randrange(w), rng.randrange(h)
            rect = (left, top, rng.randint(left, w), rng.randint(top, h))
        yield dict(seed=rng.getrandbits(32), bytes=bpp, w=w, h=h, pitch=w * bpp + rng.choice([0, 0, 2, 4, 3]),
                   rect=rect, dim=rng.choice([0, 1, 1]), format=rng.choice([0x22b, 0x235, 8, 0x378]),
                   background=rng.random() < 0.5, render_surface=rng.random() < 0.6,
                   lock=rng.choice([0] * 8 + [0x887601c2]))


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])
    ctx.uc.mem_map(0, 0x1000)                # fs:[0] reads page 0 (the EH frame link)
    ctx.data['case'] = c
    surface_vt = ctx.fake_vtable('IDirectDrawSurface7', 0x21, pops={25: 20, 32: 8},
                                 returns={25: _lock, 32: 'int'})
    assert ctx.read(surface_vt + 0x64, 4) == struct.pack('<I', LOCK)
    ctx.data['texture_vt'] = ctx.fake_vtable('PCTextureMap', 8, pops={4: 68}, returns={4: _create_surface})
    game_vt = ctx.fake_vtable('TrackGame', 12)
    ctx.data['pixels'] = ctx.alloc(c['pitch'] * c['h'], bytes(rng.getrandbits(8) for _ in range(c['pitch'] * c['h'])))

    def surface():
        s = ctx.alloc(0x10)
        ctx.pack(s, 'I', surface_vt)
        return s
    display = ctx.alloc(0x200, fill=0x44)
    ctx.pack(display + 0x19c, 'I', surface())
    ctx.pack(display + 0x1a8, 'I', surface() if c['render_surface'] else 0)
    target = ctx.alloc(0x40, fill=0x77)
    ctx.pack(target + 0x28, 'i', c['format'])
    game = ctx.alloc(0x80, fill=0x66)
    ctx.pack(game, 'I', game_vt)
    ctx.pack(game + 0x0c, 'II', display, target)
    ctx.pack(game + 0x34, 'I', ctx.alloc(0x40))
    ctx.pack(TRACK_GAME, 'I', game)
    gui = ctx.alloc(0x100, fill=0x5a)
    ctx.pack(gui + 0x3c, 'I', ctx.alloc(0x20) if c['background'] else 0)
    ctx.pack(gui + 0xd8, 'I', ctx.alloc(0x20))
    rect = 0
    if c['rect']:
        rect = ctx.alloc(16)
        ctx.pack(rect, '4i', *c['rect'])
    return dict(ecx=gui, args=[c['dim'], rect], ret_eax=True)
