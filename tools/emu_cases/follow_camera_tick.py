"""FollowCamera::Tick 0x00465c20, the follow camera's per-frame update (tools/emu_diff.py case).

The candidate is samples/camera/FollowCameraTick.cpp. The camera block is random floats with
the flags and states set from the case; offsets from src/reconstructed/FollowCamera.h, checked
against the retail body: renderContext +0x18 (currentCamera +8), fieldOfView +0x16c, eye
+0x170, min/max field of view +0x1dc/+0x1e0, FollowCamera fields +0x220..+0x304 (cameraState
+0x244, savedCameraState +0x248, resumeState +0x250, targetFieldOfView +0x258, overrideActive
+0x268, the bytes +0x274..+0x279, the eases +0x27c..+0x294), g_TrackGame (0x0056e26c) paused
+0x1c4 and controlInterface +0x14 (slot 3 IsDeviceButtonDown, four arguments, retail pushes
0xc7/0xcf).

Every FollowCamera virtual the tick calls is a logged stub in a scripted vtable (slot numbers
from the header; ABI from the retail call sites): 29 LookAt (Vector3 by value), 33/41/48/49/57
return a random Vector3 through the hidden pointer, 36/45/55/56/74/75 return random flags, 51
GetRoll returns a random float in ST0, 43/44/52 log the vector they are handed (52 also moves
its y), 67 ApplyReverseViewPreset writes a new targetFieldOfView. Camera::SetFrame 0x0042e9b0,
SetFieldOfView 0x0042e930 and Camera::Tick 0x0042e690 are stubs that log their values;
D3DRMVectorRotate (thunk 0x0053304c) and Terrain::QueryGround 0x00507c10 are stubs that write
random results; operator new 0x004a3010 hands out fresh 20-byte blocks. FastSqrt 0x00460b50
and the inline ease helpers run as code, so the compared state is the camera fields, the eased
values and the logged calls.
"""
import random
import struct

RETAIL_VA = 0x00465c20
SYMBOL = '?Tick@FollowCamera@@UAEHM@Z'
SOURCE = 'samples/camera/FollowCameraTick.cpp'
TRACK_GAME = 0x0056e26c
FLT_MAX = 3.4028234663852886e38
PROBES = {0x00465c69: 'place', 0x00465f3b: 'start blend', 0x0046607f: 'allocate eases',
          0x0046639e: 'settle', 0x00466527: 'blend', 0x004665d3: 'blend done',
          0x0046676a: 'enter state 7', 0x004667d4: 'leave state 7', 0x004668c4: 'reverse view',
          0x0046695a: 'zoom keys', 0x00466933: 'current'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), state=rng.choice([0, 1, 2, 3, 4, 5, 5, 6, 7, 7]),
                   saved=rng.choice([0, 3, 6, 6]), resume=rng.choice([0, 2, 7, 7]),
                   placed=rng.random() < 0.8, crash=rng.randrange(2), wide=rng.randrange(2),
                   blending=rng.randrange(2), settled=rng.randrange(2), override=rng.random() < 0.3,
                   eases=rng.random() < 0.8, current=rng.random() < 0.8, paused=rng.random() < 0.2,
                   yaw=rng.choice([0.5, -1.0, 2.5, -3.0, rng.uniform(-4, 4)]),
                   clock=rng.choice([0.0, 0.5, 0.99, rng.uniform(0, 1.2)]),
                   dt=rng.choice([0.0, 0.016, 0.05, 0.3, rng.uniform(0, 1)]))


def vec(cx, s=100.0):
    return [cx.rng.uniform(-s, s) for _ in range(3)]


def setup(ctx, c):
    r = random.Random(c['seed'])
    cam = ctx.alloc(0x400)
    ctx.write(cam, struct.pack('<256f', *[r.uniform(-50, 50) for _ in range(256)]))
    ctx.pack(cam + 0x16c, 'f', r.uniform(10, 70))
    ctx.pack(cam + 0x1dc, 'ff', 10.0, 70.0)
    ctx.pack(cam + 0x220, 'f', r.uniform(5, 40))
    ctx.pack(cam + 0x234, 'f', c['yaw'])
    ctx.pack(cam + 0x244, 'ii', c['state'], c['saved'])
    ctx.pack(cam + 0x24c, 'f', r.uniform(10, 70))
    ctx.pack(cam + 0x250, 'i', c['resume'])
    ctx.pack(cam + 0x258, 'f', r.uniform(5, 75))
    ctx.pack(cam + 0x268, 'ii', int(c['override']), 0)
    ctx.pack(cam + 0x274, '6B', c['settled'], int(c['placed']), c['crash'], 0, c['wide'], c['blending'])
    ctx.pack(cam + 0x2f4, '4f', 10.0, 280.0, 70.0, 20.0)
    ctx.pack(cam + 0x304, 'f', c['clock'])

    def value():
        p = ctx.alloc(0x14)
        ctx.pack(p, '5f', r.uniform(-50, 50), r.choice([0.3, r.uniform(0.01, 1)]), 1.0, FLT_MAX, -FLT_MAX)
        return p
    eases = [value() for _ in range(6)] if c['eases'] else [0] * 6
    ctx.pack(cam + 0x27c, '8I', *(eases + [value(), value()]))

    context = ctx.alloc(0x10)
    ctx.pack(context + 8, 'I', cam if c['current'] else context)
    ctx.pack(cam + 0x18, 'I', context)

    game = ctx.alloc(0x200)
    ctx.pack(game + 0x1c4, 'I', int(c['paused']))
    control = ctx.alloc(0x10)
    ctx.pack(control, 'I', ctx.vtable([0, 0, 0, ctx.stub('ControlInterface::IsDeviceButtonDown', pop=16,
                                                       ret=lambda cx, e, a: cx.rng.randrange(2))]))
    ctx.pack(game + 0x14, 'I', control)
    ctx.pack(TRACK_GAME, 'I', game)

    def note_vec(cx, p):
        cx.note(*cx.unpack(p, '3f'))

    def ret_vec(cx, e, a):
        cx.pack(a[0], '3f', *vec(cx))
        return a[0]

    def noted(cx, e, a):
        note_vec(cx, a[0])

    def clamp(cx, e, a):
        note_vec(cx, a[0])
        cx.pack(a[0] + 4, 'f', cx.rng.uniform(-50, 50))

    def flag(cx, e, a):
        return cx.rng.randrange(2)

    def settle(cx, e, a):
        note_vec(cx, a[0])
        return cx.rng.randrange(2)

    def position(cx, e, a):
        note_vec(cx, a[2])
        return ret_vec(cx, e, a)

    def reverse(cx, e, a):
        cx.pack(e + 0x258, 'f', cx.rng.uniform(5, 75))

    slots = {29: ('LookAt', 12, None, None), 33: ('GetTargetVelocity', 4, ret_vec, None),
             36: ('TrySettleOnPoint', 12, settle, None), 41: ('ComputeFollowHeading', 4, ret_vec, None),
             42: ('UpdateTurnYaw', 4, None, None), 43: ('EaseTargetToward', 4, noted, None),
             44: ('EasePositionToward', 4, noted, None), 45: ('ProcessCameraInput', 8, flag, None),
             48: ('SelectFollowPoint', 16, ret_vec, None), 49: ('ComputeCameraPosition', 16, position, None),
             51: ('GetRoll', 0, lambda cx, e, a: (0, cx.rng.uniform(-1, 1)), 'f'),
             52: ('ClampPositionAboveGround', 4, clamp, None), 53: ('HideSubject', 0, None, None),
             54: ('ShowSubject', 0, None, None), 55: ('UnknownVirtualSlot55', 0, flag, None),
             56: ('UnknownVirtualSlot56', 0, flag, None), 57: ('GetLeadTargetPoint', 8, ret_vec, None),
             59: ('SaveCameraPreset', 0, None, None), 67: ('ApplyReverseViewPreset', 0, reverse, None),
             68: ('LookAtTargetPoint', 0, None, None), 74: ('IsSubjectCrashed', 0, flag, None),
             75: ('ShouldFollowSubject', 0, flag, None)}
    table = []
    for i in range(80):
        if i in slots:
            name, pop, ret, rf = slots[i]
            table.append(ctx.stub('FollowCamera::' + name, pop=pop, ret=ret or 0, ret_float=rf))
        else:
            table.append(0)
    ctx.pack(cam, 'I', ctx.vtable(table))

    def set_frame(cx, e, a):
        note_vec(cx, a[0])
        if a[3]:
            cx.note(*cx.unpack(a[3], 'f'))
    ctx.stub_at(0x0042e9b0, 'Camera::SetFrame', pop=20, ret=set_frame)
    ctx.stub_at(0x0042e930, 'Camera::SetFieldOfView', pop=4)
    ctx.stub_at(0x0042e690, 'Camera::Tick', pop=4, ret=1)

    # The axis is a file-scope Vector3 (0x0065b458/0x0065b468 in retail, the object's own copy
    # in a standalone candidate), so its contents are compared rather than its address.
    def rotate(cx, e, a):
        cx.note('D3DRMVectorRotate', *(cx.unpack(a[1], '3f') + cx.unpack(a[2], '3f')), a[3])
        cx.pack(a[0], '3f', *vec(cx, 1.0))
    ctx.stub_at(0x0053304c, 'D3DRMVectorRotate', pop=16, ret=rotate, log=False)

    def ground(cx, e, a):
        note_vec(cx, a[0])
        cx.pack(a[0] + 4, 'f', cx.rng.uniform(-20, 20))
    ctx.stub_at(0x00507c10, 'Terrain::QueryGround', pop=16, ret=ground)
    ctx.pack(cam + 0x240, 'I', ctx.alloc(0x40))
    # The __FILE__ tag is not compared: retail's is FollowCam.cpp, the split sample's its own
    # name (TU ownership is unproven); the size and line are.
    def new(cx, e, a):
        cx.note('operator new', a[0], a[2])
        return cx.alloc(0x14, fill=0xcd)
    ctx.stub_at(0x004a3010, 'operator new', pop=0, nargs=3, ret=new, log=False)
    return dict(ecx=cam, args=[c['dt']], ret_eax=True)
