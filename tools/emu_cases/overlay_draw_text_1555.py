"""UnknownOverlayText::DrawText1555 0x0050b700, the 16-bit (1555) font blitter (tools/emu_diff.py
case, call-record/replay mode).

The inputs of ``overlay_draw_text`` (fonts with missing characters, glyph rectangles on and
between texel edges, wrapping text, packed and padded pitches) against the 1555 entry point,
with 16-bit texels and the real BlendFontTexel1555 0x0043b440; the colour is a 16-bit value.
"""
from tools.emu_cases import overlay_draw_text as base

RETAIL_VA = 0x0050b700
SYMBOL = '?DrawText1555@UnknownOverlayText@@QAEHPAVTextureMap@@HHPBDG@Z'
SOURCE = base.SOURCE
PROBES = {0x0050b8b3: 'wrap', 0x0050b937: 'blend'}
REPLAY = base.REPLAY


def cases(rng, n):
    for c in base.make_cases(rng, n, (1555,)):
        c['color'] = rng.choice([0xffff, 0x8000, 0x7c1f, rng.getrandbits(16), rng.getrandbits(32)])
        yield c


def setup(ctx, c):
    return base.setup(ctx, c, bpp=2)
