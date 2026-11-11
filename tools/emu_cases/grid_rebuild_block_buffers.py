"""DrawableGridNode::RebuildBlockBuffers 0x0047f210, the per-block vertex and index buffer
rebuild (tools/emu_diff.py case, call-record/replay mode).

The linked body is samples/render/GriddrawNearMisses.cpp. For each of the sixteen 4 x 4 blocks
the node's GetBlockTextureSize (slot 8, answering 0x40 or not, which scales the texture step by
4) is asked, the vertex cache 0x00677910 is reset (Reset 0x0047de20: count 0, an empty height
range) and EmitDiagonalTriangles 0x0047fce0 and AppendCornerTriangle 0x00480700 are replayed:
each adds 0 to a few triangles and vertices, writing random vertices into the cache and random
indices into g_gridIndices 0x0067a68c and widening the cache's height range. Emitting nothing
reaches the corner path when the node has no child triangles (b0 or childTriangleCount) and a
parent whose extra has no texture. The draw data starts with no buffers, buffers large enough or
too small (grown through DebugMalloc 0x004a2e20, the old contents copied and DebugFree'd
0x004a2e60), and DebugMalloc fails at a chosen call (the 0-return paths); g_gridDrawMemory and
its peak 0x0068a300/0x0068a2fc start below or above the new total. The age entry is
registered (AgeManager::Register 0x00401050) or resized. Block ranges, mapping records, the
parent's b1 bit and every buffer byte are compared, at both control words (centre and extent
heights, the texture step).
"""
import random
import struct

RETAIL_VA = 0x0047f210
SYMBOL = '?RebuildBlockBuffers@DrawableGridNode@@QAEHXZ'
SOURCE = 'samples/render/GriddrawNearMisses.cpp'
PROBES = {0x0047f32d: 'corner 1', 0x0047f38c: 'corner 2', 0x0047f4b4: 'vertex grow', 0x0047f4f7: 'vertex free',
          0x0047f590: 'index grow', 0x0047f5d3: 'index free', 0x0047f600: 'copy', 0x0047f6eb: 'register'}

CACHE = 0x00677910
COUNT, MIN_Y, MAX_Y, VERTICES = CACHE + 0x2d30, CACHE + 0x2d5c, CACHE + 0x2d60, CACHE + 0x910
INDICES = 0x0067a68c
MEMORY, PEAK = 0x0068a300, 0x0068a2fc
RESET, EMIT, CORNER = 0x0047de20, 0x0047fce0, 0x00480700
MALLOC, FREE, REGISTER = 0x004a2e20, 0x004a2e60, 0x00401050


def _reset(ctx, rec):
    ctx.pack(COUNT, 'i', 0)
    ctx.pack(MIN_Y, 'f', 1e30)
    ctx.pack(MAX_Y, 'f', -1e30)
    return 0


def _emit(kind):
    def run(ctx, rec):
        d = ctx.data
        r = rec.rng
        triangles = rec.args[1]
        added = r.choice(d['emits']) if kind == 'emit' else r.choice([0, 1, 1])
        count = ctx.unpack(COUNT, 'i')[0]
        verts = min(r.randint(0, 3) + added, 289 - count)
        if verts > 0:
            ctx.write(VERTICES + 32 * count, bytes(r.getrandbits(8) for _ in range(32 * verts)))
            ctx.pack(COUNT, 'i', count + verts)
        start = d['indices']
        n = 3 * added
        if n and start + n <= 0x800:
            ctx.write(INDICES + 2 * start, bytes(r.getrandbits(8) for _ in range(2 * n)))
        d['indices'] = start + n
        lo, hi = ctx.unpack(MIN_Y, 'ff')
        for _ in range(verts):
            y = r.uniform(-50.0, 300.0)
            lo, hi = min(lo, y), max(hi, y)
        ctx.pack(MIN_Y, 'ff', lo, hi)
        return triangles + added
    return run


def _malloc(ctx, rec):
    d = ctx.data
    k = d['mallocs']
    d['mallocs'] = k + 1
    ctx.note('DebugMalloc', rec.args[0], rec.args[2])
    if k == d['fail_at']:
        return 0
    return ctx.alloc(max(rec.args[0], 1) & 0xffff, fill=0xab)


def _free(ctx, rec):
    ctx.note('DebugFree', rec.args[0], rec.args[2])
    return 0


def _register(ctx, rec):
    ctx.note('Register', rec.ecx, *rec.args[:5])
    return 0


def _block_size(ctx, rec):
    return rec.rng.choice([0x40, 0x40, 0x20, 0x80, 0])


returns = {RESET: _reset, EMIT: _emit('emit'), CORNER: _emit('corner'), MALLOC: _malloc, FREE: _free,
           REGISTER: _register}
REPLAY = dict(returns=returns, unlogged=(MALLOC, FREE, REGISTER),
              pops={MALLOC: 0, FREE: 0}, args={MALLOC: 3, FREE: 3}, ecx={MALLOC: False, FREE: False})


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32),
                   emits=rng.choice([[0], [0, 1, 2], [0, 0, 0, 3], [1, 2, 4, 8], [8, 16]]),
                   vertex=rng.choice(['none', 'none', 'small', 'large']),
                   index=rng.choice(['none', 'none', 'small', 'large']),
                   fail_at=rng.choice([None] * 6 + [0, 1, 2]),
                   b0=rng.random() < 0.3, child=rng.choice([0, 0, 5]),
                   parent=rng.choice(['none', 'textured', 'bare', 'bare']),
                   age=rng.choice([0, 0, 64, 0x1234]),
                   memory=rng.choice([0, 1000, 0x7fff0000]), peak=rng.choice([0, 5000, 0x7fffffff]),
                   before=rng.choice([0, 0, 7, 30]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.data.update(emits=c['emits'], indices=0, mallocs=0, fail_at=c['fail_at'])
    ctx.pack(COUNT, 'i', r.randrange(289))
    ctx.pack(MEMORY, 'i', c['memory'])
    ctx.pack(PEAK, 'i', c['peak'])

    data = ctx.alloc(0x180 + 16 * 12, bytes(r.getrandbits(8) for _ in range(0x180 + 16 * 12)))
    flags = ctx.read(data + 0x121, 1)[0] & ~0x3
    ctx.pack(data + 0x121, 'B', flags | int(c['b0']))
    ctx.pack(data + 0x12a, 'hh', c['before'], c['child'])
    for which, at in (('vertex', 0x134), ('index', 0x138)):
        kind = c[which]
        size = {'none': 0, 'small': 64, 'large': 0x7000}[kind]
        buf = ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size))) if size else 0
        ctx.pack(data + at, 'I', buf)
        ctx.pack(data + 0x140 + (at - 0x134) // 2, 'h', size if size else r.choice([0, 32]))
    ctx.pack(data + 0x154, 'i', c['age'])
    ctx.pack(data + 0x160, 'f', r.choice([1.0, 0.25, 1 / 3.0, 0.0625 * 3]))
    ctx.pack(data + 0x17c, 'I', data + 0x180)

    records = ctx.alloc(16 * 0x2c, bytes(r.getrandbits(8) for _ in range(16 * 0x2c)))
    extra = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    ctx.pack(extra + 0x38, 'I', records)

    terrain = ctx.alloc(0xd00, bytes(r.getrandbits(8) for _ in range(0xd00)))
    ctx.pack(terrain + 0x40, 'f', r.choice([1.0, 2.5, 0.3]))
    ctx.pack(terrain + 0xc88, 'I', ctx.alloc(0x20))

    parent = 0
    if c['parent'] != 'none':
        parent = ctx.alloc(0x60, bytes(r.getrandbits(8) for _ in range(0x60)))
        pextra = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
        ctx.pack(pextra + 8, 'I', 0x1234 if c['parent'] == 'textured' else 0)
        pdata = ctx.alloc(0x180, bytes(r.getrandbits(8) for _ in range(0x180)))
        ctx.pack(pdata + 0x121, 'B', ctx.read(pdata + 0x121, 1)[0] & ~0x2)
        ctx.pack(parent + 0x38, 'I', pdata)
        ctx.pack(parent + 0x3c, 'I', pextra)

    node = ctx.alloc(0x60, bytes(r.getrandbits(8) for _ in range(0x60)))
    ctx.pack(node, 'I', ctx.fake_vtable('node', 12, pops={8: 4}, returns={8: _block_size}))
    ctx.pack(node + 8, 'I', parent)
    ctx.pack(node + 0x34, 'I', terrain)
    ctx.pack(node + 0x38, 'I', data)
    ctx.pack(node + 0x3c, 'I', extra)
    return dict(ecx=node, args=[])
