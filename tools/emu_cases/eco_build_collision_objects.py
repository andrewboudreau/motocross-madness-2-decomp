"""EcoSystem::BuildCollisionObjects 0x00457ed0 (tools/emu_diff.py case).

The linked body is src/bringup/Group4_EcoSystem.cpp. Offsets from the retail body: the
definition's +0x204 collision count, +0x208 the 0x24-byte collision definitions (+0x00 type,
+0x04 start, +0x10 end, +0x1c radius, +0x20 height), +0x20c the object table it allocates,
and model LOD 0's +0x1e8 indices (unsigned shorts), +0x1ec vertices (0x20-byte, position
first), +0x1f0 vertex count and +0x1f4 index count. g_TrackGame 0x0056e26c +0x10 display;
g_MemTagStack 0x0056df04. Types run over 0 (model hull), 1 (16-sided cylinder hull built
with fcos/fsin), 2 (sphere), 3 (capsule) and unknown values; counts include zero (early
return, table cleared).

The function has an SEH frame (install_seh_segment from kb_simulate_frame). Stubbed:
MemTagStack Push 0x004a2d00 (notes the tag text) / Pop 0x004a2d90, DebugMalloc 0x004a2e20
and operator new(size, file, line) 0x004a3010 (note size and line; the __FILE__ pointer is
per object), operator delete 0x004a30c0, the CollisionObject constructor 0x00431e70,
Configure 0x004320f0, AppendChild 0x00469190 (on the ecosystem), SetTriangleMeshHullShape
0x004328b0 (notes the vertex and index arrays; installs a hull shape at +0x54 whose
transform +0xc8 the caller sets to identity), SetSphereShape 0x004329a0 and
SetCapsuleShape 0x00432a20 (by-value vectors and radius logged).
"""
import random
import struct

from tools.emu_cases.kb_simulate_frame import install_seh_segment

RETAIL_VA = 0x00457ed0
SYMBOL = '?BuildCollisionObjects@EcoSystem@@QAEXPAVUnknownEcoDefinition@@@Z'
SOURCE = 'src/bringup/Group4_EcoSystem.cpp'
TRACK_GAME = 0x0056e26c
MEM_TAG_STACK = 0x0056df04
PROBES = {0x00457f06: 'empty', 0x00458004: 'model hull', 0x0045816d: 'sphere', 0x004581a1: 'capsule',
          0x004581eb: 'cylinder', 0x00458331: 'pop'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), count=rng.choice([0, 1, 1, 2, 3]),
                   types=[rng.choice([0, 1, 2, 3, 0, 1, 5]) for _ in range(3)],
                   verts=rng.choice([0, 3, 5, 8]), tris=rng.choice([0, 1, 2, 4]))


def setup(ctx, c):
    install_seh_segment(ctx)
    r = random.Random(c['seed'])
    definition = ctx.alloc(0x220, fill=0x5a)
    shapes = ctx.alloc(0x24 * 3)
    for i in range(3):
        ctx.pack(shapes + 0x24 * i, 'i8f', c['types'][i], *[r.uniform(-20, 20) for _ in range(6)],
                 r.uniform(0.1, 5), r.uniform(0.5, 10))
    verts = ctx.alloc(0x20 * max(c['verts'], 1))
    ctx.write(verts, struct.pack('<%df' % (8 * max(c['verts'], 1)), *[r.uniform(-5, 5) for _ in range(8 * max(c['verts'], 1))]))
    indices = ctx.alloc(6 * max(c['tris'], 1))
    for k in range(3 * c['tris']):
        ctx.pack(indices + 2 * k, 'H', r.randrange(max(c['verts'], 1)) if r.random() < 0.9 else 0xfffe)
    ctx.pack(definition + 0x1e8, 'IIii', indices, verts, c['verts'], 3 * c['tris'] + r.choice([0, 0, 1]))
    ctx.pack(definition + 0x204, 'iI', c['count'], shapes)

    eco = ctx.alloc(0x600, fill=0x21)
    game = ctx.alloc(0x40, fill=0x31)
    ctx.pack(game + 0x10, 'I', ctx.alloc(0x40))
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(MEM_TAG_STACK, 'I', ctx.alloc(0x40))

    def push_tag(cx, ecx, a):
        cx.note('Push', ecx, cx.read(a[0], 12).split(b'\0')[0])
        return 0x1234

    def malloc(cx, ecx, a):
        cx.note('DebugMalloc', a[0], a[2])
        return cx.alloc(max(a[0], 4), fill=0xdd)

    def op_new(cx, ecx, a):
        cx.note('new', a[0], a[2])
        return cx.alloc(max(a[0], 4), fill=0xcd)

    def mesh_hull(cx, ecx, a):              # (vertices, indices, triangle count, vertex count)
        nv, nt = a[3], a[2]
        cx.note('SetTriangleMeshHullShape', ecx, cx.read(a[0], 12 * min(nv, 64)).hex(),
                cx.read(a[1], 12 * min(nt, 64)).hex(), nt, nv)
        cx.pack(ecx + 0x54, 'I', cx.alloc(0x110, fill=0xee))

    ctx.stub_at(0x004a2d00, 'MemTagPush', pop=4, ret=push_tag, log=False)
    ctx.stub_at(0x004a2d90, 'MemTagPop', pop=4)
    ctx.stub_at(0x004a2e20, 'DebugMalloc', nargs=3, ret=malloc, log=False)
    ctx.stub_at(0x004a3010, 'operator new', nargs=3, ret=op_new, log=False)
    ctx.stub_at(0x004a30c0, 'operator delete', nargs=1, ret=lambda cx, ecx, a: cx.note('delete', a[0]), log=False)
    ctx.stub_at(0x00431e70, 'CollisionObject', pop=4, ret=lambda cx, ecx, a: ecx)
    ctx.stub_at(0x004320f0, 'Configure', pop=16)
    ctx.stub_at(0x00469190, 'AppendChild', pop=8)
    ctx.stub_at(0x004328b0, 'SetTriangleMeshHullShape', pop=16, ret=mesh_hull, log=False)
    ctx.stub_at(0x004329a0, 'SetSphereShape', pop=16)
    ctx.stub_at(0x00432a20, 'SetCapsuleShape', pop=28)
    return dict(ecx=eco, args=[definition], ret_eax=False)
