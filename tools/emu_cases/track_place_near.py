"""Track::PlaceNear 0x00517340, placing a point on the track graph near a position (tools/emu_diff.py case).

The game links this body (samples/track/TrackNearMisses.cpp) behind KbRaceSub::PlaceNear,
which the BikeAI code calls. A random track graph (nodes with polyline segments and strip
edges, links forming chains, branches and cycles) is walked from its start node; the point lies
on a strip, between strips or off the track. IsInsideStrip, DistanceAlong, ShortestPath,
NodeHasSegment and FreeWorkList run as retail code in both bodies. DebugCalloc (0x004a2fc0) and
DebugFree (0x004a2e60) are stubs: calloc hands out fresh zeroed blocks (failing at a chosen call
in some cases, the out-of-memory exits) and both log their size/line and block, not __FILE__.
"""
import math
import struct

RETAIL_VA = 0x00517340
SYMBOL = '?PlaceNear@Track@@QAEHUVector3@@UTrackPos@@MMHPAU3@@Z'
SOURCE = 'samples/track/TrackNearMisses.cpp'
PROBES = {0x0051741e: 'IsInsideStrip call', 0x005175ad: 'out-of-memory FreeWorkList exit',
          0x0051777e: 'path loop'}


def cases(rng, n):
    for _ in range(n):
        nodes = rng.randrange(1, 6)
        x, z, heading = 0.0, 0.0, rng.uniform(0, 2 * math.pi)
        polylines = []
        for _ in range(nodes):
            pts = []
            for _ in range(rng.randrange(1, 5)):
                pts.append((x, rng.uniform(-1, 1), z))
                step = rng.choice([0.0, rng.uniform(2, 20)]) if rng.random() < 0.1 else rng.uniform(2, 20)
                heading += rng.uniform(-0.6, 0.6)
                x += step * math.cos(heading)
                z += step * math.sin(heading)
            pts.append((x, 0.0, z))
            polylines.append(pts)
        links = []
        for i in range(nodes):
            out = []
            if i + 1 < nodes:
                out.append(i + 1)
            if rng.random() < 0.3:
                out.append(rng.randrange(nodes))          # branch or cycle
            if i == nodes - 1 and rng.random() < 0.5:
                out.append(0)                              # closed loop
            links.append(out)
        # The point: on a random segment (inside the strip), offset sideways or far away.
        ni = rng.randrange(nodes)
        si = rng.randrange(len(polylines[ni]) - 1)
        a, b = polylines[ni][si], polylines[ni][si + 1]
        u = rng.random()
        side = rng.choice([0.0, rng.uniform(-3, 3), rng.uniform(-30, 30)])
        dx, dz = b[0] - a[0], b[2] - a[2]
        ln = math.hypot(dx, dz) or 1.0
        p = (a[0] + u * dx - side * dz / ln, rng.uniform(-2, 2), a[2] + u * dz + side * dx / ln)
        if rng.random() < 0.1:
            p = (p[0] + 500, p[1], p[2])
        fn = rng.randrange(nodes)
        from_pos = (fn, rng.randrange(len(polylines[fn]) - 1), rng.choice([0.0, 1.0, rng.random()]))
        path = [rng.randrange(nodes) for _ in range(rng.choice([0, 0, 1, 2, 3]))]
        yield dict(polylines=polylines, links=links, width=rng.choice([2.0, 5.0, 10.0]), p=p,
                   from_pos=from_pos, path=path,
                   distance=rng.choice([rng.uniform(0, 100), 0.0, -1.0, 1e6]),
                   range=rng.choice([rng.uniform(0, 100), 0.0, -1.0, 1e6]),
                   out=rng.random() < 0.8, fail_at=rng.choice([None] * 9 + [rng.randrange(1, 12)]))


def setup(ctx, c):
    count = [0]

    def calloc(cx, ecx, a):
        count[0] += 1
        cx.note('calloc', a[0], a[1], a[3])
        if c['fail_at'] is not None and count[0] == c['fail_at']:
            return 0
        return cx.alloc(a[0] * a[1])

    def free(cx, ecx, a):
        cx.note('free', a[0], a[2])
        return 0
    ctx.stub_at(0x004a2fc0, 'DebugCalloc', nargs=4, ret=calloc, log=False)
    ctx.stub_at(0x004a2e60, 'DebugFree', nargs=3, ret=free, log=False)

    w = c['width']
    nodes = [ctx.alloc(0x18) for _ in c['polylines']]
    segs = []
    for ni, pts in enumerate(c['polylines']):
        addrs = [ctx.alloc(0x30) for _ in pts]
        for k, pt in enumerate(pts):
            nxt = pts[k + 1] if k + 1 < len(pts) else None
            prv = pts[k - 1] if k else None
            ref = nxt or prv
            dx, dz = (ref[0] - pt[0], ref[2] - pt[2]) if nxt else (pt[0] - ref[0], pt[2] - ref[2])
            ln = math.hypot(dx, dz) or 1.0
            nx, nz = -dz / ln * w, dx / ln * w
            hl = math.hypot(nxt[0] - pt[0], nxt[2] - pt[2]) if nxt else 0.0
            ctx.pack(addrs[k], '10fII', pt[0], pt[1], pt[2], pt[0] + nx, pt[1], pt[2] + nz,
                     pt[0] - nx, pt[1], pt[2] - nz, hl,
                     addrs[k - 1] if k else 0, addrs[k + 1] if k + 1 < len(pts) else 0)
        segs.append(addrs)
        length = sum(math.hypot(pts[k + 1][0] - pts[k][0], pts[k + 1][2] - pts[k][2]) for k in range(len(pts) - 1))
        links = c['links'][ni]
        arr = ctx.alloc(4 * max(len(links), 1))
        for j, t in enumerate(links):
            ctx.pack(arr + 4 * j, 'I', nodes[t])
        ctx.pack(nodes[ni], 'B3xfIIiI', 0, length, addrs[0], addrs[-1], len(links), arr)
    track = ctx.alloc(8)
    ctx.pack(track, 'If', nodes[0], 0.0)

    path = 0
    for t in reversed(c['path']):
        item = ctx.alloc(0x10)
        ctx.pack(item, 'B3xIII', 0, nodes[t], 0, path)
        path = item
    out = ctx.alloc(12, fill=0x77) if c['out'] else 0
    fn, fs, ft = c['from_pos']
    args = [float(v) for v in c['p']]
    args += [struct.pack('<IIf', nodes[fn], segs[fn][fs], ft)]
    args += [float(c['distance']), float(c['range']), path, out]
    return dict(ecx=track, args=args)
