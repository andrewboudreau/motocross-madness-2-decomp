"""RotatePose 0x004a7dc0, slot 7's pose rotation (tools/emu_diff.py case).

Random poses (normalised or not, unit axes for the `|v|^2 == 1` shortcut of Vec3Normalize)
rotated about random and axis-aligned axes. AxisAngleMatrix, MatrixMultiply and the vector
helpers run as retail code in both bodies; the destination pose starts filled with 0x77 so
every byte the body writes is compared.
"""
import math
import struct

RETAIL_VA = 0x004a7dc0
SYMBOL = '?RotatePose@@YAXPAUCharacterPose@@0UVec3@@M@Z'
SOURCE = 'samples/physics/motion/Motnctrl.cpp'


def random_pose(rng):
    if rng.random() < 0.3:
        z, y = [0.0, 0.0, 1.0], [0.0, 1.0, 0.0]
    else:
        z = [rng.uniform(-2, 2) for _ in range(3)]
        y = [rng.uniform(-2, 2) for _ in range(3)]
    pos = [rng.uniform(-50, 50) for _ in range(3)]
    return rng.randrange(40), rng.randrange(2), z, y, pos


def pack_pose(pose):
    node, has, z, y, pos = pose
    return struct.pack('<B3xi9f', node, has, *z, *y, *pos)


def cases(rng, n):
    for _ in range(n):
        axis = rng.choice([[0.0, 0.0, 1.0], [1.0, 0.0, 0.0],
                           [rng.uniform(-1, 1) for _ in range(3)]])
        length = math.sqrt(sum(a * a for a in axis)) or 1.0
        if rng.random() < 0.7:
            axis = [a / length for a in axis]
        angle = rng.choice([rng.uniform(-math.pi, math.pi), 1e-4, -0.5, math.pi])
        yield (random_pose(rng), axis, angle)


def setup(ctx, case):
    pose, axis, angle = case
    src = ctx.alloc(0x2c, pack_pose(pose))
    dst = ctx.alloc(0x2c, fill=0x77)
    return dict(args=[src, dst] + [float(a) for a in axis] + [float(angle)], ret_eax=False)
