"""PCTextureMap::CreateSurface 0x004c69c0 (tools/emu_diff.py case, call-record/replay mode).

The texture set-up: picks the format (from `choice` when given), counts the mip levels
down to `minimumSize`, grows the manager's scratch buffer for a converted mip chain,
reuses a shared 555/565 surface or creates one (CreateSystemSurface with a format
fallback list, else IDirectDraw7::CreateSurface), sets the palette, converts `bits`
into the locked surface, builds the mip levels and records the colour key or the
render-state pairs. The linked body is in src/reconstructed/PCTextureMap.cpp.

IsFormat4444Or8888 (0x00511ad0), FillPixelFormat (0x005119c0) and
PixelFormatBytesPerPixel (0x00511970) are leaf functions without globals and run
natively. Modelled: CreateSystemSurface (0x004c68e0, notes the description and the
format list; on success stores a surface and one of the listed formats, the fallback
being the only way the colour-key branch is reached), the surface's COM methods (AddRef +0x04,
GetCaps +0x38, Lock +0x64, SetPalette +0x7c, Unlock +0x80) and IDirectDraw7::CreateSurface
(+0x18), each with case-chosen HRESULTs; GrowSecondScratchBuffer (0x00511370),
ConvertPixels (0x004d1d20), AverageColor (0x004d24d0), DumpLevel (0x004c84e0) and the
texture's own BuildMipLevels (slot 15) and ApplyColorKey (slot 18). The shared surface
tables (0x0068a36c, 0x0068a394) and g_DebugDumpTextureLevels (0x00689968) are case
data, so writes to them are compared. A mip-mapped 888 or 8888 texture gets no surface
(that branch has no IDirectDraw7::CreateSurface fallback) and both bodies call GetCaps
through null, so mip-mapped cases use the five listed formats.

Layout (src/reconstructed/PCTextureMap.h): +0x10 owner manager, +0x14/+0x18 size, +0x1c
minimum mip size, +0x20 format, +0x24 mip count, +0x2c colour mapper, +0x3c average
colour, +0x44 state pair count, +0x48 pairs, +0x70 system surface, +0x78 palette;
g_TrackGame +0x0c display (+0x190 IDirectDraw7), +0x2d0 bladeRenderer, +0x2d5 bit 2
texturesCached; TextureContext +0x0c opaque, +0x10 alpha format.
"""
import struct

RETAIL_VA = 0x004c69c0
SYMBOL = '?CreateSurface@PCTextureMap@@'
SOURCE = 'src/reconstructed/PCTextureMap.cpp'
IS_ALPHA, FILL_FORMAT, BYTES_PER_PIXEL = 0x00511ad0, 0x005119c0, 0x00511970
CREATE_SYSTEM, GROW, CONVERT, AVERAGE, DUMP = 0x004c68e0, 0x00511370, 0x004d1d20, 0x004d24d0, 0x004c84e0
TRACK_GAME, SHARED_MIP, SHARED, DUMP_LEVELS = 0x0056e26c, 0x0068a36c, 0x0068a394, 0x00689968
PROBES = {0x004c6b30: 'shared reuse', 0x004c6bdd: 'system surface', 0x004c6c72: 'DirectDraw create',
          0x004c6cdf: 'mip level', 0x004c6d34: 'grow scratch', 0x004c6d3d: 'scratch failed',
          0x004c6e08: 'shared mip reuse', 0x004c6f3f: 'store shared mip', 0x004c6cb3: 'palette',
          0x004c7028: 'convert', 0x004c7080: 'build mips', 0x004c7087: 'failed', 0x004c70a8: 'dump',
          0x004c70db: 'colour key', 0x004c7100: 'alpha states', 0x004c716e: 'opaque states'}

# COM __stdcall methods (ECX scratch): the surface vtable is built first, the
# DirectDraw one second, so their slots sit at fixed fake addresses.
FAKE = 0x22000000
SURFACE_SLOTS = 0x21
ADD_REF, GET_CAPS, LOCK, SET_PALETTE, UNLOCK = (FAKE + o for o in (0x04, 0x38, 0x64, 0x7c, 0x80))
DD_CREATE = FAKE + 4 * SURFACE_SLOTS + 0x18
FORMATS = [0x22b, 0x235, 0x115c, 0x613, 8, 0x378, 0x22b8, 0x22b, 0x235]
MIP_FORMATS = [0x22b, 0x235, 0x115c, 0x613, 8]     # the mip path has no DirectDraw fallback


def _st(ctx):
    return ctx.data['case']


def _note_desc(ctx, label, desc):
    ctx.note(label, ctx.read(desc, 0x7c).hex())


def _create_system(ctx, rec):
    # CreateSystemSurface(desc, flags, formats): the list ends at 0
    _note_desc(ctx, 'system desc', rec.args[0])
    formats, at = [], rec.args[2]
    while len(formats) < 8:
        f = struct.unpack('<i', ctx.read(at + 4 * len(formats), 4))[0]
        formats.append(f)
        if f == 0:
            break
    ctx.note('formats', formats)
    ok = _st(ctx)['system']
    if ok:
        # the format that worked (a fallback changes the map format)
        ctx.pack(rec.ecx + 0x20, 'i', rec.rng.choice(formats[:-1] or [0]))
        ctx.pack(rec.ecx + 0x70, 'I', ctx.data['new_surface'])
    return ok


def _dd_create(ctx, rec):
    # CreateSurface(this, desc, out, outer)
    _note_desc(ctx, 'dd desc', rec.args[1])
    hr = _st(ctx)['dd']
    if hr == 0:
        ctx.pack(rec.args[2], 'I', ctx.data['new_surface'])
    return hr


def _lock(ctx, rec):
    # Lock(this, rect, desc, flags, event)
    c = _st(ctx)
    ctx.note('lock size', struct.unpack('<I', ctx.read(rec.args[2], 4))[0])
    if c['lock'] == 0:
        ctx.pack(rec.args[2] + 0x08, 'IIi', c['lock_h'], c['lock_w'], c['pitch'])
        ctx.pack(rec.args[2] + 0x24, 'I', ctx.data['pixels'])
    return c['lock']


def _hresult(key):
    return lambda ctx, rec: _st(ctx)[key]


REPLAY = dict(
    returns={CREATE_SYSTEM: _create_system, GROW: _hresult('grow')},
    pops={CREATE_SYSTEM: 12, GROW: 4, CONVERT: 0, AVERAGE: 0, DUMP: 8},
    args={CREATE_SYSTEM: 3, GROW: 1, CONVERT: 12, AVERAGE: 6, DUMP: 2},
    ecx={CONVERT: False, AVERAGE: False, ADD_REF: False, GET_CAPS: False, LOCK: False, SET_PALETTE: False,
         UNLOCK: False, DD_CREATE: False},
    passthrough=(IS_ALPHA, FILL_FORMAT, BYTES_PER_PIXEL))


def cases(rng, n):
    for _ in range(n):
        size = rng.choice([1, 2, 8, 16, 32, 64, 128, 256])
        width, height = size, rng.choice([size, size, size // 2 or 1, size * 2])
        fail = lambda p: 0 if rng.random() > p else rng.choice([0x887601c2, 0x80004005])
        flags = rng.choice([0, 2, 4, 8, 6, 0xa, 2, 2, 0x10])
        formats = MIP_FORMATS if flags & 2 else FORMATS
        yield dict(
            seed=rng.getrandbits(32), width=width, height=height, stride=width * 2 + rng.choice([0, 4]),
            minimum=rng.choice([1, 2, 4, 8, 16, 32, 64]), source=rng.choice(FORMATS), format=rng.choice(formats),
            bits=rng.random() < 0.7, palette=rng.random() < 0.7,
            flags=flags, surface_palette=rng.random() < 0.5,
            check_memory=rng.choice([0, 1]), address=(rng.randrange(4), rng.randrange(4)),
            choice=rng.random() < 0.3, choice_formats=(rng.choice(formats), rng.choice(formats)),
            alpha=rng.randrange(256), key=rng.getrandbits(32),
            existing=rng.random() < 0.15, blade=rng.random() < 0.15, cached=rng.random() < 0.15,
            shared=rng.random() < 0.4, shared_mip=rng.random() < 0.4, dump=rng.random() < 0.2,
            system=rng.choice([1, 1, 1, 1, 0]), dd=fail(0.15), set_palette=fail(0.15), caps=fail(0.1),
            lock=fail(0.1), unlock=fail(0.1), grow=rng.choice([1, 1, 1, 0]), mips=rng.choice([1, 1, 1, 0]),
            lock_w=width, lock_h=height, pitch=rng.choice([width, width * 2, width * 4, width * 2 + 8]),
            pairs=rng.choice([0, 0, 0, 1]))


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])
    ctx.uc.mem_map(0, 0x1000)                # the SEH frame link at fs:[0] (page 0)
    ctx.data['case'] = c
    surface_vt = ctx.fake_vtable('IDirectDrawSurface7', SURFACE_SLOTS,
                                 pops={1: 4, 14: 8, 25: 20, 31: 8, 32: 8},
                                 returns={1: 'int', 14: _hresult('caps'), 25: _lock,
                                          31: _hresult('set_palette'), 32: _hresult('unlock')})
    dd_vt = ctx.fake_vtable('IDirectDraw7', 0x20, pops={6: 16}, returns={6: _dd_create})
    assert ctx.read(surface_vt + 0x64, 4) == struct.pack('<I', LOCK)
    assert ctx.read(dd_vt + 0x18, 4) == struct.pack('<I', DD_CREATE)
    texture_vt = ctx.fake_vtable('PCTextureMap', 20, pops={15: 4, 18: 4},
                                 returns={15: _hresult('mips'), 18: 'void'})

    def surface():
        s = ctx.alloc(0x10)
        ctx.pack(s, 'I', surface_vt)
        return s
    ctx.data['new_surface'] = surface()
    ctx.data['pixels'] = ctx.alloc(c['pitch'] * c['height'] + 0x10, fill=0x3c)
    dd = ctx.alloc(0x10)
    ctx.pack(dd, 'I', dd_vt)
    display = ctx.alloc(0x200, fill=0x44)
    ctx.pack(display + 0x190, 'I', dd)
    game = ctx.alloc(0x300, fill=0x66)
    ctx.pack(game + 0x0c, 'I', display)
    ctx.pack(game + 0x2d0, 'I', int(c['blade']))
    ctx.pack(game + 0x2d5, 'B', rng.getrandbits(8) & ~4 | (4 if c['cached'] else 0))
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.write(SHARED_MIP, struct.pack('<10I', *(surface() if c['shared_mip'] and rng.random() < 0.7 else 0
                                                 for _ in range(10))))
    ctx.write(SHARED, struct.pack('<10I', *(surface() if c['shared'] and rng.random() < 0.7 else 0
                                             for _ in range(10))))
    ctx.pack(DUMP_LEVELS, 'I', int(c['dump']))

    texture = ctx.alloc(0xa0, fill=0x5a)
    ctx.pack(texture, 'I', texture_vt)
    ctx.pack(texture + 0x10, 'I', ctx.alloc(0x40))
    ctx.pack(texture + 0x44, 'i', c['pairs'])
    ctx.pack(texture + 0x70, 'I', surface() if c['existing'] else 0)
    choice = 0
    if c['choice']:
        choice = ctx.alloc(0x20, fill=0x11)
        ctx.pack(choice + 0x0c, 'ii', *c['choice_formats'])
    bits = ctx.alloc(c['stride'] * c['height'], fill=0x9a) if c['bits'] else 0
    palette = ctx.alloc(0x40) if c['palette'] else 0
    surface_palette = ctx.alloc(0x20) if c['surface_palette'] else 0
    args = [bits, c['width'], c['height'], c['stride'], c['minimum'], c['source'], c['format'], palette,
            c['flags'], surface_palette, c['check_memory'], 0x5eed, c['address'][0], c['address'][1], choice,
            c['alpha'], c['key']]
    return dict(ecx=texture, args=args, ret_eax=True)
