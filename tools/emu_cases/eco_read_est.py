"""EcoSystem::ReadEst 0x00457480, the .est vegetation file parser (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/bringup/Group4_EcoSystem.cpp. The .est file is an INI file read through
GetPrivateProfileStringA/IntA (IAT 0x00550204/0x00550208, the Int one also through a register)
and ReadIniFloat 0x0047b8a0; each run builds a synthetic INI: Method (NONE, Authored, AUTO,
unknown, missing), TotalObjects, a random set of Vegetation_N sections (names, NONE names,
values up to 127 characters, every float key or its default, the key colours, negative and
huge numbers), PlacementBmp and ProbabilityTga up to 127 characters, NorthAngle. The stubs
return the value or the default, truncated to the buffer size, and note (section, key) so the
order and the keys are compared. The path ends in .est, .esb, both or neither; without a stream
an .esb probe (FileStream ctor 0x00460d10, Open 0x00460f50 answering the case's
flag, dtor and delete) may rename the path in place. The EcoSystem (0x600 bytes) starts with
an empty definition table except for a few prior definitions (slot 0 among them), so the
Auto-only loop and the percent-bias normalisation run over both kinds. sprintf is modelled
(%d), _stricmp compares for real, strstr and strncpy run natively; ReadCollisionObjects
0x00458360 and ReadEsb 0x00458f70 are logged. fs:[0] is page 0.
"""
import random
import struct

RETAIL_VA = 0x00457480
SYMBOL = '?ReadEst@EcoSystem@@QAEHPADPAVFileStream@@@Z'
SOURCE = 'src/bringup/Group4_EcoSystem.cpp'
PROBES = {0x00457605: 'est', 0x00457762: 'definition', 0x00457a84: 'normalise', 0x00457b85: 'auto',
          0x00457e82: 'esb', 0x0045759c: 'probe'}

GET_STRING, GET_INT = 0x00550204, 0x00550208
GET_STRING_RAW, GET_INT_RAW = 0x00164e3a, 0x00164e22
READ_INI_FLOAT, SPRINTF, STRICMP = 0x0047b8a0, 0x00534aaf, 0x00536070
NEW, DELETE = 0x004a3010, 0x004a30c0
STREAM_CTOR, STREAM_OPEN, STREAM_DTOR = 0x00460d10, 0x00460f50, 0x00460d60
DEF_CTOR = 0x00455de0
STRSTR, STRNCPY = 0x005362c0, 0x00534b10

FLOAT_KEYS = ('MeanHeight', 'MinHeight', 'MaxHeight', 'MeanRadius', 'MinRadius', 'MaxRadius', 'ULeft',
              'URight', 'VBottom', 'VTop', 'UCenter', 'PercentBias', 'MeanSlope', 'StandardDeviationSlope',
              'MeanAspect', 'StandardDeviationAspect', 'MeanDrainage', 'StandardDeviationDrainage',
              'MeanAltitude', 'StandardDeviationAltitude')
INT_KEYS = ('UsePlanarLighting', 'BlendLODs', 'KeyColorRed', 'KeyColorGreen', 'KeyColorBlue')


def _cstr(ctx, at, limit=512):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _lookup(ctx, section, key):
    return ctx.data['ini'].get(section.decode('latin-1').lower(), {}).get(key.decode('latin-1').lower())


def _get_string(ctx, rec):
    section, key, default = (_cstr(ctx, a) for a in rec.args[:3])
    out, size = rec.args[3], rec.args[4]
    ctx.note('GetPrivateProfileString', section, key, default, size, _cstr(ctx, rec.args[5]))
    value = _lookup(ctx, section, key)
    text = default if value is None else value.encode('latin-1')
    text = text[:max(size - 1, 0)]
    ctx.write(out, text + b'\0')
    return len(text)


def _get_int(ctx, rec):
    section, key = (_cstr(ctx, a) for a in rec.args[:2])
    ctx.note('GetPrivateProfileInt', section, key, rec.args[2], _cstr(ctx, rec.args[3]))
    value = _lookup(ctx, section, key)
    if value is None:
        return rec.args[2]
    try:
        return int(value) & 0xffffffff
    except ValueError:
        return 0


def _read_float(ctx, rec):
    # float ReadIniFloat(const char* section, const char* key, double default, const char* path)
    section, key = (_cstr(ctx, a) for a in rec.args[:2])
    default = struct.unpack('<d', struct.pack('<II', rec.args[2], rec.args[3]))[0]
    ctx.note('ReadIniFloat', section, key, default, _cstr(ctx, rec.args[4]))
    value = _lookup(ctx, section, key)
    f = default if value is None else float(value)
    return 0, struct.unpack('<f', struct.pack('<f', f))[0]


def _sprintf(ctx, rec):
    out, fmt, rest = rec.args[0], _cstr(ctx, rec.args[1]), list(rec.args[2:])
    text, k, i = b'', 0, 0
    while i < len(fmt):
        if fmt[i:i + 1] == b'%' and i + 1 < len(fmt):
            value = rest[k]
            k += 1
            if fmt[i + 1:i + 2] == b's':
                text += _cstr(ctx, value, 256)
            else:
                text += str(value - (1 << 32) if value & 0x80000000 else value).encode()
            i += 2
        else:
            text += fmt[i:i + 1]
            i += 1
    ctx.write(out, text + b'\0')
    ctx.note('sprintf', fmt, text)
    return len(text)


def _stricmp(ctx, rec):
    a, b = _cstr(ctx, rec.args[0]).lower(), _cstr(ctx, rec.args[1]).lower()
    return 0 if a == b else (0xffffffff if a < b else 1)


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return ctx.alloc(max(rec.args[0], 1), fill=0xcd)


def _this(ctx, rec):
    return rec.ecx


def _open(ctx, rec):
    ctx.note('Open', _cstr(ctx, rec.args[0]), _cstr(ctx, rec.args[1]), rec.args[2])
    return int(ctx.data['open'])


returns = {GET_STRING: _get_string, ('iat', GET_STRING): _get_string, GET_STRING_RAW: _get_string,
           GET_INT: _get_int, ('iat', GET_INT): _get_int, GET_INT_RAW: _get_int,
           READ_INI_FLOAT: _read_float, SPRINTF: _sprintf, STRICMP: _stricmp, NEW: _new,
           DEF_CTOR: _this, STREAM_CTOR: _this, STREAM_OPEN: _open}
pops = {SPRINTF: 0, STRICMP: 0, NEW: 0, DELETE: 0, READ_INI_FLOAT: 0}
args = {SPRINTF: 3, STRICMP: 2, NEW: 3, READ_INI_FLOAT: 5}
ecx = {SPRINTF: False, STRICMP: False, NEW: False, READ_INI_FLOAT: False}
unlogged = [SPRINTF, STRICMP, READ_INI_FLOAT, NEW]   # new: the __FILE__ tag is the unit's own
for key, (n, raw) in ((GET_STRING, (6, GET_STRING_RAW)), (GET_INT, (4, GET_INT_RAW))):
    for k in (('iat', key), raw):
        pops[k] = 4 * n
        args[k] = n
        ecx[k] = False
        unlogged.append(k)
REPLAY = dict(returns=returns, pops=pops, args=args, ecx=ecx, unlogged=tuple(unlogged),
              float_returns=(READ_INI_FLOAT,), passthrough=(STRSTR, STRNCPY))


def _number(rng):
    return rng.choice(['0', '1', '-1', '100', '45', '180', '0.5', '-12.75', '1e3', '255', '256',
                       '%d' % rng.randrange(-300, 300), '%.4f' % rng.uniform(-500, 500), '2147483647'])


def _name(rng, limit=140):
    n = rng.choice([0, 1, 8, 20, 126, 127, 128, limit])
    return ''.join(rng.choice('abcdefgXYZ_0123456789.') for _ in range(n))


def cases(rng, n):
    for _ in range(n):
        ini = {}
        eco = {}
        method = rng.choice(['Authored', 'Auto', 'AUTO', 'authored', 'NONE', 'Other', None, 'Auto', 'Auto'])
        if method is not None:
            eco['method'] = method
        if rng.random() < 0.7:
            eco['totalobjects'] = _number(rng)
        for key in ('placementbmp', 'probabilitytga'):
            if rng.random() < 0.7:
                eco[key] = rng.choice(['NONE', _name(rng)])
        if rng.random() < 0.5:
            eco['northangle'] = _number(rng)
        ini['ecosystem'] = eco
        for index in rng.sample(range(0, 256), rng.choice([0, 1, 3, 6])) + rng.choice([[], [1], [255]]):
            sec = {}
            if rng.random() < 0.9:
                sec['name'] = rng.choice(['NONE', _name(rng), 'Pine', 'Bush'])
            if rng.random() < 0.7:
                sec['billboardname'] = _name(rng)
            for key in FLOAT_KEYS + INT_KEYS:
                if rng.random() < 0.5:
                    sec[key.lower()] = _number(rng)
            if rng.random() < 0.5:
                sec['probabilitytga'] = rng.choice(['NONE', _name(rng)])
            ini['vegetation_%d' % index] = sec
        if rng.random() < 0.2:      # biases summing to exactly 100
            secs = [s for k, s in ini.items() if k != 'ecosystem' and s.get('name', 'NONE') != 'NONE'
                    and k != 'vegetation_0']
            for s in secs:
                s['percentbias'] = '%g' % (100.0 / len(secs)) if len(secs) in (1, 2, 4, 5) else '50'
        path = rng.choice(['tracks\\a.est', 'b.esb', 'x.est.esb', 'noext', 'c.EST', 'd.est'])
        yield dict(seed=rng.getrandbits(32), ini=ini, path=path, stream=rng.random() < 0.3,
                   open=rng.random() < 0.5, prior=rng.sample(range(256), rng.choice([0, 0, 1, 3])))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])
    ctx.data['ini'] = c['ini']
    ctx.data['open'] = c['open']
    eco = ctx.alloc(0x600, bytes(r.getrandbits(8) for _ in range(0x600)))
    ctx.write(eco + 0x58, bytes(0x400))
    for k in c['prior']:
        d = ctx.alloc(0x210, bytes(r.getrandbits(8) for _ in range(0x210)))
        ctx.pack(d + 0x200, 'f', r.choice([0.0, 1.0, 25.0, r.uniform(0, 100)]))   # percentBias
        ctx.pack(eco + 0x58 + 4 * k, 'I', d)
    path = ctx.alloc(0x200, c['path'].encode() + b'\0' * (0x200 - len(c['path'])))
    stream = ctx.alloc(0x140, fill=0x53) if c['stream'] else 0
    return dict(ecx=eco, args=[path, stream])
