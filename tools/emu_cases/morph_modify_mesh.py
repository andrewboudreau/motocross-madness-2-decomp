"""MorphBastardModifier::ModifyMesh 0x004a4c60, slot 27: blends every channel's morph-target
deltas into a copy of the mesh (tools/emu_diff.py case, call-record/replay mode).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_MorphBastard.cpp). Its calls
are replayed: CopyMesh 0x004a5290 (the effect allocates ``meshCopy`` with its own drawn
vertices, as the real one does on first use), UpdateChannels 0x004a4bb0 (the channel values
are inputs here), the empty stub 0x00464e80 and the debug ``operator new`` 0x004a3010 (fresh
heap memory; its size and line are noted, not the __FILE__ pointer). The rest is the body itself: the vertex copy, the per-axis
value choice, the bracket search over the targets, both weighted delta loops with their
running minimum and maximum, and the final add into the copy.

Inputs: one to three morph objects, each with a mesh group (at a vertex offset, so ``base`` is
not zero) and one to three channels on axes 0..2; values below the first target, above the
last, between two targets and on a target exactly; targets with no deltas; a mesh copy that
already exists, delta arrays that already exist, and the draw-unchanged early return.
"""
import struct

RETAIL_VA = 0x004a4c60
SYMBOL = ('?ModifyMesh@MorphBastardModifier@@UAEXPAVD3DIMSoultreeObject@@PAUModifierSoultreeSurfaceView@@'
          'PAPAU3@@Z')
SOURCE = 'src/bringup/Group3_MorphBastard.cpp'
COPY_MESH = 0x004a5290
UPDATE_CHANNELS = 0x004a4bb0
STUB = 0x00464e80
DEBUG_NEW = 0x004a3010
PROBES = {0x004a4c74: 'draw unchanged',
          0x004a4c96: 'CopyMesh',
          0x004a4d2c: 'delta arrays allocated',
          0x004a4e7b: 'value at or below the first target',
          0x004a4e96: 'value at or above the last target',
          0x004a4ed2: 'value between two targets',
          0x004a4f14: 'no upper target (upper = 0)'}


def _copy_mesh(ctx, rec):
    # CopyMesh(ModifierSoultreeSurfaceView* mesh): this->meshCopy = a copy with its own vertices.
    mesh = rec.args[0]
    count = ctx.unpack(mesh + 8, 'i')[0]
    copy = ctx.alloc(0x38)
    ctx.pack(copy + 8, 'i', count)
    ctx.pack(copy + 0x14, 'I', ctx.alloc(32 * count + 4))
    ctx.pack(rec.ecx + 0x40, 'I', copy)


def _new(ctx, rec):
    # operator new(size, __FILE__, line): the size and line are compared, not the __FILE__
    # pointer (retail's MorphBastardModifier.cpp path, the bring-up file's own here).
    ctx.note('new', rec.args[0], rec.args[2])
    return ctx.alloc(rec.args[0] + 4)


REPLAY = dict(
    unlogged=(DEBUG_NEW,),
    returns={DEBUG_NEW: _new},
    effects={COPY_MESH: _copy_mesh},
)


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32),
                   unchanged=rng.random() < 0.05,
                   have_copy=rng.random() < 0.4,
                   have_deltas=rng.random() < 0.4,
                   objects=rng.randint(1, 3))


def setup(ctx, c):
    rng = ctx.rng

    def floats(values):
        at = ctx.alloc(4 * len(values) + 4)
        ctx.pack(at, '%df' % len(values), *values)
        return at

    # The mesh: one group per morph object, laid out one after the other.
    sizes = [rng.randint(2, 7) for _ in range(c['objects'])]
    lead = rng.randint(0, 3)
    total = lead + sum(sizes) + rng.randint(0, 2)
    vertices = ctx.alloc(32 * total)
    drawn = ctx.alloc(32 * total)
    ctx.pack(drawn, '%df' % (8 * total), *[rng.uniform(-5, 5) for _ in range(8 * total)])
    groups = ctx.alloc(0x14 * (c['objects'] + 1))
    nodes = [ctx.alloc(0x10) for _ in range(c['objects'] + 1)]
    starts = []
    at = lead
    order = list(range(c['objects']))
    rng.shuffle(order)
    for g, obj in enumerate(order):
        starts.append(None)
    for obj in range(c['objects']):
        starts[obj] = at
        at += sizes[obj]
    # an extra group for an unrelated node, so the search skips one
    entries = [(nodes[obj], sizes[obj], starts[obj]) for obj in order] + [(nodes[-1], 1, 0)]
    rng.shuffle(entries)
    for g, (node, count, start) in enumerate(entries):
        ctx.pack(groups + 0x14 * g, 'IiIII', node, count, vertices + 32 * start, 0, 0)
    mesh = ctx.alloc(0x38)
    ctx.pack(mesh, 'iIiiII', len(entries), groups, total, 0, vertices, drawn)

    objects = ctx.alloc(0x4c * c['objects'])
    for obj in range(c['objects']):
        channels = rng.randint(1, 3)
        channel_base = ctx.alloc(0x6c * channels)
        for k in range(channels):
            ch = channel_base + 0x6c * k
            count = rng.randint(1, 4)
            values = sorted(rng.uniform(-60, 60) for _ in range(count))
            targets = ctx.alloc(0x14 * count)
            for t, value in enumerate(values):
                deltas = rng.choice([0, rng.randint(1, sizes[obj] + 1)])
                indices = [starts[obj] + rng.randrange(sizes[obj]) for _ in range(deltas)]
                ctx.pack(targets + 0x14 * t, 'fiIII', value, deltas,
                         ctx.alloc(4 * deltas + 4) if not deltas else _ints(ctx, indices),
                         floats([rng.uniform(-1, 1) for _ in range(3 * deltas)]),
                         ctx.alloc(4 * deltas + 4))
            pick = rng.random()
            if pick < 0.15:
                value = values[0] - rng.uniform(0, 10)
            elif pick < 0.3:
                value = values[-1] + rng.uniform(0, 10)
            elif pick < 0.4:
                value = rng.choice(values)
            else:
                value = rng.uniform(values[0], values[-1])
            axis = rng.randrange(3)
            three = [rng.uniform(-60, 60) for _ in range(3)]
            three[axis] = value
            ctx.pack(ch, 'i', axis)
            ctx.pack(ch + 0x4c, 'iIff', count, targets, values[0], values[-1])
            ctx.pack(ch + 0x60, '3f', *three)
        ctx.pack(objects + 0x4c * obj, 'iII', channels, channel_base, nodes[obj])

    this = ctx.alloc(0x58)
    ctx.pack(this + 0x44, 'iI', c['objects'], objects)
    if c['have_copy']:
        copy = ctx.alloc(0x38)
        ctx.pack(copy + 8, 'i', total)
        ctx.pack(copy + 0x14, 'I', ctx.alloc(32 * total))
        ctx.pack(this + 0x40, 'I', copy)
    if c['have_deltas']:
        for k in range(3):
            ctx.pack(this + 0x4c + 4 * k, 'I', ctx.alloc(12 * total))
    view = ctx.alloc(0x280)
    ctx.pack(view + 0x27c, 'i', int(c['unchanged']))
    out = ctx.alloc(4)
    return dict(ecx=this, args=[view, mesh, out], ret_eax=False)


def _ints(ctx, values):
    at = ctx.alloc(4 * len(values) + 4)
    ctx.pack(at, '%di' % len(values), *values)
    return at
