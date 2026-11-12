"""FileStream::CheckHeader 0x00460db0, the "FAOE" header test (tools/emu_diff.py case).

The linked body is samples/io/FileStream.cpp. Field offsets are in src/reconstructed/FileStream.h
(header bytes +0x02/+0x00/+0x20/+0x18, fileEncoded +0x08, fileHandle +0x14, bufferPos/bufferEnd
+0x124/+0x128). fseek 0x00534eac answers 0 or nonzero per call (scripted), fclose 0x00534c3d is
logged and ReadByte 0x00461980 answers the scripted bytes: the header, the header with one byte
changed (also by sign: 0xc6 against a (char)0xc6 header byte), or -1. The fields the body
writes (fileEncoded restored or not, fileHandle cleared, bufferPos) are compared as memory.
"""
RETAIL_VA = 0x00460db0
SYMBOL = '?CheckHeader@FileStream@@QAEHXZ'
SOURCE = 'samples/io/FileStream.cpp'
PROBES = {0x00460dc8: 'first seek ok', 0x00460e27: 'header matched'}


def cases(rng, n):
    for _ in range(n):
        header = rng.choice([[0x46, 0x41, 0x4f, 0x45], [0xc6, 0x41, 0xcf, 0x80], [0, 0, 0, 0]])
        reads = list(header)
        if rng.randrange(4):
            k = rng.randrange(4)
            reads[k] = rng.choice([-1, reads[k] ^ 1, reads[k] - 256 if reads[k] >= 0x80 else reads[k] + 256])
        yield dict(header=header, reads=reads, seeks=[rng.choice([0, 0, 0, -1, 1]) for _ in range(2)],
                   encoded=rng.choice([0, 1, 7]), handle=rng.choice([0x1234, 0]))


def setup(ctx, c):
    stream = ctx.alloc(0x134, fill=0x33)
    ctx.pack(stream + 2, 'B', c['header'][0])
    ctx.pack(stream + 0, 'B', c['header'][1])
    ctx.pack(stream + 0x20, 'B', c['header'][2])
    ctx.pack(stream + 0x18, 'B', c['header'][3])
    ctx.pack(stream + 8, 'i', c['encoded'])
    ctx.pack(stream + 0x14, 'I', c['handle'])
    ctx.pack(stream + 0x124, 'II', stream + 0x30, stream + 0x90)
    seeks, reads = list(c['seeks']), list(c['reads'])

    def fseek(cx, e, a):
        cx.note('fseek', *a)
        return seeks.pop(0) & 0xffffffff

    def read_byte(cx, e, a):
        return reads.pop(0) & 0xffffffff if reads else 0xffffffff

    ctx.stub_at(0x00534eac, 'fseek', nargs=3, ret=fseek, log=False)
    ctx.stub_at(0x00534c3d, 'fclose', nargs=1, ret=lambda cx, e, a: cx.note('fclose', *a), log=False)
    ctx.stub_at(0x00461980, 'FileStream::ReadByte', pop=0, ret=read_byte)
    return dict(ecx=stream, args=[])
