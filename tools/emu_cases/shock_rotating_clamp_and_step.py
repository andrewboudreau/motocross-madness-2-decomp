"""RotatingShock::ClampAndStep 0x004fa8b0 (tools/emu_diff.py case).

src/bringup/Group3_KrustyBikeAISupport.cpp re-implements it as BikeQ::ClampAndStep with a
shared static step tail. Offsets are retail's (0x004fa8b0 disassembly): local matrix +0x04,
scene node +0x44, atLimit +0x50, extending +0x54, ratio +0x70, displacement +0x74, step
displacement +0x80, previous length +0x8c, length +0x98, velocity +0xb0, max angle +0xc0,
chord +0xc4, arm length +0xc8, angle +0xd0, rotation axis +0xd4.

Random: length below, at and above the chord, zero length, zero chord and zero arm length
(infinite or NaN angle, so `_finite` 0x00534d07 skips the node update; it runs as retail
code), the previous length above and below (the clamp to +-maxDelta both ways), dt and
maxDelta. Stubbed: SetLocalMatrix 0x004fca30 (matrix bytes noted) and Rotate 0x004fceb0
(axis and angle noted).
"""
import random

RETAIL_VA = 0x004fa8b0
SYMBOL = '?ClampAndStep@BikeQ@@QAEXMM@Z'
SOURCE = 'src/bringup/Group3_KrustyBikeAISupport.cpp'
PROBES = {0x004fa972: 'node update', 0x004faab6: 'zero length'}


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    s = ctx.alloc(0xe0, fill=0x3c)
    node = ctx.alloc(0x40, fill=0)
    ctx.pack(s + 0x44, 'I', node)
    ctx.write(s + 0x04, bytes(r.randrange(256) for _ in range(0x40)))
    ctx.pack(s + 0x50, 'ii', r.choice([0, 1]), r.choice([0, 1]))
    ctx.pack(s + 0x74, '3f', *[r.uniform(-1, 1) for _ in range(3)])
    chord = r.choice([r.uniform(0.1, 1.0), 0.5, 0.0])
    kind = r.randrange(5)
    length = [r.uniform(0, chord), chord, chord + r.uniform(0, 0.5), 0.0, r.uniform(0, 1)][kind]
    ctx.pack(s + 0x8c, 'f', r.choice([length, r.uniform(0, 1.5), length + r.uniform(-0.01, 0.01)]))
    ctx.pack(s + 0x98, 'f', length)
    ctx.pack(s + 0xc0, '3f', r.uniform(0, 1.5), chord, r.choice([r.uniform(0.1, 1), 0.0]))
    ctx.pack(s + 0xd4, '3f', *[r.uniform(-1, 1) for _ in range(3)])

    def set_matrix(cx, ecx, a):
        cx.note('SetLocalMatrix', ecx, cx.read(a[0], 0x40).hex())
        return 0

    def rotate(cx, ecx, a):
        cx.note('Rotate', ecx, *a[:4])
        return 0

    ctx.stub_at(0x004fca30, 'SetLocalMatrix', pop=4, ret=set_matrix, log=False)
    ctx.stub_at(0x004fceb0, 'Rotate', pop=16, ret=rotate, log=False)
    md = r.choice([r.uniform(0, 0.2), 0.0, 1e9])
    dt = r.choice([1 / 60.0, r.uniform(0.001, 0.1)])
    return dict(ecx=s, args=[md, dt], ret_eax=False)
