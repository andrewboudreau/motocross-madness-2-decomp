"""TrackGameMode::ResetDefaults 0x00522440, the default settings (tools/emu_diff.py case).

The candidate is samples/game/UiInfoNearMisses.cpp. The mode object is 0x3000 random bytes,
so every store, the two 0x1ec-byte raceSettings copies (+0x27f8 -> +0x2bd0 and +0x29e4), the
bikeUnlockFlags bit clear and the detail-level fan-out are compared byte for byte. The eight
callees (ResetInterfaceOptions 0x00522720, ResetSoundOptions 0x00522780, ResetGraphicsOptions
0x005227d0, ChooseDisplayMode 0x00522e20 on g_TrackGame+0x578, ResetControlOptions 0x00522800,
ReloadResourceStrings 0x00522840, ResetGarageOptions 0x00522cb0 x3, ResetSettingsEntries
0x00522cd0; pops from their ret instructions) are stubs that log their this/argument and
write a few random bytes at their argument, so the later copies pick up what they changed.
The detail level (read before the fan-out) is set from the run's random stream.
"""
RETAIL_VA = 0x00522440
SYMBOL = '?ResetDefaults@TrackGameMode@@QAEXXZ'
SOURCE = 'samples/game/UiInfoNearMisses.cpp'
TRACK_GAME = 0x0056e26c


def cases(rng, n):
    for _ in range(n):
        yield dict(seed=rng.randrange(1 << 30))


def setup(ctx, c):
    import random
    r = random.Random(c['seed'])
    mode = ctx.alloc(0x3000)
    ctx.write(mode, r.randbytes(0x3000))
    game = ctx.alloc(0x600)
    ctx.pack(TRACK_GAME, 'I', game)

    def scribble(cx, e, a):
        if a:
            cx.write(a[-1], cx.rng.randbytes(16))

    for va, name, pop in [(0x00522720, 'ResetInterfaceOptions', 4), (0x00522780, 'ResetSoundOptions', 4),
                          (0x005227d0, 'ResetGraphicsOptions', 4), (0x00522e20, 'ChooseDisplayMode', 8),
                          (0x00522800, 'ResetControlOptions', 4), (0x00522840, 'ReloadResourceStrings', 4),
                          (0x00522cb0, 'ResetGarageOptions', 4), (0x00522cd0, 'ResetSettingsEntries', 0)]:
        ctx.stub_at(va, name, pop=pop, ret=scribble)
    return dict(ecx=mode, args=[], ret_eax=False)
