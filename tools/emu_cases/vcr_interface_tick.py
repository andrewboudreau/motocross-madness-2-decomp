"""VCRInterface::Tick 0x004e7d60, slot 10 (shared by KrustyVCR): the playback step (emu_diff case).

The candidate is samples/race/RecorderTick.cpp. Offsets from the retail body: the critical
section +0x60 (Enter/LeaveCriticalSection through their IAT slots 0x0055025c/0x00550284, which
point at stubs noting the section pointer), recordMode +0x78 (1 = playback), the record buffer
+0x7c (time float, a word, count), the cdecl callback +0x80 (kind, data, flag, id, &keep,
timeMs, &milliseconds), recordPending +0x84, readingReverse +0x88, recordType/recordId/
recordPosition +0xa0/+0xa4/+0xa8, the frame ring +0xb0, fileReady +0xbc, readsReset +0xc0,
reverseReadRestarted +0xc4 and recordTimeMs +0xd4 (the (int)(time * 1000) by retail __ftol).

Each run plays a random script. TakeRecord 0x004e8810 (thiscall, 6 arguments, pop 24) answers the
next scripted result (0..4; 4 once the script ends, which ends every loop) and for 0 writes a
random record: type (-1 = tick record, or 1/10/13), id, ring position and a header with a time
and a count of 0..4 records. The callback answers the next scripted result (0, 1, 2, 3, 5, 7,
11, 12; 0 once the script ends) and writes random keep/milliseconds; it is noted by its arguments only (ECX is scratch at
this cdecl call; the two out-pointers are stack slots, compared as 'stack'). StopReading 0x004e8990,
RestartReverseRead 0x004e89c0, Seek 0x004e8a20 and Rewind 0x004e8a70 (thiscall), and the ring's
AppendPosition 0x00524590 / SeekPosition 0x00524870 are logged stubs. Random: the flags,
mode, the pending record and the scripts; 40% of the runs start with a forward tick record
and a switch answer so every arm of the answer switch runs.
"""
import random
import struct

from tools.emu_diff import _frame_token

RETAIL_VA = 0x004e7d60
SYMBOL = '?Tick@VCRInterface@@UAEHM@Z'
SOURCE = 'samples/race/RecorderTick.cpp'
ENTER_CS_IAT, LEAVE_CS_IAT = 0x0055025c, 0x00550284
PROBES = {0x004e7d80: 'file not ready', 0x004e7dd0: 'restart reverse', 0x004e7e01: 'reverse loop',
          0x004e7f19: 'forward', 0x004e8190: 'answer switch', 0x004e84b8: 'answer 1', 0x004e81b0: 'answer 2',
          0x004e859c: 'answer 7', 0x004e8690: 'append end marker'}


def cases(rng, n):
    for _ in range(n):
        takes = [rng.choice([0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4]) for _ in range(rng.choice([0, 1, 3, 8, 14]))]
        answers = [rng.choice([0, 1, 1, 2, 2, 3, 5, 7, 11, 11, 12]) for _ in range(rng.choice([0, 1, 3, 8, 14]))]
        c = dict(seed=rng.randrange(1 << 30), ready=rng.random() < 0.9,
                 mode=rng.choice([1, 1, 1, 0, 2]), reverse=rng.random() < 0.3,
                 reset=rng.random() < 0.3, restarted=rng.random() < 0.7,
                 pending=rng.random() < 0.3, takes=takes, answers=answers, tick=False)
        if rng.random() < 0.4:  # a forward tick record reaches the answer switch
            c.update(ready=True, mode=1, reverse=False, pending=False, takes=[0] + takes, tick=True,
                     answers=[rng.choice([1, 2, 2, 3, 5, 7, 11, 12])] + answers)
        yield c


def record(rng):
    return struct.pack('<fii', rng.choice([0.0, 0.25, rng.uniform(0, 600)]), rng.randrange(1 << 16),
                       rng.choice([0, 1, 2, 4, rng.randrange(5)]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    vcr = ctx.alloc(0x100, bytes(r.randrange(256) for _ in range(0x100)))
    buf = ctx.alloc(0x400, record(r) + bytes(0x400 - 12))
    ring = ctx.alloc(0x40)
    ctx.pack(vcr + 0x78, 'iI', c['mode'], buf)
    ctx.pack(vcr + 0x84, 'ii', int(c['pending']), int(c['reverse']))
    ctx.pack(vcr + 0xa0, 'i', r.choice([-1, 1, 10]))
    ctx.pack(vcr + 0xb0, 'I', ring)
    ctx.pack(vcr + 0xbc, 'iii', int(c['ready']), int(c['reset']), int(c['restarted']))
    ctx.pack(vcr + 0xd4, 'i', r.randrange(1 << 20))
    ctx.pack(ENTER_CS_IAT, 'I', ctx.stub('EnterCriticalSection', pop=4, ret=lambda cx, e, a: cx.note('Enter', *a), log=False))
    ctx.pack(LEAVE_CS_IAT, 'I', ctx.stub('LeaveCriticalSection', pop=4, ret=lambda cx, e, a: cx.note('Leave', *a), log=False))

    takes = list(c['takes'])
    answers = list(c['answers'])

    def take(cx, e, a):
        first = len(takes) == len(c['takes'])
        res = takes.pop(0) if takes else 4
        if res == 0:
            cx.pack(a[0], 'i', -1 if first and c['tick'] else cx.rng.choice([-1, -1, -1, 1, 10, 13]))
            cx.pack(a[1], 'i', cx.rng.randrange(8))
            cx.pack(a[2], 'i', cx.rng.randrange(1 << 16))
            cx.write(a[3], record(cx.rng))
            cx.pack(a[4], 'I', 12 + cx.rng.randrange(64))
        return res
    ctx.stub_at(0x004e8810, 'VCRInterface::TakeRecord', pop=24, ret=take)

    def callback(cx, e, a):
        cx.note('recordCallback', *[_frame_token(v) for v in a])
        cx.pack(a[4], 'i', cx.rng.randrange(2))
        cx.pack(a[6], 'i', cx.rng.randrange(1 << 20))
        return answers.pop(0) if answers else 0
    ctx.pack(vcr + 0x80, 'I', ctx.stub('recordCallback', pop=0, nargs=7, ret=callback, log=False))
    ctx.stub_at(0x004e8990, 'VCRInterface::StopReading', pop=0)
    ctx.stub_at(0x004e89c0, 'VCRInterface::RestartReverseRead', pop=0)
    ctx.stub_at(0x004e8a20, 'VCRInterface::Seek', pop=4)
    ctx.stub_at(0x004e8a70, 'VCRInterface::Rewind', pop=0)
    ctx.stub_at(0x00524590, 'UnknownVcr::AppendPosition', pop=8)
    ctx.stub_at(0x00524870, 'UnknownVcr::SeekPosition', pop=4)
    return dict(ecx=vcr, args=[r.uniform(0, 0.1)], ret_eax=True)
