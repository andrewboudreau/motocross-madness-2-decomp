"""KrustyBikeCamera::Tick 0x00497e20, slot 10 of the bike camera (tools/emu_diff.py case).

The linked body is the bring-up copy (src/bringup/Group2_KrustyBikeCameraTick.cpp). Offsets from
the retail body: raceView +0x3b8 (set on first use from g_TrackGame (0x0056e26c) +0x2d74 event
type through the jump table to +0x55c/+0x560/+0x564/+0x558/+0x568/+0x560, each event's view at
+0x34; event type 7 leaves it unset only when one is already present), the view's
networkStartPending byte +0x18e, its localRacer +0x38 (position y +0x10), cameraState +0x244.

The unit's file statics are bound to retail's: the 'network start view active' byte
0x0067c3f4 and the up axis 0x0067c3e8, given (0, 1, 0) as its initializer leaves it. The byte
is set for half the cases by a warm-up call of the body under test (ctx.call, stubs silent)
with the start pending, so both the enter and the leave paths run. Stubbed and logged:
BikeRace::WriteRelativePositions 0x004210f0 (writes two random vectors), FollowCamera's
BuildOrientationBasis 0x004654e0 and PlayRecordedFrame 0x00465720 (log the vectors they get),
FollowCamera::Tick 0x00465c20 and the virtual RestoreCameraState (slot 62) and SetCameraState
(slot 71).
"""
import random

RETAIL_VA = 0x00497e20
SYMBOL = '?Tick@KrustyBikeCamera@@UAEHM@Z'
SOURCE = 'src/bringup/Group2_KrustyBikeCameraTick.cpp'
TRACK_GAME = 0x0056e26c
BINDINGS = {'_s_networkStartViewActive': 0x0067c3f4, '_s_upAxis': 0x0067c3e8}
EVENTS = {0: 0x55c, 1: 0x560, 2: 0x564, 3: 0x558, 4: 0x568, 5: 0x560}
PROBES = {0x00497e41: 'pick view', 0x00497eaf: 'enter start view', 0x00497eeb: 'start view',
          0x00497f46: 'leave start view', 0x00497f68: 'follow tick'}


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30), has_view=rng.random() < 0.5,
                   event=rng.choice([0, 1, 2, 3, 4, 5]), pending=rng.randrange(2),
                   active=rng.randrange(2), state=rng.randrange(8), dt=rng.uniform(0, 0.2))


def setup(ctx, c):
    r = random.Random(c['seed'])
    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x2d74, 'i', c['event'])
    views = {}
    for off in sorted(set(EVENTS.values())):
        event = ctx.alloc(0x40)
        view = ctx.alloc(0x200)
        racer = ctx.alloc(0x20)
        ctx.pack(racer + 0x10, 'f', r.uniform(-100, 100))
        ctx.pack(view + 0x38, 'I', racer)
        ctx.pack(event + 0x34, 'I', view)
        ctx.pack(game + off, 'I', event)
        views[off] = view
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(0x0067c3e8, '3f', 0.0, 1.0, 0.0)

    def ret_vecs(cx, e, a):
        cx.pack(a[0], '3f', *[cx.rng.uniform(-100, 100) for _ in range(3)])
        cx.pack(a[1], '3f', *[cx.rng.uniform(-100, 100) for _ in range(3)])
    ctx.stub_at(0x004210f0, 'BikeRace::WriteRelativePositions', pop=16, ret=ret_vecs)

    def basis(cx, e, a):
        cx.note('BuildOrientationBasis', *(cx.unpack(a[0], '3f') + cx.unpack(a[1], '3f')))
    ctx.stub_at(0x004654e0, 'FollowCamera::BuildOrientationBasis', pop=8, ret=basis, log=False)

    def play(cx, e, a):
        cx.note(a[0], *cx.unpack(a[1], '3f'))
        return 1
    ctx.stub_at(0x00465720, 'FollowCamera::PlayRecordedFrame', pop=8, ret=play)
    ctx.stub_at(0x00465c20, 'FollowCamera::Tick', pop=4, ret=1)

    table = [0] * 80
    table[62] = ctx.stub('FollowCamera::RestoreCameraState', pop=0)
    table[71] = ctx.stub('FollowCamera::SetCameraState', pop=4)
    cam = ctx.alloc(0x400, fill=0x5a)
    ctx.pack(cam, 'I', ctx.vtable(table))
    ctx.pack(cam + 0x244, 'i', c['state'])
    own = ctx.alloc(0x200)
    racer = ctx.alloc(0x20)
    ctx.pack(racer + 0x10, 'f', r.uniform(-100, 100))
    ctx.pack(own + 0x38, 'I', racer)
    view = own if c['has_view'] else views[EVENTS[c['event']]]
    if c['active']:
        ctx.pack(cam + 0x3b8, 'I', view)
        ctx.pack(view + 0x18e, 'B', 1)
        ctx.call(ctx.entry, [c['dt']], ecx=cam)
    ctx.pack(cam + 0x3b8, 'I', own if c['has_view'] else 0)
    ctx.pack(view + 0x18e, 'B', c['pending'])
    return dict(ecx=cam, args=[c['dt']], ret_eax=True)
