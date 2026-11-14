"""RecorderWorkerThread 0x004e6f80, the VCR (replay) tape worker thread (tools/emu_diff.py
case, call-record/replay mode).

The linked body is samples/race/RecorderWorker.cpp. The parameter block holds the header,
its size, the tape file name and the VCRInterface, whose +0x30..+0x58 are the eleven events,
+0x60 the lock, +0xac the tape file, +0xb0 the frame ring and +0xb8 the busy flag (set in
some runs). Each run plays a random script
of WaitForMultipleObjects results (every event 0..10, an out-of-range value) ending in
WAIT_FAILED, so the write, create, reopen, forward and reverse read, seek, rewind, stop,
close and kill paths run in sequence with the handle and flags they leave behind.

Every call is replayed. Imports are left out of the log and noted by name with their
arguments (WaitForMultipleObjects with the event array), because a body may call one through
its IAT slot or through a register loaded from it. The ring's and the file's predicates and
results are scripted per call: Open fails or returns a handle, Read and Write succeed or
fail (a successful Read fills the buffer, so a record size can exceed 0x400), the position
queries return 0, -1 or a position, and ClaimReadSlot hands out a real buffer (the reverse
path stores 0xff through it). _endthreadex is replayed.
"""
import random

RETAIL_VA = 0x004e6f80
SYMBOL = '?RecorderWorkerThread@@YGIPAX@Z'
SOURCE = 'samples/race/RecorderWorker.cpp'
PROBES = {0x004e7055: 'write', 0x004e70f3: 'create', 0x004e7148: 'reopen', 0x004e71f9: 'read forward',
          0x004e7327: 'open playback', 0x004e73f9: 'read reverse', 0x004e757e: 'seek',
          0x004e75eb: 'rewind', 0x004e7650: 'stop reading', 0x004e7686: 'close', 0x004e774b: 'kill'}

# IAT slot -> (name, stack arguments, the slot's unbound contents a register call reaches)
IMPORTS = {
    0x005500d8: ('WaitForMultipleObjects', 4, 0x001650b0),
    0x005500e0: ('ResetEvent', 1, 0x0016510e),
    0x005500e8: ('SetEvent', 1, 0x001650a4),
    0x0055025c: ('EnterCriticalSection', 1, 0x00165066),
    0x00550284: ('LeaveCriticalSection', 1, 0x0016504e),
}
CLAIM_READ_SLOT, OPEN, READ, WRITE = 0x00524390, 0x00524dd0, 0x00525070, 0x005251f0
PREDICATES = (0x00524350, 0x00524560, 0x005254a0)        # HasSlotToWrite, HasRoomToRead, IsEndOfFile
POSITIONS = (0x005246c0, 0x005247c0)                     # StepPositionBack, PeekPosition


def _import(name, count):
    def call(ctx, rec):
        args = list(rec.args[:count])
        if name == 'WaitForMultipleObjects':
            ctx.note(name, args[0], args[2], args[3], ctx.read(args[1], 4 * min(args[0], 16)))
            script = ctx.data['script']
            return script[rec.occurrence] if rec.occurrence < len(script) else 0xffffffff
        ctx.note(name, *args)
        return 1
    return call


def _bool(ctx, rec):
    return int(rec.rng.random() < 0.6)


def _open(ctx, rec):
    return 0xffffffff if rec.rng.random() < 0.2 else 3 + rec.occurrence


def _read(ctx, rec):
    # Read(buffer, size, count, handle)
    if rec.rng.random() < 0.25:
        return 0
    n = rec.args[1] * rec.args[2]
    if n == 4:
        data = rec.rng.choice([rec.rng.randrange(1, 0x400), 0x400, 0x401, rec.rng.getrandbits(32)])
        ctx.pack(rec.args[0], 'I', data)
    elif 0 < n <= 0x1000:
        ctx.write(rec.args[0], bytes(rec.rng.getrandbits(8) for _ in range(min(n, 64))))
    return 1


def _write(ctx, rec):
    return int(rec.rng.random() < 0.8)


def _claim(ctx, rec):
    # ClaimReadSlot(size, &index, &data)
    ctx.pack(rec.args[1], 'i', rec.occurrence)
    ctx.pack(rec.args[2], 'I', ctx.alloc(0x410, fill=0x52))
    return 1


def _position(ctx, rec):
    if rec.args and rec.args[0]:
        ctx.pack(rec.args[0], 'i', rec.rng.randrange(100000))
    return rec.rng.choice([0, 0xffffffff, rec.rng.randrange(1, 1 << 20)])


returns = {OPEN: _open, READ: _read, WRITE: _write, CLAIM_READ_SLOT: _claim}
returns.update({va: _bool for va in PREDICATES})
returns.update({va: _position for va in POSITIONS})
pops, args, ecx, unlogged = {}, {CLAIM_READ_SLOT: 3}, {}, []
for slot, (name, count, entry) in IMPORTS.items():
    for key in (('iat', slot), entry):
        returns[key] = _import(name, count)
        pops[key] = 4 * count
        args[key] = count
        ecx[key] = False
        unlogged.append(key)
REPLAY = dict(returns=returns, pops=pops, args=args, ecx=ecx, unlogged=tuple(unlogged),
              out_writes=True)


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.getrandbits(32), busy=int(rng.random() < 0.2),
                   script=[rng.choice(list(range(11)) + [0, 3, 5, 11]) for _ in range(rng.randrange(1, 14))])


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.data['script'] = c['script']
    vcr = ctx.alloc(0x100, bytes(r.getrandbits(8) for _ in range(0x100)))
    for k in range(11):
        ctx.pack(vcr + 0x30 + 4 * k, 'I', 0x100 + k)
    ctx.pack(vcr + 0xb8, 'i', c['busy'])                    # isBusy: the read events skip
    ctx.pack(vcr + 0xac, 'I', ctx.alloc(0x240, fill=0x46))
    ctx.pack(vcr + 0xb0, 'I', ctx.alloc(0x1020, fill=0x52))
    header = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
    name = ctx.alloc(32, b'tape%d.vcr\0' % r.randrange(10))
    params = ctx.alloc(0x10)
    ctx.pack(params, 'IiII', header, r.choice([0x20, 0x40]), name, vcr)
    return dict(args=[params])
