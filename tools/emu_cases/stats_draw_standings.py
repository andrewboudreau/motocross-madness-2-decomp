"""StatsOverlay::DrawStandings 0x00519a20, the in-race standings rows (tools/emu_diff.py case).

Linked from samples/track/StatsStandingsDraw.cpp. Offsets are retail's (0x00519a20
disassembly): g_TrackGame (0x0056e26c) +0x55c the event (raceView +0x34, elapsed minutes
and seconds +0x70/+0x74), +0x2d70 the race mode (0 and 4 have no time row), +0x2eb0 the
time limit in minutes; the view's local racer +0x38; a racer's score +0x768, name +0x5e0
and enabled bit at +0x29 of the virtual base its vbtable (+4) names. The overlay holds the
upload texture +0x2c, the shared texture +0x34 (surface +0x70), the source rect +0xf8, the
font +0x11c, row rects +0x130 (16 bytes each), the drawn rows +0x1b0 and the new rows
+0x5b0 (0x80 bytes each) and the first row's extent +0x9b0.

Random: 1..10 racers (the local one always enabled; others disabled at random) with
distinct scores, race modes 0..5, limit and elapsed times, and drawn rows that are stale,
garbage or (in some runs) the rows this call writes, so both the unchanged early return
and the redraw run. qsort (0x00534426) is retail's CRT in both runs; the comparator is
each body's own. sprintf (0x00534aaf) is stubbed (its %f path calls through CRT pointers
startup fills): the stub formats "%s" and "%d) %s : %.0f" and notes the score's double bits.

Stubbed: LoadResourceString 0x00521970 (thiscall; writes a label), FormatMinutesSeconds
0x00518640 (cdecl; the remaining-time float is noted), BikeRace::NextBike 0x004204e0
(walks the racer list through the iterator), RestoreRect 0x004b6710, TintRows 0x004b6880,
the surface's GetDC/ReleaseDC (vtable +0x44/+0x68, fail in some runs), the upload
(+0x24) and the GDI imports SetBkColor/SetBkMode/SetTextColor/SelectObject/
GetTextExtentPoint32A/DrawTextA through their IAT slots (strings noted).
"""
import random
import struct

RETAIL_VA = 0x00519a20
SYMBOL = '?DrawStandings@StatsOverlay@@QAEHXZ'
SOURCE = 'samples/track/StatsStandingsDraw.cpp'
TRACK_GAME = 0x0056e26c
IAT = {0x00550070: ('SetBkColor', 8), 0x00550074: ('SetBkMode', 8), 0x00550080: ('SetTextColor', 8),
       0x00550078: ('SelectObject', 8), 0x00550084: ('GetTextExtentPoint32A', 16),
       0x005502e8: ('DrawTextA', 20)}
PROBES = {0x00519a68: 'time row', 0x00519aeb: 'enabled racer', 0x00519bdc: 'own row',
          0x00519c08: 'next row', 0x00519c8d: 'unchanged', 0x00519cc9: 'GetDC ok',
          0x00519d35: 'extent', 0x00519dec: 'tint and upload', 0x00519dd0: 'DC failure'}


def cases(rng, n):
    for _ in range(n):
        yield rng.randrange(1 << 30)


def expected_rows(mode, label, racers, local):
    """The rows the function writes, for drawn-row presets (distinct scores, no ties)."""
    rows = [label] if mode not in (0, 4) else []
    scores = sorted([(s, name, rr) for rr, s, name, on in racers if on], key=lambda x: -x[0])
    own = [k for k, x in enumerate(scores) if x[2] == local][0]
    count = len(scores) + (1 if rows else 0)
    count = min(count, 6)
    first = 1 if rows else 0
    shown = 0
    for _ in range(first, count - 1):
        rows.append('%d) %s : %.0f' % (shown + 1, scores[shown][1], scores[shown][0]))
        shown += 1
    pick = own if own >= shown else shown
    rows.append('%d) %s : %.0f' % (pick + 1, scores[pick][1], scores[pick][0]))
    return rows


def setup(ctx, seed):
    r = random.Random(seed)
    game = ctx.alloc(0x2f00, fill=0x47)
    ctx.pack(TRACK_GAME, 'I', game)
    event = ctx.alloc(0x80, fill=0x45)
    view = ctx.alloc(0x40, fill=0x56)
    mode = r.randrange(6)
    ctx.pack(game + 0x55c, 'I', event)
    ctx.pack(game + 0x2d70, 'i', mode)
    ctx.pack(game + 0x2eb0, 'f', r.choice([3.0, 5.0, r.uniform(1, 20)]))
    ctx.pack(event + 0x34, 'I', view)
    ctx.pack(event + 0x70, '2f', float(r.randrange(10)), r.uniform(0, 60))

    count = r.randrange(1, 11)
    scores = r.sample(range(0, 50000), count)
    racers = []
    for k in range(count):
        racer = ctx.alloc(0x780, fill=0x52)
        vbase = 0x40
        ctx.pack(racer + 4, 'I', ctx.alloc(8, struct.pack('<ii', 0, vbase)))
        on = r.random() < 0.8
        ctx.pack(racer + vbase + 0x29, 'B', 1 if on else r.choice([0, 2]))
        score = scores[k] + r.choice([0.0, 0.25, 0.75])
        ctx.pack(racer + 0x768, 'f', score)
        name = 'Rider%d' % k
        ctx.write(racer + 0x5e0, name.encode() + b'\0')
        racers.append([racer, score, name, on])
    local = r.choice(racers)
    local[3] = True
    ctx.pack(local[0] + 0x40 + 0x29, 'B', 1)
    ctx.pack(view + 0x38, 'I', local[0])
    label = 'Time left %d' % r.randrange(100)

    overlay = ctx.alloc(0xa00, fill=0x4f)
    rows = expected_rows(mode, label, racers, local[0])
    preset = r.random()
    for k in range(8):
        if preset < 0.4 and k < len(rows):
            text = rows[k].encode()
        elif preset < 0.6:
            text = b'stale %d' % k
        else:
            text = bytes(r.randrange(1, 256) for _ in range(5))
        ctx.write(overlay + 0x1b0 + 0x80 * k, text + b'\0')
    if preset < 0.4 and r.random() < 0.3 and rows:
        ctx.write(overlay + 0x1b0 + 0x80 * (len(rows) - 1), b'changed\0')
    upload = ctx.alloc(0x10, fill=0)
    upload_vt = ctx.alloc(0x40, fill=0)
    ctx.pack(upload, 'I', upload_vt)
    ctx.pack(upload_vt + 0x24, 'I', ctx.stub('Upload', pop=8))
    shared = ctx.alloc(0x80, fill=0x53)
    surface = ctx.alloc(0x10, fill=0)
    surface_vt = ctx.alloc(0x80, fill=0)
    ctx.pack(surface, 'I', surface_vt)
    ctx.pack(shared + 0x70, 'I', surface)
    ctx.pack(overlay + 0x2c, 'I', upload)
    ctx.pack(overlay + 0x34, 'I', shared)
    ctx.pack(overlay + 0x11c, 'I', 0x0f0e0000)
    dc_fail = r.random() < 0.1
    release_fail = r.random() < 0.1

    def get_dc(cx, ecx, a):         # (this, HDC*)
        cx.note('GetDC', a[0])
        cx.pack(a[1], 'I', 0x0dc00000)
        return 0x88760000 if dc_fail else 0

    def release_dc(cx, ecx, a):
        cx.note('ReleaseDC', *a)
        return 1 if release_fail else 0
    ctx.pack(surface_vt + 0x44, 'I', ctx.stub('GetDC', pop=8, ret=get_dc, log=False))
    ctx.pack(surface_vt + 0x68, 'I', ctx.stub('ReleaseDC', pop=8, ret=release_dc, log=False))

    def gdi(name):
        def call(cx, ecx, a):
            if name in ('GetTextExtentPoint32A', 'DrawTextA'):
                cx.note(name, a[0], cx.read(a[1], a[2]), *a[2:])
            else:
                cx.note(name, *a)
            return 0x0f0e0001 if name == 'SelectObject' else 1
        return call
    for slot, (name, pop) in IAT.items():
        ctx.pack(slot, 'I', ctx.stub(name, pop=pop, ret=gdi(name), log=False))

    order = [x[0] for x in racers]

    def next_bike(cx, ecx, a):      # (int* iterator)
        k = cx.unpack(a[0], 'i')[0]
        if k >= len(order):
            return 0
        cx.pack(a[0], 'i', k + 1)
        return order[k]

    def load_string(cx, ecx, a):    # (id, buffer, size)
        cx.write(a[1], label.encode() + b'\0')
        return 0

    def remaining(cx, ecx, a):      # (char* out, float seconds)
        cx.note('FormatMinutesSeconds', a[1])
        cx.write(a[0], b'1:23\0')
        return 0

    def sprintf(cx, ecx, a):        # (buffer, format, ...)
        fmt = cx.read(a[1], 32).split(b'\0')[0]
        if fmt == b'%s':
            text = cx.read(a[2], 0x80).split(b'\0')[0]
            cx.note('sprintf', fmt, text)
        else:
            score = struct.unpack('<d', struct.pack('<II', a[4], a[5]))[0]
            name = cx.read(a[3], 0x40).split(b'\0')[0]
            text = b'%d) %s : %.0f' % (a[2], name, score)
            cx.note('sprintf', fmt, a[2], name, a[4], a[5])
        cx.write(a[0], text + b'\0')
        return len(text)

    ctx.stub_at(0x00534aaf, 'sprintf', nargs=6, ret=sprintf, log=False)
    ctx.stub_at(0x00521970, 'LoadResourceString', pop=12, ret=load_string)
    ctx.stub_at(0x00518640, 'FormatMinutesSeconds', nargs=2, ret=remaining, log=False)
    ctx.stub_at(0x004204e0, 'NextBike', pop=4, ret=next_bike)
    ctx.stub_at(0x004b6710, 'RestoreRect', pop=4)
    ctx.stub_at(0x004b6880, 'TintRows', pop=12)
    return dict(ecx=overlay)
