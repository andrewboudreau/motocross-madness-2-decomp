"""MotionManager::LoadVue 0x004a5e40, the text .VUE motion loader (tools/emu_diff.py case).

The linked body is samples/physics/motion/Motnctrl.cpp. The file is a scripted list of lines:
"FRAME n" headers and TRANSFORM lines naming a node in quotes (known to the character or not)
with twelve numbers (integers, decimals, exponents, signs and junk suffixes), plus blank lines,
lower-case keywords, other keywords and short TRANSFORM lines. The stream (`new` 0x004a3010,
constructor 0x00460d10, destructor 0x00460d60, delete 0x004a30c0) is a stub: Open 0x00460f50
answers per case, ReadLine 0x00461aa0 copies the next line (0 at the end) and Seek 0x00461340
rewinds. _strupr 0x00535d3d, strtok 0x00536397 (with its saved pointer and the NULs it writes),
_stricmp 0x00536070 and atof 0x00536340 (a double in ST0) are modelled on the emulated memory;
Character::FindNode 0x004a6b50 answers the node index for known names in AL with junk above it,
else 0xff. DebugMalloc 0x004a2e20 / DebugRealloc 0x004a2ec0 allocate and note size and line; the
Motion's fields and the pose lists are compared as memory. frameCount runs 0..5 against files
of 0..4 frames.
"""
import re

RETAIL_VA = 0x004a5e40
SYMBOL = '?LoadVue@MotionManager@@QAEHPAVCharacter@@PBDPAUMotion@@@Z'
SOURCE = 'samples/physics/motion/Motnctrl.cpp'
PROBES = {0x004a5ebe: 'open failed', 0x004a60b4: 'pose kept', 0x004a6244: 'realloc'}

NODES = {b'HIP': 0, b'KNEE': 3, b'HEAD': 7, b'L_FOOT': 12}
NUMBER = re.compile(rb'^[ \t]*[-+]?(\d+\.?\d*|\.\d+)([eE][-+]?\d+)?')


def _cstr(ctx, at, limit=0x200):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _number(rng):
    return rng.choice([b'0', b'1', b'-2.5', b'3.25e1', b'+.5', b'1e-3', b'7x', b'-0', b'12345.678'])


def _transform(rng):
    name = rng.choice([b'Hip', b'knee', b'HEAD', b'L_Foot', b'Tail', b'hip'])
    numbers = [_number(rng) for _ in range(rng.choice([12, 12, 12, 13]))]
    return b'TRANSFORM "' + name + b'" ' + b' '.join(numbers)


def cases(rng, n):
    for _ in range(n):
        transforms = [_transform(rng) for _ in range(rng.randrange(0, 5))]
        lines = []
        for f in range(rng.randrange(0, 5)):
            lines.append(rng.choice([b'FRAME %d' % f, b'frame %d' % f]))
            lines += transforms if f == 0 or rng.random() < 0.8 else [_transform(rng) for _ in transforms]
        if rng.random() < 0.2 and lines:
            lines.insert(rng.randrange(len(lines)), rng.choice([b'COMMENT x', b'   ', b'TRANSFORM']))
        yield dict(lines=lines, opened=rng.random() < 0.95, frame_count=rng.randrange(0, 6),
                   junk=rng.getrandbits(24))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)               # fs:[0] (the EH frame's link) is page 0
    state = {'line': 0, 'saved': 0}
    lines = c['lines']

    def new(cx, e, a):
        cx.note('new', a[0], a[2])
        stream = cx.alloc(a[0] + 4, fill=0x5c)
        cx.pack(stream + 0x130, 'i', 0x1234)
        return stream

    def read_line(cx, e, a):
        if state['line'] >= len(lines):
            return 0
        text = lines[state['line']][:a[1] - 1]
        state['line'] += 1
        cx.write(a[0], text + bytes(1))
        return 1

    def seek(cx, e, a):
        state['line'] = 0
        return 0

    def strupr(cx, e, a):
        text = _cstr(cx, a[0])
        cx.write(a[0], text.upper())
        return a[0]

    def strtok(cx, e, a):
        delims = set(_cstr(cx, a[1]))
        p = a[0] or state['saved']
        if not p:
            return 0
        while cx.read(p, 1)[0] and cx.read(p, 1)[0] in delims:
            p += 1
        if not cx.read(p, 1)[0]:
            state['saved'] = p
            return 0
        start = p
        while cx.read(p, 1)[0] and cx.read(p, 1)[0] not in delims:
            p += 1
        if cx.read(p, 1)[0]:
            cx.write(p, bytes(1))
            p += 1
        state['saved'] = p
        return start

    def stricmp(cx, e, a):
        x = _cstr(cx, a[0]).lower() if a[0] else b''
        return 0 if a[0] and x == _cstr(cx, a[1]).lower() else 1

    def atof(cx, e, a):
        m = NUMBER.match(_cstr(cx, a[0])) if a[0] else None
        return 0, float(m.group(0)) if m else 0.0

    def find_node(cx, e, a):
        name = _cstr(cx, a[0]) if a[0] else b''
        cx.note('FindNode', name)
        return (c['junk'] << 8) | NODES.get(name, 0xff)

    def malloc(cx, e, a):
        cx.note('DebugMalloc', a[0], a[2])
        return cx.alloc(a[0] + 4, fill=0x99)

    def realloc(cx, e, a):
        cx.note('DebugRealloc', a[1], a[3])
        block = cx.alloc(a[1] + 4, fill=0x98)
        cx.write(block, cx.read(a[0], a[1]))
        return block

    ctx.stub_at(0x004a3010, 'new', nargs=3, ret=new, log=False)
    ctx.stub_at(0x00460d10, 'FileStream::FileStream', pop=4, ret=lambda cx, e, a: e)
    ctx.stub_at(0x00460f50, 'FileStream::Open', pop=12, ret=int(c['opened']))
    ctx.stub_at(0x00461aa0, 'FileStream::ReadLine', pop=8, ret=read_line, log=False)
    ctx.stub_at(0x00461340, 'FileStream::Seek', pop=12, ret=seek)
    ctx.stub_at(0x00460d60, 'FileStream::~FileStream', pop=0)
    ctx.stub_at(0x004a30c0, 'delete', nargs=1, ret=lambda cx, e, a: cx.note('delete', a[0]), log=False)
    ctx.stub_at(0x00534aaf, 'sprintf', nargs=3, ret=lambda cx, e, a: cx.note('sprintf', _cstr(cx, a[2])),
                log=False)
    ctx.stub_at(0x00535d3d, '_strupr', nargs=1, ret=strupr, log=False)
    ctx.stub_at(0x00536397, 'strtok', nargs=2, ret=strtok, log=False)
    ctx.stub_at(0x00536070, '_stricmp', nargs=2, ret=stricmp, log=False)
    ctx.stub_at(0x00536340, 'atof', nargs=1, ret=atof, ret_float='d', log=False)
    ctx.stub_at(0x004a6b50, 'Character::FindNode', pop=4, ret=find_node, log=False)
    ctx.stub_at(0x004a2e20, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(0x004a2ec0, 'DebugRealloc', nargs=4, ret=realloc, log=False)

    character = ctx.alloc(0x100, fill=0x43)
    motion = ctx.alloc(0xd0, fill=0x4d)
    ctx.pack(motion + 0xa8, 'i', c['frame_count'])
    path = ctx.alloc(0x40, b'motions\\rider.vue' + bytes(1))
    manager = ctx.alloc(0x40, fill=0x4e)
    return dict(ecx=manager, args=[character, path, motion])
