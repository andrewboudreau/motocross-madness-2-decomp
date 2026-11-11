"""UnknownOverlayText::DrawText4444 0x0050b400, the 16-bit (4444) font blitter (tools/emu_diff.py
case, call-record/replay mode).

The inputs of ``overlay_draw_text`` (fonts with missing characters, glyph rectangles on and
between texel edges, wrapping text, packed and padded pitches) against the 4444 entry point,
with 16-bit texels and the real BlendFontTexel4444 0x0043b320; the colour is a 16-bit value.
"""
from tools.emu_cases import overlay_draw_text as base

RETAIL_VA = 0x0050b400
SYMBOL = '?DrawText4444@UnknownOverlayText@@QAEHPAVTextureMap@@HHPBDG@Z'
SOURCE = base.SOURCE
PROBES = {0x0050b5b3: 'wrap', 0x0050b637: 'blend'}
REPLAY = base.REPLAY


def cases(rng, n):
    for c in base.make_cases(rng, n, (4444,)):
        c['color'] = rng.choice([0xffff, 0xf000, 0x0f0f, rng.getrandbits(16), rng.getrandbits(32)])
        yield c


def setup(ctx, c):
    return base.setup(ctx, c, bpp=2)
