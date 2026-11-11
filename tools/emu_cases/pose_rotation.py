"""PoseRotation 0x004a7fd0, the axis/angle taking one pose to another (tools/emu_diff.py case).

Pose pairs with differing axisY (the first path), equal axisY and differing axisZ (the second
path) and identical axes (the (0, 0, 1)/0 fallback), including unit and nearly parallel axes
so the acos argument reaches the [-1, 1] clamp. acos, FastInvSqrt and the vector helpers run
as retail code in both bodies.
"""
from tools.emu_cases.rotate_pose import pack_pose, random_pose

RETAIL_VA = 0x004a7fd0
SYMBOL = '?PoseRotation@@YAXPAUCharacterPose@@0PAUVec3@@PAM@Z'
SOURCE = 'samples/physics/motion/Motnctrl.cpp'


def cases(rng, n):
    for _ in range(n):
        a = random_pose(rng)
        b = random_pose(rng)
        kind = rng.randrange(5)
        if kind == 1:            # same axisY: axisZ path
            b = (b[0], b[1], b[2], list(a[3]), b[4])
        elif kind == 2:          # identical axes: fallback
            b = (b[0], b[1], list(a[2]), list(a[3]), b[4])
        elif kind == 3:          # nearly parallel axisY (clamp edge)
            k = rng.choice([1.0, 1.0000001, 2.0])
            b = (b[0], b[1], b[2], [x * k + rng.choice([0.0, 1e-7]) for x in a[3]], b[4])
        elif kind == 4:          # antiparallel axisZ with equal axisY
            b = (b[0], b[1], [-x for x in a[2]], list(a[3]), b[4])
        yield (a, b)


def setup(ctx, case):
    a, b = case
    pa = ctx.alloc(0x2c, pack_pose(a))
    pb = ctx.alloc(0x2c, pack_pose(b))
    axis = ctx.alloc(12, fill=0x77)
    angle = ctx.alloc(4, fill=0x77)
    return dict(args=[pa, pb, axis, angle], ret_eax=False)
