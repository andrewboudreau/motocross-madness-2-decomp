"""RaceSound::RefillStream 0x004e4d10, the engine stream's half-buffer refill (tools/emu_diff.py
case, call-record/replay mode).

The linked body is samples/race/RaceSoundRefill.cpp. Its calls are replayed: Sound's
GetBufferPosition 0x004bcd40 (an effect writes the play and write cursors), LockBuffer
0x004bd020 (an effect hands out a fresh 44100-byte lock region), UnlockBuffer 0x004bd080 and
rand 0x00534c1b (0..32767). memcpy is inline in both bodies.

The case draws the half being filled (1, 2 or a stale value), a write cursor on both sides of
each half's thresholds, 8- or 16-bit samples, a sample with room for a whole half or one that
wraps (looping, or switching to a random idle, power-band or sputter sample of the engine's
set), and an unknown next kind. The sample buffer, the stream samples and the lock region are
filled with seeded bytes, so every copy's source, length and destination is compared.
"""
RETAIL_VA = 0x004e4d10
SYMBOL = '?RefillStream@RaceSound@@QAEXXZ'
SOURCE = 'samples/race/RaceSoundRefill.cpp'
GET_POSITION = 0x004bcd40
LOCK = 0x004bd020
RAND = 0x00534c1b
PROBES = {0x004e4d8a: 'lock first half', 0x004e511d: 'lock second half', 0x004e4eae: 'switch sample'}


def _position(ctx, rec):
    # GetBufferPosition(&play, &write)
    ctx.pack(rec.args[0], 'I', ctx.data['play'])
    ctx.pack(rec.args[1], 'I', ctx.data['write'])


def _lock(ctx, rec):
    # LockBuffer(offset, bytes, &first, &firstBytes, &second, &secondBytes, flags)
    region = ctx.alloc(44100 + 64)
    ctx.write(region, bytes(rec.rng.getrandbits(8) for _ in range(64)))
    ctx.pack(rec.args[2], 'I', region)
    ctx.pack(rec.args[3], 'I', 44100)
    ctx.pack(rec.args[4], 'I', 0)
    ctx.pack(rec.args[5], 'I', 0)
    return 0


def _rand(ctx, rec):
    return rec.rng.randrange(32768)


REPLAY = dict(effects={GET_POSITION: _position, LOCK: _lock}, returns={RAND: _rand})


def cases(rng, n):
    for _ in range(n):
        bps = rng.choice([1, 2])
        half = 11025 * bps
        fill = rng.choice([1, 1, 2, 2, 0, 3])
        if fill == 1:
            write = rng.choice([16537 * bps - 1, 16537 * bps, rng.randrange(22050 * bps)])
        else:
            write = rng.choice([5512 * bps - 1, 5512 * bps, 11025 * bps, 11025 * bps + 1,
                                rng.randrange(22050 * bps)])
        sample_bytes = rng.choice([rng.randrange(half, 3 * half), rng.randrange(1, half), half])
        offset = rng.choice([0, rng.randrange(sample_bytes + 1), sample_bytes,
                             max(0, sample_bytes - half), max(0, sample_bytes - half + 1)])
        yield dict(
            seed=rng.getrandbits(32), bps=bps, fill=fill, write=write, play=rng.getrandbits(16),
            sample_bytes=sample_bytes, offset=offset,
            next_kind=rng.choice([0, 0, 1, 3, 4, 2]), engine=rng.randrange(3),
            has_sound=rng.random() < 0.95,
            counts=[rng.randrange(1, 11) for _ in range(9)],
        )


def setup(ctx, c):
    import random
    rng = random.Random(c['seed'])

    def noise(size):
        at = ctx.alloc(size)
        ctx.write(at, bytes(rng.getrandbits(8) for _ in range(min(size, 4096))) * (size // 4096 + 1)
                  if size > 4096 else bytes(rng.getrandbits(8) for _ in range(size)))
        return at

    ctx.data['play'] = c['play']
    ctx.data['write'] = c['write']
    sound = ctx.alloc(0x2000)
    sound_obj = ctx.alloc(0x40)
    ch = ctx.alloc(0x60)
    ctx.pack(ch + 0x04, 'I', sound_obj if c['has_sound'] else 0)
    ctx.pack(ch + 0x0c, 'i', c['engine'])
    ctx.pack(ch + 0x10, 'i', 7)
    ctx.pack(ch + 0x14, 'ii', c['sample_bytes'], c['offset'])
    ctx.pack(ch + 0x1c, 'ii', c['fill'], c['next_kind'])
    ctx.pack(ch + 0x38, 'I', noise(c['sample_bytes'] + 64))
    ctx.pack(ch + 0x50, 'I', noise(44100 + 64))
    ctx.pack(sound + 0x434, 'I', ch)
    ctx.pack(sound + 0x1298, 'i', c['bps'])
    k = 0
    for table, sizes, counts in ((0x840, 0xc00, 0x1128), (0xa20, 0xde0, 0x1158), (0xb10, 0xed0, 0x1170)):
        for e in range(3):
            cnt = c['counts'][k]
            k += 1
            ctx.pack(sound + counts + 4 * e, 'i', cnt)
            for j in range(10):
                size = rng.randrange(1, 3 * 11025 * c['bps'])
                ctx.pack(sound + table + 40 * e + 4 * j, 'I', noise(size + 64) if j < cnt else 0)
                ctx.pack(sound + sizes + 40 * e + 4 * j, 'i', size)
    return dict(ecx=sound, args=[])
