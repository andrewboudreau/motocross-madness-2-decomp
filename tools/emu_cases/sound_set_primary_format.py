"""PCSoundInterface::SetPrimaryFormat 0x004be910, the primary buffer's PCM format
(tools/emu_diff.py case, call-record/replay mode).

The linked body is the build's PrimarySoundFormat object. +0x2c bit 0 says sound is up;
+0x468 is the primary IDirectSoundBuffer, whose SetFormat (vtable +0x38, stdcall, 2 words)
is replayed with the WAVEFORMATEX logged (0x12 bytes: tag 1, channels 1 or 2, rate, average
bytes per second, block align, bits). The result is S_OK, S_FALSE or an error.
"""
RETAIL_VA = 0x004be910
SYMBOL = '?SetPrimaryFormat@PCSoundInterface@@QAEHHHH@Z'
SOURCE = 'samples/audio/PrimarySoundFormat.cpp'
REPLAY = dict()


def cases(rng, n):
    for _ in range(n):
        yield dict(up=rng.choice([0, 1, 3, 2, 0xff]),
                   rate=rng.choice([11025, 22050, 44100, rng.randrange(1, 1 << 20), 0xffffffff]),
                   stereo=rng.choice([0, 1, 2, -1]), bits=rng.choice([8, 16, 0, 7, 0x10010]),
                   result=rng.choice([0, 1, 0x88780032, 0x80004005]))


def setup(ctx, c):
    def set_format(cx, rec):
        # SetFormat(this, format)
        cx.note('SetFormat', cx.read(rec.args[1], 0x12))
        return c['result']

    sound = ctx.alloc(0x500, fill=0x53)
    ctx.pack(sound + 0x2c, 'B', c['up'])
    buffer = ctx.alloc(0x10)
    ctx.pack(buffer, 'I', ctx.fake_vtable('primary', 24, pops={14: 8}, returns={14: set_format}))
    ctx.pack(sound + 0x468, 'I', buffer)
    return dict(ecx=sound, args=[c['rate'] & 0xffffffff, c['stereo'] & 0xffffffff, c['bits'] & 0xffffffff])
