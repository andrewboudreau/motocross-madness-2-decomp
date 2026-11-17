"""ResourceManager::AddArchive 0x004e9030 (tools/emu_diff.py case, call-record/replay mode).

Opens an "RS2" archive and appends its directory to the manager: grows the item table,
appends the archive (retail does so only in the grown-storage branch), then reads each
entry (name size, name, offset, size) into a new ResourceItem and drops entries that
MergeWithExisting folds into an existing one. The linked body is in
src/reconstructed/ResourceManager.cpp.

Every callee is replayed. Modelled: operator new 0x004a3010 (fresh 0xcd blocks), the
ResourceItem constructor 0x004e8d70 (zeroed, fake vtable so Release is a logged slot
call), DebugRealloc 0x004a2ec0 (fresh block holding the old contents, or 0 when the
case makes that call fail), FileStream::Read 0x00461640 (serves the header, the count and
the entries; the case can cut it short at any read), _stricmp 0x00536070, Open 0x00460f50,
IsValidFile 0x00460e70 and MergeWithExisting 0x004e8f40 (case-chosen results). new and
DebugRealloc note size and line but not __FILE__ (the build's path differs from retail's).

Manager layout (src/reconstructed/ResourceManager.h): +0x00 item capacity, +0x04 archive
capacity, +0x08 item count, +0x0c archive count, +0x10 archives, +0x14 items.
"""
import random
import struct

RETAIL_VA = 0x004e9030
SYMBOL = '?AddArchive@ResourceManager@@QAEPAVFileStream@@PBDH@Z'
SOURCE = 'src/reconstructed/ResourceManager.cpp'

NEW, CTOR, ISVALID, OPEN, READ, STRICMP = 0x004a3010, 0x00460d10, 0x00460e70, 0x00460f50, 0x00461640, 0x00536070
REALLOC, ITEM_CTOR, MERGE = 0x004a2ec0, 0x004e8d70, 0x004e8f40
PROBES = {0x004e90e4: 'magic compare', 0x004e9132: 'grow items', 0x004e9193: 'grow archives',
          0x004e91fb: 'entry', 0x004e92e6: 'merged release', 0x004e9330: 'delete archive'}


def _st(ctx):
    return ctx.data['case']


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return ctx.alloc(max(min(rec.args[0], 0x10000), 16) + 0x20, fill=0xcd)


def _item_ctor(ctx, rec):
    ctx.write(rec.ecx, bytes(0x20))
    ctx.pack(rec.ecx, 'I', ctx.data['item_vt'])
    return rec.ecx


def _realloc(ctx, rec):
    c = _st(ctx)
    k = ctx.data['reallocs'] = ctx.data.get('reallocs', 0) + 1
    old, size = rec.args[0], rec.args[1]
    ctx.note('realloc', size, rec.args[3])
    if k in c['fail_realloc'] or size <= 0 or size > 0x10000:
        return 0
    new = ctx.alloc(size + 0x10, fill=0xab)
    keep = ctx.data.setdefault('sizes', {}).get(old, 0)
    if old and keep:
        ctx.write(new, ctx.read(old, min(keep, size)))
    ctx.data['sizes'][new] = size
    return new


def _read(ctx, rec):
    c = _st(ctx)
    k = ctx.data['reads'] = ctx.data.get('reads', 0) + 1
    dst, size, count = rec.args[0], rec.args[1], rec.args[2]
    ctx.note('read', k, size, count)
    if k == c['short_read']:
        return 0
    if k == 1:
        ctx.write(dst, c['magic'])
    elif k == 2:
        ctx.pack(dst, 'i', c['count'])
    else:
        field = (k - 3) % 4
        if field == 0:
            ctx.pack(dst, 'i', c['name_sizes'][((k - 3) // 4) % len(c['name_sizes'])])
        elif field == 1:
            ctx.write(dst, bytes((0x61 + (k + i) % 26) for i in range(max(0, min(size, 64) - 1))) + b'\0')
        else:
            ctx.pack(dst, 'i', (k * 977) & 0xffff)
    return count


def _stricmp(ctx, rec):
    def s(a):
        b = ctx.read(a, 16)
        return (b[:b.index(0)] if 0 in b else b).lower()
    a, b = s(rec.args[0]), s(rec.args[1])
    return 0 if a == b else (0xffffffff if a < b else 1)


def _merge(ctx, rec):
    c = _st(ctx)
    k = ctx.data['merges'] = ctx.data.get('merges', 0) + 1
    return 1 if k in c['merged'] else 0


REPLAY = dict(
    returns={NEW: _new, CTOR: lambda ctx, rec: rec.ecx, ITEM_CTOR: _item_ctor, REALLOC: _realloc,
             READ: _read, STRICMP: _stricmp, MERGE: _merge,
             OPEN: lambda ctx, rec: _st(ctx)['open'], ISVALID: lambda ctx, rec: _st(ctx)['valid']},
    pops={NEW: 0, REALLOC: 0, STRICMP: 0}, args={NEW: 3, REALLOC: 4, STRICMP: 2},
    ecx={NEW: False, REALLOC: False, STRICMP: False}, unlogged=(NEW, REALLOC))


def cases(rng, n):
    for _ in range(n):
        count = rng.choice([0, 1, 2, 3, rng.randint(1, 12)])
        yield dict(
            seed=rng.getrandbits(32), flags=rng.choice([0, 0, 1, 3]), valid=rng.choice([0, 1, 1]),
            open=rng.choice([0, 1, 1, 1, 1]),
            magic=rng.choice([b'RS2\0', b'RS2\0', b'rs2\0', b'RS3\0', b'RS2X']), count=count,
            short_read=rng.choice([0, 0, 0, 1, 2, rng.randint(3, 3 + 4 * max(count, 1))]),
            name_sizes=[rng.choice([1, 2, 8, 33]) for _ in range(3)],
            merged=set(rng.sample(range(1, 14), rng.choice([0, 0, 1, 3]))),
            fail_realloc=set(rng.sample(range(1, 4), rng.choice([0, 0, 0, 1]))),
            item_cap=rng.choice([0, 4, 16]), item_fill=rng.choice([0, 2, 4]),
            arch_cap=rng.choice([0, 1, 8]), arch_fill=rng.choice([0, 1]),
            null_items=rng.random() < 0.15, null_archives=rng.random() < 0.15)


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)                # fs:[0] reads page 0 (the EH frame link)
    r = random.Random(c['seed'])
    ctx.data['case'] = c
    ctx.data['item_vt'] = ctx.fake_vtable('ResourceItem', 4)
    sizes = ctx.data.setdefault('sizes', {})
    item_fill = min(c['item_fill'], c['item_cap'])
    arch_fill = min(c['arch_fill'], c['arch_cap'])
    items = 0 if c['null_items'] else ctx.alloc(4 * c['item_cap'] + 0x10,
                                                    struct.pack('<%dI' % (c['item_cap'] + 4),
                                                                *[r.getrandbits(32) for _ in range(c['item_cap'] + 4)]))
    archives = 0 if c['null_archives'] else ctx.alloc(4 * c['arch_cap'] + 0x10, fill=0x5a)
    if items:
        sizes[items] = 4 * c['item_cap']
    if archives:
        sizes[archives] = 4 * c['arch_cap']
    manager = ctx.alloc(0x40)
    ctx.pack(manager, 'iiiiII', c['item_cap'], c['arch_cap'], item_fill, arch_fill, archives, items)
    path = ctx.alloc(32, b'data\\tracks.rs2\0')
    return dict(ecx=manager, args=[path, c['flags']], ret_eax=True)
