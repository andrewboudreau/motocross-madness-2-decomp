"""MPBikeRiderDlg::FillBikeRiderLists 0x004f8220, the multiplayer bike and rider list filler
(tools/emu_diff.py case, call-record/replay mode).

The linked body is src/bringup/Group2_MPBikeRiderDlg.cpp. g_TrackGame (0x0056e26c) carries the
KrustyUI at +0x56c (manufacturers +0x48, 200-byte records with the kind at +0xc4; bikes +0x50,
0x94-byte records: model +0, name +4, engine-size-selectable +0x88, engine size +0x8c; bike count
+0x54; riders +0x58, 200-byte records: name +0, model name +0x40, UI model +0xc0, kind +0xc4;
rider count +0x5c), the lobby preset flags +0x214c (0, 4, 8, 12, others), the bike unlock flags
+0x1ee8, the network object +0x3410 (manufacturer +0x298), the chosen bike +0x1fb0 and rider
name +0x1f6c. Engine sizes straddle every g_BikeClassEngineSizes bound (125/250/400/500/600),
the counts include zero and negative, names run up to 40 characters and the rider names repeat
or match the chosen one. FindControl 0x0046ebf0 answers controls whose +0x1fc list is the
case's; RemoveAllRows, AddRow (noted with its text), SelectRowByData, SelectRow and Sort are
logged; sprintf ("%s %s") is modelled.
"""
import random

RETAIL_VA = 0x004f8220
SYMBOL = '?FillBikeRiderLists@MPBikeRiderDlg@@QAEXXZ'
SOURCE = 'src/bringup/Group2_MPBikeRiderDlg.cpp'
PROBES = {0x004f82c6: 'plain', 0x004f830e: 'by model', 0x004f8399: 'by class', 0x004f8429: 'refused',
          0x004f845a: 'select bike', 0x004f8529: 'rider match'}

FIND_CONTROL, ADD_ROW, SPRINTF = 0x0046ebf0, 0x00476d80, 0x00534aaf
ENGINE = (-1, 0, 50, 125, 126, 249, 250, 251, 300, 399, 400, 401, 499, 500, 501, 599, 600, 601, 1000)


def _cstr(ctx, at, limit=256):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _sprintf(ctx, rec):
    out, a, b = rec.args[0], _cstr(ctx, rec.args[2]), _cstr(ctx, rec.args[3])
    text = a + b' ' + b
    ctx.note('sprintf', _cstr(ctx, rec.args[1]), text)
    ctx.write(out, text + b'\0')
    return len(text)


def _find(ctx, rec):
    ctx.note('FindControl', _cstr(ctx, rec.args[0]), rec.args[1])
    control = ctx.alloc(0x200, fill=0x43)
    ctx.pack(control + 0x1fc, 'I', ctx.data['lists'][_cstr(ctx, rec.args[0])])
    return control


def _add_row(ctx, rec):
    ctx.note('AddRow', rec.ecx, _cstr(ctx, rec.args[0]), rec.args[1], rec.args[2])
    return 1


REPLAY = dict(returns={FIND_CONTROL: _find, ADD_ROW: _add_row, SPRINTF: _sprintf},
              pops={SPRINTF: 0}, args={SPRINTF: 4}, ecx={SPRINTF: False}, unlogged=(SPRINTF,))


def _name(rng):
    n = rng.choice([0, 1, 5, 12, 40])
    return ''.join(rng.choice('abcXYZ 09-') for _ in range(n)).encode()


def cases(rng, n):
    for _ in range(n):
        names = [_name(rng) for _ in range(4)] + [b'Joe', b'Ann']
        bikes = [dict(model=rng.choice([0, 1, 2, 7, 7, 11]), name=_name(rng), selectable=rng.choice([0, 0, 1, 2]),
                      engine=rng.choice(ENGINE)) for _ in range(rng.choice([0, 1, 3, 8, 14]))]
        riders = [dict(name=_name(rng), model=rng.choice(names), ui=rng.choice([0, 0, 0x1234]),
                       kind=rng.choice([0, 1, 2, 3, 10])) for _ in range(rng.choice([0, 1, 4, 9]))]
        yield dict(seed=rng.getrandbits(32), bikes=bikes, riders=riders,
                   kinds=[rng.choice([0, 1, 2, 3, 10, 10]) for _ in range(12)],
                   bike_count=rng.choice([len(bikes)] * 6 + [0, -1]),
                   rider_count=rng.choice([len(riders)] * 6 + [0, -3]),
                   flags=rng.choice([0, 0, 4, 8, 12, 4, 12, 1, 0x80000004]),
                   unlock=rng.choice([0, 1, 2, 3]),
                   net_model=rng.choice([0, 0, 1, 2, 7, 11]),
                   chosen=rng.choice([0, 0, 1, 2, 3, 7, -1]),
                   chosen_rider=rng.choice(names + [b'']))


def setup(ctx, c):
    r = random.Random(c['seed'])

    def noise(n):
        return bytes(r.getrandbits(8) for _ in range(n))

    manufacturers = ctx.alloc(12 * 200, noise(12 * 200))
    for k in range(12):
        ctx.write(manufacturers + 200 * k, _name(r) + b'\0')
        ctx.pack(manufacturers + 200 * k + 0xc4, 'i', c['kinds'][k])
    bikes = ctx.alloc(max(len(c['bikes']), 1) * 0x94, noise(max(len(c['bikes']), 1) * 0x94))
    for k, b in enumerate(c['bikes']):
        at = bikes + 0x94 * k
        ctx.pack(at, 'i', b['model'])
        ctx.write(at + 4, b['name'] + b'\0')
        ctx.pack(at + 0x88, 'i', b['selectable'])
        ctx.pack(at + 0x8c, 'i', b['engine'])
    riders = ctx.alloc(max(len(c['riders']), 1) * 200, noise(max(len(c['riders']), 1) * 200))
    for k, b in enumerate(c['riders']):
        at = riders + 200 * k
        ctx.write(at, b['name'] + b'\0')
        ctx.write(at + 0x40, b['model'] + b'\0')
        ctx.pack(at + 0xc0, 'I', b['ui'])
        ctx.pack(at + 0xc4, 'i', b['kind'])
    ui = ctx.alloc(0x100, noise(0x100))
    ctx.pack(ui + 0x48, 'I', manufacturers)
    ctx.pack(ui + 0x50, 'I', bikes)
    ctx.pack(ui + 0x54, 'i', c['bike_count'])
    ctx.pack(ui + 0x58, 'I', riders)
    ctx.pack(ui + 0x5c, 'i', c['rider_count'])
    net = ctx.alloc(0x300, noise(0x300))
    ctx.pack(net + 0x298, 'i', c['net_model'])
    game = ctx.alloc(0x3500, noise(0x3500))
    ctx.pack(game + 0x56c, 'I', ui)
    ctx.pack(game + 0x214c, 'I', c['flags'])
    ctx.pack(game + 0x1ee8, 'I', c['unlock'])
    ctx.pack(game + 0x3410, 'I', net)
    ctx.pack(game + 0x1fb0, 'i', c['chosen'])
    ctx.write(game + 0x1f6c, c['chosen_rider'] + b'\0')
    ctx.pack(0x0056e26c, 'I', game)
    ctx.data['lists'] = {b'DDLBikes': ctx.alloc(0x40, fill=0x4c), b'DDLRiders': ctx.alloc(0x40, fill=0x52)}
    dialog = ctx.alloc(0x400, noise(0x400))
    return dict(ecx=dialog, args=[])
