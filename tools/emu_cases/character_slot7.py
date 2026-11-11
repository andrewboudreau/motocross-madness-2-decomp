"""Character::CharacterVirtualSlot7 0x004a70c0, non-looping playback with blending (tools/emu_diff.py case).

A character with a current motion and (when blending) a blend-from motion, each with random
frame counts, fps and poses. Time steps land inside the motion (the interpolation loop), past
its end (the extrapolation loop and the finished/looping tail) and, with a blend active,
across the blend duration (the smoothstep weight, both the interpolated and the extrapolated
blend-from pose). AdvanceClamped, PoseRotation, RotatePose, InterpolatePose and the vector
helpers run as retail code in both bodies; the pose sinks (slots 4 and 5) are stubs that log
every 0x2c-byte pose, and BlendToMotion (0x004a8b60) is a stub.
"""
import struct

from tools.emu_cases.rotate_pose import pack_pose, random_pose

RETAIL_VA = 0x004a70c0
SYMBOL = '?CharacterVirtualSlot7@Character@@UAEHMHH@Z'
SOURCE = 'samples/physics/motion/Motnctrl.cpp'
PROBES = {0x004a7dc0: 'RotatePose (extrapolation)', 0x004a8470: 'InterpolatePose (blend)',
          0x004a8b60: 'BlendToMotion (looping end)'}
NODES = 8


def cases(rng, n):
    for _ in range(n):
        poses = rng.choice([1, 2, 3])
        motions = []
        for _ in range(2):
            fc = rng.choice([2, 3, 5, 17])
            fps = rng.choice([10, 15, 30])
            frames = [[random_pose(rng) for _ in range(poses)] for _ in range(fc)]
            # Poses of one node in consecutive frames: often the same axes (zero angle).
            for fr in frames:
                for k, p in enumerate(fr):
                    fr[k] = (k % NODES,) + p[1:]
                    if rng.random() < 0.2:
                        fr[k] = fr[k][:2] + (list(frames[0][k][2]), list(frames[0][k][3])) + fr[k][4:]
            motions.append((fc, fps, rng.randrange(2), frames))
        state = dict(
            stopped=int(rng.random() < 0.05),
            clock=rng.choice([0.0, rng.uniform(0, 0.2)]),
            frame=rng.randrange(motions[0][0]),
            last=rng.randrange(motions[1][0]),
            last_time=rng.uniform(0, 0.2),
            blend_from=rng.uniform(0, 0.4),
            blend_dur=rng.choice([0.25, 0.5, 0.1]),
            blend=rng.randrange(2),
        )
        time = rng.choice([1 / 60.0, 1 / 30.0, rng.uniform(0, 0.1), rng.uniform(0.1, 2.0)])
        flags = [rng.randrange(2) for _ in range(NODES)]
        yield (motions, state, time, rng.randrange(2), rng.randrange(-1, 4), flags)


def setup(ctx, case):
    motions, st, time, mirror, mask, flags = case

    def sink(c, ecx, a):
        c.note(c.read(a[0], 0x2c).hex(), a[1])
    slot4 = ctx.stub('Character::slot4', pop=8, ret=sink)
    slot5 = ctx.stub('Character::slot5', pop=8, ret=sink)
    other = ctx.stub('Character::other', pop=0)
    vt = ctx.vtable([other] * 4 + [slot4, slot5] + [other] * 6)
    ctx.stub_at(0x004a8b60, 'Character::BlendToMotion', pop=4)

    addrs = []
    frame_ptrs = []
    for fc, fps, looping, frames in motions:
        arr = ctx.alloc(12 * fc)
        for i, fr in enumerate(frames):
            blob = b''.join(pack_pose(p) for p in fr)
            pa = ctx.alloc(len(blob), blob)
            ctx.pack(arr + 12 * i, 'iIi', i, pa, len(fr))
        m = ctx.alloc(0xcc)
        ctx.pack(m + 0xa4, 'IiiiiffII', arr, fc, len(frames[0]), looping, fps,
                 struct.unpack('<f', struct.pack('<f', 1.0 / fps))[0], 0.25, 1, 1)
        addrs.append(m)
        frame_ptrs.append(arr)

    names = ctx.alloc(0x40 * NODES)
    for k in range(NODES):
        ctx.pack(names + 0x40 * k + 0x38, 'ii', flags[k], 1)

    ch = ctx.alloc(0x200)
    ctx.pack(ch, 'I', vt)
    ctx.pack(ch + 0x0c, 'if', st['stopped'], st['clock'])
    ctx.pack(ch + 0x18, 'IIIIfffi', addrs[0], frame_ptrs[0] + 12 * st['frame'], addrs[1],
             frame_ptrs[1] + 12 * st['last'], st['last_time'], st['blend_from'], st['blend_dur'],
             st['blend'])
    ctx.pack(ch + 0x188, 'I', names)
    return dict(ecx=ch, args=[float(time), mirror, mask])
