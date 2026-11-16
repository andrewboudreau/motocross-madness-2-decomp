"""PCTextureMap::CopyRectTo 0x004c7b40 (tools/emu_diff.py case).

Copies `rect` of the texture's system surface (+0x70) to (x, y) in `destination`. Without
a table it is IDirectDrawSurface7::BltFast; with one, both surfaces are locked and each
destination pixel becomes table[source << 8 | destination], clipped (unsigned compares)
to the destination. The linked body is samples/render/TextureRectCopy.cpp.

The surfaces are fake COM objects (__stdcall, ECX scratch, so the stack arguments alone
are logged): BltFast (+0x1c), Lock (+0x64, writes height, width, pitch, the pixels and
the bit count into the description) and Unlock (+0x80), each with case-chosen HRESULTs.
The pixel buffers are real and compared after the run.

Retail's 16-bit path reads a byte through the destination pixel's value as an address
(0x004c7d9f `mov bl, [esi]` after `mov si, [eax]`); the source keeps that. Page 0 up to
0x10000 is mapped with case bytes so both bodies read the same values there, and 16-bit
source pixels stay below 0x400 so table[source << 8 | byte] stays inside the table.
"""
import struct

from tools.emu_diff import _frame_token

RETAIL_VA = 0x004c7b40
SYMBOL = '?CopyRectTo@PCTextureMap@@'
SOURCE = 'samples/render/TextureRectCopy.cpp'
PROBES = {0x004c7b92: 'success', 0x004c7c62: 'clip right', 0x004c7c7f: 'clip bottom',
          0x004c7cef: '8-bit pixel', 0x004c7d93: '16-bit pixel', 0x004c7e21: 'failure',
          0x004c7def: 'unlock destination'}


def _surface(rng, bpp, big):
    w, h = rng.randint(1, 40 if big else 24), rng.randint(1, 40 if big else 24)
    pitch = w * bpp // 8 + rng.choice([0, 0, 2, 4, 1 if bpp == 16 else 3])
    return dict(w=w, h=h, pitch=pitch, bpp=bpp, lock=rng.choice([0] * 9 + [0x887601c2]),
                unlock=rng.choice([0] * 9 + [0x887600a8]))


def cases(rng, n):
    for _ in range(n):
        bpp = rng.choice([8, 8, 16, 16, 24, 32])
        src = _surface(rng, 8 if bpp == 8 else 16, False)
        dst = _surface(rng, bpp, rng.random() < 0.8)
        left, top = rng.randrange(src['w']), rng.randrange(src['h'])
        right = rng.randint(max(left - 2, 0), src['w'] - 1)
        bottom = rng.randint(max(top - 2, 0), src['h'] - 1)
        x = rng.choice([0, 0, rng.randrange(dst['w']), rng.randrange(dst['w']), dst['w'] - 1, dst['w'], 0xfffffff0])
        y = rng.choice([0, 0, rng.randrange(dst['h']), rng.randrange(dst['h']), dst['h'] - 1, dst['h'], 0xffffffff])
        yield dict(seed=rng.getrandbits(32), src=src, dst=dst, rect=(left, top, right, bottom), x=x, y=y,
                   table=rng.random() < 0.85, flags=rng.choice([0, 0x10, 0x11]),
                   blt=rng.choice([0, 0, 0, 0x8876021c]))


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])
    ctx.uc.mem_map(0, 0x10000)
    ctx.write(0, bytes(rng.getrandbits(8) for _ in range(0x10000)))

    def com(name, pop, ret):
        def call(ctx_, ecx, a):
            ctx_.calls.append((name, None, tuple(_frame_token(v) for v in a)))
            return ret(ctx_, a) if callable(ret) else ret
        return ctx.stub(name, pop=pop, ret=call, log=False)

    def surface(label, s, pixels):
        buf = ctx.alloc(len(pixels), pixels)

        def lock(ctx_, a):
            # Lock(this, rect, desc, flags, event)
            ctx_.note(label + ' lock size', struct.unpack('<I', ctx_.read(a[2], 4))[0])
            if s['lock'] == 0:
                ctx_.pack(a[2] + 0x08, 'IIi', s['h'], s['w'], s['pitch'])
                ctx_.pack(a[2] + 0x24, 'I', buf)
                ctx_.pack(a[2] + 0x54, 'I', s['bpp'])
            return s['lock']
        slots = [com('%s slot %d' % (label, k), 4, 0) for k in range(33)]
        slots[7] = com(label + ' BltFast', 24, lambda ctx_, a: c['blt'])
        slots[25] = com(label + ' Lock', 20, lock)
        slots[32] = com(label + ' Unlock', 8, lambda ctx_, a: s['unlock'])
        obj = ctx.alloc(0x10)
        ctx.pack(obj, 'I', ctx.vtable(slots))
        return obj

    src, dst = c['src'], c['dst']
    if src['bpp'] == 8:
        src_pixels = bytes(rng.getrandbits(8) for _ in range(src['pitch'] * src['h']))
    else:
        src_pixels = b''.join(struct.pack('<H', rng.randrange(0x400)) for _ in range(src['pitch'] * src['h'] // 2 + 1))
    dst_pixels = bytes(rng.getrandbits(8) for _ in range(dst['pitch'] * dst['h'] + 4))
    system = surface('system', src, src_pixels)
    destination = surface('destination', dst, dst_pixels)
    texture = ctx.alloc(0x80, fill=0x5a)
    ctx.pack(texture + 0x70, 'I', system)
    rect = ctx.alloc(16)
    ctx.pack(rect, '4i', *c['rect'])
    table = 0
    if c['table']:
        size = 0x10000 if dst['bpp'] == 8 else 0x40100
        table = ctx.alloc(size, bytes(rng.getrandbits(8) for _ in range(size)))
    return dict(ecx=texture, args=[c['x'], c['y'], destination, rect, c['flags'], table], ret_eax=True)
