"""PCTextureMap::Clone 0x004c71c0 (tools/emu_diff.py case, call-record/replay mode).

A copy of the texture: a ManagedTexture registered with this texture's group when the
map is cached (+0x68 bit 0), else a PCTextureMap of g_TrackGame's texture manager. The
system surface (+0x70) is locked read-only, the copy's CreateSurface (slot 4, 17
arguments) gets its pixels, pitch / bytes per pixel and flags from the surface caps
(8 without 0x30000000, 4 without 0x1000); after Unlock the colour key is applied (slot
18) as 8-bit palette RGB or expanded 555/565. Any failure deletes the copy and returns 0.
The linked body is samples/render/TextureClone.cpp.

Modelled: new, both constructors (writing fake vtables), the surface's Lock (+0x64,
COM, writes pitch, pixels and caps) and Unlock (+0x80), PixelFormatBytesPerPixel
(0x00511970, case-chosen, never 0), and the copy's CreateSurface. The group's AddTexture
(0x0050c6c0) is replayed. Both bodies dereference a null copy whenever a surface exists
or the map is cached, so allocation fails only without either.

Layout (src/reconstructed/PCTextureMap.h): +0x10 owner manager, +0x14/+0x18 size, +0x1c
minimum mip size, +0x20 format, +0x2c colour mapper (3-byte entries from +0x10), +0x30
uses colour key, +0x34 key, +0x68 flags, +0x70 system surface, +0x78 palette, +0x90 the
group; g_TrackGame +0x3c the texture manager.
"""
import struct

RETAIL_VA = 0x004c71c0
SYMBOL = '?Clone@PCTextureMap@@'
SOURCE = 'samples/render/TextureClone.cpp'
NEW, MANAGED_CTOR, TEXTURE_CTOR, ADD_TEXTURE, BYTES_PER_PIXEL = 0x004a3010, 0x00510500, 0x004c5f00, 0x0050c6c0, 0x00511970
TRACK_GAME = 0x0056e26c
PROBES = {0x004c71f1: 'cached', 0x004c724d: 'plain', 0x004c72d2: 'locked', 0x004c7372: '8-bit key',
          0x004c73af: '555 key', 0x004c73c6: '565 key', 0x004c73ec: 'no key', 0x004c73f4: 'delete copy'}

# The surface's methods are COM __stdcall (ECX scratch); setup builds its fake vtable first,
# so Lock and Unlock sit at fixed fake addresses.
FAKE = 0x22000000
LOCK, UNLOCK = FAKE + 0x64, FAKE + 0x80


def _st(ctx):
    return ctx.data['case']


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return 0 if _st(ctx)['new_fails'] else ctx.alloc(rec.args[0] + 0x10, fill=0xcd)


def _ctor(vt):
    def call(ctx, rec):
        ctx.pack(rec.ecx, 'I', ctx.data[vt])
        ctx.pack(rec.ecx + 0x68, 'I', _st(ctx)['copy_flags'])
        return rec.ecx
    return call


def _lock(ctx, rec):
    # Lock(this, rect, desc, flags, event)
    c = _st(ctx)
    ctx.note('lock size', struct.unpack('<I', ctx.read(rec.args[2], 4))[0])
    if c['lock'] == 0:
        ctx.pack(rec.args[2] + 0x10, 'i', c['pitch'])
        ctx.pack(rec.args[2] + 0x24, 'I', ctx.data['pixels'])
        ctx.pack(rec.args[2] + 0x68, 'I', c['caps'])
    return c['lock']


REPLAY = dict(
    returns={NEW: _new, MANAGED_CTOR: _ctor('managed_vt'), TEXTURE_CTOR: _ctor('texture_vt'),
             BYTES_PER_PIXEL: lambda ctx, rec: _st(ctx)['bytes']},
    pops={NEW: 0, BYTES_PER_PIXEL: 0, MANAGED_CTOR: 4, TEXTURE_CTOR: 8, ADD_TEXTURE: 4},
    args={NEW: 3, BYTES_PER_PIXEL: 1, MANAGED_CTOR: 1, TEXTURE_CTOR: 2, ADD_TEXTURE: 1},
    ecx={NEW: False, BYTES_PER_PIXEL: False, LOCK: False, UNLOCK: False},
    unlogged=(NEW,))


def cases(rng, n):
    for _ in range(n):
        cached = rng.random() < 0.4
        surface = rng.random() < 0.9
        yield dict(
            seed=rng.getrandbits(32), cached=cached, surface=surface,
            new_fails=not cached and not surface and rng.random() < 0.5,
            flags=rng.choice([0, 2, 0x10]) | int(cached), copy_flags=rng.choice([0, 2, 0x80]),
            lock=rng.choice([0] * 6 + [0x887601c2]), unlock=rng.choice([0] * 6 + [0x887600a8]),
            pitch=rng.choice([256, 512, 513, 768, 1024, 64, -512]), bytes=rng.choice([1, 2, 2, 3, 4]),
            caps=rng.choice([0x10001000, 0x20001000, 0x00001000, 0x10000000, 0, 0x30005000]),
            created=rng.choice([1, 1, 1, 0]), key=rng.choice([0, 1, 1]),
            format=rng.choice([8, 8, 0x22b, 0x22b, 0x235, 0x378]), key_value=rng.getrandbits(16),
            width=rng.choice([64, 128, 256]), height=rng.choice([32, 128, 256]), mip=rng.choice([1, 4, 8]))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)                # fs:[0] reads page 0 (the EH frame link)
    ctx.data['case'] = c
    surface_vt = ctx.fake_vtable('IDirectDrawSurface7', 0x21, pops={25: 20, 32: 8},
                                 returns={25: _lock, 32: lambda ctx_, rec: c['unlock']})
    assert ctx.read(surface_vt + 0x64, 4) == struct.pack('<I', LOCK)
    copy_returns = {4: lambda ctx_, rec: c['created'], 18: 'int', 0: 'ptr'}
    ctx.data['texture_vt'] = ctx.fake_vtable('PCTextureMap', 20, pops={0: 4, 4: 68, 18: 4}, returns=copy_returns)
    ctx.data['managed_vt'] = ctx.fake_vtable('ManagedTexture', 20, pops={0: 4, 4: 68, 18: 4}, returns=copy_returns)
    ctx.data['pixels'] = ctx.alloc(0x40)

    game = ctx.alloc(0x80, fill=0x66)
    ctx.pack(game + 0x3c, 'I', ctx.alloc(0x40))
    ctx.pack(TRACK_GAME, 'I', game)
    mapper = ctx.alloc(0x10 + 3 * 0x10000 + 4, bytes(range(256)) * (0x30014 // 256 + 1))
    texture = ctx.alloc(0xa0, fill=0x5a)
    ctx.pack(texture + 0x10, 'I', ctx.alloc(0x40))
    ctx.pack(texture + 0x14, 'iii', c['width'], c['height'], c['mip'])
    ctx.pack(texture + 0x20, 'i', c['format'])
    ctx.pack(texture + 0x2c, 'Iii', mapper, c['key'], c['key_value'])
    ctx.pack(texture + 0x68, 'I', c['flags'])
    surface = 0
    if c['surface']:
        surface = ctx.alloc(0x10)
        ctx.pack(surface, 'I', surface_vt)
    ctx.pack(texture + 0x70, 'I', surface)
    ctx.pack(texture + 0x78, 'I', ctx.alloc(0x20))
    ctx.pack(texture + 0x90, 'I', ctx.alloc(0x40))
    return dict(ecx=texture, args=[], ret_eax=True)
