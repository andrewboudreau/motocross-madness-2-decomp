"""CacheTexture::CopyMipLevels 0x005102d0 (tools/emu_diff.py case, call-record/replay mode).

Blits a region occupant's mip levels into the cache texture: the region's float
rectangle (shrunk by 1 << level) becomes a pixel rect, the occupant's surface chain is
walked down (GetAttachedSurface) to the level no wider than the rect, then each level
is Blt into the cache surface chain with the rect halved per level, counting texels in
the owner group. Any Blt or first-walk failure returns 0; otherwise the whole rect is
uploaded (slot 9) when the display's partial upload result is positive, else +0x188 is
set. The linked body is samples/render/CacheTextureNearMisses.cpp.

Modelled: the occupant's UpdatePageCoords (0x00510610), the surfaces' COM
GetAttachedSurface (+0x30: the next level, DDERR_NOTFOUND past a chain's 1-pixel
level, or a case-chosen failure; the caps noted) and
Blt (+0x14: the rect noted), and the cache texture's Upload (slot 9). _ftol runs
natively.

Layout (src/reconstructed/CacheTexture.h, PCVideoCard.h, TextureMapManager.h): region
+0x14 occupant, +0x1c..+0x28 left/top/right/bottom floats; texture +0x14/+0x18 size,
+0x70 system surface, +0x80 owner group (+0x1d0 texels blitted), +0x188; g_TrackGame
+0x0c display (+0x5bc partial texture upload result).
"""
import struct

RETAIL_VA = 0x005102d0
SYMBOL = '?CopyMipLevels@CacheTexture@@'
SOURCE = 'samples/render/CacheTextureNearMisses.cpp'
UPDATE_PAGE_COORDS = 0x00510610
TRACK_GAME = 0x0056e26c
PROBES = {0x005103d3: 'walked down', 0x0051040a: 'blitted', 0x005104a3: 'next level',
          0x005104a8: 'return 0', 0x005104d3: 'upload', 0x005104e8: 'mark'}

# COM __stdcall (ECX scratch): the surface vtable is built first, so its slots sit at
# fixed fake addresses.
FAKE = 0x22000000
BLT, GET_ATTACHED = FAKE + 0x14, FAKE + 0x30


def _st(ctx):
    return ctx.data['case']


def _get_attached(ctx, rec):
    # GetAttachedSurface(this, caps, out)
    ctx.note('caps', ctx.read(rec.args[1], 16).hex())
    levels = struct.unpack('<i', ctx.read(rec.args[0] + 4, 4))[0]
    if levels <= 1:
        return 0x887600ff                     # DDERR_NOTFOUND: the chain's smallest level
    if rec.rng.random() < _st(ctx)['attach_fail']:
        return rec.rng.choice([0x887600ff, 0x80004005])
    ctx.pack(rec.args[2], 'I', ctx.data['surface'](levels - 1))
    return 0


def _blt(ctx, rec):
    # Blt(this, destination rect, source, source rect, flags, fx)
    ctx.note('blt rect', struct.unpack('<4i', ctx.read(rec.args[1], 16)))
    return 0 if rec.rng.random() >= _st(ctx)['blt_fail'] else 0x8876021c


REPLAY = dict(
    pops={UPDATE_PAGE_COORDS: 4},
    args={UPDATE_PAGE_COORDS: 1},
    ecx={BLT: False, GET_ATTACHED: False})


def cases(rng, n):
    for _ in range(n):
        left, top = rng.random() * 0.75, rng.random() * 0.75
        yield dict(
            level=rng.choice([0, 0, 1, 2, 3, 5]), width=rng.choice([64, 128, 256, 512]),
            height=rng.choice([64, 128, 256, 512]), occupant=rng.choice([8, 16, 32, 64, 128, 256, 512]),
            region=(left, top, left + rng.random() * (1 - left), top + rng.random() * (1 - top)),
            attach_fail=rng.choice([0, 0, 0.05, 0.3]), blt_fail=rng.choice([0, 0, 0.05, 0.3]),
            partial=rng.choice([1, 0, -1, 2]), texels=rng.getrandbits(20), flag=rng.getrandbits(32))


def setup(ctx, c):
    ctx.data['case'] = c
    surface_vt = ctx.fake_vtable('IDirectDrawSurface7', 0x21, pops={5: 24, 12: 12},
                                 returns={5: _blt, 12: _get_attached})
    assert ctx.read(surface_vt + 0x30, 4) == struct.pack('<I', GET_ATTACHED)
    texture_vt = ctx.fake_vtable('CacheTexture', 16, pops={9: 8}, returns={9: 'void'})

    def surface(levels):
        s = ctx.alloc(0x10)
        ctx.pack(s, 'Ii', surface_vt, levels)
        return s
    ctx.data['surface'] = surface
    display = ctx.alloc(0x600, fill=0x44)
    ctx.pack(display + 0x5bc, 'i', c['partial'])
    game = ctx.alloc(0x40, fill=0x66)
    ctx.pack(game + 0x0c, 'I', display)
    ctx.pack(TRACK_GAME, 'I', game)

    occupant = ctx.alloc(0x80, fill=0x3a)
    ctx.pack(occupant + 0x14, 'i', c['occupant'])
    ctx.pack(occupant + 0x70, 'I', surface(c['occupant'].bit_length()))
    region = ctx.alloc(0x30, fill=0x2b)
    ctx.pack(region + 0x14, 'I', occupant)
    ctx.pack(region + 0x1c, '4f', *c['region'])
    group = ctx.alloc(0x200, fill=0x1c)
    ctx.pack(group + 0x1d0, 'i', c['texels'])
    texture = ctx.alloc(0x190, fill=0x5a)
    ctx.pack(texture, 'I', texture_vt)
    ctx.pack(texture + 0x14, 'ii', c['width'], c['height'])
    ctx.pack(texture + 0x70, 'I', surface(c['width'].bit_length()))
    ctx.pack(texture + 0x80, 'I', group)
    ctx.pack(texture + 0x188, 'I', c['flag'])
    return dict(ecx=texture, args=[region, c['level']], ret_eax=True)
