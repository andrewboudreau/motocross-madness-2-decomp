"""FileStream::Read 0x00461640 (tools/emu_diff.py case).

Reads `count` items of `size` bytes: from the innermost stream's read buffer first
(decoding with the running key at +0x01 when the file is encoded), then from the file,
plain through fread or encoded through ReadXorDecoded (0x0045c7b0, retail code here),
in 0x400-byte blocks when the buffer is empty. The return value is the item count.

Layout (src/reconstructed/FileStream.h, offsets from the retail body): +0x01 key,
+0x08 encoded, +0x14 FILE*, +0x1c inner stream, +0x21 read buffer, +0x124/+0x128 buffer
position and end. Zero to two outer streams wrap the innermost one.

`fread` (0x00534f65) is a stub that serves a deterministic byte stream of limited
length, so short reads hit every failure return; it reads whole items only and sets
_IOEOF in the FILE's _flag (+0x0c) on a short read, which ReadXorDecoded tests. Its calls
are logged (the destination is a heap address, equal in both runs). The result is the
destination buffer, the stream fields, the call log and EAX.
"""
import struct

RETAIL_VA = 0x00461640
SYMBOL = '?Read@FileStream@@QAEHPAXHH@Z'
SOURCE = 'samples/io/FileStream.cpp'
FREAD = 0x00534f65
PROBES = {0x00461640: 'entry'}

SIZES = (1, 1, 2, 3, 4, 4, 7, 12, 0x40, 0x400)


def cases(rng, n):
    for k in range(n):
        size = rng.choice(SIZES)
        count = rng.choice([0, 1, 2, rng.randint(1, 40), rng.randint(1, 3000 // size + 1)])
        total = size * count
        yield dict(
            seed=rng.getrandbits(32), size=size, count=count, wrappers=rng.choice([0, 0, 1, 2]),
            encoded=rng.choice([0, 1]), key=rng.getrandbits(8),
            buffered=rng.choice([0, 0, rng.randint(1, 0x103), min(total, 0x103), rng.randint(0, 8)]),
            file_bytes=rng.choice([10 ** 6, total, max(0, total - rng.randint(1, 64)), rng.randint(0, total + 1),
                                   rng.randint(0, 0x500)]))


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])
    state = {'left': c['file_bytes'], 'pos': 0}

    def fread(cx, ecx, a):
        dst, size, n, _file = a
        n_items = min(n, state['left'] // size) if size else 0
        nbytes = n_items * size
        data = bytes(((state['pos'] + i) * 131 + 17) & 0xff for i in range(nbytes))
        if nbytes:
            cx.write(dst, data)
        state['pos'] += nbytes
        state['left'] -= nbytes
        if n_items < n:
            cx.pack(state['file'] + 0xc, 'I', 0x10)    # _IOEOF, as the CRT sets on a short read
        return n_items

    ctx.stub_at(FREAD, 'fread', nargs=4, ret=fread)

    inner = ctx.alloc(0x140, bytes(rng.getrandbits(8) for _ in range(0x140)))
    ctx.pack(inner + 0x01, 'B', c['key'])
    ctx.pack(inner + 0x08, 'i', c['encoded'])
    state['file'] = ctx.alloc(0x20)                # CRT FILE: ReadXorDecoded tests _flag (+0xc)
    ctx.pack(inner + 0x14, 'I', state['file'])
    ctx.pack(inner + 0x1c, 'I', 0)
    start = 0x21 + rng.randint(0, 0x103 - c['buffered'])
    ctx.pack(inner + 0x124, '2I', inner + start, inner + start + c['buffered'])
    stream = inner
    for _ in range(c['wrappers']):
        outer = ctx.alloc(0x140, bytes(rng.getrandbits(8) for _ in range(0x140)))
        ctx.pack(outer + 0x1c, 'I', stream)
        stream = outer
    dest = ctx.alloc(c['size'] * c['count'] + 0x20, fill=0xcc)
    return dict(ecx=stream, args=[dest, c['size'], c['count']], ret_float=False, ret_eax=True)
