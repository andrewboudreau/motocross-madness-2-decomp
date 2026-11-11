"""ProjectedShadow::Init 0x004da7b0, the shadow's setup from the device capabilities
(tools/emu_diff.py case, call-record/replay mode).

The linked body is samples/physics/shadow/ProjectedShadowNearMisses.cpp (reached from the
quarry loader's UnknownQuarryShadow::Init forward). The render target (this+0x18, set before
the call; GameObject::SetRenderContext is logged) carries the capability words +0x1b0/+0x1b4/
+0x1bc, a display whose driver name (+0x4bc, up to 0xe0 characters, the query buffer being 0x100) feeds the
"DriverInfo\\%s\\DestColorShadow" query (g_TrackGame slot 22, answering the case's flag), the
saved width/height/surface (+0xc/+0x10/+0x48), the owner (+0x8) and the device (+0x50, slot 13
logged). IsFormatSupported (slot 13) answers per format. operator new returns null or a
fresh object with a fake vtable; the texture's Create (slot 4) and PageLock (surface slot 37)
answer per case, its pixels may be null and its size is 0x10..0x100. The camera's
SetRenderContext (slot 8) may fail. Every failure releases this (slot 2) and returns 0; the
success path restores the target and stores the clip rectangle and size shift. fs:[0] is page 0.
"""
import random

RETAIL_VA = 0x004da7b0
SYMBOL = '?Init@ProjectedShadow@@QAEPAV1@PAVShadowRenderTarget@@PAX@Z'
SOURCE = 'samples/physics/shadow/ProjectedShadowNearMisses.cpp'
PROBES = {0x004da82e: 'dest colour', 0x004da89c: 'caps 0x800', 0x004da8f1: 'format 0x613',
          0x004daa5b: 'camera ok', 0x004daacd: 'format fail', 0x004daad6: 'release'}

SPRINTF, NEW = 0x00534aaf, 0x004a3010
TEXTURE_CTOR, CAMERA_CTOR = 0x004c5f00, 0x004da520
TRACK_GAME = 0x0056e26c


def _cstr(ctx, at, limit=512):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _sprintf(ctx, rec):
    out, fmt = rec.args[0], _cstr(ctx, rec.args[1])
    text = fmt.replace(b'%s', _cstr(ctx, rec.args[2], 0x200), 1)
    ctx.write(out, text + b'\0')
    ctx.note('sprintf', fmt, text)
    return len(text)


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    kind = {0x80: 'texture', 0x220: 'camera'}.get(rec.args[0])
    obj = ctx.data.get(kind)
    return obj or 0


def _this(ctx, rec):
    return rec.ecx


# The device's slot 13 and the surface's PageLock (slot 37) are COM __stdcall calls: `this` is
# pushed and ECX is scratch. setup builds those two fake vtables first, so their slots sit at
# fixed fake addresses (Context.fake_vtable hands them out from 0x22000000 in order).
DEVICE_SLOT13 = 0x22000000 + 4 * 13
SURFACE_SLOT37 = 0x22000000 + 4 * 32 + 4 * 37
REPLAY = dict(returns={SPRINTF: _sprintf, NEW: _new, TEXTURE_CTOR: _this, CAMERA_CTOR: _this},
              pops={SPRINTF: 0, NEW: 0, DEVICE_SLOT13: 8, SURFACE_SLOT37: 8},
              args={SPRINTF: 3, NEW: 3, DEVICE_SLOT13: 2, SURFACE_SLOT37: 2},
              ecx={SPRINTF: False, NEW: False, DEVICE_SLOT13: False, SURFACE_SLOT37: False},
              unlogged=(SPRINTF, NEW))


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32),
                   query=rng.choice([0, 1, 1]),
                   capsA=rng.choice([0, 0x100, 0x110, 0x10, 0x1, 0x800, 0x911, rng.getrandbits(16)]),
                   capsB=rng.choice([0, 1, 0x20, 0x10, 0x31, rng.getrandbits(8)]),
                   capsC=rng.choice([0, 0x2000, rng.getrandbits(16)]),
                   fmt=rng.choice([(1, 1), (1, 0), (0, 1), (0, 0)]),
                   texture=rng.random() < 0.85, create=rng.random() < 0.85,
                   pixels=rng.random() < 0.85, camera=rng.random() < 0.85,
                   camera_ok=rng.random() < 0.85,
                   size=rng.choice([0x80, 0x80, 0x10, 0x100, 1]),
                   driver=rng.choice([0, 1, 12, 0xe0]))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])

    def rnd(n):
        return bytes(r.getrandbits(8) for _ in range(n))

    def ret(v):
        return lambda ctx, rec: v

    def query(ctx, rec):
        ctx.note('QueryFlag', _cstr(ctx, rec.args[0]), rec.args[1])
        return c['query']

    def supported(ctx, rec):
        return c['fmt'][0] if rec.args[0] == 0x115c else c['fmt'][1] if rec.args[0] == 0x613 else 0

    device_vt = ctx.fake_vtable('Device', 32)
    surface_vt = ctx.fake_vtable('Surface', 48)
    track = ctx.alloc(0x40, rnd(0x40))
    ctx.pack(track, 'I', ctx.fake_vtable('TrackGame', 32, pops={22: 8}, returns={22: query}))
    ctx.pack(TRACK_GAME, 'I', track)

    display = ctx.alloc(0x600, rnd(0x600))
    name = ''.join(r.choice('abcXYZ 019') for _ in range(c['driver'])).encode()
    ctx.write(display + 0x4bc, name + b'\0')
    device = ctx.alloc(0x40, rnd(0x40))
    ctx.pack(device, 'I', device_vt)
    owner = ctx.alloc(0x200, rnd(0x200))
    host = ctx.alloc(0x200, rnd(0x200))
    ctx.pack(host, 'I', ctx.fake_vtable('RenderTarget', 32, pops={13: 4}, returns={13: supported}))
    ctx.pack(host + 4, 'II', display, owner)
    ctx.pack(host + 0x50, 'I', device)
    ctx.pack(host + 0x1b0, 'II', c['capsA'], c['capsB'])
    ctx.pack(host + 0x1bc, 'I', c['capsC'])

    if c['texture']:
        surface = ctx.alloc(0x40, rnd(0x40))
        ctx.pack(surface, 'I', surface_vt)
        tex = ctx.alloc(0x80, rnd(0x80))
        ctx.pack(tex, 'I', ctx.fake_vtable('ShadowTexture', 16, pops={4: 0x44, 8: 0xc},
                                           returns={4: ret(int(c['create']))}))
        ctx.pack(tex + 0x14, 'ii', c['size'], r.choice([c['size'], 0x40]))
        ctx.pack(tex + 0x70, 'II', surface, ctx.alloc(0x10) if c['pixels'] else 0)
        ctx.data['texture'] = tex
    if c['camera']:
        cam = ctx.alloc(0x220, rnd(0x220))
        ctx.pack(cam, 'I', ctx.fake_vtable('Camera', 16, pops={8: 4}, returns={8: ret(int(c['camera_ok']))}))
        ctx.data['camera'] = cam

    this = ctx.alloc(0x134, rnd(0x134))
    ctx.pack(this, 'I', ctx.fake_vtable('ProjectedShadow', 16))
    ctx.pack(this + 0x18, 'I', host)
    ctx.pack(this + 0x88, 'i', r.choice([0, 1, -3]))
    owner_arg = ctx.alloc(0x10)
    return dict(ecx=this, args=[host, owner_arg])
