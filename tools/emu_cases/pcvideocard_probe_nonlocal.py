"""PCVideoCard::ProbeNonLocalTextureMemory 0x004ca5a0 (tools/emu_diff.py case, call-record/replay mode).

Whether a 128 x 128 texture of the target's pixel format can be created in non-local video
memory: with no target it first switches to the 640 x 480 x 16 mode and builds a temporary
PCRenderTarget, then asks IDirectDraw7::CreateSurface for the texture and reads its caps
(DDSCAPS_NONLOCALVIDMEM, 0x20000000). `*value` is cleared on every failure; the result is 1
only when the caps carry the flag. The linked body is samples/render/NonLocalTextureMemory.cpp.

Fake COM objects: the card's IDirectDraw7 (+0x190) answers CreateSurface (slot 6) with a
case-chosen HRESULT and writes a fake surface whose GetCaps (slot 14) returns a case-chosen
HRESULT and caps; Release is slot 2. The temporary target's deleting destructor is slot 0
of a fake vtable. SetFullscreenDisplayMode (0x004ca900), InitializeRenderTarget (0x004c4f80,
the target or 0), new and the PCRenderTarget constructor are modelled from the case;
FillPixelFormat (0x005119c0) is replayed. The surface description and caps the body passes
are logged by pointee.

Layout (src/reconstructed/PCVideoCard.h): +0x08 the mode count, +0x10 the 36-byte mode
table, +0x78 the frame buffer count, +0x190 IDirectDraw7, +0x1a0 the back buffer; a render
target's pixel format is at +0x28. With no target and no usable 640 x 480 x 16 mode both
bodies read the format through a null target (page 0 is mapped for fs:[0]); real retail
would fault there, so such cases are kept rare.
"""
import struct

RETAIL_VA = 0x004ca5a0
SYMBOL = '?ProbeNonLocalTextureMemory@PCVideoCard@@QAEHPAHPAVRenderTarget@@@Z'
SOURCE = 'samples/render/NonLocalTextureMemory.cpp'
SET_MODE, NEW, TARGET_CTOR, INIT_TARGET = 0x004ca900, 0x004a3010, 0x004c4ee0, 0x004c4f80
PROBES = {0x004ca616: 'set 640x480x16', 0x004ca674: 'temporary target', 0x004ca6f8: 'GetCaps',
          0x004ca718: 'non-local', 0x004ca746: 'delete temporary', 0x004ca75a: 'delete after failure'}

MODES = [(640, 480, 16), (640, 480, 32), (800, 600, 16), (640, 400, 16), (1024, 768, 16)]


def _st(ctx):
    return ctx.data['case']


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return 0 if _st(ctx)['new_fails'] else ctx.alloc(0xbc4 + 0x10, fill=0xcd)


def _target_ctor(ctx, rec):
    ctx.pack(rec.ecx, 'I', ctx.data['target_vt'])
    ctx.pack(rec.ecx + 0x28, 'i', _st(ctx)['temp_format'])
    return rec.ecx


def _create_surface(ctx, rec):
    # IDirectDraw7::CreateSurface(this, desc, &surface, outer)
    c = _st(ctx)
    ctx.note('desc', ctx.read(rec.args[1], 0x7c).hex())
    if c['create_hr'] == 0:
        ctx.pack(rec.args[2], 'I', ctx.data['surface'])
    return c['create_hr']


def _get_caps(ctx, rec):
    # IDirectDrawSurface7::GetCaps(this, DDSCAPS2*)
    c = _st(ctx)
    if c['caps_hr'] == 0:
        ctx.pack(rec.args[1], '4I', c['caps'], 0, 0, 0)
    return c['caps_hr']


# The COM methods are __stdcall (`this` pushed, ECX scratch). setup builds the surface's and
# then DirectDraw's fake vtables first, so their slots sit at fixed fake addresses
# (Context.fake_vtable hands them out from 0x22000000 in order).
FAKE, SURFACE_SLOTS, DD_SLOTS = 0x22000000, 0x30, 0x20
SURFACE_RELEASE, SURFACE_GETCAPS = FAKE + 4 * 2, FAKE + 4 * 14
DD_CREATE_SURFACE = FAKE + 4 * (SURFACE_SLOTS + 6)
REPLAY = dict(
    returns={SET_MODE: lambda ctx, rec: _st(ctx)['set_mode'], NEW: _new, TARGET_CTOR: _target_ctor,
             INIT_TARGET: lambda ctx, rec: rec.ecx if _st(ctx)['init_ok'] else 0},
    pops={NEW: 0}, args={NEW: 3},
    ecx={NEW: False, SURFACE_RELEASE: False, SURFACE_GETCAPS: False, DD_CREATE_SURFACE: False},
    # new passes __FILE__, which names the sample here and PCVideoCard.cpp in retail.
    unlogged=(NEW,))


def cases(rng, n):
    for _ in range(n):
        with_target = rng.random() < 0.4
        modes = [rng.choice(MODES) for _ in range(rng.randint(1, 5))]
        if not with_target and rng.random() < 0.93 and (640, 480, 16) not in modes:
            modes[rng.randrange(len(modes))] = (640, 480, 16)
        yield dict(
            seed=rng.getrandbits(32), with_target=with_target, modes=modes,
            set_mode=rng.choice([1, 1, 1, 0]), new_fails=rng.random() < 0.1, init_ok=rng.random() < 0.85,
            temp_format=rng.randrange(12), target_format=rng.randrange(12), frames=rng.choice([1, 2, 3]),
            create_hr=rng.choice([0, 0, 0, 0x887600e1, 0x8007000e]),
            caps_hr=rng.choice([0, 0, 0, 0x88760096]),
            caps=rng.choice([0x20005000, 0x00005000, 0x20001000, 0x0000d000, 0xffffffff, 0]))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)                # fs:[0] reads page 0 (the EH frame link)
    ctx.data['case'] = c
    surface_vt = ctx.fake_vtable('IDirectDrawSurface7', SURFACE_SLOTS, pops={2: 4, 14: 8},
                                 returns={2: 'int', 14: _get_caps})
    dd_vt = ctx.fake_vtable('IDirectDraw7', DD_SLOTS, pops={6: 16}, returns={6: _create_surface})
    assert ctx.read(dd_vt + 4 * 6, 4) == struct.pack('<I', DD_CREATE_SURFACE)
    ctx.data['target_vt'] = ctx.fake_vtable('PCRenderTarget', 4, pops={0: 4}, returns={0: 'ptr'})
    surface = ctx.alloc(0x20)
    ctx.pack(surface, 'I', surface_vt)
    ctx.data['surface'] = surface
    dd = ctx.alloc(0x20)
    ctx.pack(dd, 'I', dd_vt)

    table = ctx.alloc(36 * len(c['modes']), b''.join(struct.pack('<4i20x', w, h, bpp, 60) for w, h, bpp in c['modes']))
    card = ctx.alloc(0x200, fill=0x5a)
    ctx.pack(card + 0x08, 'i', len(c['modes']))
    ctx.pack(card + 0x10, 'I', table)
    ctx.pack(card + 0x78, 'i', c['frames'])
    ctx.pack(card + 0x190, 'I', dd)
    ctx.pack(card + 0x1a0, 'I', ctx.alloc(0x20))
    target = 0
    if c['with_target']:
        target = ctx.alloc(0x40, fill=0x77)
        ctx.pack(target + 0x28, 'i', c['target_format'])
    value = ctx.alloc(8, fill=0xee)
    return dict(ecx=card, args=[value, target], ret_eax=True)
