"""TextService::LayoutText 0x0050ba00, the textured-quad text layout (tools/emu_diff.py
case, call-record/replay mode).

The linked body is src/bringup/Group2_OverlayText.cpp. The font (+0x00) answers FindCell
0x00467340 with a CharacterCell (a real Rectangle2D at +8) or NULL for characters it lacks;
CharacterCell::GetRect and the Rectangle2D constructor, assignment and destructor run
natively. Cell rectangles are random texture coordinates (on 1/255 steps, arbitrary, empty or
inverted), the rectangle's left and top are negative, zero or positive ints (clamped at 0),
the vertex depth (+0x0c) and colour are random. Text is empty, one character or up to 24
characters (the vertex array's capacity), some characters missing. Every vertex word, the
count and the returned pointer are compared at both precisions.
"""
import random
import struct

RETAIL_VA = 0x0050ba00
SYMBOL = '?LayoutText@TextService@@QAEPAXPBUUnknownOverlayRect@@PBDIPAH@Z'
SOURCE = 'src/bringup/Group2_OverlayText.cpp'
PROBES = {0x0050ba67: 'left clamp', 0x0050baeb: 'cell'}

FIND_CELL = 0x00467340
PASS = (0x0050beb0, 0x004e8ad0, 0x004e8b60, 0x004e8bc0, 0x004e8c20)
RECT_VTABLE, POINT_VTABLE = 0x005577fc, 0x00556f78


def _cell(ctx, rec):
    return ctx.data['cells'].get(rec.args[0] & 0xff, 0)


REPLAY = dict(returns={FIND_CELL: _cell}, passthrough=PASS)


def cases(rng, n):
    for _ in range(n):
        length = rng.choice([0, 1, 2, 5, 11, 24])
        text = ''.join(rng.choice('Ab C9.!\nxyz') for _ in range(length))
        yield dict(seed=rng.getrandbits(32), text=text,
                   left=rng.choice([-50, -1, 0, 0, 1, 17, 639, 0x7fffffff]),
                   top=rng.choice([-3, 0, 0, 2, 100, 479]),
                   color=rng.getrandbits(32), steps=rng.random() < 0.5)


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)       # fs:[0] (the EH frame chain) reads page 0
    r = random.Random(c['seed'])
    cells = {}
    for ch in set(c['text']):
        if r.random() < 0.15:
            continue
        if c['steps']:
            x1, y1 = r.randrange(256) / 255.0, r.randrange(256) / 255.0
            x2, y2 = x1 + r.randrange(20) / 255.0, y1 + r.randrange(20) / 255.0
        else:
            x1, y1, x2, y2 = (r.uniform(-0.1, 1.1) for _ in range(4))
        cell = ctx.alloc(0x30, fill=0x5c)
        ctx.pack(cell + 8, 'IIffIff', RECT_VTABLE, POINT_VTABLE, x1, y1, POINT_VTABLE, x2, y2)
        cells[ord(ch)] = cell
    ctx.data['cells'] = cells
    size = 0x1400
    overlay = ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size)))
    ctx.pack(overlay, 'I', ctx.alloc(0x20, fill=0x4f))
    ctx.pack(overlay + 0xc, 'f', r.choice([0.001, 0.0, 0.5, r.random()]))
    rect = ctx.alloc(16, struct.pack('<4i', c['left'], c['top'], c['left'] // 2 + 10, c['top'] + 10))
    text = ctx.alloc(len(c['text']) + 1, c['text'].encode() + b'\0')
    count = ctx.alloc(4, fill=0x33)
    return dict(ecx=overlay, args=[rect, text, c['color'], count])
