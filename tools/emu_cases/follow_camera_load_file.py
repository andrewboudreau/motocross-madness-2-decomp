"""FollowCamera::LoadCameraFile 0x004650e0, loads the CAMERA records of a text file (emu_diff case).

The candidate is samples/camera/FollowCameraLoadFile.cpp. File I/O is stubbed with a
case-generated file: the stream (0x134 bytes from operator new 0x004a3010, FollowCam.cpp line
0x3c7) is built by a logged stub of FileStream's constructor 0x00460d10 that sets
sliceOffset +0x130; Open 0x00460f50 answers the case's flag; ReadLine 0x00461aa0 copies the
next line (up to its size argument) and returns 0 at the end; Seek 0x00461340 rewinds; the
destructor 0x00460d60 and operator delete 0x004a30c0 are logged. sscanf 0x0053530b is a stub
that reads the 10 %f fields after five skipped tokens (binary32 round to nearest) and logs the
format. The file mixes CAMERA lines (any case, 'CAMERAS', camera records with random
directions, up hints, positions and a time), other text and empty lines; zero records
happen. _strnicmp 0x005476f0, CrossProductCall 0x00515600 and Vec3Normalize 0x005087b0 run as
retail code, so the compared state is frameCount +0x334, the frames +0x338 (64 bytes each,
from operator new line 0x3e1), the times +0x33c (line 0x3e2) and the call log. The __FILE__
tag is not compared (the sample's own name; TU ownership is unproven); fs:[0] is page 0.
"""
import random
import struct

RETAIL_VA = 0x004650e0
SYMBOL = '?LoadCameraFile@FollowCamera@@QAEXPBD@Z'
SOURCE = 'samples/camera/FollowCameraLoadFile.cpp'
PROBES = {0x00465131: 'stream constructed', 0x004651d5: 'allocate frames', 0x004652b3: 'record',
          0x004654ae: 'done'}


def record(rng, word):
    nums = [rng.uniform(-1, 1) for _ in range(6)] + [rng.uniform(-500, 500) for _ in range(3)]
    nums.append(rng.uniform(0, 30))
    head = ['%.3f' % rng.uniform(-9, 9) for _ in range(3)]
    return ('%s cam%d %s %s' % (word, rng.randrange(9), ' '.join(head),
                                 ' '.join(rng.choice(['%.6f', '%.9g', '%g']) % x for x in nums)))


def cases(rng, n):
    for _ in range(n):
        lines = []
        for _ in range(rng.choice([0, 1, 3, 8])):
            k = rng.random()
            if k < 0.6:
                lines.append(record(rng, rng.choice(['CAMERA', 'camera', 'Camera', 'CAMERAS'])))
            elif k < 0.8:
                lines.append(rng.choice(['', '# comment', 'CAMER', 'TARGET 1 2 3', 'xCAMERA']))
            else:
                lines.append('%d %d' % (rng.randrange(100), rng.randrange(100)))
        yield dict(seed=rng.randrange(1 << 30), open=rng.random() < 0.9, lines=lines,
                   slice=rng.randrange(0, 1000))


def to_f32(s):
    return struct.unpack('<f', struct.pack('<f', float(s)))[0]


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])
    cam = ctx.alloc(0x400, bytes(r.randrange(256) for _ in range(0x400)))
    pos = [0]

    def new(cx, e, a):
        cx.note('operator new', a[0], a[2])
        return cx.alloc(max(a[0], 1), fill=0xcd)
    ctx.stub_at(0x004a3010, 'operator new', pop=0, nargs=3, ret=new, log=False)
    ctx.stub_at(0x004a30c0, 'operator delete', pop=0, nargs=1)

    def construct(cx, e, a):
        cx.pack(e + 0x130, 'i', c['slice'])
        return e
    ctx.stub_at(0x00460d10, 'FileStream::FileStream', pop=4, ret=construct)
    ctx.stub_at(0x00460d60, 'FileStream::~FileStream', pop=0)

    def open_(cx, e, a):
        cx.note('Open', cx.read(a[1], 8).split(b'\0')[0])
        return int(c['open'])
    ctx.stub_at(0x00460f50, 'FileStream::Open', pop=12, ret=open_)

    def read_line(cx, e, a):
        if pos[0] >= len(c['lines']):
            return 0
        text = c['lines'][pos[0]].encode()[:a[1] - 1]
        pos[0] += 1
        cx.write(a[0], text + b'\0')
        return 1
    ctx.stub_at(0x00461aa0, 'FileStream::ReadLine', pop=8, ret=read_line)

    def seek(cx, e, a):
        pos[0] = 0
        return 1
    ctx.stub_at(0x00461340, 'FileStream::Seek', pop=12, ret=seek)

    def sscanf(cx, e, a):
        line = cx.read(a[0], 256).split(b'\0')[0].decode()
        cx.note('sscanf', cx.read(a[1], 40).split(b'\0')[0])
        fields = line.split()[5:15]
        for p, s in zip(a[2:], fields):
            cx.pack(p, 'f', to_f32(s))
        return len(fields)
    ctx.stub_at(0x0053530b, 'sscanf', pop=0, nargs=12, ret=sscanf, log=False)

    path = ctx.alloc(32, b'camera.txt\0')
    return dict(ecx=cam, args=[path], ret_eax=False)
