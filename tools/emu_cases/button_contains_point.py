"""UIButton::ContainsPoint 0x004734c0, the opaque-pixel hit test (tools/emu_diff.py case).

Points inside and outside the control's screen rectangle (PtInRect stubbed through its
import slot 0x005502d4, evaluated in Python), dialog scales that are and are not whole
numbers (the two __ftol divisions), 8-bit (format 8) and 16-bit (0x22b, 0x235) render
targets (g_TrackGame 0x0056e26c -> +0x10 -> format +0x28), pixels equal and unequal to the colour key, and the surface
Lock (slot 25) or Unlock (slot 32) failing. Offsets from retail: screen area +0x2c, owner
dialog +0xb8 (scales +0x9c4/+0x9c8), GetStateTexture slot 48, texture system surface
+0x70; the surface description's pitch +0x10, bits +0x24 and key +0x40.
PixelFormatBytesPerPixel (0x00511970) runs as retail code. ``multistate_contains_point``
reuses this module for UIMultiState.
"""
import struct

RETAIL_VA = 0x004734c0
SYMBOL = '?ContainsPoint@UIButton@@UAEHUtagPOINT@@H@Z'
SOURCE = 'samples/ui/ControlHitTest.cpp'
PTINRECT_IAT = 0x005502d4
BINDINGS = {'__imp__PtInRect@12': PTINRECT_IAT}


def cases(rng, n):
    for _ in range(n):
        left, top = rng.randrange(0, 600), rng.randrange(0, 400)
        w, h = rng.randrange(1, 120), rng.randrange(1, 60)
        scale = (rng.choice([1.0, 2.0, 1.5, 0.75, rng.uniform(0.5, 2.5)]),
                 rng.choice([1.0, 2.0, 1.25, rng.uniform(0.5, 2.5)]))
        inside = rng.random() < 0.8
        if inside:
            px, py = left + rng.randrange(w), top + rng.randrange(h)
        else:
            px, py = rng.choice([(left - 1, top), (left + w, top), (left, top + h), (left - 50, top - 9)])
        fmt = rng.choice([8, 0x22b, 0x235])
        key = rng.randrange(0x10000)
        hit = rng.random() < 0.5
        fail = rng.choice([None] * 6 + ['lock', 'unlock'])
        yield ((left, top, left + w, top + h), (px, py), scale, fmt, key, hit, fail, rng.randrange(4))


def setup(ctx, case):
    rect, (px, py), scale, fmt, key, hit, fail, state = case

    def pt_in_rect(c, ecx, args):
        l, t, r, b = c.unpack(args[0], '4i')
        x, y = struct.unpack('<ii', struct.pack('<II', args[1], args[2]))
        return int(l <= x < r and t <= y < b)
    ctx.pack(PTINRECT_IAT, 'I', ctx.stub('PtInRect', pop=12, ret=pt_in_rect))

    # The bitmap: at least as large as any (point / scale) the case can index.
    cols = int((rect[2] - rect[0]) / min(scale[0], 1.0)) + 4
    rows = int((rect[3] - rect[1]) / min(scale[1], 1.0)) + 4
    bpp = 1 if fmt == 8 else 2
    pitch = cols * bpp + 4
    x = int((px - rect[0]) / scale[0])
    y = int((py - rect[1]) / scale[1])
    data = bytearray(ctx.rng.randrange(256) for _ in range(pitch * rows))
    offset = (pitch * y // bpp + x) * bpp
    if 0 <= offset < len(data) - 1:
        if hit:
            data[offset:offset + bpp] = struct.pack('<H', key)[:bpp]
    bits = ctx.alloc(len(data) + 8, bytes(data))

    def lock(c, ecx, args):
        if fail == 'lock':
            return 0x80004005
        desc = args[2]
        c.pack(desc + 0x10, 'i', pitch)
        c.pack(desc + 0x24, 'I', bits)
        c.pack(desc + 0x40, 'I', key)
        return 0
    surface_vt = [ctx.stub('Surface::other', pop=4)] * 40
    surface_vt[25] = ctx.stub('Surface::Lock', pop=20, ret=lock)
    surface_vt[32] = ctx.stub('Surface::Unlock', pop=8, ret=0 if fail == 'unlock' else 1)
    surface = ctx.alloc(0x10)
    ctx.pack(surface, 'I', ctx.vtable(surface_vt))
    texture = ctx.alloc(0x100)
    ctx.pack(texture + 0x70, 'I', surface)

    render_target = ctx.alloc(0x40)
    ctx.pack(render_target + 0x28, 'i', fmt)
    game = ctx.alloc(0x40)
    ctx.pack(game + 0x10, 'I', render_target)
    ctx.pack(0x0056e26c, 'I', game)

    dialog = ctx.alloc(0xa00)
    ctx.pack(dialog + 0x9c4, 'ff', *scale)
    control_vt = [ctx.stub('Control::other', pop=0)] * 60
    control_vt[48] = ctx.stub('Control::GetStateTexture', pop=4, ret=texture)
    this = ctx.alloc(0x100)
    ctx.pack(this, 'I', ctx.vtable(control_vt))
    ctx.pack(this + 0x2c, '4i', *rect)
    ctx.pack(this + 0xb8, 'I', dialog)
    return dict(ecx=this, args=[px & 0xffffffff, py & 0xffffffff, state])
