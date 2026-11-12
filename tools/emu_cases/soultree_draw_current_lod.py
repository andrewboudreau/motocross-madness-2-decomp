"""D3DIMSoultreeObject::DrawCurrentLod 0x00440f30, the tree's per-material draw loop
(tools/emu_diff.py case, call-record/replay mode).

The linked body is src/bringup/Group5_D3DIMSoulTreeDraw.cpp. FastInvSqrt 0x00460c00 (tables
built by 0x00460ae0/0x00460bb0) and its estimate 0x00460c70, the out-of-line dot product
0x0040ae30, the Vector3 constructor 0x00404e60, ReflectAndNormalize 0x004a10e0 and the CRT
helpers (__ftol, __CIpow, asin 0x00534db0, acos 0x00535240) run natively; every other call is
replayed. The render target (+0x24) has a fake vtable whose GetRenderState (+0x24) writes the
state's value and whose DrawIndexedPrimitive (+0x3c) and DrawPrimitive (+0x40) note the
vertices they are handed, so each mapping mode's texture coordinates are compared at draw time
even when the restore loop puts them back afterwards. The camera (render target +8) has
SetWorldMatrix at +0x78; the modifiers' ModifyMesh (+0x6c) hands back the surface or a seeded
replacement. GetPositionIn 0x004fc9a0 and the node's direction/point transforms (0x004fd710,
0x004fd7f0, 0x004fd5c0) write seeded or input-derived vectors.

Inputs: 1..3 surfaces of 0..12 vertices and 0..3 face groups (whose source vertices alias the
surface's or not), 1..3 materials per surface with mapping types 0..8 and out of range, a
light manager absent, empty, without a usable light or with type 1/2 lights, frame times and
scroll values that wrap the 1.0 and 100.0 limits, unit and zero direction vectors (positions
one unit from the camera or a light, for the normalisations' squared == 1 and == 0 shortcuts),
0..2 modifiers, the texture remap flag, the debug overlays, lighting, shading and the drawing
function's static switches and guard bits.

Mode 5 indexes its face group with the restore loop's leftover counter, which retail leaves
uninitialised before the first restore (the bring-up source starts it at 0). Cases that use
mode 5 therefore begin with a restoring mode 1 or 7 material, and every surface's group array
has room for any leftover count. The 32-vertex ring mode 5 draws ends with circle[16], one past
its array (stack garbage in both bodies), so that vertex's position is left out of the note.
"""
import math
import random
import struct

RETAIL_VA = 0x00440f30
SYMBOL = '?DrawCurrentLod@D3DIMSoultreeObject@@QAEXXZ'
SOURCE = 'src/bringup/Group5_D3DIMSoulTreeDraw.cpp'
FAST_TABLE_INITS = (0x00460ae0, 0x00460bb0)
TRANSFORM_GROUPS = 0x00440d40
APPLY_STATES = 0x004ff180
RESTORE_STATES = 0x004ff410
GET_POSITION_IN = 0x004fc9a0
WORLD_TO_LOCAL_DIR = 0x004fd710
WORLD_TO_LOCAL_POINT = 0x004fd7f0
LOCAL_TO_WORLD_DIR = 0x004fd5c0
UPDATE_WORLD = 0x004fb4f0
REMAP_TEXCOORDS = 0x00510910
DRAW_WIREFRAME, DRAW_NORMALS, DRAW_CROSSES = 0x00442f70, 0x00442fe0, 0x004431f0
ATEXIT = 0x00535430
GUARD = 0x0057ef24
EYE, VIEW_DIRECTION, VIEW_POSITION = 0x0057eef8, 0x0057ef38, 0x0057eee8
UPDATE_EYE, DRAW_REFLECTIONS, UPDATE_VIEW, DRAW_SPHERE, EXPONENT = (
    0x00568930, 0x00568934, 0x00568938, 0x0056893c, 0x00568940)
# The function-local statics are file-local symbols in the object; both bodies use retail's.
_SCOPE = '@??DrawCurrentLod@D3DIMSoultreeObject@@QAEXXZ@4'
BINDINGS = {'_?$S1@?CL' + _SCOPE + 'EA': GUARD,
            '_?eye@?CL' + _SCOPE + 'UUnknownSoultreeStaticVector@@A': EYE,
            '_?updateEye@?CL' + _SCOPE + 'HA': UPDATE_EYE,
            '_?drawReflections@?CL' + _SCOPE + 'HA': DRAW_REFLECTIONS,
            '_?viewDirection@?EK' + _SCOPE + 'UUnknownSoultreeStaticVector@@A': VIEW_DIRECTION,
            '_?viewPosition@?EK' + _SCOPE + 'UUnknownSoultreeStaticVector@@A': VIEW_POSITION,
            '_?updateView@?EK' + _SCOPE + 'HA': UPDATE_VIEW,
            '_?drawSphere@?EK' + _SCOPE + 'HA': DRAW_SPHERE,
            '_?exponent@?GO' + _SCOPE + 'MA': EXPONENT}
PROBES = {0x004410ae: 'mode 0', 0x004410bb: 'mode 7', 0x00441115: 'mode 7 x <= 0',
          0x00441141: 'mode 1', 0x00441166: 'mode 1 scroll wrapped',
          0x004411b8: 'mode 3', 0x004411f7: 'mode 3 light found', 0x004412d9: 'mode 3 unit direction',
          0x00441410: 'mode 4', 0x004414fd: 'mode 4 unit direction',
          0x00441636: 'mode 8', 0x00441716: 'mode 8 vertices', 0x00441805: 'mode 8 reflection drawn',
          0x00441a14: 'mode 2', 0x00441e1f: 'mode 5', 0x00442041: 'mode 5 vertices',
          0x00442798: 'mode 6', 0x004427d7: 'mode 6 light found', 0x00442889: 'mode 6 vertices',
          0x00442c13: 'modifiers', 0x00442c6f: 'texture remap', 0x00442cc0: 'wireframe',
          0x00442d19: 'restore loop'}


def _vec(rng, lo=-60.0, hi=60.0):
    return [rng.uniform(lo, hi) for _ in range(3)]


def _unit(rng):
    v = [rng.uniform(-1.0, 1.0) for _ in range(3)]
    k = 1.0 / math.sqrt(sum(c * c for c in v) or 1.0)
    return [c * k for c in v]


def _axis(rng):
    v = [0.0, 0.0, 0.0]
    v[rng.randrange(3)] = rng.choice([1.0, -1.0])
    return v


# The camera and light positions setup chose: GetPositionIn sometimes answers one unit away
# from one of them, so modes 3 and 4 take their squared == 1 shortcut.
_ANCHORS = []


def _f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def _get_position(ctx, rec):        # (frame, Vector3* out)
    r = rec.rng
    kind = r.random()
    if kind < 0.15:
        v = [0.0, 0.0, 0.0]
    elif kind < 0.3:
        v = [float(r.randrange(-3, 4)) for _ in range(3)]
    elif kind < 0.5 and _ANCHORS:
        v = list(r.choice(_ANCHORS))
        i = r.randrange(3)
        v[i] = _f32(v[i] + r.choice([1.0, -1.0]))
    else:
        v = _vec(r)
    ctx.pack(rec.args[1], '3f', *v)


def _transformed(ctx, rec):         # hidden return: (Vector3* result, const Vector3* v)
    r = rec.rng
    x, y, z = ctx.unpack(rec.args[1], '3f')
    choice = r.random()
    if choice < 0.4:
        out = [x, y, z]                       # identity node
    elif choice < 0.7:
        out = [-z, y, x]                      # a quarter turn about y
    else:
        out = [x * 0.5 + r.uniform(-3, 3), y + r.uniform(-3, 3), z * -1.5]
    ctx.pack(rec.args[0], '3f', *out)
    return rec.args[0]


def _transformed_unit(ctx, rec):    # the world-to-local direction of a unit vector
    r = rec.rng
    x, y, z = ctx.unpack(rec.args[1], '3f')
    out = [x, y, z] if r.random() < 0.5 else [-z, y, x]
    ctx.pack(rec.args[0], '3f', *out)
    return rec.args[0]


REPLAY = dict(
    returns={TRANSFORM_GROUPS: 'void', APPLY_STATES: 'void', RESTORE_STATES: 'void',
             GET_POSITION_IN: 'void', UPDATE_WORLD: 'void', REMAP_TEXCOORDS: 'void',
             DRAW_WIREFRAME: 'void', DRAW_NORMALS: 'void', DRAW_CROSSES: 'void', ATEXIT: 'int',
             WORLD_TO_LOCAL_DIR: _transformed_unit, WORLD_TO_LOCAL_POINT: _transformed,
             LOCAL_TO_WORLD_DIR: _transformed_unit},
    effects={GET_POSITION_IN: _get_position},
    pointees={WORLD_TO_LOCAL_DIR: [(1, 12)], WORLD_TO_LOCAL_POINT: [(1, 12)],
              LOCAL_TO_WORLD_DIR: [(1, 12)],
              REMAP_TEXCOORDS: [(0, 4), (1, 4), (2, 4)]},
    passthrough=(0x00460c00, 0x00460c70, 0x0040ae30, 0x00404e60, 0x004a10e0, 0x00534db0,
                 0x00535240),
    out_writes=False,
)


def cases(rng, n):
    for _ in range(n):
        yield rng.getrandbits(32)


def _vertex_bytes(ctx, address, count):
    return ctx.read(address, 32 * count).hex() if count else ''


def setup(ctx, seed):
    r = random.Random(seed)
    for init in FAST_TABLE_INITS:
        ctx.call(init)

    def draw_indexed(cx, rec):      # (type, fvf, vertices, vertexCount, indices, indexCount, flags)
        a = rec.args
        nv = a[3] if 0 <= a[3] <= 64 else 0
        ni = a[5] if 0 <= a[5] <= 64 else 0
        cx.note('DrawIndexedPrimitive', a[0], a[1], nv, ni, a[6], _vertex_bytes(cx, a[2], nv),
                cx.read(a[4], 2 * ni).hex() if ni else '')
        return 0

    def draw(cx, rec):              # (type, fvf, vertices, vertexCount, flags)
        a = rec.args
        nv = a[3] if 0 <= a[3] <= 64 else 0
        words = []
        for i in range(nv):
            v = a[2] + 32 * i
            # Only what the body stores: the line vertices leave specular/tu/tv unset, and the
            # ring's last vertex copies circle[16], one past the array.
            pos = '' if (nv == 32 and i == 31) else cx.read(v, 12).hex()
            words.append(pos + cx.read(v + 16, 4).hex())
        cx.note('DrawPrimitive', a[0], a[1], nv, a[4], ''.join(words))
        return 0

    def get_state(cx, rec):         # (state, int* out)
        cx.pack(rec.args[1], 'I', 0x100 + rec.args[0])
        return 0

    view_vt = ctx.fake_vtable('render target', 24,
                              pops={7: 12, 8: 12, 9: 8, 10: 8, 15: 28, 16: 20},
                              returns={7: 'int', 8: 'int', 9: get_state, 10: 'int',
                                       15: draw_indexed, 16: draw})
    camera_vt = ctx.fake_vtable('camera', 40, pops={30: 4}, returns={30: 'void'})

    camera = ctx.alloc(0x200)
    ctx.pack(camera, 'I', camera_vt)
    ctx.pack(camera + 0x170, '3f', *_vec(r, -100.0, 100.0))
    ctx.pack(camera + 0x17c, '3f', *_unit(r))
    view = ctx.alloc(0x80)
    ctx.pack(view, 'I', view_vt)
    ctx.pack(view + 8, 'I', camera)

    obj = ctx.alloc(0x300, bytes(r.getrandbits(8) for _ in range(0x300)))
    ctx.pack(obj + 0x24, 'I', view)
    ctx.pack(obj + 0x18c, 'I', int(r.random() < 0.3))                     # subtree dirty
    for off in (0x1a4, 0x1a8, 0x1ac, 0x1b0):                               # lighting, debug
        ctx.pack(obj + off, 'I', int(r.random() < 0.3))
    ctx.pack(obj + 0x1b4, 'I', r.choice([0, 1]))                           # perspective
    ctx.pack(obj + 0x1b8, 'I', r.choice([0, 1]))                           # gouraud
    ctx.pack(obj + 0xf8, '16f', *[r.uniform(-1.0, 1.0) for _ in range(12)],
             *_vec(r, -200.0, 200.0), 1.0)                                 # world matrix
    position = ctx.unpack(obj + 0x128, '3f')
    kind = r.random()
    if kind < 0.2:
        last = list(position)                                              # not moved
    elif kind < 0.35:
        last = list(position)
        last[r.randrange(3)] += r.choice([1.0, -1.0])                      # moved one unit
    else:
        last = [c + r.uniform(-30.0, 30.0) for c in position]
    ctx.pack(obj + 0x2bc, '3f', *last)
    ctx.pack(obj + 0x2b8, 'f', r.choice([0.0, 0.016, 0.05, r.uniform(0.0, 3.0)]))
    ctx.pack(obj + 0x2c8, 'f', r.choice([0.0, 0.5, 0.999, 2.5, 99.9, r.uniform(0.0, 100.0)]))

    # light manager: lightCount +0x2c, lights[] +0x30; a light's type +0x2c, position +0x64
    lights = 0
    if r.random() < 0.8:
        n = r.choice([0, 1, 2, 3])
        lights = ctx.alloc(0x30 + 4 * max(n, 1))
        ctx.pack(lights + 0x2c, 'i', n)
        for i in range(n):
            light = ctx.alloc(0x80)
            ctx.pack(light + 0x2c, 'i', r.choice([0, 1, 2, 3]))
            ctx.pack(light + 0x64, '3f', *_vec(r, -300.0, 300.0))
            ctx.pack(lights + 0x30 + 4 * i, 'I', light)
    ctx.pack(obj + 0x1bc, 'I', lights)
    _ANCHORS[:] = [ctx.unpack(camera + 0x170, '3f')]
    if lights:
        _ANCHORS.extend(ctx.unpack(ctx.unpack(lights + 0x30 + 4 * i, 'I')[0] + 0x64, '3f')
                        for i in range(ctx.unpack(lights + 0x2c, 'i')[0]))

    # the drawing function's statics
    ctx.pack(GUARD, 'B', r.randrange(8))
    for va in (EYE, VIEW_DIRECTION, VIEW_POSITION):
        ctx.pack(va, '3f', *_vec(r, -100.0, 100.0))
    for va in (UPDATE_EYE, DRAW_REFLECTIONS, UPDATE_VIEW, DRAW_SPHERE):
        ctx.pack(va, 'i', int(r.random() < 0.6))
    ctx.pack(EXPONENT, 'f', r.choice([20.0, 1.0, 0.5, 3.0]))

    # materials: surfaceMap +0x70 (texture at +8, flags at texture +0x68), speed +0xc4,
    # mapping type (word) +0xc8
    modes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    surfaces = r.choice([1, 1, 2, 3])
    plan = [[r.choice(modes) for _ in range(r.choice([1, 1, 2, 3]))] for _ in range(surfaces)]
    if any(5 in p for p in plan):
        plan[0].insert(0, r.choice([1, 7]))   # a restore before mode 5 reads the counter
    materials = []
    for p in plan:
        for mode in p:
            m = ctx.alloc(0x100, bytes(r.getrandbits(8) for _ in range(0x100)))
            surface_map = 0
            if r.random() < 0.5:
                texture = ctx.alloc(0x80)
                ctx.pack(texture + 0x68, 'I', r.choice([0, 1, 3]))
                surface_map = ctx.alloc(0x20)
                ctx.pack(surface_map + 8, 'I', texture)
            ctx.pack(m + 0x70, 'I', surface_map)
            ctx.pack(m + 0xc4, 'f', r.choice([0.0, 0.5, r.uniform(0.0, 40.0)]))
            ctx.pack(m + 0xc8, 'H', mode if mode < 9 else r.choice([9, 0x100, 0xffff]))
            materials.append(m)
    table = ctx.alloc(4 * len(materials))
    for i, m in enumerate(materials):
        ctx.pack(table + 4 * i, 'I', m)
    ctx.pack(obj + 0x290, 'I', table)

    eye = ctx.unpack(camera + 0x170, '3f')

    def make_surface(address, material_ids):
        nv = r.choice([0, 1, 3, 6, 12])
        vertices = ctx.alloc(32 * max(nv, 1))
        drawn = ctx.alloc(32 * max(nv, 1))
        uvs = ctx.alloc(8 * max(nv, 1))
        for i in range(nv):
            ctx.pack(vertices + 32 * i, '3f', *_vec(r))
            ctx.pack(vertices + 32 * i + 12, '3I', r.getrandbits(32), r.getrandbits(32),
                     r.getrandbits(32))
            ctx.pack(vertices + 32 * i + 24, '2f', r.uniform(0, 1), r.uniform(0, 1))
            p = _vec(r)
            if r.random() < 0.15:
                p[0] = 0.0
            ctx.pack(drawn + 32 * i, '3f', *p)
            ctx.pack(uvs + 8 * i, '2f', r.uniform(0, 1), r.uniform(0, 1))
        faces = r.choice([0, 1, 4])
        indices = ctx.alloc(6 * max(faces, 1))
        for i in range(3 * faces):
            ctx.pack(indices + 2 * i, 'H', r.randrange(max(nv, 1)))
        # face groups; room for 16 so mode 5's leftover index stays inside the array
        groups_n = r.choice([0, 1, 2, 3])
        groups = ctx.alloc(0x14 * 16)
        for g in range(16):
            gv = r.choice([0, 1, 2, 4, 6])
            node = ctx.alloc(0x200)
            ctx.pack(node + 0xf8, '16f', *[r.uniform(-1.0, 1.0) for _ in range(12)],
                     *_vec(r, -200.0, 200.0), 1.0)
            if nv >= gv and r.random() < 0.5:
                source = vertices + 32 * r.randrange(nv - gv + 1)        # alias the surface's
            else:
                source = ctx.alloc(32 * max(gv, 1))
                for i in range(gv):
                    ctx.pack(source + 32 * i, '3f', *_vec(r))
                    ctx.pack(source + 32 * i + 24, '2f', r.uniform(0, 1), r.uniform(0, 1))
            transformed = ctx.alloc(32 * max(gv, 1))
            normals = ctx.alloc(12 * max(gv, 1))
            for i in range(gv):
                t = _vec(r)
                if r.random() < 0.1:
                    t = [eye[0], eye[1], eye[2]]
                    t[r.randrange(3)] += 1.0
                ctx.pack(transformed + 32 * i, '3f', *t)
                n = r.random()
                ctx.pack(normals + 12 * i, '3f', *(_axis(r) if n < 0.3 else
                                                   [0.0, 0.0, 0.0] if n < 0.35 else _unit(r)))
            ctx.pack(groups + 0x14 * g, '5I', node, gv, source, transformed, normals)
        ctx.pack(address, '6I', groups_n, groups, nv, faces, vertices, drawn)
        ctx.pack(address + 0x18, '2I', ctx.alloc(12 * max(nv, 1)), indices)
        ids = ctx.alloc(4 * len(material_ids))
        for i, mid in enumerate(material_ids):
            ctx.pack(ids + 4 * i, 'I', mid)
        ctx.pack(address + 0x20, '3I', len(material_ids), ids, uvs)
        ctx.pack(address + 0x2c, '3f', r.uniform(0, 2), r.uniform(-1, 1), r.uniform(-1, 1))

    # level-of-detail table: 8-byte entries (surfaceCount, surfaces), surface 0x38 bytes
    lods = r.choice([1, 2])
    lod_table = ctx.alloc(8 * lods)
    current = r.randrange(lods)
    next_id = 0
    for lod in range(lods):
        if lod != current:
            ctx.pack(lod_table + 8 * lod, 'iI', 0, 0)
            continue
        array = ctx.alloc(0x38 * len(plan))
        for s, p in enumerate(plan):
            make_surface(array + 0x38 * s, list(range(next_id, next_id + len(p))))
            next_id += len(p)
        ctx.pack(lod_table + 8 * lod, 'iI', len(plan), array)
    ctx.pack(obj + 0x27c, 'i', current)
    ctx.pack(obj + 0x28c, 'I', lod_table if r.random() < 0.97 else 0)

    # modifiers: ModifyMesh (+0x6c) writes the mesh to draw through its third argument
    def modify(cx, rec):            # (object, surface, surface** drawn)
        if rec.rng.random() < 0.7:
            cx.pack(rec.args[2], 'I', rec.args[1])
        else:
            replacement = cx.alloc(0x38)
            nv = rec.rng.choice([0, 2])
            vertices = cx.alloc(32 * 2)
            for i in range(2):
                cx.pack(vertices + 32 * i, '8f', *[rec.rng.uniform(-5, 5) for _ in range(8)])
            indices = cx.alloc(6)
            cx.pack(replacement, '6I', 0, 0, nv, 1, vertices, vertices)
            cx.pack(replacement + 0x1c, 'I', indices)
            cx.pack(rec.args[2], 'I', replacement)
        return 0

    modifier_vt = ctx.fake_vtable('modifier', 32, pops={27: 12}, returns={27: modify})
    modifiers = r.choice([0, 0, 0, 1, 2])
    mlist = ctx.alloc(4 * max(modifiers, 1))
    for i in range(modifiers):
        mod = ctx.alloc(0x20)
        ctx.pack(mod, 'I', modifier_vt)
        ctx.pack(mlist + 4 * i, 'I', mod)
    ctx.pack(obj + 0x264, 'i', modifiers)
    ctx.pack(obj + 0x268, 'I', mlist)
    return dict(ecx=obj, args=[])
