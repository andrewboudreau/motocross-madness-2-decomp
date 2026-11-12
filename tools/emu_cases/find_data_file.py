"""TrackGameMode::FindDataFile 0x005238f0, the data file lookup (tools/emu_diff.py case).

The linked body is samples/game/UiInfoNearMisses.cpp. Offsets from the retail body: the mode's
install type +0x23cc (2: no CD fallback), install directory +0x23d0 and CD data directory
+0x24d4. Names start with either directory in any letter case, a prefix of it, or neither; the
CD directory may be empty. _strnicmp 0x005476f0 is modelled (ASCII case folding, as the C
locale), sprintf 0x00534aaf is modelled for "%s\\%s" and noted, fopen 0x00534ceb answers a
fake FILE for the paths the case lists as present (either directory, both or neither) and
fclose 0x00534c3d is noted; FindCdDirectory 0x00523bf0 and WaitForCd 0x00523800 answer per
case. `path` is compared as memory.
"""
RETAIL_VA = 0x005238f0
SYMBOL = '?FindDataFile@TrackGameMode@@QAEHPBDPAD@Z'
SOURCE = 'samples/game/UiInfoNearMisses.cpp'
PROBES = {0x0052394d: 'install prefix', 0x0052397c: 'cd prefix', 0x005239b1: 'not installed',
          0x005239d4: 'cd retry', 0x00523a01: 'found'}


def _cstr(ctx, at, limit=0x200):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def cases(rng, n):
    for _ in range(n):
        install = rng.choice([b'C:\\Games\\MCM2', b'c:\\mcm2\\data', b'', b'D'])
        cd = rng.choice([b'D:\\data', b'', b'C:\\Games\\MCM2', b'e:\\MCM2'])
        base = rng.choice([b'tracks\\quarry.trk', b'x', b''])
        prefix = rng.choice([b'', install, install.lower(), install.upper(), cd, cd.swapcase(),
                             install[:3], cd[:2]])
        sep = rng.choice([b'\\', b''])
        name = prefix + (sep if prefix else b'') + base
        yield dict(install=install, cd=cd, name=name, install_type=rng.choice([0, 1, 2]),
                   present=rng.choice(['install', 'cd', 'both', 'none', 'cd']),
                   find_cd=rng.choice([0, 1]), wait_cd=rng.choice([0, 1]))


def setup(ctx, c):
    mode = ctx.alloc(0x2600, fill=0x41)
    ctx.pack(mode + 0x23cc, 'i', c['install_type'])
    ctx.write(mode + 0x23d0, c['install'] + b'\0')
    ctx.write(mode + 0x24d4, c['cd'] + b'\0')
    name = ctx.alloc(0x200, c['name'] + b'\0')
    path = ctx.alloc(0x200, fill=0x5f)

    def strnicmp(cx, e, a):
        x, y = cx.read(a[0], a[2]), cx.read(a[1], a[2])
        for p, q in zip(x, y):
            p, q = (p + 32 if 65 <= p <= 90 else p), (q + 32 if 65 <= q <= 90 else q)
            if p != q or not p:
                return (p - q) & 0xffffffff
        return 0

    def sprintf(cx, e, a):
        text = _cstr(cx, a[2]) + b'\\' + _cstr(cx, a[3])
        cx.note('sprintf', _cstr(cx, a[1]), text)
        cx.write(a[0], text + b'\0')
        return len(text)

    def fopen(cx, e, a):
        text, present = _cstr(cx, a[0]), c['present']
        cx.note('fopen', text, _cstr(cx, a[1]))
        hit = ((present in ('install', 'both') and text.startswith(c['install'] + b'\\'))
               or (present in ('cd', 'both') and text.startswith(c['cd'] + b'\\')))
        return 0xf11e0000 if hit else 0

    ctx.stub_at(0x005476f0, '_strnicmp', nargs=3, ret=strnicmp, log=False)
    ctx.stub_at(0x00534aaf, 'sprintf', nargs=4, ret=sprintf, log=False)
    ctx.stub_at(0x00534ceb, 'fopen', nargs=2, ret=fopen, log=False)
    ctx.stub_at(0x00534c3d, 'fclose', nargs=1, ret=lambda cx, e, a: cx.note('fclose', a[0]), log=False)
    ctx.stub_at(0x00523bf0, 'TrackGameMode::FindCdDirectory', pop=0, ret=c['find_cd'])
    ctx.stub_at(0x00523800, 'TrackGameMode::WaitForCd', pop=0, ret=c['wait_cd'])
    return dict(ecx=mode, args=[name, path])
