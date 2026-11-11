"""PathSteeringDirection 0x0040e510, the AI path steering direction (tools/emu_diff.py case).

The linked body is the Group 3 bring-up copy (src/bringup/Group3_KrustyBikeAI.cpp), cdecl
(outDir, Vec3 pos, path, speed, side). The path (UnknownBikeAIPath, KrustyBikeRecords.h: count
+0x0, index +0x4, t +0x8, points +0xc, lookAhead +0x974) holds 0..12 points: random, lattice
(exact ties between the nearest-point candidates), repeated (zero segments) or along a line
(perpendicular feet on the segment ends). The event type at g_TrackGame+0x2d74 selects the
|offset| look-ahead extension (kinds 2 and 3). Null path / outDir and the single-point path
(which tests the incoming *outDir against zero, as retail does) are included. IntersectLines
0x0047b800, SafeNormalize 0x0040d120 and FastInvSqrt run as retail code.
"""
RETAIL_VA = 0x0040e510
SYMBOL = '?PathSteeringDirection@@YAHPAUVec3@@U1@PAUUnknownBikeAIPath@@MM@Z'
SOURCE = 'src/bringup/Group3_KrustyBikeAI.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x0040e6a3: 'previous segment', 0x0040e79c: 'next segment', 0x0040e80a: 'offset ahead',
          0x0040eb99: 'zero direction', 0x0040ebdb: 'normalised', 0x0040ebdc: 'single point', 0x0040ec95: 'rejected',
          0x0040ea3c: 'path end', 0x0040ea78: 'step'}


def cases(rng, n):
    for _ in range(n):
        kind = rng.randrange(6)
        count = rng.choice([0, 1, 2, 2, 3, 4, 6, 8, 12]) if rng.random() < 0.95 else 200
        s = rng.choice([1.0, 10.0, 100.0])
        if kind == 0:     # lattice
            pts = [[float(rng.randrange(-3, 4)) * s / 2, 0.0, float(rng.randrange(-3, 4)) * s / 2]
                   for _ in range(count)]
        elif kind == 1:   # a line
            d = [rng.uniform(-1, 1), rng.uniform(-0.2, 0.2), rng.uniform(-1, 1)]
            pts = [[i * s * d[0], i * s * d[1], i * s * d[2]] for i in range(count)]
        else:
            pts = [[rng.uniform(-s, s), rng.uniform(-s / 10, s / 10), rng.uniform(-s, s)] for _ in range(count)]
            if pts and rng.random() < 0.2:
                k = rng.randrange(len(pts))
                pts.insert(k, list(pts[k]))
                pts = pts[:count]
        if pts and rng.random() < 0.2:
            pos = list(rng.choice(pts))
        elif rng.random() < 0.1:
            pos = [float(rng.randrange(-3, 4)) * s / 4, 0.0, float(rng.randrange(-3, 4)) * s / 4]
        else:
            pos = [rng.uniform(-s, s) for _ in range(3)]
        out = rng.choice([[0.0, 0.0, 0.0], [rng.uniform(-1, 1) for _ in range(3)]])
        yield dict(pts=pts, pos=pos, out=out, event=rng.choice([0, 1, 2, 3, 4, 5]),
                   ahead=rng.choice([0.0, rng.uniform(0, s), rng.uniform(0, 5 * s), rng.randrange(6) * s / 2]),
                   side=rng.choice([0.0, rng.uniform(-3, 3)]), null=rng.choice([None] * 18 + ['path', 'out']),
                   garbage=(rng.randrange(-5, 5), rng.uniform(-2, 2)))


def setup(ctx, c):
    game = ctx.alloc(0x3000)
    ctx.pack(game + 0x2d74, 'i', c['event'])
    ctx.pack(TRACK_GAME, 'I', game)
    path = ctx.alloc(0x978, fill=0x41)
    ctx.pack(path, 'iif', len(c['pts']), *c['garbage'])
    for i, p in enumerate(c['pts']):
        ctx.pack(path + 0xc + 12 * i, '3f', *p)
    ctx.pack(path + 0x974, 'f', c['ahead'])
    out = ctx.alloc(12)
    ctx.pack(out, '3f', *c['out'])
    return dict(args=[0 if c['null'] == 'out' else out, *map(float, c['pos']),
                      0 if c['null'] == 'path' else path, 12.5, float(c['side'])])
