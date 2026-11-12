"""TextService::DrawTextInto 0x0050b080, the font blitter into a locked texture
(tools/emu_diff.py case, call-record/replay mode).

The linked body is src/bringup/Group2_OverlayText.cpp. The overlay's font (+0x00) answers
GetTexture 0x004673c0 (a font texture of random size with random 32- or 16-bit texels) and
FindCell 0x00467340 (a CharacterCell per character, or NULL for characters the font lacks).
Each cell holds a real Rectangle2D at +8 whose texture coordinates land on texel edges or
between them (the glyph sizes are __ftol truncations). CharacterCell::GetRect and the
Rectangle2D constructor, assignment and destructor run natively, as do the strict-exact
pixel helpers (ConvertColorToFormat and the three BlendFontTexel blends), so every blended
texel lands in the destination and is compared. LockSurface (vtable +0x34) answers a buffer
and a pitch (packed, padded or odd), UnlockSurface +0x38 and Upload +0x24 are logged. Text
is empty, one character, or long enough to wrap (px + glyph width >= texture width) several
times; target widths go down to 1. Format 1555 and 4444 targets dispatch to the 16-bit
entry points, which are replayed here and run in ``overlay_draw_text_1555``/``_4444``.
"""
import random
import struct

RETAIL_VA = 0x0050b080
SYMBOL = '?DrawTextInto@TextService@@QAEHPAVTextureMap@@HHPBDH@Z'
SOURCE = 'src/bringup/Group2_OverlayText.cpp'
PROBES = {0x0050b0bd: '1555', 0x0050b0fb: '4444', 0x0050b2b6: 'wrap', 0x0050b338: 'blend',
          0x0050b395: 'next char'}

GET_TEXTURE, FIND_CELL = 0x004673c0, 0x00467340
PASS = (0x0050beb0, 0x004e8ad0, 0x004e8b60, 0x004e8bc0, 0x004e8c20, 0x0043b320, 0x0043b440,
        0x0043b560, 0x0043b660)
RECT_VTABLE, POINT_VTABLE = 0x005577fc, 0x00556f78
BPP = 4
FORMATS = (888, 8888, 565, 1555, 4444)


def _texture(ctx, rec):
    return ctx.data['font_texture']


def _cell(ctx, rec):
    return ctx.data['cells'].get(rec.args[0] & 0xff, 0)


REPLAY = dict(returns={GET_TEXTURE: _texture, FIND_CELL: _cell}, passthrough=PASS)


def make_cases(rng, n, formats):
    for _ in range(n):
        fw = rng.choice([1, 8, 16, 37, 64, 128, 256])
        fh = rng.choice([1, 8, 16, 21, 64, 256])
        width = rng.choice([1, 2, 7, 16, 33, 64, 100])
        kind = rng.choice(['empty', 'one', 'short', 'long', 'long'])
        alphabet = 'AbC xy9!.\n'
        length = {'empty': 0, 'one': 1, 'short': rng.randint(2, 6), 'long': rng.randint(8, 30)}[kind]
        text = ''.join(rng.choice(alphabet) for _ in range(length))
        yield dict(seed=rng.getrandbits(32), fw=fw, fh=fh, width=width, text=text,
                   format=rng.choice(formats), x=rng.choice([0, 0, 1, 3, width // 2]),
                   y=rng.choice([0, 1, 5]), color=rng.choice([0xffffffff, 0x80ff0000, rng.getrandbits(32)]),
                   pad=rng.choice([0, 0, 1, 2, 3, 8]), fpad=rng.choice([0, 0, 1, 4]),
                   grid=rng.random() < 0.5)


def cases(rng, n):
    return make_cases(rng, n, FORMATS)


def _coordinate(r, size, grid):
    if grid or size == 1:
        return r.randrange(size + 1) / size
    return r.random()


def setup(ctx, c, bpp=BPP):
    ctx.uc.mem_map(0, 0x1000)       # fs:[0] (the EH frame chain) reads page 0
    r = random.Random(c['seed'])
    fw, fh = c['fw'], c['fh']
    cells = {}
    max_h = 1
    total_h = 0
    for ch in set(c['text']):
        if r.random() < 0.15:
            continue                     # the font lacks this character
        x1 = _coordinate(r, fw, c['grid'])
        y1 = _coordinate(r, fh, c['grid'])
        x2 = min(1.0, x1 + r.choice([0, 1, 2, 3, 5]) / fw + (0 if c['grid'] else r.random() * 0.02))
        y2 = min(1.0, y1 + r.choice([0, 1, 2, 4, 7]) / fh + (0 if c['grid'] else r.random() * 0.02))
        if r.random() < 0.1:
            x1, y1, x2, y2 = 0.0, 0.0, 1.0, 1.0
        cell = ctx.alloc(0x30, fill=0x5c)
        ctx.pack(cell + 8, 'IIffIff', RECT_VTABLE, POINT_VTABLE, x1, y1, POINT_VTABLE, x2, y2)
        cells[ord(ch)] = cell
        gh = int(fh * (y2 - y1)) + 2
        max_h = max(max_h, gh)
    total_h = max_h * (len(c['text']) + 2)
    ctx.data['cells'] = cells

    # The font texture: texels the glyph rectangles read (up to the full texture).
    fpitch = fw * bpp + c['fpad'] * bpp
    font_bits = ctx.alloc(fpitch * (fh + 2), bytes(r.getrandbits(8) for _ in range(fpitch * (fh + 2))))
    tex_vt = ctx.fake_vtable('texture', 20, pops={9: 8, 13: 12, 14: 4})
    font_vt = ctx.fake_vtable('font texture', 20, pops={9: 8, 13: 12, 14: 4})
    font_tex = ctx.alloc(0x80, fill=0x6f)
    ctx.pack(font_tex, 'I', font_vt)
    ctx.pack(font_tex + 0x14, 'II', fw, fh)
    ctx.data['font_texture'] = font_tex

    # The target: rows past its height (the blitter does not clip vertically) stay mapped.
    width = c['width']
    pitch = width * bpp + c['pad'] * (bpp if r.random() < 0.7 else 1)
    pitch -= pitch % 2 if bpp == 2 else pitch % 4
    pitch = max(pitch, width * bpp)
    rows = c['y'] + total_h + 4
    span = pitch * rows + (width + 300) * bpp * 2
    bits = ctx.alloc(span, bytes(r.getrandbits(8) for _ in range(span)))
    target = ctx.alloc(0x80, fill=0x7e)
    ctx.pack(target, 'I', tex_vt)
    ctx.pack(target + 0x14, 'II', width, 8)
    ctx.pack(target + 0x20, 'I', c['format'])

    def lock_for(buffer, pitch_value):
        def lock(cx, rec):
            cx.pack(rec.args[1], 'i', pitch_value)
            return buffer
        return lock
    for vt, buf, p in ((tex_vt, bits, pitch), (font_vt, font_bits, fpitch)):
        slot = ctx.unpack(vt + 13 * 4, 'I')[0]
        name, pop, _ = ctx.fakes[slot]
        ctx.fakes[slot] = (name, pop, lock_for(buf, p))

    font = ctx.alloc(0x20, fill=0x4f)
    overlay = ctx.alloc(0x100, fill=0x0e)
    ctx.pack(overlay, 'I', font)
    text = ctx.alloc(len(c['text']) + 1, c['text'].encode() + b'\0')
    return dict(ecx=overlay, args=[target, c['x'], c['y'], text, c['color']])
