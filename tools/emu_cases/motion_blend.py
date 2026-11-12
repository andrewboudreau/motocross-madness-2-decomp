"""Character::ApplyMotionBlendAtPhase, the two-motion pose blend (tools/emu_diff.py case).

Two motions with random frame counts and poses. The vector helpers, the pose constructor and
InterpolatePose run as retail code in both bodies. The pose sink (virtual slot 4) is stubbed
and logs the 0x2c-byte pose it receives. Phases include the exact `(frameCount - 1) * phase -
0.5f` ties of either motion and their binary32 neighbours.
"""
import struct

from tools.emu_cases.four_pose import POSE
from tools.emu_cases.motion_start import f32, next32

RETAIL_VA = 0x004a8c50
SYMBOL = '?ApplyMotionBlendAtPhase@Character@@QAEXHHMM@Z'
SOURCE = 'samples/physics/motion/CharacterMotionBlend.cpp'
PROBES = {0x004a8c50: 'entry'}


def cases(rng, n):
    for _ in range(n):
        fcs = [rng.choice([1, 2, 3, 5, 17, 60, 61]) for _ in range(2)]
        kind = rng.randrange(4)
        if kind == 0:
            phase = rng.choice([0.0, 1.0, 0.5, rng.random(), 1.0 - 1e-7, 1e-7])
        else:
            fc = rng.choice(fcs)
            k = rng.randrange(fc)
            phase = f32((k + 0.5) / max(fc - 1, 1))
            if kind == 2:
                phase = next32(phase, rng.choice([-1, 1]))
            phase = min(phase, 1.0)
        w = rng.choice([0.0, 1.0, rng.random(), rng.uniform(-0.5, 1.5)])
        yield (fcs, rng.choice([1, 2, 4]), (phase, w))


def setup(ctx, case):
    frame_counts, poses, args = case
    rng = ctx.rng
    motions = []
    for fc in frame_counts:
        frames = ctx.alloc(12 * fc)
        for fi in range(fc):
            blob = bytearray()
            for _ in range(poses):
                vals = [rng.uniform(-2, 2) for _ in range(9)]
                blob += struct.pack('<B3xI9f', rng.randrange(40), rng.randrange(2), *vals)
            parr = ctx.alloc(len(blob), blob)
            ctx.pack(frames + 12 * fi, 'iIi', fi, parr, poses)
        m = ctx.alloc(0xb0)
        ctx.pack(m + 0xa4, 'IiI', frames, fc, poses)
        motions.append(m)

    def sink(c, ecx, a):
        c.note(c.read(a[0], POSE).hex(), a[1])
    slot = ctx.stub('Character::vslot', pop=8, ret=sink)
    vt = ctx.vtable([slot] * 16)
    ch = ctx.alloc(0x40)
    ctx.pack(ch, 'I', vt)
    return dict(ecx=ch, args=motions + [float(x) for x in args], ret_eax=False)
