"""PCVideoCard::ProbePartialTextureUploads 0x004cab00 (tools/emu_diff.py case, call-record/replay mode).

The "PartialTexBlt" test (+0x5bc): loads partblt.tga, uploads it whole and then as sixteen
64 x 64 tiles through TestPartialTextureUpload (0x004caf70), and stores and returns 1 (works),
0 (a tile was wrong), -1 (the full upload failed), -2 (no image or texture), -3 (no
640 x 480 x 16 target) or -4 (no texture surface). With no target it switches to the
640 x 480 x 16 mode, builds a temporary PCRenderTarget, publishes it in g_TrackGame
(+0x10 the target, +0x0c the display) and deletes it again at the end. The linked body is
samples/render/PartialTextureUploads.cpp.

Modelled from the case: LoadFile (0x005125c0, a TgaFile or 0), SetFullscreenDisplayMode
(0x004ca900), new, the PCRenderTarget constructor and InitializeRenderTarget (0x004c4f80,
the target or 0), the PCTextureMap constructor (0x004c5f00), whose fake vtable answers
CreateSurface (slot 4, 17 arguments; the image's result is case-chosen), CreateTextureSurface
(slot 8) and Release (slot 2), and TestPartialTextureUpload, whose results and rectangles
are noted. FreeFile (0x00512dd0) is replayed. The textures' allocations never fail: both
bodies dereference the image unchecked.

Layout (src/reconstructed/PCVideoCard.h, Tgafile.h): card +0x08 the mode count, +0x10 the
36-byte mode table, +0x78 the frame buffer count, +0x1a0 the back buffer, +0x5bc the result;
a TgaFile's +0x0e width, +0x10 height (shorts), +0x12 bits per pixel, +0x14 the pixels; a
render target's +0x04 display (+0x198 its palette) and +0x28 pixel format.
"""
import struct

RETAIL_VA = 0x004cab00
SYMBOL = '?ProbePartialTextureUploads@PCVideoCard@@QAEHPAVRenderTarget@@@Z'
SOURCE = 'samples/render/PartialTextureUploads.cpp'
LOAD_FILE, FREE_FILE, SET_MODE, NEW = 0x005125c0, 0x00512dd0, 0x004ca900, 0x004a3010
TARGET_CTOR, INIT_TARGET, TEXTURE_CTOR, TEST_UPLOAD = 0x004c4ee0, 0x004c4f80, 0x004c5f00, 0x004caf70
TRACK_GAME = 0x0056e26c
PROBES = {0x004cabb9: 'mode set', 0x004cac3b: 'temporary target', 0x004cad19: 'image created',
          0x004caeb1: 'texture surface', 0x004caee4: 'full upload ok', 0x004caf3b: 'tile failed',
          0x004cae7b: 'delete temporary', 0x004caf5c: 'no file'}

MODES = [(640, 480, 16), (640, 480, 32), (800, 600, 16), (640, 400, 16), (1024, 768, 16)]


def _st(ctx):
    return ctx.data['case']


def _render(ctx, at, fmt):
    display = ctx.alloc(0x200, fill=0x44)
    ctx.pack(display + 0x198, 'I', ctx.alloc(0x10))
    ctx.pack(at + 0x04, 'I', display)
    ctx.pack(at + 0x28, 'i', fmt)


def _load_file(ctx, rec):
    c = _st(ctx)
    ctx.note('load', ctx.read(rec.args[0], 12), rec.args[1], rec.args[2])
    if not c['file']:
        return 0
    w, h, bpp = c['file']
    file = ctx.alloc(0x20, fill=0x33)
    ctx.pack(file + 0x0e, 'hhB', w, h, bpp)
    ctx.pack(file + 0x14, 'I', ctx.alloc(0x10))
    return file


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    if rec.args[0] == 0xbc4 and _st(ctx)['new_fails']:
        return 0
    return ctx.alloc(rec.args[0] + 0x10, fill=0xcd)


def _target_ctor(ctx, rec):
    ctx.pack(rec.ecx, 'I', ctx.data['target_vt'])
    _render(ctx, rec.ecx, _st(ctx)['temp_format'])
    return rec.ecx


def _texture_ctor(ctx, rec):
    ctx.pack(rec.ecx, 'I', ctx.data['texture_vt'])
    return rec.ecx


def _create_surface(ctx, rec):
    ctx.data['surfaces'] += 1
    return _st(ctx)['image_ok'] if ctx.data['surfaces'] == 1 else rec.rng.choice([0, 1])


def _test_upload(ctx, rec):
    # TestPartialTextureUpload(target, rect, image, expected, rendered, card, 1)
    ctx.note('rect', ctx.read(rec.args[1], 16).hex())
    k = ctx.data['tests']
    ctx.data['tests'] += 1
    results = _st(ctx)['tests']
    return results[k] if k < len(results) else 1


REPLAY = dict(
    returns={LOAD_FILE: _load_file, SET_MODE: lambda ctx, rec: _st(ctx)['set_mode'], NEW: _new,
             TARGET_CTOR: _target_ctor, TEXTURE_CTOR: _texture_ctor, TEST_UPLOAD: _test_upload,
             INIT_TARGET: lambda ctx, rec: rec.ecx if _st(ctx)['init_ok'] else 0},
    pops={LOAD_FILE: 0, NEW: 0, TEST_UPLOAD: 0, FREE_FILE: 0}, args={LOAD_FILE: 3, NEW: 3, TEST_UPLOAD: 7, FREE_FILE: 1},
    ecx={LOAD_FILE: False, NEW: False, TEST_UPLOAD: False, FREE_FILE: False},
    # The file name and new's __FILE__ are literals at different addresses in the two images.
    unlogged=(LOAD_FILE, NEW))


def cases(rng, n):
    for _ in range(n):
        with_target = rng.random() < 0.4
        modes = [rng.choice(MODES) for _ in range(rng.randint(1, 5))]
        first = rng.choice([1, 1, 1, 1, 0, -1, 2])
        tiles = [1 if rng.random() < 0.9 else 0 for _ in range(16)] if rng.random() < 0.6 else []
        yield dict(
            seed=rng.getrandbits(32), with_target=with_target, modes=modes,
            set_mode=rng.choice([1, 1, 1, 0]), new_fails=rng.random() < 0.1, init_ok=rng.random() < 0.85,
            temp_format=rng.randrange(12), target_format=rng.randrange(12), frames=rng.choice([1, 2, 3]),
            file=None if rng.random() < 0.1 else (rng.choice([256, 128, 64]), rng.choice([256, 128]),
                                                  rng.choice([16, 16, 24, 32, 8])),
            image_ok=rng.choice([1, 1, 1, 1, 0]), texture_ok=rng.choice([1, 1, 1, 1, 0]),
            tests=[first] + tiles)


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)                # fs:[0] reads page 0 (the EH frame link)
    ctx.data.update(case=c, surfaces=0, tests=0)
    ctx.data['target_vt'] = ctx.fake_vtable('PCRenderTarget', 4, pops={0: 4}, returns={0: 'ptr'})
    ctx.data['texture_vt'] = ctx.fake_vtable(
        'PCTextureMap', 12, pops={4: 68, 8: 12},
        returns={2: 'int', 4: _create_surface, 8: lambda ctx_, rec: c['texture_ok']})

    table = ctx.alloc(36 * len(c['modes']), b''.join(struct.pack('<4i20x', w, h, bpp, 60) for w, h, bpp in c['modes']))
    card = ctx.alloc(0x600, fill=0x5a)
    ctx.pack(card + 0x08, 'i', len(c['modes']))
    ctx.pack(card + 0x10, 'I', table)
    ctx.pack(card + 0x78, 'i', c['frames'])
    ctx.pack(card + 0x1a0, 'I', ctx.alloc(0x20))
    game = ctx.alloc(0x40, fill=0x66)
    ctx.pack(TRACK_GAME, 'I', game)
    target = 0
    if c['with_target']:
        target = ctx.alloc(0x40, fill=0x77)
        _render(ctx, target, c['target_format'])
    return dict(ecx=card, args=[target], ret_eax=True)
