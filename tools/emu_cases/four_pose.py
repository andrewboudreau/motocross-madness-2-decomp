"""Character::ApplyFourMotionBlendAtPhase, the four-motion pose blend (tools/emu_diff.py case).

Four motions with random frame counts and poses; the blend phase sweeps [0, 1] so the
`frameCount * phase - 0.5f` fistp sites land on both sides of each frame boundary. Every
virtual call on the character (the pose sink) is logged with the 0x2c-byte pose it receives.
"""
import struct

RETAIL_VA = 0x004a9050
SYMBOL = '?ApplyFourMotionBlendAtPhase@Character@@QAEXHHHHMMM@Z'
SOURCE = 'samples/physics/motion/CharacterMotionFourPose.cpp'
POSE = 0x2c


def cases(rng, n):
    for i in range(n):
        fcs = [rng.choice([1, 2, 3, 5, 17, 60, 61]) for _ in range(4)]
        phase = rng.choice([0.0, 1.0, 0.5, rng.random(), rng.random(), 1.0 - 1e-7, 1e-7, 0.999])
        w = rng.choice([0.0, 1.0, rng.random(), rng.uniform(-0.5, 1.5)])
        lean = rng.choice([0.0, 1.0, rng.random(), rng.uniform(-0.5, 1.5)])
        yield (fcs, rng.choice([1, 2, 5]), (phase, w, lean))
    # A phase grid over a fixed 61-frame motion probes the rounding edges.
    for k in range(max(n // 2, 20)):
        yield ([61, 31, 2, 7], 1, (k / (max(n // 2, 20) - 1.0), 0.3, 0.6))


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

    def sink(ctx, ecx, a):
        ctx.note(ctx.read(a[0], POSE).hex(), a[1])
    slot = ctx.stub('Character::vslot', pop=8, ret=sink)
    vt = ctx.vtable([slot] * 16)
    ch = ctx.alloc(0x40)
    ctx.pack(ch, 'I', vt)
    return dict(ecx=ch, args=motions + [float(x) for x in args])
