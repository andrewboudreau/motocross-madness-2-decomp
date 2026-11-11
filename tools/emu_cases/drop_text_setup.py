"""DropTextOverlay::Setup 0x0051af00 (tools/emu_diff.py case).

SetRenderContext (0x004692f0) is stubbed; the render target is preset at +0x18. The
CreateFontIndirectA import slot (0x0055006c) points at a stub that logs the LOGFONT it receives
and returns the case's handle (0 takes the Release path, virtual slot 2). g_TrackGame->ui->gui
carries the tooltip face, bold and italic flags, or is null.

The x/y scaling (`size * (1/640) * x - 0.5`) gives exact ties at 640x480 and near-ties at other
sizes; both coordinates (+0x3c, +0x40, +0x44) are compared.
"""
RETAIL_VA = 0x0051af00
SYMBOL = '?Setup@DropTextOverlay@@QAEPAV1@PAXPAVTextureMapManager@@MHHPBD@Z'
SOURCE = 'samples/track/DropTextOverlaySetup.cpp'
PROBES = {0x0051af3e: 'x fistp', 0x0051af64: 'y fistp'}
LOGFONT = 0x3c


def cases(rng, n):
    for _ in range(n):
        width, height = rng.choice([(640, 480), (800, 600), (1024, 768), (512, 384), (320, 240),
                                    (rng.randrange(1, 4096), rng.randrange(1, 4096))])
        x = rng.choice([rng.randrange(-100, 700), rng.randrange(0, 641, 8), 320, 0, 1, 639])
        y = rng.choice([rng.randrange(-100, 520), rng.randrange(0, 481, 8), 240, 0, 1, 479])
        gui = rng.choice([None, ('', 0, 0), ('Tahoma', 1, 0), ('Verdana', 0, 1), ('Arial Black', 7, 255)])
        text = rng.choice(['', 'GO!', 'Lap 2 of 3', 'x' * 60])
        yield (width, height, x, y, rng.uniform(0.2, 4.0), gui, text, rng.choice([0, 0x1234]))


def setup(ctx, case):
    width, height, x, y, seconds, gui, text, font = case
    ctx.stub_at(0x004692f0, 'SetRenderContext', pop=4)

    def create_font(c, ecx, args):
        c.note('LOGFONT', c.read(args[0], LOGFONT).hex())
        return font
    ctx.pack(0x0055006c, 'I', ctx.stub('CreateFontIndirectA', pop=4, ret=create_font))
    game = ctx.alloc(0x600, fill=0x11)
    ui = ctx.alloc(0x40, fill=0x12)
    if gui is None:
        ctx.pack(ui + 0x2c, 'I', 0)
    else:
        face, bold, italic = gui
        g = ctx.alloc(0x400, fill=0x13)
        ctx.write(g + 0x350, face.encode() + b'\0')
        ctx.pack(g + 0x3d4, 'II', bold, italic)
        ctx.pack(ui + 0x2c, 'I', g)
    ctx.pack(game + 0x56c, 'I', ui)
    ctx.pack(0x0056e26c, 'I', game)
    target = ctx.alloc(0x40, fill=0x22)
    ctx.pack(target + 0xc, 'ii', width, height)
    vt = ctx.vtable([ctx.stub('DropTextOverlay::vslot%d' % i) for i in range(12)])
    overlay = ctx.alloc(0x100, fill=0x44)
    ctx.pack(overlay, 'I', vt)
    ctx.pack(overlay + 0x18, 'I', target)
    caption = ctx.alloc(len(text) + 1, text.encode() + b'\0')
    return dict(ecx=overlay, args=[0x5151, 0x6161, seconds, x, y, caption])
