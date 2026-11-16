"""PCGame::ProfileDisplays 0x004c0d10 (tools/emu_diff.py case, call-record/replay mode).

Unless every display's cached profile loads (LoadDisplayProfile, 0x004c16f0), profiles each
display into the registry under DriverInfo\\<guid>\\: its mode list, whether windowed,
full screen (IDirectDraw7::SetDisplayMode 640 x 480 x 16), software and hardware rendering
work, its video memory (GetAvailableVidMem), AGP (ProbeNonLocalTextureMemory), whether ten
256 x 256 textures fit (CreateSurface) and the partial texture blit result
(ProbePartialTextureUploads). Returns 1. The linked body is samples/game/PCGameNearMisses.cpp.

Fakes: PCGame's registry helpers are virtual (+0x50 GetRegistryInt, +0x58 GetRegistryFlag,
+0x64 SetRegistryInt, +0x6c SetRegistryFlag, +0x74 SetRegistryBinary, +0x88
FilterDisplayModes); each notes the key text, and the Get answers are a hash of the case
seed and the key, so both bodies see the same registry. Each display has a fake vtable
(+0x08 SetDisplayMode, 7 arguments) and its own IDirectDraw7 (COM, ECX scratch: +0x18
CreateSurface failing at a case-chosen index, +0x54 SetDisplayMode, +0x5c
GetAvailableVidMem). sprintf (0x00534aaf) formats for real; the format literals sit at
different addresses in the two images, so the call itself is unlogged and the text noted.
The display methods (0x004c9d20, 0x004c9c90, 0x004c9b50), both probes, new, the
PCRenderTarget constructor and InitializeRenderTarget are modelled per display;
LimitDisplayModes (0x004c0760) and FillPixelFormat (0x005119c0) are replayed.

Layout (src/reconstructed/PCGame.h, PCVideoCard.h): game +0x0c display, +0x10 render
target, +0x434 the OS platform id; display +0x08 mode count, +0x10 mode table, +0x54
total video memory, +0x78 frame buffers, +0x190 IDirectDraw7, +0x1a0 back buffer, +0x1b8
driver caps, +0x4bc the GUID text, +0x5bc the partial upload result, +0x9f0 isAGP; target
+0x28 pixel format, +0x164 device caps. g_Displays 0x0068a754, g_DisplayCount 0x0068a764.
"""
import random
import struct
import zlib

from tools.emu_cases import bike_race_create as base

RETAIL_VA = 0x004c0d10
SYMBOL = '?ProfileDisplays@PCGame@@QAEHXZ'
SOURCE = 'samples/game/PCGameNearMisses.cpp'
SPRINTF, NEW, LOAD_PROFILE, LIMIT_MODES = 0x00534aaf, 0x004a3010, 0x004c16f0, 0x004c0760
SET_WINDOWED, SET_FULLSCREEN, REFRESH_CAPS = 0x004c9d20, 0x004c9c90, 0x004c9b50
TARGET_CTOR, INIT_TARGET, PROBE_AGP, PROBE_PARTIAL = 0x004c4ee0, 0x004c4f80, 0x004ca5a0, 0x004cab00
DISPLAYS, DISPLAY_COUNT = 0x0068a754, 0x0068a764
PROBES = {0x004c0d67: 'not all cached', 0x004c0eb8: 'windowed works', 0x004c0ef8: 'full screen tried',
          0x004c0f3d: 'full screen works', 0x004c1079: 'software works', 0x004c10f3: 'hardware works',
          0x004c118c: 'limit 800x600', 0x004c1198: 'limit 640x480', 0x004c11e3: 'AGP probe',
          0x004c130f: 'ten textures', 0x004c13be: 'texture failed', 0x004c1367: 'partial blt probe'}

# COM methods are __stdcall (`this` pushed, ECX scratch). setup builds DirectDraw's and then
# the surfaces' fake vtables first, so their slots sit at fixed fake addresses
# (Context.fake_vtable hands them out from 0x22000000 in order).
FAKE, DD_SLOTS = 0x22000000, 0x20
DD_CREATE_SURFACE, DD_SET_MODE, DD_VIDMEM = FAKE + 0x18, FAKE + 0x54, FAKE + 0x5c
SURFACE_RELEASE = FAKE + 4 * DD_SLOTS + 8
COM = (DD_CREATE_SURFACE, DD_SET_MODE, DD_VIDMEM, SURFACE_RELEASE)
MODES = [(640, 480, 16), (800, 600, 16), (1024, 768, 32), (512, 384, 16)]


def _st(ctx):
    return ctx.data['case']


def _display(ctx, at):
    return ctx.data['displays'][at]


def _key(ctx, rec):
    return base._cstr(ctx, rec.args[0])


def _registry_answer(ctx, key, choices):
    return random.Random(zlib.crc32(key) ^ _st(ctx)['seed']).choice(choices)


def _get_flag(ctx, rec):
    key = _key(ctx, rec)
    ctx.note('GetRegistryFlag', key, rec.args[1])
    return _registry_answer(ctx, key, [0, 0, 0, 0, 1])


def _get_int(ctx, rec):
    key = _key(ctx, rec)
    ctx.note('GetRegistryInt', key, rec.args[1])
    return _registry_answer(ctx, key, [1, 1, 1, 2, 0, -1])


def _set(name):
    def call(ctx, rec):
        ctx.note(name, _key(ctx, rec), *rec.args[1:])
        return rec.rng.getrandbits(32)
    return call


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return 0 if _st(ctx)['new_fails'] else ctx.alloc(rec.args[0] + 0x10, fill=0xcd)


def _init_target(ctx, rec):
    d = _display(ctx, rec.args[0])
    if not rec.ecx or not d['init_ok']:
        return 0
    ctx.pack(rec.ecx, 'I', ctx.data['target_vt'])
    ctx.pack(rec.ecx + 0x28, 'i', d['format'])
    ctx.pack(rec.ecx + 0x164, 'I', d['device_caps'])
    return rec.ecx


def _create_surface(ctx, rec):
    # IDirectDraw7::CreateSurface(this, desc, &surface, outer)
    d = _display(ctx, ctx.data['dd_owner'][rec.args[0]])
    ctx.note('desc', ctx.read(rec.args[1], 0x7c).hex())
    k = d['created'] = d.get('created', 0) + 1
    if k - 1 == d['surface_fail']:
        return 0x887600e1
    surface = ctx.alloc(0x10)
    ctx.pack(surface, 'I', ctx.data['surface_vt'])
    ctx.pack(rec.args[2], 'I', surface)
    return 0


def _vidmem(ctx, rec):
    # IDirectDraw7::GetAvailableVidMem(this, DDSCAPS2*, &total, &free)
    d = _display(ctx, ctx.data['dd_owner'][rec.args[0]])
    ctx.note('caps', ctx.read(rec.args[1], 16).hex())
    ctx.pack(rec.args[2], 'I', d['total'])
    ctx.pack(rec.args[3], 'I', d['total'] // 2)
    return 0


def _probe_partial(ctx, rec):
    d = _display(ctx, rec.ecx)
    ctx.pack(rec.ecx + 0x5bc, 'i', d['partial'])
    return d['partial']


def _per_display(field):
    return lambda ctx, rec: _display(ctx, rec.ecx)[field]


REPLAY = dict(
    returns={SPRINTF: base._sprintf, NEW: _new, TARGET_CTOR: lambda ctx, rec: rec.ecx, INIT_TARGET: _init_target,
             LOAD_PROFILE: lambda ctx, rec: _display(ctx, rec.args[0])['loaded'],
             SET_WINDOWED: _per_display('windowed'), SET_FULLSCREEN: _per_display('fullscreen'),
             PROBE_AGP: _per_display('agp'), PROBE_PARTIAL: _probe_partial},
    pops={SPRINTF: 0, NEW: 0, LOAD_PROFILE: 4, LIMIT_MODES: 12, SET_WINDOWED: 16, SET_FULLSCREEN: 0,
          REFRESH_CAPS: 0, TARGET_CTOR: 0, INIT_TARGET: 20, PROBE_AGP: 8, PROBE_PARTIAL: 4},
    args={SPRINTF: 3, NEW: 3, LOAD_PROFILE: 1, LIMIT_MODES: 3, SET_WINDOWED: 4, SET_FULLSCREEN: 0,
          REFRESH_CAPS: 0, TARGET_CTOR: 0, INIT_TARGET: 5, PROBE_AGP: 2, PROBE_PARTIAL: 1},
    ecx={SPRINTF: False, NEW: False, **{va: False for va in COM}},
    unlogged=(SPRINTF, NEW))


def _display_case(rng):
    return dict(
        loaded=rng.choice([0, 0, 1]), guid='{%08X-%04X}' % (rng.getrandbits(32), rng.getrandbits(16)),
        modes=[rng.choice(MODES) for _ in range(rng.randint(1, 4))], frames=rng.choice([1, 2]),
        windowed=rng.choice([0, 1, 1]), fullscreen=rng.choice([0, 1, 1, 1, 1]),
        dd_mode_hr=rng.choice([0, 0, 0, 0, 0x88760091]), total=rng.choice([4 << 20, 8 << 20, 16 << 20, 0]),
        set_mode=[rng.choice([0, 1, 1]), rng.choice([0, 1, 1, 1])],
        driver_caps=rng.choice([1, 1, 1, 0x401, 0, 0x400, 0xffffffff]),
        init_ok=rng.random() < 0.9, format=rng.randrange(12), device_caps=rng.choice([0x4000, 0, 0, 0xbfff, 0xffffffff]),
        agp=rng.choice([0, 1]), surface_fail=rng.choice([None] * 6 + [0, 3, 9]),
        partial=rng.choice([1, 0, -1, -2, -3, -4]))


def cases(rng, n):
    for _ in range(n):
        displays = [_display_case(rng) for _ in range(rng.choice([1, 1, 2, 3]))]
        if rng.random() < 0.1:
            for d in displays:
                d['loaded'] = 1
        yield dict(seed=rng.getrandbits(32), displays=displays, platform=rng.choice([1, 1, 2]),
                   new_fails=rng.random() < 0.05)


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)                # fs:[0] reads page 0 (the EH frame link)
    dd_vt = ctx.fake_vtable('IDirectDraw7', DD_SLOTS, pops={6: 16, 21: 24, 23: 16},
                            returns={6: _create_surface, 21: lambda ctx_, rec: ctx_.data['dd_hr'][rec.args[0]],
                                     23: _vidmem})
    ctx.data['surface_vt'] = ctx.fake_vtable('IDirectDrawSurface7', 4, pops={2: 4}, returns={2: 'int'})
    assert ctx.read(dd_vt + 0x18, 4) == struct.pack('<I', DD_CREATE_SURFACE)
    ctx.data['target_vt'] = ctx.fake_vtable('PCRenderTarget', 4, pops={0: 4}, returns={0: 'ptr'})
    game_vt = ctx.fake_vtable('PCGame', 0x24, pops={0x50 // 4: 8, 0x58 // 4: 8, 0x64 // 4: 8, 0x6c // 4: 8,
                                                    0x74 // 4: 12, 0x88 // 4: 4},
                              returns={0x50 // 4: _get_int, 0x58 // 4: _get_flag, 0x64 // 4: _set('SetRegistryInt'),
                                       0x6c // 4: _set('SetRegistryFlag'), 0x74 // 4: _set('SetRegistryBinary'),
                                       0x88 // 4: 'int'})
    ctx.data.update(case=c, displays={}, dd_owner={}, dd_hr={})

    def display_set_mode(ctx_, rec):
        d = _display(ctx_, rec.ecx)
        k = d['set_mode_calls'] = d.get('set_mode_calls', 0) + 1
        return d['set_mode'][min(k, 2) - 1]
    display_vt = ctx.fake_vtable('PCVideoCard', 4, pops={2: 28}, returns={2: display_set_mode})

    ctx.pack(DISPLAY_COUNT, 'i', len(c['displays']))
    for k in range(4):
        ctx.pack(DISPLAYS + 4 * k, 'I', 0)
    for k, d in enumerate(c['displays']):
        d = dict(d)
        card = ctx.alloc(0xa00, fill=0x5a)
        ctx.data['displays'][card] = d
        table = ctx.alloc(36 * len(d['modes']), b''.join(struct.pack('<4i20x', w, h, bpp, 60) for w, h, bpp in d['modes']))
        dd = ctx.alloc(0x10)
        ctx.pack(dd, 'I', dd_vt)
        ctx.data['dd_owner'][dd] = card
        ctx.data['dd_hr'][dd] = d['dd_mode_hr']
        ctx.pack(card, 'I', display_vt)
        ctx.pack(card + 0x08, 'i', len(d['modes']))
        ctx.pack(card + 0x10, 'I', table)
        ctx.pack(card + 0x78, 'i', d['frames'])
        ctx.pack(card + 0x190, 'I', dd)
        ctx.pack(card + 0x1a0, 'I', ctx.alloc(0x20))
        ctx.pack(card + 0x1b8, 'I', d['driver_caps'])
        ctx.write(card + 0x4bc, d['guid'].encode() + b'\0')
        ctx.pack(DISPLAYS + 4 * k, 'I', card)
        if d['surface_fail'] is None:
            d['surface_fail'] = 10
    game = ctx.alloc(0x500, fill=0x6a)
    ctx.pack(game, 'I', game_vt)
    ctx.pack(game + 0x434, 'i', c['platform'])
    return dict(ecx=game, args=[], ret_eax=True)
