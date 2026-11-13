"""UIDialog::ReadDialogResource 0x0046a920, the dialog resource (.dtm) parser (tools/emu_diff.py
case, call-record/replay mode).

The linked body is src/bringup/Group2_UIDialogResource.cpp. The resource is an INI-like
parameter block (ParameterBlock: GetSectionNames 0x004b8240, SelectSection 0x004b78f0,
GetString 0x004b7ec0, GetInt 0x004b7f10, GetFloat 0x004b7f40), modelled from a synthetic
resource: section names joined by '?' and split by the real Tokenizer (ctor 0x00515dc0
and NextToken 0x00515df0 run natively), with Set_Anim/Set_Sound/Set_Control markers (and
names that merely start with them), Set_Info and Set_Default sections, image sections
(MSecDelay, LoopCount, Files1 lists), sound sections (Static, Copies, File) and control
sections of every type (BUTTON, SCROLLUP/DOWN, MULTISTATE, LISTBOX, RADIOBUTTON, STATIC,
V/HSCROLLBAR, EDITBOX, STATICTEXT, DROPDOWNLIST and its prefix forms, mixed case), with
Xy/Xywh/Xyxy in comma, dot and malformed forms, states and items (and '@' image items),
fonts, alignments, effects, sounds, tool tips and localised ids. The getters note (key,
default) and answer the value or the default, as the retail getters do; sscanf, sprintf,
_stricmp and _strnicmp are modelled. operator new (unlogged: its __FILE__ tag is the unit's
own; noted by size and line) answers zeroed objects with a shared fake vtable (control slots
39 ScaleArea, 48 GetStateTexture, 49 SetEnabled, 52 SetText; texture slot 4 CreateSurface),
images for the state slots and a state table; every constructor answers its object.
FindSectionObject 0x0046e9a0 answers one object per name (0 for ""). The dialog starts with
an empty section table (sometimes a stale count), image and sound counts near their limits
or not, a background, a parent, a palette and a GUI manager (with or without a tool-tip face,
a font-size offset and a language module) as the case picks. fs:[0] is page 0.
"""
import random
import re
import struct

RETAIL_VA = 0x0046a920
SYMBOL = '?ReadDialogResource@UIDialog@@QAEHPAXH@Z'
SOURCE = 'src/bringup/Group2_UIDialogResource.cpp'
PROBES = {0x0046b5f7: 'background file', 0x0046b697: 'parent background', 0x0046ddd9: 'set text',
          0x0046b460: 'popup align', 0x0046b888: 'dialog font', 0x0046b955: 'palette', 0x0046bc1f: 'anim file',
          0x0046bd39: 'entry sound', 0x0046bda9: 'new sound', 0x0046bec1: 'xywh', 0x0046c03e: 'button',
          0x0046c0e7: 'scroll up', 0x0046c452: 'state entry', 0x0046c801: 'image row', 0x0046c9a6: 'radio',
          0x0046cdf2: 'thumb', 0x0046ce78: 'edit box', 0x0046d05d: 'static text', 0x0046d61b: 'drop-down',
          0x0046dff5: 'control font', 0x0046e0bb: 'text id', 0x0046e0e7: 'tool tip id', 0x0046e129: 'tip copy',
          0x0046e1ce: 'tool tip', 0x0046e46f: 'fx anim', 0x0046e629: 'background image'}

GET_STRING, GET_INT, GET_FLOAT = 0x004b7ec0, 0x004b7f10, 0x004b7f40
SECTION_NAMES, SELECT_SECTION = 0x004b8240, 0x004b78f0
TOKEN_CTOR, NEXT_TOKEN = 0x00515dc0, 0x00515df0
FIND_OBJECT, FIND_CONTROL, ADD_CONTROL = 0x0046e9a0, 0x0046ebf0, 0x0046a8a0
NEW, DELETE, MALLOC, FREE = 0x004a3010, 0x004a30c0, 0x004a2e20, 0x004a2e60
STRICMP, STRNICMP, SPRINTF, SSCANF = 0x00536070, 0x005476f0, 0x00534aaf, 0x0053530b
CURRENT_TEXTURE, LOAD_TGA, FIND_ITEM = 0x00472f90, 0x005125c0, 0x004e9360
CHKSTK = 0x00535ed0
CTORS = (0x00478f60, 0x00474820, 0x004731f0, 0x004781f0, 0x00475c70, 0x00479240, 0x00474ba0,
         0x00473630, 0x00478ff0, 0x00479ea0, 0x00472e30, 0x004bba10, 0x004c5f00, 0x004b6c00,
         0x004b7130)
IAT = {  # slot: (raw contents, name, stack words)
    0x005502e4: (0x00165214, 'LoadStringA', 4),
    0x00550068: (0x00165556, 'CreateFontA', 14),
    0x00550054: (0x001654ba, 'DeleteObject', 1),
}


def _cstr(ctx, at, limit=512):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _section(ctx):
    return ctx.data['ini'].get(ctx.data['current'].lower(), {})


def _lookup(ctx, key):
    return _section(ctx).get(key.decode('latin-1').lower())


def _select(ctx, rec):
    name = _cstr(ctx, rec.args[0]).decode('latin-1')
    ctx.data['current'] = name
    return int(name.lower() in ctx.data['ini'])


def _section_names(ctx, rec):
    text = b'?'.join(ctx.data['names'])[:rec.args[1] - 1]
    ctx.write(rec.args[0], text + b'\0')
    return len(ctx.data['names'])


def _get_string(ctx, rec):
    key, default = _cstr(ctx, rec.args[0]), _cstr(ctx, rec.args[1])
    ctx.note('GetString', key, default, rec.args[3])
    value = _lookup(ctx, key)
    text = default if value is None else value.encode('latin-1')
    ctx.write(rec.args[2], text + b'\0')
    return int(value is not None)


def _get_int(ctx, rec):
    key = _cstr(ctx, rec.args[0])
    ctx.note('GetInt', key, rec.args[1])
    value = _lookup(ctx, key)
    ctx.pack(rec.args[2], 'I', rec.args[1] if value is None else value & 0xffffffff)
    return int(value is not None)


def _get_float(ctx, rec):
    key = _cstr(ctx, rec.args[0])
    ctx.note('GetFloat', key, rec.args[1])
    value = _lookup(ctx, key)
    if value is None:
        ctx.pack(rec.args[2], 'I', rec.args[1])
    else:
        ctx.pack(rec.args[2], 'f', value)
    return int(value is not None)


def _stricmp(ctx, rec):
    a, b = _cstr(ctx, rec.args[0]).lower(), _cstr(ctx, rec.args[1]).lower()
    return 0 if a == b else (0xffffffff if a < b else 1)


def _strnicmp(ctx, rec):
    n = rec.args[2]
    a, b = _cstr(ctx, rec.args[0])[:n].lower(), _cstr(ctx, rec.args[1])[:n].lower()
    return 0 if a == b else (0xffffffff if a < b else 1)


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
    return len(text)


def _sscanf(ctx, rec):
    """sscanf for "%d", "%*[,.]" and blanks (the parser's only formats)."""
    src, fmt = _cstr(ctx, rec.args[0]), _cstr(ctx, rec.args[1])
    ctx.note('sscanf', src, fmt)
    outs = list(rec.args[2:])
    pos, count, i = 0, 0, 0
    while i < len(fmt):
        if fmt.startswith(b'%d', i):
            while pos < len(src) and src[pos:pos + 1] in b' \t\n':
                pos += 1
            m = re.match(rb'[+-]?\d+', src[pos:])
            if not m:
                break
            ctx.pack(outs.pop(0), 'i', ((int(m.group(0)) + (1 << 31)) % (1 << 32)) - (1 << 31))
            count += 1
            pos += len(m.group(0))
            i += 2
        elif fmt.startswith(b'%*[,.]', i):
            m = re.match(rb'[,.]+', src[pos:])
            if not m:
                break
            pos += len(m.group(0))
            i += 6
        elif fmt[i:i + 1] == b' ':
            while pos < len(src) and src[pos:pos + 1] in b' \t\n':
                pos += 1
            i += 1
        else:
            break
    return count if count or pos < len(src) else 0xffffffff


def _object(ctx, size):
    d = ctx.data
    at = ctx.alloc(max(size, 0x300))
    ctx.pack(at, 'I', d['vtable'])
    bounds = d['rng'].choice(d['bounds'])
    ctx.write(at + 0x3c, struct.pack('<4i', *bounds))
    ctx.write(at + 0x2c, struct.pack('<4i', *bounds))
    ctx.pack(at + 0x5c, 'i', d['rng'].choice([1, 2, 3, 5, 7, 9, 11, 12, 13]))
    ctx.pack(at + 0x128, 'I', d['rng'].choice([0, 0x1f0e7]))
    ctx.pack(at + 0x160, 'i', d['rng'].choice([0, 12, 20]))
    ctx.pack(at + 0xf0, 'I', d['rng'].choice([0, d['old_tip']]))
    for k in range(5):
        ctx.pack(at + 0x16c + 4 * k, 'I', d['image'])
    ctx.pack(at + 0x1ec, 'I', at + 0x2a0)       # drop-down parts (the calls are replayed)
    ctx.pack(at + 0x1f0, 'i', 0)
    ctx.pack(at + 0x1f4, 'I', d['states'])
    ctx.pack(at + 0x1fc, 'I', at + 0x2b0)
    ctx.pack(at + 0x200, 'I', at + 0x2c0)
    return at


def _new(ctx, rec):
    ctx.note('new', rec.args[0], rec.args[2])
    return _object(ctx, rec.args[0])


def _this(ctx, rec):
    return rec.ecx


def _add_control(ctx, rec):
    return rec.args[0]


def _find_object(ctx, rec):
    name = _cstr(ctx, rec.args[0])
    ctx.note('FindSectionObject', name)
    if not name:
        return 0
    objects = ctx.data['objects']
    if name not in objects:
        objects[name] = _object(ctx, 0x300) if ctx.data['rng'].random() < 0.9 else 0
    return objects[name]


def _find_control(ctx, rec):
    ctx.note('FindControl', _cstr(ctx, rec.args[0]), rec.args[1])
    return _object(ctx, 0x300)


def _current_texture(ctx, rec):
    t = ctx.alloc(0x40, fill=0x5a)
    ctx.write(t + 0x14, struct.pack('<ii', *ctx.data['rng'].choice([(256, 128), (640, 480), (0, 0), (31, 7)])))
    return t


def _load_tga(ctx, rec):
    ctx.note('LoadFile', _cstr(ctx, rec.args[0]), rec.args[1])
    if not ctx.data['tga']:
        return 0
    f = ctx.alloc(0x40, bytes(ctx.data['rng'].getrandbits(8) for _ in range(0x40)))
    return f


def _find_item(ctx, rec):
    name = _cstr(ctx, rec.args[0])
    ctx.note('FindItem', name, rec.args[1])
    return 0 if name.lower() in ctx.data['missing'] else ctx.alloc(0x20, fill=0x49)


def _malloc(ctx, rec):
    ctx.note('DebugMalloc', rec.args[0], rec.args[2])
    return ctx.alloc(max(rec.args[0], 1), fill=0x6d)


def _free(ctx, rec):
    ctx.note('DebugFree', rec.args[0], rec.args[2])
    return 0


def _iat(name):
    def run(ctx, rec):
        a = rec.args
        if name == 'LoadStringA':
            ctx.note(name, a[0], a[1], a[3])
            text = ctx.data['strings'].get(a[1])
            if text is None:
                return 0
            ctx.write(a[2], text[:a[3] - 1] + b'\0')
            return len(text[:a[3] - 1])
        if name == 'CreateFontA':
            ctx.note(name, a[:13], _cstr(ctx, a[13]))
            return 0x00f0e700 + len(ctx.data['fonts'])
        ctx.note(name, *a)
        return 1
    return run


returns = {GET_STRING: _get_string, GET_INT: _get_int, GET_FLOAT: _get_float, SECTION_NAMES: _section_names,
           SELECT_SECTION: _select, FIND_OBJECT: _find_object, FIND_CONTROL: _find_control,
           ADD_CONTROL: _add_control, NEW: _new, STRICMP: _stricmp, STRNICMP: _strnicmp, SPRINTF: _sprintf,
           SSCANF: _sscanf, CURRENT_TEXTURE: _current_texture, LOAD_TGA: _load_tga, MALLOC: _malloc,
           FREE: _free, FIND_ITEM: _find_item}
for va in CTORS:
    returns[va] = _this
pops = {NEW: 0, STRICMP: 0, STRNICMP: 0, SPRINTF: 0, SSCANF: 0, MALLOC: 0, FREE: 0, LOAD_TGA: 0}
args = {NEW: 3, STRICMP: 2, STRNICMP: 3, SPRINTF: 4, SSCANF: 6, MALLOC: 3, FREE: 3}
ecx = {NEW: False, STRICMP: False, STRNICMP: False, SPRINTF: False, SSCANF: False, MALLOC: False, FREE: False}
unlogged = [NEW, STRICMP, STRNICMP, SPRINTF, SSCANF, GET_STRING, GET_INT, GET_FLOAT, FIND_OBJECT,
            FIND_CONTROL, MALLOC, FREE, LOAD_TGA, FIND_ITEM]
for slot, (raw, name, n) in IAT.items():
    for k in (('iat', slot), raw):
        returns[k] = _iat(name)
        pops[k] = 4 * n
        args[k] = n
        ecx[k] = False
        unlogged.append(k)
REPLAY = dict(returns=returns, pops=pops, args=args, ecx=ecx, unlogged=tuple(unlogged),
              passthrough=(TOKEN_CTOR, NEXT_TOKEN), inline=(CHKSTK,), out_writes=False)

TYPES = ['BUTTON', 'SCROLLUP', 'SCROLLDOWN', 'MULTISTATE', 'LISTBOX', 'RADIOBUTTON', 'STATIC', 'VSCROLLBAR',
         'HSCROLLBAR', 'EDITBOX', 'STATICTEXT', 'DROPDOWNLIST', 'DropDownListX', 'button', 'StaticText']
ANIMS = ['', 'ImgA', 'ImgB', 'Missing', 'NONE']


def _text(rng, limit=127):
    n = rng.choice([0, 1, 6, 30, limit])
    return ''.join(rng.choice('abcXYZ_09 .') for _ in range(n))


def _int(rng):
    return rng.choice([0, 1, -1, 2, 3, 5, 7, 10, 50, 100, 501, 0x501, 700, -20, 0x7fffffff,
                       rng.randrange(-1000, 1000)])


def _maybe(rng, d, key, value, p=0.5):
    if rng.random() < p:
        d[key.lower()] = value


def _common(rng, d):
    for key in ('FontName', 'Anchor', 'TextAlign', 'TextAlignV', 'FX', 'FXSound', 'FXAnimIn', 'FXSoundIn',
                'FXAnimOut', 'FXSoundOut', 'MouseAnim', 'SoundNorm', 'SoundFocus', 'SoundPush', 'SoundClick',
                'Text', 'ToolTipText', 'AnimNorm', 'AnimFocus', 'AnimPush', 'AnimDisable', 'SortLBName',
                'AnimOff', 'AnimOn', 'AnimThumb', 'SoundEnd', 'AnimTextBack', 'AnimListBack', 'AnimScrollBack'):
        if key in ('TextAlign',):
            _maybe(rng, d, key, rng.choice(['RIGHT', 'CENTER', 'left', 'center', 'x']), 0.6)
        elif key == 'TextAlignV':
            _maybe(rng, d, key, rng.choice(['TOP', 'BOTTOM', 'bottom', 'MIDDLE']), 0.6)
        elif key == 'FX':
            _maybe(rng, d, key, rng.choice(['GROW', 'LSLIDE', 'TSLIDE', 'RSLIDE', 'BSLIDE', 'ANIM', 'anim', 'NONE']), 0.6)
        elif key == 'FontName':
            _maybe(rng, d, key, rng.choice(['Arial', '', 'Courier New']), 0.3)
        elif key in ('Text', 'ToolTipText'):
            _maybe(rng, d, key, _text(rng), 0.5)
        else:
            _maybe(rng, d, key, rng.choice(ANIMS + [_text(rng)]), 0.4)
    for key in ('GroupId', 'RelAnchor', 'Moveable', 'TextLMargin', 'TextTMargin', 'FXDelay', 'AttachId',
                'TextColor', 'DropColor', 'KeyBind', 'Italic', 'Bold', 'TextDrop', 'SourceBlit', 'Enable',
                'Show', 'Pre3D', 'Permanent', 'ShapeBounds', 'LocalTextId', 'LocalToolTipId', 'MaxItems',
                'Default', 'AutoSort', 'SelectColor', 'Selectable', 'AllowWScroll', 'SelectBoxColor',
                'ItemBoxColor', 'TickNum', 'Scale', 'MaxLen', 'ListHeight', 'NumStates'):
        _maybe(rng, d, key, _int(rng), 0.35)
    if rng.random() < 0.4:
        d['fontheight'] = rng.choice([0.0, 12.0, 12.5, -3.0, 0.25, 40.0])
    for k in range(rng.choice([0, 0, 1, 3])):
        d['state%d' % k] = rng.choice(ANIMS)
        if rng.random() < 0.6:
            d['statetext%d' % k] = _text(rng, 40)
    for k in range(1, 1 + rng.choice([0, 0, 1, 4])):
        d['item%d' % k] = rng.choice(['@ImgA', '@', 'Row', _text(rng)])


def cases(rng, n):
    for _ in range(n):
        names, ini = [], {}
        info = {}
        for key, v in (('ScreenWidth', [0, 320, 640, 1024]), ('ScreenHeight', [0, 240, 480, 768]),
                       ('NoScale', [0, 1, 2]), ('Popup', [0, 1]), ('Bold', [0, 1]), ('Italic', [0, 1])):
            _maybe(rng, info, key, rng.choice(v), 0.6)
        for key, v in (('DlgAlignV', ['TOP', 'BOTTOM', 'middle', 'top']), ('DlgAlignH', ['RIGHT', 'LEFT', 'x', 'left']),
                       ('MouseCursorAnim', ANIMS), ('BackgroundFile', ['', 'bg.tga']), ('FontName', ['', 'Arial']),
                       ('PaletteFile', ['', 'pal.act']), ('BackgroundImage', ['', 'ImgA', 'Missing'])):
            _maybe(rng, info, key, rng.choice(v), 0.5)
        for key in ('PointSize', 'FontHeight'):
            _maybe(rng, info, key, rng.choice([0.0, 10.0, 10.5, -2.0, 0.5]), 0.5)
        for k in range(10):
            _maybe(rng, info, 'TextColor%d' % k, _int(rng), 0.2)
        order = []
        if rng.random() < 0.9:
            order.append(('Set_Info', info))
        order.append(('Set_Anim', None))
        for k in range(rng.choice([0, 1, 3])):
            files = ','.join(rng.choice(['a.tga', 'b.tga', 'ImgA', '']) for _ in range(rng.choice([0, 1, 2, 3])))
            sec = {}
            _maybe(rng, sec, 'MSecDelay', _int(rng))
            _maybe(rng, sec, 'LoopCount', _int(rng))
            _maybe(rng, sec, 'Files1', files, 0.8)
            order.append((rng.choice(['ImgA', 'ImgB', 'Img%d' % k]), sec))
        order.append(('Set_Sound', None))
        for k in range(rng.choice([0, 1, 2])):
            sec = {}
            _maybe(rng, sec, 'Static', rng.choice([0, 1, 5]))
            _maybe(rng, sec, 'Copies', _int(rng))
            _maybe(rng, sec, 'File', rng.choice(['', 'click.wav', 'Snd']))
            order.append(('Snd%d' % k, sec))
        order.append(('Set_Control', None))
        for k in range(rng.choice([1, 2, 4, 6])):
            sec = {}
            default = rng.random() < 0.2
            _common(rng, sec)
            if not default or rng.random() < 0.5:
                sec['type'] = rng.choice(TYPES)
            for key, v in (('Xy', ['10,20', '10. 20', '5', 'x', '-3,,-4']), ('Xywh', ['1,2,30,40', '1.2. 3.4', '7,8']),
                           ('Xyxy', ['0,0,0,0', '100,50,10,5'])):
                _maybe(rng, sec, key, rng.choice(v), 0.35)
            order.append(('Set_Default%d' % k if default else 'Ctl%d' % k, sec))
        if rng.random() < 0.2:
            order.insert(rng.randrange(len(order) + 1), ('Set_Animation', None))
        for name, sec in order:
            names.append(name.encode())
            if sec is not None:
                ini[name.lower()] = sec
        yield dict(seed=rng.getrandbits(32), ini=ini, names=names, tga=rng.random() < 0.6,
                   stale=rng.choice([0, 0, 0, 0, 1, 2]), images=rng.choice([0, 0, 498, 500]),
                   sounds=rng.choice([0, 0, 19, 20]), gui=rng.choice(['plain', 'face', 'lang', 'lang']),
                   background=rng.random() < 0.7, parent=rng.random() < 0.5, palette=rng.random() < 0.5,
                   depth=rng.choice([8, 8, 16, 32]), offset=rng.choice([0.0, 0.0, 2.0, -1.5]),
                   strings={7: b'Seven', 0x501: b'x' * 40} if rng.random() < 0.7 else {},
                   missing=rng.sample(['imga', 'imgb', 'img0', 'img1', 'img2', 'a.tga', 'b.tga', 'snd0', 'snd1',
                                       'click.wav', 'pal.act'], rng.choice([0, 3, 6, 11])),
                   bounds=[rng.choice([(0, 0, 0, 0), (5, 6, 5, 6), (1, 2, 30, 40)]) for _ in range(3)])


def setup(ctx, c):
    ctx.uc.mem_map(0, 0x1000)
    r = random.Random(c['seed'])
    d = ctx.data
    d.update(ini=c['ini'], names=c['names'], current='', objects={}, tga=c['tga'], rng=random.Random(c['seed'] ^ 1),
             bounds=[tuple(b) for b in c['bounds']], strings=c['strings'], fonts=[],
             missing={m.encode() for m in c['missing']})

    def noise(n):
        return bytes(r.getrandbits(8) for _ in range(n))

    d['vtable'] = ctx.fake_vtable('object', 64, pops={4: 68, 39: 0, 48: 4, 49: 4, 52: 4})
    frame = ctx.alloc(0x20)
    ctx.pack(frame + 0xc, 'i', r.choice([16, 64, 0]))
    ctx.pack(frame + 0x10, 'i', r.choice([8, 32, 0]))
    image = ctx.alloc(0x100, fill=0)
    ctx.pack(image + 0x28, 'I', frame)
    d['image'] = image
    d['states'] = ctx.alloc(0x400, struct.pack('<256I', *([image] * 256)))
    d['old_tip'] = ctx.alloc(0x10, b'old\0')

    target = ctx.alloc(0x40, noise(0x40))
    ctx.write(target + 0xc, struct.pack('<ii', r.choice([640, 800, 1024]), r.choice([480, 600, 768])))
    ctx.pack(target + 0x28, 'i', c['depth'])
    game = ctx.alloc(0x40, noise(0x40))
    ctx.pack(game + 0x10, 'I', target)
    ctx.pack(0x0056e26c, 'I', game)
    ctx.pack(0x00572b44, 'I', ctx.alloc(0x40, fill=0x52))

    gui = ctx.alloc(0x400, bytes(0x400))
    ctx.pack(gui + 0x34, 'I', ctx.alloc(0x710, fill=0x50))
    ctx.write(gui + 0x50, b'Tahoma\0')
    if c['gui'] == 'face':
        ctx.write(gui + 0x350, b'Verdana\0')
        ctx.pack(gui + 0x3d4, 'i', 1)
        ctx.pack(gui + 0x3d8, 'i', 1)
    if c['gui'] == 'lang':
        ctx.pack(gui + 0x348, 'I', 0x10000000)
    ctx.pack(gui + 0x3d0, 'f', c['offset'])

    dialog = ctx.alloc(0x8000, noise(0x8000))
    ctx.pack(dialog + 0x18, 'I', target)
    ctx.pack(dialog + 0x30, 'I', gui)
    ctx.pack(dialog + 0xd8, 'I', r.choice([0, 0x1f0e1]))
    ctx.pack(dialog + 0xdc, 'i', r.choice([0, 14]))
    ctx.write(dialog + 0x108, struct.pack('<ii', r.choice([0, 1]), r.choice([0, 1])))
    background = ctx.alloc(0x40, noise(0x40)) if c['background'] else 0
    ctx.pack(dialog + 0x110, 'I', background)
    if c['parent']:
        parent = ctx.alloc(0x200, noise(0x200))
        ctx.pack(parent + 0x110, 'I', ctx.alloc(0x40, noise(0x40)))
        ctx.pack(dialog + 0x2c, 'I', parent)
    else:
        ctx.pack(dialog + 0x2c, 'I', 0)
    ctx.pack(dialog + 0x140, 'I', ctx.alloc(0x710, noise(0x710)) if c['palette'] else 0)
    ctx.write(dialog + 0x160, struct.pack('<4i', 10, 20, 330, 260))
    ctx.write(dialog + 0x9b0, struct.pack('<ii', c['images'], c['sounds']))
    ctx.write(dialog + 0x9c4, struct.pack('<ff', 1.0, 1.0))
    ctx.pack(dialog + 0x7efc, 'i', c['stale'])
    ctx.pack(dialog + 0x7f3c, 'I', ctx.alloc(0x40, fill=0x3c))
    # g_LastMultiStateText 0x0065b600 holds the address of a stack buffer (frame layout).
    return dict(ecx=dialog, args=[ctx.alloc(0x40, fill=0x73), r.choice([0, 0x1234])], ignore=[(0x0065b600, 4)])
