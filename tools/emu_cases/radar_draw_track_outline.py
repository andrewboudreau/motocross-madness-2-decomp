"""RadarOverlay::DrawTrackOutline 0x0051c720, the radar map's track edge lines (tools/emu_diff.py case).

Linked from src/bringup/Group3_RadarOverlay.cpp. Offsets are retail's (0x0051c720 disassembly):
radarView +0x12c, its track +0x48 whose first dword is the start node; the render target +0x18
(DrawPrimitive at vtable +0x40); map radius +0x19c (float), centre x/y +0x1a0/+0x1a4 (ints);
the two TL line vertices +0x1b8/+0x1d8 (colour +0x10 each). A track node has its visit flag in
bit 2 of byte +0, first segment +8, link count +0x10 and link array +0x14; a segment has edge A
+0x0c, edge B +0x18 and next +0x2c. The work list items are 16-byte DebugCalloc blocks (node +4,
next +0xc).

Random track graphs: 1..5 nodes, each with 1..6 segments as a closed ring (the walk stops back
at the first segment) or an open chain, random links between nodes (self links, repeated links
and back links included) and nodes that start already flagged. Modes 0..3; modes 0 and 3 copy
no edge, so WorldToMap gets the point left from the previous segment or the uninitialised
local, and the bytes it gets are noted.

Stubbed: DebugCalloc 0x004a2fc0 (zeroed block, size and line noted), DebugFree 0x004a2e60
(pointer and line noted), WorldToMap 0x0051c360 (point bytes noted; random map x/y, rim point
and outside flag 0/1) and DrawPrimitive (both vertices noted). IntersectLineCircle 0x0051c4f0
runs as each image's own code (its own case is radar_line_circle).
"""
import random
import struct

RETAIL_VA = 0x0051c720
SYMBOL = '?DrawTrackOutline@RadarOverlay@@QAEXH@Z'
SOURCE = 'src/bringup/Group3_RadarOverlay.cpp'
PROBES = {0x0051c775: 'flagged pop', 0x0051c7b2: 'mode 1', 0x0051c7d1: 'mode 2',
          0x0051c959: 'exit rim', 0x0051c9f6: 'enter rim', 0x0051ca6a: 'inside line',
          0x0051cad9: 'push link'}


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    nodes = []
    count = r.choice([1, 1, 2, 3, 5])
    for _ in range(count):
        node = ctx.alloc(0x20, fill=0)
        nodes.append(node)
    for node in nodes:
        segs = [ctx.alloc(0x30, fill=0x5b) for _ in range(r.randrange(1, 7))]
        for k, seg in enumerate(segs):
            ctx.pack(seg + 0x0c, '3f', *[r.uniform(-500, 500) for _ in range(3)])
            ctx.pack(seg + 0x18, '3f', *[r.uniform(-500, 500) for _ in range(3)])
            nxt = segs[k + 1] if k + 1 < len(segs) else (segs[0] if r.random() < 0.7 else 0)
            ctx.pack(seg + 0x2c, 'I', nxt)
        links = [r.choice(nodes) for _ in range(r.choice([0, 0, 1, 2, 3]))]
        array = ctx.alloc(4 * max(len(links), 1), fill=0)
        if links:
            ctx.pack(array, '%dI' % len(links), *links)
        flags = r.choice([0, 0, 0, 1, 0xf0, 4]) if node != nodes[0] else r.choice([0, 0, 0x10])
        ctx.pack(node, 'B', flags)
        ctx.pack(node + 8, 'I', segs[0])
        ctx.pack(node + 0x10, 'iI', len(links), array)

    track = ctx.alloc(0x10, fill=0)
    ctx.pack(track, 'I', nodes[0])
    view = ctx.alloc(0x200, fill=0x56)
    ctx.pack(view + 0x48, 'I', track)
    radar = ctx.alloc(0x220, fill=0x3a)
    ctx.pack(radar + 0x12c, 'I', view)
    target = ctx.alloc(0x10, fill=0)
    vtable = ctx.alloc(0x80, fill=0)
    ctx.pack(target, 'I', vtable)
    ctx.pack(radar + 0x18, 'I', target)
    ctx.pack(radar + 0x19c, 'fii', r.uniform(20, 80), r.randrange(0, 200), r.randrange(0, 200))
    ctx.write(radar + 0x1b8, bytes(r.randrange(256) for _ in range(0x40)))

    def draw(cx, ecx, a):           # (type, fvf, vertices, count, flags)
        cx.note('DrawPrimitive', a[0], a[1], a[3], a[4], cx.read(a[2], 0x40).hex())
        return 0
    ctx.pack(vtable + 0x40, 'I', ctx.stub('DrawPrimitive', pop=20, ret=draw))

    def calloc(cx, ecx, a):         # (count, size, file, line)
        cx.note('DebugCalloc', a[0], a[1], a[3])
        return cx.alloc(a[0] * a[1], fill=0)

    def free(cx, ecx, a):           # (pointer, file, line)
        cx.note('DebugFree', a[0], a[2])
        return 0

    def world_to_map(cx, ecx, a):   # (Vector3 point, int* x, int* y, float* rimX, float* rimY)
        cx.note('WorldToMap', ecx, *a[:3])
        cx.pack(a[3], 'i', r.randrange(-50, 250))
        cx.pack(a[4], 'i', r.randrange(-50, 250))
        cx.pack(a[5], 'f', r.uniform(-50, 250))
        cx.pack(a[6], 'f', r.uniform(-50, 250))
        return r.choice([0, 0, 1])

    ctx.stub_at(0x004a2fc0, 'DebugCalloc', nargs=4, ret=calloc, log=False)
    ctx.stub_at(0x004a2e60, 'DebugFree', nargs=3, ret=free, log=False)
    ctx.stub_at(0x0051c360, 'WorldToMap', pop=28, ret=world_to_map, log=False)
    return dict(ecx=radar, args=[r.choice([0, 1, 2, 2, 3])], ret_eax=False)
