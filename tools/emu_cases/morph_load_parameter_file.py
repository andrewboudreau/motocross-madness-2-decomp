"""MorphBastardModifier::LoadParameterFile 0x004a33b0, the rider morph (.mbf) loader
(tools/emu_diff.py case, call-record/replay mode).

The linked body is src/bringup/Group0_KbMorphMod.cpp. The .mbf file is a parameter-block file
read through UnknownParameterBlock: SelectSection 0x004b78f0, GetInt 0x004b7f10, ReadString
0x004b7b30, ReadFloat 0x004b7e70 (answers whether the key exists), BeginRows 0x004b7f70,
ReadRow 0x004b8010, GetRowInt 0x004b8180 and GetRowFloat 0x004b81c0. Each run builds a synthetic
file: NumberOfMorphBastards (0..3, negative, missing), per object an ObjectName (known, unknown
or missing) and NumberOfTargets (0..5, negative), per target a ControllerName drawn from a
small set (so controllers and axes repeat and the channel-merge arm runs) and any subset of
XAxis/YAxis/ZAxis (negative, zero, fractional), and per delta section 0..4 rows (negative
indices, missing columns). The stubs note the section and key order. FindByName 0x004fdae0
answers a node per known name (0 otherwise) and GetMatrixIn 0x004fca80 hands back that node's
random rigid-ish matrix, so both inverse expansions run on varied floats. operator new,
DebugMalloc 0x004a2e20 and DebugRealloc 0x004a2ec0 are modelled (realloc copies) and note
size and line; sprintf is modelled; retail's qsort 0x00534426 runs natively with retail's
comparator 0x004a3bb0 in both runs. fs:[0] is page 0.
"""
import random
import struct

RETAIL_VA = 0x004a33b0
SYMBOL = '?LoadParameterFile@MorphBastardModifier@@QAEPAV1@PAXPBDPAVSoultreeObject@@@Z'
SOURCE = 'src/bringup/Group0_KbMorphMod.cpp'
PROBES = {0x004a34f4: 'object', 0x004a36ef: 'merge', 0x004a376e: 'new channel',
          0x004a3a7d: 'row', 0x004a3b15: 'qsort'}

NEW, DEBUG_MALLOC, DEBUG_REALLOC, SPRINTF, QSORT = 0x004a3010, 0x004a2e20, 0x004a2ec0, 0x00534aaf, 0x00534426
STREAM_CTOR, BLOCK_CTOR = 0x00460d10, 0x004b7130
SELECT, GET_INT, READ_STRING, READ_FLOAT = 0x004b78f0, 0x004b7f10, 0x004b7b30, 0x004b7e70
BEGIN_ROWS, READ_ROW, ROW_INT, ROW_FLOAT = 0x004b7f70, 0x004b8010, 0x004b8180, 0x004b81c0
FIND_BY_NAME, GET_MATRIX_IN = 0x004fdae0, 0x004fca80

NAMES = ('Head', 'Torso', 'LArm', 'RArm', 'Pelvis')


def _cstr(ctx, at, limit=256):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _section(ctx):
    return ctx.data['file'].get(ctx.data['section'], {})


def _select(ctx, rec):
    name = _cstr(ctx, rec.args[0]).decode('latin-1')
    ctx.data['section'] = name
    ctx.note('SelectSection', name)
    return int(name in ctx.data['file'])


def _get_int(ctx, rec):
    key = _cstr(ctx, rec.args[0]).decode('latin-1')
    value = _section(ctx).get(key)
    ctx.note('GetInt', key, rec.args[1])
    ctx.pack(rec.args[2], 'i', rec.args[1] if value is None else value)
    return int(value is not None)


def _read_string(ctx, rec):
    key = _cstr(ctx, rec.args[0]).decode('latin-1')
    value = _section(ctx).get(key)
    ctx.note('ReadString', key, rec.args[2] & 0xffffffff)
    if value is None:
        return 0
    ctx.write(rec.args[1], value.encode('latin-1') + b'\0')
    return 1


def _read_float(ctx, rec):
    key = _cstr(ctx, rec.args[0]).decode('latin-1')
    value = _section(ctx).get(key)
    ctx.note('ReadFloat', key)
    if value is None:
        return 0
    ctx.pack(rec.args[1], 'f', value)
    return 1


def _begin_rows(ctx, rec):
    name = _cstr(ctx, rec.args[0]).decode('latin-1')
    ctx.note('BeginRows', name)
    rows = ctx.data['file'].get(name, {}).get('rows')
    ctx.data['rows'] = rows or []
    ctx.data['row'] = -1
    return len(rows) if rows is not None else ctx.data['missing_rows']


def _read_row(ctx, rec):
    ctx.data['row'] += 1
    ctx.note('ReadRow', ctx.data['row'])
    return int(ctx.data['row'] < len(ctx.data['rows']))


def _row_value(ctx, column):
    rows, k = ctx.data['rows'], ctx.data['row']
    if 0 <= k < len(rows) and column < len(rows[k]):
        return rows[k][column]
    return None


def _row_int(ctx, rec):
    value = _row_value(ctx, rec.args[0])
    ctx.note('GetRowInt', rec.args[0])
    if value is None:
        return 0
    ctx.pack(rec.args[1], 'i', int(value))
    return 1


def _row_float(ctx, rec):
    value = _row_value(ctx, rec.args[0])
    ctx.note('GetRowFloat', rec.args[0])
    if value is None:
        return 0
    ctx.pack(rec.args[1], 'f', float(value))
    return 1


def _find(ctx, rec):
    name = _cstr(ctx, rec.args[0]).decode('latin-1')
    ctx.note('FindByName', rec.ecx, name)
    return ctx.data['nodes'].get(name, 0)


def _matrix_in(ctx, rec):
    ctx.note('GetMatrixIn', rec.ecx, rec.args[0])
    m = ctx.data['matrices'].get(rec.ecx)
    if m is not None:
        ctx.write(rec.args[1], struct.pack('<16f', *m))
    return 0


def _alloc(ctx, size):
    if size <= 0 or size > 0x100000:
        size = 16
    at = ctx.alloc(size, fill=0xcd)
    ctx.data['sizes'][at] = size
    return at


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return _alloc(ctx, rec.args[0])


def _malloc(ctx, rec):
    ctx.note('DebugMalloc', rec.args[0], rec.args[2])
    return _alloc(ctx, rec.args[0])


def _realloc(ctx, rec):
    old, size = rec.args[0], rec.args[1]
    ctx.note('DebugRealloc', old != 0, size, rec.args[3])
    at = _alloc(ctx, size)
    if old:
        keep = min(ctx.data['sizes'].get(old, 0), ctx.data['sizes'][at])
        ctx.write(at, ctx.read(old, keep))
    return at


def _sprintf(ctx, rec):
    out, fmt, rest = rec.args[0], _cstr(ctx, rec.args[1]), list(rec.args[2:])
    text, k, i = b'', 0, 0
    while i < len(fmt):
        if fmt[i:i + 1] == b'%' and i + 1 < len(fmt):
            value = rest[k]
            k += 1
            text += str(value - (1 << 32) if value & 0x80000000 else value).encode()
            i += 2
        else:
            text += fmt[i:i + 1]
            i += 1
    ctx.write(out, text + b'\0')
    ctx.note('sprintf', text)
    return len(text)


def _this(ctx, rec):
    return rec.ecx


returns = {NEW: _new, DEBUG_MALLOC: _malloc, DEBUG_REALLOC: _realloc, SPRINTF: _sprintf,
           STREAM_CTOR: _this, BLOCK_CTOR: _this, SELECT: _select, GET_INT: _get_int,
           READ_STRING: _read_string, READ_FLOAT: _read_float, BEGIN_ROWS: _begin_rows,
           READ_ROW: _read_row, ROW_INT: _row_int, ROW_FLOAT: _row_float, FIND_BY_NAME: _find,
           GET_MATRIX_IN: _matrix_in}
pops = {NEW: 0, DEBUG_MALLOC: 0, DEBUG_REALLOC: 0, SPRINTF: 0}
args = {NEW: 3, DEBUG_MALLOC: 3, DEBUG_REALLOC: 4, SPRINTF: 4}
ecx = {NEW: False, DEBUG_MALLOC: False, DEBUG_REALLOC: False, SPRINTF: False}
# The stubs note what matters; new/malloc/realloc carry the unit's own __FILE__ pointer.
unlogged = (NEW, DEBUG_MALLOC, DEBUG_REALLOC, SPRINTF, SELECT, GET_INT, READ_STRING, READ_FLOAT,
            BEGIN_ROWS, READ_ROW, ROW_INT, ROW_FLOAT, FIND_BY_NAME, GET_MATRIX_IN)
REPLAY = dict(returns=returns, pops=pops, args=args, ecx=ecx, unlogged=unlogged, passthrough=(QSORT,),
              out_writes=False)


def _float(rng):
    return rng.choice([0.0, 1.0, -1.0, 0.5, -0.25, 100.0, rng.uniform(-2, 2), rng.uniform(-50, 50),
                       float(rng.randrange(-5, 6))])


def cases(rng, n):
    for _ in range(n):
        f = {}
        head = {}
        count = rng.choice([0, 1, 1, 2, 2, 3, -1, None])
        if count is not None:
            head['NumberOfMorphBastards'] = count
        f['MorphBastard Information'] = head
        for i in range(max(count or 0, 0)):
            obj = {}
            r = rng.random()
            if r < 0.8:
                obj['ObjectName'] = rng.choice(NAMES)
            elif r < 0.9:
                obj['ObjectName'] = 'Unknown'
            targets = rng.choice([0, 1, 2, 3, 4, 5, -2, None])
            if targets is not None:
                obj['NumberOfTargets'] = targets
            f['MorphBastard %d' % i] = obj
            controllers = rng.sample(NAMES + ('Nobody',), rng.choice([1, 2, 3]))
            for j in range(max(targets or 0, 0)):
                t = {}
                if rng.random() < 0.95:
                    t['ControllerName'] = rng.choice(controllers)
                for axis in ('XAxis', 'YAxis', 'ZAxis'):
                    if rng.random() < 0.5:
                        t[axis] = _float(rng)
                f['MorphBastard %d Target %d' % (i, j)] = t
                if rng.random() < 0.9:
                    rows = []
                    for _ in range(rng.choice([0, 1, 2, 4])):
                        rows.append([0, rng.choice([0, 7, -1, rng.randrange(500)])] +
                                    [_float(rng) for _ in range(rng.choice([3, 3, 3, 2]))])
                    f['MorphBastard %d Target %d Deltas' % (i, j)] = dict(rows=rows)
        yield dict(seed=rng.getrandbits(32), file=f, missing_rows=rng.choice([0, 0, 1]))


def _matrix(r):
    m = [r.uniform(-1, 1) for _ in range(16)]
    for k in range(3):
        m[12 + k] = r.uniform(-100, 100)
    m[3] = m[7] = m[11] = 0.0
    m[15] = 1.0
    return struct.unpack('<16f', struct.pack('<16f', *m))


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])
    ctx.data.update(file=c['file'], section='', rows=[], row=-1, sizes={}, missing_rows=c['missing_rows'],
                    nodes={}, matrices={})
    for name in NAMES:
        node = ctx.alloc(0x40, fill=0x4e)
        ctx.data['nodes'][name] = node
        ctx.data['matrices'][node] = _matrix(r)
    this = ctx.alloc(0x58, bytes(r.getrandbits(8) for _ in range(0x58)))
    model = ctx.alloc(0x40, fill=0x4d)
    path = ctx.alloc(0x20, b'RiderMorph.mbf\0')
    return dict(ecx=this, args=[0x1234, path, model], ret_eax=True)
