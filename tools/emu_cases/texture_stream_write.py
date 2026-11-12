"""UnknownTextureStream::Write 0x004618e0, the stream write (tools/emu_diff.py case).

The linked body is samples/io/FileStream.cpp. The stream follows 0..2 inner streams (+0x1c) to
the innermost one; an encoded file (+0x08) resynchronises with ftell 0x005365d6 / fseek
0x00534eac (fclose 0x00534c3d and fileHandle cleared when the seek fails) and writes through
WriteXorEncoded 0x0045c6a0 with the running key (+0x01), a plain one through fwrite 0x005350b7.
Every callee is a stub that notes its arguments; the answers (seek result, encoded success,
fwrite count) are drawn per case.
"""
RETAIL_VA = 0x004618e0
SYMBOL = '?Write@UnknownTextureStream@@QAEHPBXHH@Z'
SOURCE = 'samples/io/FileStream.cpp'
PROBES = {0x00461903: 'seek failed', 0x00461959: 'plain write'}


def cases(rng, n):
    for _ in range(n):
        yield dict(depth=rng.randrange(3), encoded=rng.choice([0, 1, 1, 5]), seek=rng.choice([0, 0, -1]),
                   encoded_ok=rng.choice([0, 1, 1]), size=rng.choice([0, 1, 4, 0x10]),
                   count=rng.choice([0, 1, 3, 100, -1]), wrote=rng.choice([0, 1, 2, 100]),
                   position=rng.choice([0, 4, 0x1234]))


def _cdecl(name, value):
    # A logged cdecl stub: ECX at the call is scratch, so only the arguments are noted.
    def f(cx, e, a):
        cx.note(name, *a)
        return value
    return f


def setup(ctx, c):
    streams = []
    for k in range(c['depth'] + 1):
        stream = ctx.alloc(0x134, fill=0x40 + k)
        ctx.pack(stream + 8, 'i', c['encoded'] if k == c['depth'] else 0x11)
        ctx.pack(stream + 0x14, 'I', 0xf11e0000 + k)
        ctx.pack(stream + 0x1c, 'I', 0)
        if streams:
            ctx.pack(streams[-1] + 0x1c, 'I', stream)
        streams.append(stream)
    buffer = ctx.alloc(0x40, fill=0x77)
    for va, name, nargs, value in ((0x005365d6, 'ftell', 1, c['position']),
                                   (0x00534eac, 'fseek', 3, c['seek'] & 0xffffffff),
                                   (0x00534c3d, 'fclose', 1, 0),
                                   (0x0045c6a0, 'WriteXorEncoded', 5, c['encoded_ok']),
                                   (0x005350b7, 'fwrite', 4, c['wrote'])):
        ctx.stub_at(va, name, nargs=nargs, ret=_cdecl(name, value), log=False)
    return dict(ecx=streams[0], args=[buffer, c['size'], c['count'] & 0xffffffff])
