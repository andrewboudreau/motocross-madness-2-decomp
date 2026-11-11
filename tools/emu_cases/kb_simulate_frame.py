"""KrustyBike::SimulateFrame 0x00496790 (vtable slot 49; tools/emu_diff.py case).

The linked body is canonical src/krusty2/vehicle/KrustyBike.cpp. Offsets from the retail
body: +0x0c position, +0x3bc model node, +0x5c4 rider character (+0x1a0 its node, +0x10
cleared on a win), +0x740 race (+0x38 local racer, +0x50 race camera with +0x244 state and
+0x3b0 followed bike, +0xb8 forceAiStep, +0x189 startBeforeFinish, +0x18a isRacing),
+0x754 totalRunningTime, +0x7a0/+0x7a2 lap count and previous, +0x7a4 hasFinished,
+0x784 finish position (jump table 1..11), +0x444 crash state, +0x734 aiKind, +0x6a8 rider
pose handle 15, +0x431 motionCompleted, +0x1540 last frame position, +0x153c/+0x153e trick
pose held and previous, +0x154c drawnAtLastPosition. g_TrackGame 0x0056e26c: +0x18 player
count, +0x570 event manager, +0x2d70 race mode, +0x2d74 event type.

The function sets up an SEH frame (fs:[0]); the case points FS at a heap block whose head is
restored on return. Stubbed: SetPosition 0x004fc660 (notes the vector), FindTextQueue
0x0045d340 (sink or null), LoadResourceString 0x00521970 (writes a text naming the id),
operator new(size, file, line) 0x004a3010 (notes size and line; null in some runs), the
message constructor 0x0051b200 (notes the text and the duration), ReplaceHeadMessage
0x0051b540, operator delete 0x004a30c0, SetMotion 0x004a8b40, the AI step 0x00413200,
Bike::SimulateFrame 0x0040ca20 and EndMotion (vtable +0xa4). Each case is one frame with
random flags; the first-frame, lap-1 and finish (all positions, one and two players)
messages, the AI/Bike choice and the camera pin all run.
"""
import struct

RETAIL_VA = 0x00496790
SYMBOL = '?SimulateFrame@KrustyBike@@UAEXM@Z'
SOURCE = 'src/krusty2/vehicle/KrustyBike.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x00496814: 'sink', 0x0049689c: 'first message', 0x0049693f: 'lap message',
          0x00496981: 'finished', 0x00496aa7: 'position message', 0x00496b0d: 'multiplayer message',
          0x00496b8c: 'win motion', 0x00496c27: 'ai step', 0x00496c38: 'bike step', 0x00496c5a: 'pin'}


def cases(rng, n):
    for _ in range(n):
        yield dict(drawn=rng.randrange(2), local=rng.random() < 0.8, mode=rng.choice([4, 4, 4, 1]),
                   sink=rng.random() < 0.85, time=rng.choice([0.0, 0.0, -0.0, 1.5]),
                   laps=rng.choice([(1, 0), (1, 1), (2, 1), (0, 0)]), before=rng.randrange(2),
                   new_ok=rng.random() < 0.85, finished=rng.choice([0, 1, 1, 1, 2, 3]),
                   players=rng.choice([1, 1, 2]), position=rng.choice(list(range(0, 13)) + [1, 1]),
                   event=rng.choice([0, 0, 2]), crash=rng.choice([0, 0, 1]), ai=rng.randrange(2),
                   force=rng.choice([0, 0, 1]), racing=rng.randrange(2), cam=rng.choice([3, 3, 2]),
                   followed=rng.random() < 0.7, dt=rng.choice([1 / 60.0, 1 / 30.0]), seed=rng.randrange(1 << 30))


def install_seh_segment(ctx):
    """Point FS at a heap block whose head (the SEH chain, -1) VC6's EH prologue saves and
    its epilogue restores. 32-bit unicorn ignores FS_BASE, so FS is loaded from a GDT entry
    (present ring-0 read/write data, byte granular, limit 0xfff); entry 2 is a flat 4 GiB
    data segment for SS, which unicorn re-checks once a GDT is set."""
    from unicorn import x86_const
    tib = ctx.alloc(0x40)
    ctx.pack(tib, 'I', 0xffffffff)
    gdt = ctx.alloc(0x18)
    descriptor = (0xfff | (tib & 0xffffff) << 16 | 0x93 << 40 | 0x4 << 52 | (tib >> 24) << 56)
    flat = 0xffff | 0x93 << 40 | 0xf << 48 | 0xc << 52
    ctx.pack(gdt + 8, 'QQ', descriptor, flat)
    ctx.uc.reg_write(x86_const.UC_X86_REG_GDTR, (0, gdt, 0x17, 0))
    ctx.uc.reg_write(x86_const.UC_X86_REG_FS, 8)
    ctx.uc.reg_write(x86_const.UC_X86_REG_SS, 0x10)


def setup(ctx, c):
    install_seh_segment(ctx)

    def note_vec(cx, ecx, a):
        cx.note('SetPosition', ecx, cx.read(a[0], 12).hex())

    def load_string(cx, ecx, a):
        cx.write(a[1], b'text %#x\0' % a[0])

    def op_new(cx, ecx, a):
        cx.note('new', a[0], a[2])                # the __FILE__ pointer is per object
        return cx.alloc(max(a[0], 4), fill=0xcd) if c['new_ok'] else 0

    def message(cx, ecx, a):
        text = cx.read(a[0], 0x20).split(b'\0')[0]
        cx.note('UnknownMessage', text, a[1])
        return ecx

    ctx.stub_at(0x004fc660, 'SetPosition', pop=4, ret=note_vec, log=False)
    sink = ctx.alloc(0x40, fill=0x22) if c['sink'] else 0
    ctx.stub_at(0x0045d340, 'FindTextQueue', ret=sink)
    ctx.stub_at(0x00521970, 'LoadResourceString', pop=12, ret=load_string)
    ctx.stub_at(0x004a3010, 'operator new', nargs=3, ret=op_new, log=False)
    ctx.stub_at(0x0051b200, 'UnknownMessage', pop=8, ret=message, log=False)
    ctx.stub_at(0x0051b540, 'ReplaceHeadMessage', pop=4)
    ctx.stub_at(0x004a30c0, 'operator delete', nargs=1, ret=lambda cx, ecx, a: cx.note('delete', a[0]), log=False)
    ctx.stub_at(0x004a8b40, 'SetMotion', pop=4)
    ctx.stub_at(0x00413200, 'AiStep', pop=4)
    ctx.stub_at(0x0040ca20, 'Bike::SimulateFrame', pop=4)
    end_motion = ctx.stub('EndMotion')
    vtable = ctx.vtable([ctx.stub('slot%d' % i) if i != 0xa4 // 4 else end_motion for i in range(60)])

    import random
    r = random.Random(c['seed'])
    bike = ctx.alloc(0x1560)
    ctx.write(bike, bytes(r.randrange(256) for _ in range(0x1560)))
    ctx.pack(bike, 'I', vtable)
    ctx.pack(bike + 0xc, '3f', *[r.uniform(-100, 100) for _ in range(3)])
    ctx.pack(bike + 0x1540, '3f', *[r.uniform(-100, 100) for _ in range(3)])
    ctx.pack(bike + 0x154c, 'B', c['drawn'])
    ctx.pack(bike + 0x3bc, 'I', ctx.alloc(0x40))
    rider = ctx.alloc(0x1b0, fill=0x44)
    ctx.pack(rider + 0x1a0, 'I', ctx.alloc(0x40))
    ctx.pack(bike + 0x5c4, 'I', rider)
    race = ctx.alloc(0x200, fill=0x55)
    camera = ctx.alloc(0x3c0, fill=0x66)
    ctx.pack(camera + 0x244, 'i', c['cam'])
    ctx.pack(camera + 0x3b0, 'I', bike if c['followed'] else race)
    ctx.pack(race + 0x38, 'I', bike if c['local'] else race)
    ctx.pack(race + 0x50, 'I', camera)
    ctx.pack(race + 0xb8, 'i', c['force'])
    ctx.pack(race + 0x189, 'BB', c['before'], c['racing'])
    ctx.pack(bike + 0x740, 'I', race)
    ctx.pack(bike + 0x754, 'f', c['time'])
    ctx.pack(bike + 0x7a0, 'hh', *c['laps'])
    ctx.pack(bike + 0x7a4, 'B', c['finished'])
    ctx.pack(bike + 0x784, 'i', c['position'])
    ctx.pack(bike + 0x444, 'i', c['crash'])
    ctx.pack(bike + 0x734, 'B', c['ai'])
    game = ctx.alloc(0x2e00, fill=0x77)
    ctx.pack(game + 0x18, 'i', c['players'])
    ctx.pack(game + 0x570, 'I', ctx.alloc(0x40))
    ctx.pack(game + 0x2d70, 'ii', c['mode'], c['event'])
    ctx.pack(TRACK_GAME, 'I', game)
    return dict(ecx=bike, args=[float(c['dt'])], ret_eax=False)
