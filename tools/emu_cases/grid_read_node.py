"""DrawableGridNode::ReadNode 0x0047e600, the terrain quadtree node loader (tools/emu_diff.py
case, call-record/replay mode).

The linked body is src/reconstructed/Griddraw.cpp. FileStream::Read 0x00461640
serves a synthetic node record from a byte queue: the four scale/height floats (ordinary,
negative, zero, huge), maxError, a block index, the texture header, the 16 block words (all
0xfffe/0xffff markers or one real reference, so both allocations run), an extra byte count
(0 or up to 300 bytes), the flag byte, the child list offset and the 256-entry child table (empty,
sparse or full). A drained queue reads zeros. Seek 0x00461340 is logged; DebugCalloc 0x004a2fc0,
DebugMalloc 0x004a2e20 and operator new 0x004a3010 return fresh memory and note (count, size,
line) without the unit's own __FILE__. Virtual slot 5 (the post-read hook) and slot 2
(CreateChildNode) go through a fake vtable. Bounds pointers are given or NULL, the parent is
given or NULL, shift and level vary, so the bounds updates, the stale-extent clamp (retail tests
extent.z before storing it) and the child arguments are compared at both control words.
"""
import random
import struct

RETAIL_VA = 0x0047e600
SYMBOL = ('?ReadNode@DrawableGridNode@@QAEPAV1@PAVFileStream@@HPAV1@HHHHHPAM2@Z')
SOURCE = 'src/reconstructed/Griddraw.cpp'
PROBES = {0x0047e697: 'bounds', 0x0047ebc5: 'records', 0x0047e8ae: 'extra', 0x0047ea68: 'clamp',
          0x0047eacd: 'children', 0x0047eb39: 'child'}

READ, SEEK = 0x00461640, 0x00461340
CALLOC, MALLOC, NEW = 0x004a2fc0, 0x004a2e20, 0x004a3010


def _read(ctx, rec):
    buf, size, count = rec.args[:3]
    n = max(size * count, 0)
    queue = ctx.data['queue']
    take = bytes(queue[:n])
    del queue[:n]
    ctx.write(buf, take + bytes(n - len(take)))
    ctx.note('Read', rec.ecx, size, count)
    return count


def _seek(ctx, rec):
    ctx.note('Seek', rec.ecx, *rec.args[:3])
    return 0


def _calloc(ctx, rec):
    ctx.note('DebugCalloc', rec.args[0], rec.args[1], rec.args[3])
    return ctx.alloc(max(rec.args[0] * rec.args[1], 1), fill=0)


def _malloc(ctx, rec):
    ctx.note('DebugMalloc', rec.args[0], rec.args[2])
    return ctx.alloc(max(rec.args[0], 1), fill=0xcd)


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return ctx.alloc(max(rec.args[0], 1), fill=0xcd)


returns = {READ: _read, SEEK: _seek, CALLOC: _calloc, MALLOC: _malloc, NEW: _new}
pops = {READ: 12, SEEK: 12, CALLOC: 0, MALLOC: 0, NEW: 0}
args = {READ: 3, SEEK: 3, CALLOC: 4, MALLOC: 3, NEW: 3}
ecx = {READ: True, SEEK: True, CALLOC: False, MALLOC: False, NEW: False}
REPLAY = dict(returns=returns, pops=pops, args=args, ecx=ecx, unlogged=(READ, SEEK, CALLOC, MALLOC, NEW),
              log_pointees=False)


def _float(rng):
    return rng.choice([0.0, 1.0, -1.0, 0.5, 2.0, 1e-30, -3.25, 1e20, rng.uniform(-100, 100),
                       rng.uniform(0, 4000)])


def cases(rng, n):
    for _ in range(n):
        words = [rng.choice([0xffff, 0xfffe, 0x1fffe, 0xfffffffe]) for _ in range(16)]
        if rng.random() < 0.5:
            words[rng.randrange(16)] = rng.choice([0, 1, 0xfffd, 0x7ffe, rng.getrandbits(32)])
        table = rng.choice(['none', 'sparse', 'full', 'zero'])
        yield dict(seed=rng.getrandbits(32), floats=[_float(rng) for _ in range(4)], words=words,
                   extra=rng.choice([0, 0, 1, 7, 300]), table=table,
                   child_list=0 if table == 'none' else rng.choice([4, 0x100, 0x7fffffff]),
                   a1=rng.choice([0, 1, 1]), parent=rng.random() < 0.4, bounds=rng.random() < 0.7,
                   level=rng.choice([0, 1, 3, 255]), shift=rng.choice([0, 4, 8, 12, 20]),
                   x=rng.choice([0, 16, 240, -16, 70000]), z=rng.choice([0, 32, 255, -1]),
                   index=rng.randrange(-1, 300), normals=rng.getrandbits(8), cell=_float(rng),
                   bmin=[_float(rng) for _ in range(3)], bmax=[_float(rng) for _ in range(3)])


def setup(ctx, c):
    r = random.Random(c['seed'])
    q = bytearray(struct.pack('<4f', *c['floats']))
    q += struct.pack('<H', r.getrandbits(16))                     # maxError
    q += struct.pack('<H', r.randrange(8))                        # blockIndex
    q += struct.pack('<I', r.getrandbits(32))                     # texture header
    q += struct.pack('<16I', *c['words'])
    q += struct.pack('<I', c['extra']) + bytes(r.getrandbits(8) for _ in range(c['extra']))
    q += bytes([r.getrandbits(8)])
    q += struct.pack('<I', c['child_list'])
    if c['table'] == 'sparse':
        offsets = [r.choice([0, 0, 0, 0, 0, 0, 0x40, r.getrandbits(31)]) for _ in range(256)]
    elif c['table'] == 'full':
        offsets = [r.getrandbits(32) or 1 for _ in range(256)]
    else:
        offsets = [0] * 256
    q += struct.pack('<256I', *offsets)
    ctx.data['queue'] = q

    def terrain():
        t = ctx.alloc(0xc40, bytes(r.getrandbits(8) for _ in range(0xc40)))
        blocks = ctx.alloc(32, struct.pack('<8I', *(0x7000 + 0x10 * k for k in range(8))))
        ctx.pack(t + 0xc14, 'I', blocks)
        ctx.pack(t + 0xc34, 'B', c['normals'])
        ctx.pack(t + 0x40, 'f', c['cell'])
        return t

    node = ctx.alloc(0x80, bytes(r.getrandbits(8) for _ in range(0x80)))
    ctx.pack(node, 'I', ctx.fake_vtable('gridNode', 8, pops={2: 40, 5: 4}, returns={2: 'ptr', 5: 'int'}))
    ctx.pack(node + 0x34, 'I', terrain())
    parent = 0
    if c['parent']:
        parent = ctx.alloc(0x80, bytes(r.getrandbits(8) for _ in range(0x80)))
        ctx.pack(parent + 0x34, 'I', terrain())
    bmin = bmax = 0
    if c['bounds']:
        bmin = ctx.alloc(12, struct.pack('<3f', *c['bmin']))
        bmax = ctx.alloc(12, struct.pack('<3f', *c['bmax']))
    elif r.random() < 0.5:
        bmin = ctx.alloc(12, struct.pack('<3f', *c['bmin']))
    stream = ctx.alloc(0x40, fill=0x53)
    return dict(ecx=node, args=[stream, c['a1'], parent, c['level'], c['shift'], c['x'] & 0xffffffff,
                                c['z'] & 0xffffffff, c['index'] & 0xffffffff, bmin, bmax])
