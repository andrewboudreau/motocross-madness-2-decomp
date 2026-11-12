"""Character::ApplyMotionAtPhase, start a motion at a phase (tools/emu_diff.py case).

ApplyPoseListSlot4 (0x004a8a80) is stubbed and logs the frame pointer it receives, so the
rounded `(frameCount - 1) * phase - 0.5f` index is compared with the current motion and frame
fields. Phases include exact ties ((k + 0.5) / (frameCount - 1)), their binary32 neighbours and
out-of-range values (retail's masked invalid store gives 0x80000000).
"""
import struct

RETAIL_VA = 0x004a8bf0
SYMBOL = '?ApplyMotionAtPhase@Character@@QAEXHM@Z'
SOURCE = 'samples/physics/motion/CharacterMotionStart.cpp'
PROBES = {0x004a8c1c: 'frame fistp'}


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def next32(x, d):
    b = struct.unpack('<i', struct.pack('<f', x))[0]
    return struct.unpack('<f', struct.pack('<i', b + (d if b >= 0 else -d)))[0]


def phase_for(rng, fc):
    kind = rng.randrange(6)
    if kind == 0:
        return rng.random()
    if kind == 5:
        return rng.choice([-1.0, 1.0 + 1e-7, 1e10, -1e10, float('inf'), float('nan'), 0.0, 1.0])
    k = rng.randrange(max(fc, 1) + 1)
    p = f32((k + rng.choice([0.5, 0.0, 1.0])) / max(fc - 1, 1))
    return next32(p, rng.choice([-1, 0, 0, 1])) if kind >= 3 else p


def cases(rng, n):
    for _ in range(n):
        fc = rng.choice([1, 2, 3, 4, 5, 17, 60, 61, 256])
        yield (fc, phase_for(rng, fc))


def setup(ctx, case):
    fc, phase = case
    frames = ctx.alloc(12 * fc + 64)
    motion = ctx.alloc(0xb0, fill=0x2d)
    ctx.pack(motion + 0xa4, 'Ii', frames, fc)
    ctx.stub_at(0x004a8a80, 'ApplyPoseListSlot4', pop=8)
    character = ctx.alloc(0x40, fill=0x3c)
    return dict(ecx=character, args=[motion, phase], ret_eax=False)
