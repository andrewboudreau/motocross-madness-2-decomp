"""ParticleManager::Render3D, ParticleManager slot 14 (tools/emu_diff.py case).

The function's two float-to-int conversions are the per-vertex fog level
(`fog * 255`, retail 0x004bb271) and the fade alpha (`(1 - age / lifetime) * 127.5 - 0.5`,
0x004bb2ab); only the low byte of either survives (`<< 24`). The case drives the whole
body: the three vertex buffers (IDirect3DVertexBuffer7 Lock/Unlock/ProcessVertices), the
cameras, the texture, the render target and LightManager::LightVertices are stubs, and
FastInvSqrt is a float-exact stub. ProcessVertices fills the transformed buffer with per-case
screen positions (inside, outside and straddling the target, z in and out of [0, 1]), so
the visibility, depth sort (retail's qsort and CompareParticleDepth run for both bodies),
clipping and quad output are all compared. LightVertices records the light frame it is
handed and writes random corner colours (high bytes set, to check the rgb mask).

Fog cases come in two kinds: random screen matrices, and a depth-only matrix with positions
solved so that `fog * 255` lands on or next to a .5 fraction. Fade cases use lifetimes and
ages that put the fade on or next to .5 fractions too.
"""
import math
import struct

from tools.emu_diff import _frame_token

RETAIL_VA = 0x004bac60
SYMBOL = '?Render3D@ParticleManager@@UAEHXZ'
SOURCE = 'src/bringup/Group1_ParticleRender.cpp'

COMPARE_DEPTH = 0x004bac20
BINDINGS = {
    '?CompareParticleDepth@@YAHPBX0@Z': COMPARE_DEPTH,
    # The function static and its guard byte; the case sets the guard so neither body
    # rebuilds the identity matrix (IdentityMatrix/atexit stay out of the comparison).
    '_?s_particleWorld@?3??Render3D@ParticleManager@@UAEHXZ@4UMatrix4@@A': 0x006898f8,
    '_?$S1@?3??Render3D@ParticleManager@@UAEHXZ@4EA': 0x00689154,
}
PROBES = {0x004bb271: 'fog fistp', 0x004bb2ab: 'fade fistp'}

TRACK_GAME = 0x0056e26c
CELL_UVS = 0x00689158
GUARD = 0x00689154
FAST_INV_SQRT = 0x00460c00
LIGHT_VERTICES = 0x0049e4a0


def _f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def _near(rng, x):
    """x as float32, or one of its float32 neighbours."""
    bits = struct.unpack('<I', struct.pack('<f', x))[0] + rng.choice([-2, -1, 0, 0, 1, 2])
    return struct.unpack('<f', struct.pack('<I', bits & 0xffffffff))[0]


def _particle(rng, width, height, depth_only, near, far):
    p = {}
    # Screen position from ProcessVertices.
    p['sx'] = _f32(rng.choice([rng.uniform(0, width), rng.uniform(-60, width + 60),
                               rng.choice([-5.0, 0.0, float(width), float(width) + 3])]))
    p['sy'] = _f32(rng.choice([rng.uniform(0, height), rng.uniform(-60, height + 60),
                               rng.choice([-5.0, 0.0, float(height), float(height) + 3])]))
    p['sz'] = _f32(rng.choice([rng.random(), rng.random(), rng.random(), 0.0, 1.0, -0.01, 1.01,
                               0.5]))
    p['rhw'] = _f32(rng.choice([rng.uniform(0.001, 1.0), 1.0, rng.uniform(0.01, 0.2)]))
    p['size'] = _f32(rng.choice([rng.uniform(0.1, 3.0), 0.5, 1.0]))
    if depth_only:
        # fog * 255 == k + 0.5 (or close): fog = 1 - (z - near) / (far - near).
        k = rng.randrange(0, 255)
        fog = (k + 0.5) / 255.0
        if rng.random() < 0.2:
            fog = rng.choice([0.0, 1.0, -0.1, 1.1, rng.random()])
        z = near + (1.0 - fog) * (far - near)
        p['pos'] = [_f32(rng.uniform(-50, 50)), _f32(rng.uniform(-50, 50)), _near(rng, z)]
    else:
        p['pos'] = [_f32(rng.uniform(-200, 200)) for _ in range(3)]
    # Fade = (1 - age / lifetime) * 127.5 - 0.5: ages of m / 255 lifetimes give halves.
    life = rng.choice([1.0, 2.0, 0.5, 255.0, 127.5, 3.0, rng.uniform(0.1, 10.0)])
    m = rng.randrange(0, 256)
    age = rng.choice([life * m / 255.0, rng.uniform(0, life), 0.0, life])
    p['life'] = _f32(life)
    p['age'] = _near(rng, age) if age else 0.0
    p['flags'] = rng.choice([0, 1, 2, 3, 2, 2, 0x800, 0x802, 0x803, 0x801])
    p['color'] = rng.randrange(1 << 32)
    p['frame'] = rng.randrange(61)
    return p


def cases(rng, n):
    for i in range(n):
        width = rng.choice([640, 800, 320, 512])
        height = rng.choice([480, 600, 240, 384])
        near = _f32(rng.choice([1.0, 0.5, rng.uniform(0.1, 10.0)]))
        far = _f32(near + rng.choice([1000.0, 500.0, rng.uniform(1.0, 5000.0)]))
        depth_only = rng.random() < 0.6
        count = rng.choice([1, 1, 2, 3, 5, 8, 16])
        yield dict(
            width=width, height=height, near=near, far=far, depth_only=depth_only,
            matrix=[_f32(rng.uniform(-2, 2)) for _ in range(16)],
            focal=_f32(rng.choice([rng.uniform(100, 1000), 400.0])),
            caps=rng.choice([0, 0x100, 0x180, 0x80, 0x100]),
            fmt=rng.choice([0x115c, 0x1555, 0x565]),
            hidden=rng.random() < 0.03,
            eye=[_f32(rng.uniform(-500, 500)) for _ in range(3)],
            view=[_f32(rng.uniform(-1, 1)) for _ in range(3)],
            up=rng.choice([[0.0, 1.0, 0.0], [_f32(rng.uniform(-1, 1)) for _ in range(3)]]),
            particles=[_particle(rng, width, height, depth_only, near, far) for _ in range(count)],
        )


def setup(ctx, case):
    rng = ctx.rng
    ps = case['particles']
    count = len(ps)
    ctx.pack(GUARD, 'B', 1)
    ctx.write(CELL_UVS, struct.pack('<%df' % (61 * 8), *[rng.uniform(0, 1) for _ in range(61 * 8)]))

    # Cameras.
    def camera_block(eye, view, up):
        cam = ctx.alloc(0x220)
        if case['depth_only']:
            m = [0.0] * 16
            m[10] = 1.0
        else:
            m = case['matrix']
        ctx.pack(cam + 0xec, '16f', *m)
        ctx.pack(cam + 0x170, '9f', *(eye + view + up))
        ctx.pack(cam + 0x198, 'f', case['focal'])
        ctx.pack(cam + 0x1bc, 'ffii', case['near'], case['far'], case['width'], case['height'])
        return cam
    set_world = ctx.stub('Camera::SetWorldMatrix', pop=4)
    cam_vt = ctx.vtable([set_world] * 40)
    camera = camera_block(case['eye'], case['view'], case['up'])
    ctx.pack(camera, 'I', cam_vt)
    scene_camera = camera_block([_f32(rng.uniform(-9, 9)) for _ in range(3)],
                                case['view'], case['up'])
    ctx.pack(scene_camera, 'I', cam_vt)
    # The scene camera carries the case's eye/view/up (the light frame reads it).
    ctx.pack(scene_camera + 0x170, '9f', *(case['eye'] + case['view'] + case['up']))

    scene_target = ctx.alloc(0x200)
    ctx.pack(scene_target + 8, 'I', scene_camera)
    game = ctx.alloc(0x300)
    ctx.pack(game + 0x10, 'I', scene_target)
    ctx.pack(game + 0x2d0, 'i', 1 if case['hidden'] else 0)
    ctx.pack(TRACK_GAME, 'I', game)

    # Render target (PCRenderTarget view): slots 8 SetRenderState, 10 SetTextureBlend,
    # 17 DrawIndexedPrimitiveVB.
    rt_slots = [ctx.stub('RenderTarget::slot%d' % k, pop=0) for k in range(24)]
    rt_slots[8] = ctx.stub('RenderTarget::SetRenderState', pop=0xc)
    rt_slots[10] = ctx.stub('RenderTarget::SetTextureBlend', pop=8)
    rt_slots[17] = ctx.stub('RenderTarget::DrawIndexedPrimitiveVB', pop=0x1c)
    target = ctx.alloc(0x200)
    ctx.pack(target, 'I', ctx.vtable(rt_slots))
    ctx.pack(target + 8, 'I', camera)
    ctx.pack(target + 0x50, 'I', 0xd3d0d3d0)
    ctx.pack(target + 0x1a8, 'I', case['caps'])

    # Vertex buffers.
    source_data = ctx.alloc(0x20 * count + 0x20)
    screen_data = ctx.alloc(0x20 * count + 0x20)
    quad_data = ctx.alloc(0x80 * count + 0x80)
    buffers = {}

    def lock(ctx_, ecx, a):
        ctx_.pack(a[2], 'I', buffers[a[0]])
        return 0

    def process(ctx_, ecx, a):
        for i, p in enumerate(ps):
            ctx_.pack(screen_data + 0x20 * i, '4f', p['sx'], p['sy'], p['sz'], p['rhw'])
        return 0
    def com(name, pop, ret=0):
        # stdcall COM methods: ECX is scratch, so log the stack arguments only.
        def call(ctx_, ecx, a):
            ctx_.calls.append((name, None, tuple(_frame_token(v) for v in a)))
            return ret(ctx_, ecx, a) if callable(ret) else ret
        return ctx.stub(name, pop=pop, ret=call, log=False)
    vb_slots = [com('VB::slot%d' % k, 4) for k in range(8)]
    vb_slots[3] = com('VB::Lock', 0x10, lock)
    vb_slots[4] = com('VB::Unlock', 4)
    vb_slots[5] = com('VB::ProcessVertices', 0x20, process)
    vb_vt = ctx.vtable(vb_slots)
    vbs = []
    for data in (screen_data, source_data, quad_data):
        vb = ctx.alloc(0x10)
        ctx.pack(vb, 'I', vb_vt)
        buffers[vb] = data
        vbs.append(vb)

    texture = ctx.alloc(0x40)
    ctx.pack(texture, 'I', ctx.vtable([ctx.stub('Texture::slot%d' % k) for k in range(24)]))
    ctx.pack(texture + 0x20, 'I', case['fmt'])

    def inv_sqrt(ctx_, ecx, a):
        # cdecl: ECX is scratch, so only the argument is logged.
        ctx_.calls.append(('FastInvSqrt', None, (a[0],)))
        x = struct.unpack('<f', struct.pack('<I', a[0]))[0]
        return 0, (1.0 / math.sqrt(x) if x > 0 else 0.0)
    ctx.stub_at(FAST_INV_SQRT, 'FastInvSqrt', pop=0, nargs=1, ret=inv_sqrt, ret_float='f',
                log=False)

    def light(ctx_, ecx, a):
        ctx_.note('light frame', ctx_.read(a[0], 0x40).hex(), a[1:])
        for i in range(4):
            ctx_.pack(a[3] + 0x20 * i + 0x10, 'I', rng.randrange(1 << 32))
        return 0
    ctx.stub_at(LIGHT_VERTICES, 'LightManager::LightVertices', pop=0x18, ret=light)

    mgr = ctx.alloc(0x2060)
    ctx.pack(mgr + 0x18, 'I', target)
    ctx.pack(mgr + 0x2c, 'ii', count, rng.randrange(100))
    for i, p in enumerate(ps):
        part = ctx.alloc(0x50)
        ctx.pack(part, '3fIi', *p['pos'], p['color'], p['frame'])
        ctx.pack(part + 0x14, 'fff', p['age'], p['life'], p['size'])
        ctx.pack(part + 0x34, 'I', p['flags'])
        ctx.pack(mgr + 0x40 + 4 * i, 'I', part)
    ctx.pack(mgr + 0x1f80, 'IIII', texture, *vbs)
    ctx.pack(mgr + 0x1f94, 'I', ctx.alloc(0x10))
    ctx.write(mgr + 0x1fa4, bytes(rng.randrange(256) for _ in range(0x80)))
    ctx.pack(mgr + 0x2054, 'I', ctx.alloc(0x10))
    return dict(ecx=mgr, args=())
