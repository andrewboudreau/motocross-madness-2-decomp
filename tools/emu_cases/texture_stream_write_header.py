"""UnknownTextureStream::WriteHeader 0x00461b90, the "FAOE" header and the name key
(tools/emu_diff.py case).

The linked body is samples/io/FileStream.cpp. The stream follows 0..2 inner streams (+0x1c) to
the innermost one, writes the four header bytes through WriteByte 0x00461a60 (a stub answering
the byte or, scripted, -1 at any of the four), then sets fileEncoded (+0x08) and headerSize
(+0x10) and seeds the running key (+0x01, copied to +0x03) from the file name plus extension:
_splitpath 0x005359d2 runs natively on paths with and without drive, directory and extension,
names with high-bit characters and up to 200 characters. The key and fields are compared as
memory.
"""
RETAIL_VA = 0x00461b90
SYMBOL = '?WriteHeader@UnknownTextureStream@@QAEHPBD@Z'
SOURCE = 'samples/io/FileStream.cpp'
PROBES = {0x00461bf8: 'header written', 0x00461c67: 'key loop'}


def _part(rng, alphabet='abcXYZ_09\xe9\xff'):
    return ''.join(rng.choice(alphabet) for _ in range(rng.choice([0, 1, 3, 8, 40])))


def cases(rng, n):
    for _ in range(n):
        path = rng.choice(['', 'c:', 'D:\\']) + rng.choice(['', 'data\\', 'a\\b\\']) + _part(rng)
        path += rng.choice(['', '.tex', '.' + _part(rng, 'abc'), '.e.x'])
        if rng.random() < 0.05:
            path = 'x' * 200 + '.dat'
        yield dict(depth=rng.randrange(3), fail=rng.choice([None, None, None, 0, 1, 2, 3]),
                   header=rng.choice([[0x46, 0x41, 0x4f, 0x45], [0xff, 0x41, 0x4f, 0x45]]),
                   path=path.encode('latin-1'))


def setup(ctx, c):
    streams = []
    for k in range(c['depth'] + 1):
        stream = ctx.alloc(0x134, fill=0x40 + k)
        ctx.pack(stream + 2, 'B', c['header'][0])
        ctx.pack(stream + 0, 'B', c['header'][1])
        ctx.pack(stream + 0x20, 'B', c['header'][2])
        ctx.pack(stream + 0x18, 'B', c['header'][3])
        ctx.pack(stream + 0x1c, 'I', 0)
        if streams:
            ctx.pack(streams[-1] + 0x1c, 'I', stream)
        streams.append(stream)
    written = []

    def write_byte(cx, e, a):
        written.append(a[0])
        return 0xffffffff if len(written) - 1 == c['fail'] else a[0] & 0xff

    ctx.stub_at(0x00461a60, 'UnknownTextureStream::WriteByte', pop=4, ret=write_byte)
    path = ctx.alloc(len(c['path']) + 8, c['path'] + bytes(8))
    return dict(ecx=streams[0], args=[path])
