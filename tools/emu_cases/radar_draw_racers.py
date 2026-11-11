"""RadarOverlay::DrawRacers 0x0051bed0, the radar map's racer dots and name-tag anchors.

tools/emu_diff.py case; linked from src/bringup/Group3_RadarOverlay.cpp. Offsets are retail's
(0x0051bed0 disassembly): render target +0x18, radarView +0x12c (chatOverlay +0x19c, whose
name tags sit at +0x2d8 + 4k with trackedRacer +0x11c), radarCamera +0x130 (followedRacer
+0x3b0), racers +0x134 (count +0x160), cueIndex +0x16c, followedIndex +0x170, cueShown
+0x174, mapOrigin +0x17c, mapAngle/held angle/cos/sin +0x188..+0x194, mapCenterX +0x1a0,
the two TL line vertices +0x1b8/+0x1d8 and mapRect.top +0x1fc. A racer has position +0x0c,
heading +0x50, crashState +0x444, racerName +0x5e0 and its enabled bit at +0x29 of a
virtual base found through the vbtable at +4. g_TrackGame (0x0056e26c) +0x2d74 is the
event type (2 and 3 hide outside racers).

Random: 0..6 racers, the followed racer present or not, crashed or not, enabled bits, the
cue racer and flag (set only with a chat overlay), a chat overlay or none, tags tracking each racer or not, mapRect.top
of 0 or -1 (retail tests top + outside), event types 0..5 and modes 0..5.

Stubbed: WorldToMap 0x0051c360 (thiscall; position noted, writes random map x/y and rim
point, returns 0, 1 or 2), ChatOverlay::SetCueRacer 0x0051d9c0 (distance bits and name
logged), NameOverlay::SetRadarAnchor 0x005190a0 (point x/y, side and outside noted; the
point's z is never written by either body), the target's DrawPrimitive (vtable +0x40;
both line vertices noted at each call), DrawTrackOutline 0x0051c720 and DrawGates
0x0051cb20.
"""
import random
import struct

RETAIL_VA = 0x0051bed0
SYMBOL = '?DrawRacers@RadarOverlay@@QAEXH@Z'
SOURCE = 'src/bringup/Group3_RadarOverlay.cpp'
TRACK_GAME = 0x0056e26c
PROBES = {0x0051bf48: 'crashed: held angle', 0x0051c061: 'SetCueRacer', 0x0051c07f: 'hidden cue',
          0x0051c0be: 'rim y', 0x0051c13f: 'cue anchor', 0x0051c1d1: 'outside dot',
          0x0051c1f6: 'inside dot', 0x0051c2fa: 'disabled anchor', 0x0051c32f: 'outline',
          0x0051c349: 'gates'}


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def setup(ctx, seed):
    r = random.Random(seed)
    game = ctx.alloc(0x2d80, fill=0x47)
    ctx.pack(TRACK_GAME, 'I', game)
    ctx.pack(game + 0x2d74, 'i', r.randrange(6))

    count = r.choice([0, 1, 2, 4, 6])
    racers = []
    for _ in range(count):
        racer = ctx.alloc(0x600, fill=0x52)
        vbase = r.choice([0x20, 0x100])
        vbtable = ctx.alloc(8, struct.pack('<ii', 0, vbase))
        ctx.pack(racer + 4, 'I', vbtable)
        ctx.pack(racer + 0x0c, '3f', *[r.uniform(-2000, 2000) for _ in range(3)])
        ctx.pack(racer + 0x50, 'f', r.choice([0.0, r.uniform(-7, 7)]))
        ctx.pack(racer + vbase + 0x29, 'B', r.choice([0, 1, 1, 1, 3]))
        ctx.pack(racer + 0x444, 'i', r.choice([0, 0, 1]))
        ctx.write(racer + 0x5e0, b'Racer%d\0' % len(racers))
        racers.append(racer)

    radar = ctx.alloc(0x220, fill=0x3a)
    target = ctx.alloc(0x10, fill=0)
    vtable = ctx.alloc(0x80, fill=0)
    ctx.pack(target, 'I', vtable)
    ctx.pack(radar + 0x18, 'I', target)

    def draw(cx, ecx, a):           # (type, fvf, vertices, count, flags)
        cx.note('DrawPrimitive', cx.read(a[2], 0x40).hex())
        return 0
    ctx.pack(vtable + 0x40, 'I', ctx.stub('DrawPrimitive', pop=20, ret=draw))

    view = ctx.alloc(0x200, fill=0x56)
    chat = 0
    if r.random() < 0.8:
        chat = ctx.alloc(0x2d8 + 4 * max(count, 1), fill=0x43)
        for k in range(count):
            tag = ctx.alloc(0x120, fill=0x54)
            ctx.pack(tag + 0x11c, 'I', r.choice(racers + [0]) if r.random() < 0.5 else racers[k])
            ctx.pack(chat + 0x2d8 + 4 * k, 'I', tag)
    ctx.pack(view + 0x19c, 'I', chat)
    camera = ctx.alloc(0x400, fill=0x63)
    ctx.pack(camera + 0x3b0, 'I', r.choice(racers) if racers and r.random() < 0.85 else 0x0badbad0)
    ctx.pack(radar + 0x12c, 'II', view, camera)
    ctx.pack(radar + 0x134, '%dI' % count, *racers)
    ctx.pack(radar + 0x160, 'i', count)
    # cue index, the previous frame's followed index (a valid slot) and the cue flag; the cue
    # path dereferences the chat overlay's tags unchecked, as in the game where a cue implies one
    ctx.pack(radar + 0x16c, 'iii', r.randrange(-1, max(count, 1)), r.randrange(max(count, 1)),
             r.choice([0, 1, 1]) if chat else 0)
    ctx.pack(radar + 0x17c, '3f', *[r.uniform(-100, 100) for _ in range(3)])
    ctx.pack(radar + 0x188, '4f', r.uniform(-3, 3), r.uniform(-3, 3), 9.0, 9.0)
    ctx.pack(radar + 0x1a0, 'i', r.randrange(0, 200))
    ctx.write(radar + 0x1b8, bytes(r.randrange(256) for _ in range(0x40)))
    ctx.pack(radar + 0x1fc, 'i', r.choice([0, -1, 50]))

    def world_to_map(cx, ecx, a):   # (Vector3 position, int* x, int* y, float* rimX, float* rimY)
        cx.note('WorldToMap', ecx, *a[:3])
        cx.pack(a[3], 'i', r.randrange(-50, 250))
        cx.pack(a[4], 'i', r.randrange(-50, 250))
        cx.pack(a[5], 'f', r.uniform(-50, 250))
        cx.pack(a[6], 'f', r.uniform(-50, 250))
        return r.choice([0, 0, 1, 2])

    def anchor(cx, ecx, a):         # (Vector3 point, int right, int outside)
        cx.note('SetRadarAnchor', ecx, a[0], a[1], a[3], a[4])
        return 0

    ctx.stub_at(0x0051c360, 'WorldToMap', pop=28, ret=world_to_map, log=False)
    ctx.stub_at(0x0051d9c0, 'SetCueRacer', pop=8)
    ctx.stub_at(0x005190a0, 'SetRadarAnchor', pop=20, ret=anchor, log=False)
    ctx.stub_at(0x0051c720, 'DrawTrackOutline', pop=4)
    ctx.stub_at(0x0051cb20, 'DrawGates')
    return dict(ecx=radar, args=[r.randrange(6)], ret_eax=False)
