"""UIDropDownList::LayoutParts 0x0047a400, the drop-down list's part layout (tools/emu_diff.py
case, call-record/replay mode).

The linked body is src/bringup/Group2_UIDialogResource.cpp. The list's six parts (button
+0x1ec, list backgrounds +0x1f0/+0x1f4/+0x1f8, list +0x1fc, scroll bar +0x200) are random
objects with a fake vtable (Disable is slot 4). The button has a state image whose first
frame width is random, or none (then the button rectangle is empty and the button is
disabled). The list's screen area (positive, zero-sized, inverted) and row height are random;
the text colour is white or not (SetSelectColor only for non-white). GetName 0x00470df0
answers the control's name (empty, short or 40 characters), sprintf 0x00534aaf is modelled
and SetName 0x00470dc0 notes the composed name. Every part rectangle (screen area and
bounds), the open area (+0x20c) and the margin copies are compared.
"""
import random
import struct

RETAIL_VA = 0x0047a400
SYMBOL = '?LayoutParts@UIDropDownList@@QAEXXZ'
SOURCE = 'src/bringup/Group2_UIDialogResource.cpp'
FPCWS = (0x027f,)   # integer only
PROBES = {0x0047a433: 'image', 0x0047a454: 'no image', 0x0047a53d: 'disable', 0x0047a7a3: 'select colour'}

GET_NAME, SPRINTF, SET_NAME = 0x00470df0, 0x00534aaf, 0x00470dc0
PARTS = (0x1ec, 0x1f0, 0x1f4, 0x1f8, 0x1fc, 0x200)


def _cstr(ctx, at, limit=256):
    s = ctx.read(at, limit)
    return s[:s.index(0)] if 0 in s else s


def _get_name(ctx, rec):
    return ctx.data['names'].get(rec.ecx, ctx.data['name'])


def _sprintf(ctx, rec):
    out, fmt, value = rec.args[0], _cstr(ctx, rec.args[1]), rec.args[2]
    text = fmt.replace(b'%s', _cstr(ctx, value), 1)
    ctx.write(out, text + b'\0')
    ctx.note('sprintf', fmt, text)
    return len(text)


def _set_name(ctx, rec):
    ctx.note('SetName', rec.ecx, _cstr(ctx, rec.args[0]))
    return 0


REPLAY = dict(returns={GET_NAME: _get_name, SPRINTF: _sprintf, SET_NAME: _set_name},
              pops={SPRINTF: 0, GET_NAME: 0, SET_NAME: 4}, args={SPRINTF: 3, SET_NAME: 1},
              ecx={SPRINTF: False}, unlogged=(SPRINTF,))


def cases(rng, n):
    for _ in range(n):
        left, top = rng.choice([0, 5, 100, 600]), rng.choice([0, 3, 200])
        w, h = rng.choice([0, 1, 50, 120, -10, -1]), rng.choice([0, 1, 16, 20, -4])
        yield dict(seed=rng.getrandbits(32), area=(left, top, left + w, top + h),
                   image=rng.random() < 0.7, frame=rng.choice([0, 1, 16, 21, 300]),
                   row=rng.choice([0, 1, 13, 80, 200]),
                   color=rng.choice([0xffffff, 0xffffff, 0, 0xff0000, 0x1ffffff]),
                   name=rng.choice(['', 'DDL', 'Track_Select', 'x' * 40]))


def setup(ctx, c):
    r = random.Random(c['seed'])
    ctx.data['name'] = ctx.alloc(64, c['name'].encode() + b'\0')
    ctx.data['names'] = {}
    size = 0x300
    control = ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size)))
    ctx.pack(control, 'I', ctx.fake_vtable('dropdown', 64))
    ctx.pack(control + 0x2c, '4i', *c['area'])
    ctx.pack(control + 0x204, 'i', c['row'])
    ctx.pack(control + 0xc4, 'I', c['color'])
    vt = ctx.fake_vtable('part', 64)
    for k, off in enumerate(PARTS):
        part = ctx.alloc(size, bytes(r.getrandbits(8) for _ in range(size)))
        ctx.pack(part, 'I', vt)
        ctx.pack(control + off, 'I', part)
        if off == 0x1ec:
            image = 0
            if c['image']:
                frame = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
                ctx.pack(frame + 0xc, 'i', c['frame'])
                image = ctx.alloc(0x40, bytes(r.getrandbits(8) for _ in range(0x40)))
                ctx.pack(image + 0x28, 'I', frame)
            ctx.pack(part + 0x16c, 'I', image)
    return dict(ecx=control, args=[], ret_eax=False)
