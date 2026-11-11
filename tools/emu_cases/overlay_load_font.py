"""DebugOverlay::LoadFont 0x00447a00, the GDI font-atlas builder (tools/emu_diff.py case,
call-record/replay mode).

The linked body is samples/render/DebugOverlayNearMisses.cpp. TerrainLoadTexture 0x0050a590
answers a loaded texture (straight to CreateTextureSurface, vtable +0x20) or null, which builds
the atlas: operator new 0x004a3010 (unlogged, its __FILE__ tag is the unit's own; noted by
size and line), the PCTextureMap ctor 0x004c5f00 (fills the object: fake vtable, map format
+0x20, system surface +0x70), CreateSurface +0x10, Lock +0x34 (a buffer the memset clears,
PixelFormatBytesPerPixel 0x00511970 answering 1 or 2), Unlock +0x38, then the GDI section
through the gdi32 IAT: CreateFontIndirectA (LOGFONT noted up to the face name; null takes
Release, vtable slot 2), GetDC/ReleaseDC on the surface (+0x44/+0x68, failing or not),
SetBkColor/SetBkMode/SelectObject/SetTextCharacterExtra/SetTextColor, GetTextExtentPoint32A
modelled from per-character widths (0..40, some runs with wide glyphs or a constant width)
and a line height (0, 1, 8, 13, 255, 256, 300, negative), TextOutA (noted with its text) and
DeleteObject. The overlay's font height is random (negative and huge among them). The glyph
table (+0x26d0, 16 bytes per character) and the line height (+0x26b4) are compared.
"""
import random
import struct

RETAIL_VA = 0x00447a00
SYMBOL = '?LoadFont@DebugOverlay@@QAEHXZ'
SOURCE = 'samples/render/DebugOverlayNearMisses.cpp'
PROBES = {0x00447a64: 'build', 0x00447bac: 'no font', 0x00447c75: 'row', 0x00447d17: 'textout',
          0x00447d9e: 'create surface'}

LOAD_TEXTURE, NEW, CTOR, BPP = 0x0050a590, 0x004a3010, 0x004c5f00, 0x00511970
GDI = {  # IAT slot: (raw contents, name, stack words)
    0x00550054: (0x001654ba, 'DeleteObject', 1),
    0x0055006c: (0x00165540, 'CreateFontIndirectA', 1),
    0x00550070: (0x00165532, 'SetBkColor', 2),
    0x00550074: (0x00165526, 'SetBkMode', 2),
    0x00550078: (0x00165516, 'SelectObject', 2),
    0x0055007c: (0x001654fe, 'SetTextCharacterExtra', 2),
    0x00550080: (0x001654ee, 'SetTextColor', 2),
    0x00550084: (0x001654d6, 'GetTextExtentPoint32A', 4),
    0x00550088: (0x001654ca, 'TextOutA', 5),
}


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return ctx.alloc(max(rec.args[0], 1), fill=0xcd)


def _texture(ctx, at):
    d = ctx.data
    ctx.pack(at, 'I', d['tex_vtable'])
    ctx.pack(at + 0x20, 'I', d['format'])
    ctx.pack(at + 0x70, 'I', d['surface'])
    return at


def _ctor(ctx, rec):
    return _texture(ctx, rec.ecx)


def _load(ctx, rec):
    if not ctx.data['loaded']:
        return 0
    return _texture(ctx, ctx.alloc(0x100, fill=0x71))


def _bpp(ctx, rec):
    return ctx.data['bpp']


def _gdi(name):
    def run(ctx, rec):
        d = ctx.data
        a = rec.args
        if name == 'CreateFontIndirectA':
            ctx.note(name, ctx.read(a[0], 28 + 8))
            return d['font']
        if name == 'GetTextExtentPoint32A':
            text = ctx.read(a[1], a[2]) if 0 < a[2] <= 0x400 else b''
            ctx.note(name, a[0], text, a[2])
            cx = sum(d['widths'][ch] for ch in text) & 0xffffffff
            ctx.write(a[3], struct.pack('<Ii', cx, d['height']))
            return 1
        if name == 'TextOutA':
            n = a[4] if a[4] < 0x400 else 0
            ctx.note(name, a[:5], ctx.read(a[3], n))
            return 1
        ctx.note(name, *a)
        if name == 'SelectObject':
            return 0x5e1ec7
        return 1
    return run


returns = {LOAD_TEXTURE: _load, NEW: _new, CTOR: _ctor, BPP: _bpp}
pops = {NEW: 0, LOAD_TEXTURE: 0, BPP: 0}
args = {NEW: 3}
ecx = {NEW: False}
unlogged = [NEW]
for slot, (raw, name, n) in GDI.items():
    for k in (('iat', slot), raw):
        returns[k] = _gdi(name)
        pops[k] = 4 * n
        args[k] = n
        ecx[k] = False
        unlogged.append(k)
REPLAY = dict(returns=returns, pops=pops, args=args, ecx=ecx, unlogged=tuple(unlogged))


def cases(rng, n):
    for _ in range(n):
        kind = rng.choice(['random', 'random', 'constant', 'wide', 'zero'])
        if kind == 'constant':
            w = rng.choice([1, 7, 8, 16, 85, 128, 255])
            widths = [w] * 256
        elif kind == 'wide':
            widths = [rng.choice([0, 3, 100, 200, 255]) for _ in range(256)]
        elif kind == 'zero':
            widths = [0] * 256
        else:
            widths = [rng.randrange(0, 41) for _ in range(256)]
        yield dict(seed=rng.getrandbits(32), widths=widths,
                   height=rng.choice([0, 1, 8, 13, 13, 16, 255, 256, 300, -5]),
                   loaded=rng.random() < 0.2, font=rng.choice([0, 0x0f0e7, 0x0f0e7, 0x0f0e7]),
                   getdc=rng.choice([0, 0, 0, 0x80004005]), releasedc=rng.choice([0, 0, 0, 1]),
                   create=rng.choice([0, 1, 1]), bpp=rng.choice([1, 2]),
                   font_height=rng.choice([0, 13, -13, 1, 0x7fffffff, -0x80000000]))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])
    d = ctx.data
    d.update(widths=c['widths'], height=c['height'], loaded=c['loaded'], font=c['font'],
             bpp=c['bpp'], format=r.choice([3, 5, 8]))

    def getdc(cx, rec):
        cx.pack(rec.args[1], 'I', 0xdc0001)
        return c['getdc']

    surface = ctx.alloc(0x20)
    ctx.pack(surface, 'I', ctx.fake_vtable('surface', 40, pops={17: 8, 26: 8},
                                           returns={17: getdc, 26: lambda cx, rec: c['releasedc']}))
    d['surface'] = surface
    buffer = ctx.alloc(c['bpp'] << 16, fill=0xbb)
    d['tex_vtable'] = ctx.fake_vtable('texture', 20, pops={4: 68, 8: 12, 13: 12, 14: 4},
                                      returns={4: 'int', 8: lambda cx, rec: c['create'],
                                               13: lambda cx, rec: buffer, 14: 'int'})
    game = ctx.alloc(0x100, fill=0x67)
    ctx.pack(0x0056e26c, 'I', game)
    render = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    overlay = ctx.alloc(0x3700, bytes(r.getrandbits(8) for _ in range(0x3700)))
    ctx.pack(overlay, 'I', ctx.fake_vtable('overlay', 8))
    ctx.pack(overlay + 0x18, 'I', render)
    ctx.pack(overlay + 0x36d0, 'i', c['font_height'])
    return dict(ecx=overlay, args=[])
