// Near misses for GUIManager.cpp (canonical file:
// src/reconstructed/GUIManager.cpp, included below for its types). Check:
// compile this file and compare each function with
// src/reconstructed/GUIManager.bindings.json.
//
// 0x004853b0 (1085 bytes, 1079 match): only stack slots differ. Retail keeps
//   the ControlInterface pointer in the fontSize argument slot (+0x3c) and
//   the `new` temporaries in the target slot (+0x20); VC6 here uses +0x20
//   and +0x28. Local and expression forms of the devices do not change it.
// 0x00485a70 (345 bytes): same calls and argument order; retail keeps
//   `flags & 4` in edi and `wait` in ebp, VC6 here keeps `a`/`parent` in
//   ebp (a `flags & 4` local makes it worse).
// 0x00485c80 (192 bytes): only the allocation size differs. Retail
//   allocates 0x7f58 bytes (the whole UIDialog); UIDialog.h declares 0x2c.
//   With UIDialog padded to 0x7f58 it is strict exact (checked), but that
//   shifts TransDlg/Intro1Dlg/Exit1Dlg/TrackRecordDlg padding.
// 0x00486170 (896 bytes): same logic; retail keeps three more stack locals
//   (frame 0xb0 against 0xa4) and allocates registers differently around
//   the row copy and the 8-bit dim loop.
// 0x00486b10 (106 bytes): retail stores the centred x before loading the
//   control's bottom and passes the text in eax; VC6 here hoists the load.
// 0x00486b80 (553 bytes, ratio 0.97): retail computes the width limit as
//   `sub eax, 2` (VC6 here: `add eax, -2`) and spills width/height into the
//   SIZE slots later.
// 0x00487870 (283 bytes): same flow; retail caches `result` in edi and
//   spills it around the inline strcmp; VC6 here keeps it on the stack.
// 0x00488120 (49 bytes): retail keeps the `device ? owner : 0` branch
//   (jmp + xor) that VC6 here threads into the owner test (also with an
//   inline helper, an if/else assignment or early-return layouts).

#include "../../src/reconstructed/GUIManager.cpp"

// 0x004853b0
GUIManager* GUIManager::UnknownFunction4853b0(void* target, Palette8* palette, TextureMapManager* textures,
                                              BackgroundImage* background, int startSound, SoundGroup* sound,
                                              const char* font, int fontSize, const char* cursor,
                                              int callback) {
    ControlInterface* controls;
    int i;

    GameObject::UnknownVirtualSlot8(target);
    field_0xd8 = textures;
    field_0x44 = sound;
    if (startSound && SoundSystem()->UnknownFunction4be5a0(22050, 1, 16, 4000000, 0) == 0)
        SoundSystem()->UnknownFunction4be910(22050, 1, 16);
    if (!sound) {
        field_0x44 = new(__FILE__, 130) SoundGroup(1);
        UnknownFunction469190(field_0x44, -1);
        field_0x4c = 1;
    }
    field_0x344 = (GameObject*)UnknownFunction469190(new(__FILE__, 136) UIDlgContainer, -1);
    field_0x38 = palette;
    field_0x34 = palette;
    field_0x3c = background;
    strcpy(field_0x50, font);
    field_0xd0 = fontSize;
    if (cursor)
        strcpy(field_0x200, cursor);
    UnknownVirtualSlot18();
    field_0xd4 = callback;
    field_0x1fc = CreateFontA(12, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 2, 2, "Arial");
    controls = g_UnknownGlobal56e26c->field_0x14;
    if (controls->mouse) {
        field_0x308 = (GUIInputDevice*)UnknownFunction469190(
            (new(__FILE__, 162) GUIInputDevice)
                ->UnknownFunction486fb0(field_0x18, (InputDevice*)controls->mouse, 0, 0, 0, 0),
            -1);
        field_0x308->UnknownFunction469260(field_0x344, -1);
        field_0x308->UnknownFunction487150();
    }
    if (controls->keyboard) {
        field_0x30c = (GUIInputDevice*)UnknownFunction469190(
            (new(__FILE__, 169) GUIInputDevice)
                ->UnknownFunction486fb0(field_0x18, (InputDevice*)controls->keyboard, 0, 0, 0, 0),
            -1);
        field_0x30c->UnknownFunction469260(field_0x344, -1);
        field_0x30c->UnknownFunction487150();
    }
    if (controls->joystickCount) {
        for (i = 0; i < controls->joystickCount; i++) {
            field_0x310[i] = (GUIInputDevice*)UnknownFunction469190(
                (new(__FILE__, 177) GUIInputDevice)
                    ->UnknownFunction486fb0(field_0x18, (InputDevice*)controls->joysticks[i], -2.0f, 2.0f, -2.0f,
                                            2.0f),
                -1);
            field_0x310[i]->UnknownFunction469260(field_0x344, -1);
            field_0x310[i]->UnknownFunction487150();
        }
    }
    field_0x330[0] = (GUIUser*)UnknownFunction469190(
        (new(__FILE__, 188) GUIUser)->UnknownFunction487650(field_0x18, this), -1);
    if (field_0x330[0]) {
        field_0x340 = 1;
        field_0x330[0]->UnknownFunction487d00();
        if (field_0x308) {
            field_0x330[0]->UnknownFunction488120(field_0x308);
            UnknownFunction486590(0, 1);
            UnknownFunction4865e0(0, 0);
        }
    }
    field_0x330[0]->UnknownFunction487990(0);
    field_0x348 = LoadLibraryA("uilang.dll");
    return this;
}

// 0x00485a70
UnknownGuiDialog* GUIManager::UnknownFunction485a70(UnknownGuiDialog* dialog, int a, int flags, int b,
                                                   UnknownGuiDialog* parent, int c, int d, int wait) {
    CameraRect screen;
    CameraRect* rect;

    if (!(flags & 4) && !(flags & 2)) {
        if (!UnknownFunction485df0() || !UnknownFunction485df0()->field_0x148)
            UnknownFunction4860f0();
    } else {
        rect = 0;
        if (flags & 0x10) {
            screen.left = screen.top = 0;
            screen.right = ((RenderTarget*)field_0x18)->field_0x0c;
            screen.bottom = ((RenderTarget*)field_0x18)->field_0x10;
            rect = &screen;
        }
        if (parent)
            parent->UnknownFunction470070(1, flags & 8, rect);
        else if ((flags & 4) && (flags & 0x10))
            UnknownFunction4860a0(flags & 8);
    }
    if (wait)
        UnknownFunction4865e0(field_0x280, field_0x1f0 == 0);
    if (dialog->UnknownVirtualSlot27(field_0x18, b, a, flags, field_0x44, field_0xd8, field_0xf0, parent,
                                     field_0x3c, field_0x50, field_0xd0, this, field_0x330[0], d)) {
        dialog->field_0x144 = flags;
        UnknownFunction485bd0(dialog, a, wait);
        return dialog;
    }
    if (wait)
        UnknownFunction4865e0(field_0x200, field_0x1f0 == 0);
    return 0;
}

// 0x00485c80
int GUIManager::UnknownFunction485c80(const char* resource) {
    UnknownFunction485d50();
    field_0x30 = (UnknownGuiDialog*)new(__FILE__, 523) UIDialog(0, resource);
    if (field_0x30) {
        field_0x30->UnknownVirtualSlot27(field_0x18, 0, 0, 0, field_0x44, field_0xd8, field_0xf0, 0, 0, "Arial",
                                         14, 0, 0, 0);
        return 1;
    }
    return 0;
}

// 0x00486170
PCTextureMap* GUIManager::UnknownFunction486170(int dim, CameraRect* rect) {
    UnknownSurfaceDesc desc;
    UnknownSurfaceInterface* surface;
    UnknownDisplay* display;
    PCTextureMap* texture = 0;
    unsigned char* bits;
    unsigned char* row;
    unsigned char* pixel;
    unsigned short* pixel16;
    unsigned short mask;
    int format;
    int bytesPerPixel;
    int left;
    int top;
    int right;
    int bottom;
    int width;
    int height;
    int pixels;
    int y;
    unsigned int line;
    unsigned int x;

    format = g_UnknownGlobal56e26c->field_0x10->field_0x28;
    bytesPerPixel = UnknownFunction511970(format);
    if (field_0x3c)
        field_0x3c->UnknownFunction404c80();
    g_UnknownGlobal56e26c->field_0x34->UnknownFunction468dd0("GUICursor");
    g_UnknownGlobal56e26c->field_0x34->UnknownFunction468dd0("ToolTip");
    g_UnknownGlobal56e26c->UnknownVirtualSlot8();
    g_UnknownGlobal56e26c->UnknownVirtualSlot9();
    g_UnknownGlobal56e26c->field_0x10->UnknownFunction4e8cc0();
    g_UnknownGlobal56e26c->field_0x34->UnknownFunction468f10("GUICursor");
    g_UnknownGlobal56e26c->field_0x34->UnknownFunction468f10("ToolTip");
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    display = g_UnknownGlobal56e26c->field_0x0c;
    if (display->field_0x1a8)
        surface = display->field_0x1a8;
    else
        surface = display->field_0x19c;
    if (!surface->UnknownMethod25(0, &desc, 0x811, 0)) {
        if (rect) {
            left = rect->left;
            top = rect->top;
            right = rect->right;
            bottom = rect->bottom;
        } else {
            left = 0;
            top = 0;
            right = desc.width;
            bottom = desc.height;
        }
        width = right - left;
        height = bottom - top;
        pixels = width * height;
        bits = (unsigned char*)DebugMalloc(pixels * bytesPerPixel, __FILE__, 849);
        row = bits;
        for (y = 0; y < height; y++) {
            memcpy(row, (unsigned char*)desc.surface + (y + top) * desc.pitch + left * bytesPerPixel,
                   width * bytesPerPixel);
            row += width * bytesPerPixel;
        }
        surface->UnknownMethod32(0);
        if (dim) {
            if (bytesPerPixel == 1) {
                for (line = 0; line < (unsigned int)height; line++) {
                    for (x = line * width + (line & 1); x < line * width + width; x += 2)
                        bits[x] = 0;
                }
            } else if (bytesPerPixel == 2) {
                mask = format == 0x22b ? 0x7bde : 0xf7de;
                pixel16 = (unsigned short*)bits;
                for (; pixels > 0; pixels--, pixel16++)
                    *pixel16 = (*pixel16 >> 1) & (mask >> 1);
            } else if (bytesPerPixel == 3) {
                for (pixel = bits; pixels > 0; pixels--, pixel += 3) {
                    pixel[0] >>= 1;
                    pixel[1] >>= 1;
                    pixel[2] >>= 1;
                }
            } else if (bytesPerPixel == 4) {
                for (pixel = bits; pixels > 0; pixels--, pixel += 4) {
                    pixel[0] >>= 1;
                    pixel[1] >>= 1;
                    pixel[2] >>= 1;
                }
            }
        }
        texture = new(__FILE__, 916) PCTextureMap(field_0xd8, 1);
        texture->UnknownVirtualSlot4(bits, width, height, width, width, format, format, 0, 4, 0, 0, 0, 2, 1, 0, 0x80,
                                     0xff00ff);
        operator delete(bits, __FILE__, 921);
    }
    return texture;
}

// 0x00486b10
void ToolTip::UnknownFunction486b10(UnknownGuiControl* control) {
    int position[2];
    GUIInputDevice* device = control->field_0xb8->field_0x34->field_0x2c;

    if (device) {
        position[0] = device->field_0xa4.x;
        position[1] = device->field_0xa4.y + 16;
    } else {
        position[0] = control->field_0x3c.left + (control->field_0x3c.right - control->field_0x3c.left) / 2;
        position[1] = control->field_0x3c.bottom;
    }
    UnknownFunction486b80(control->field_0xf0, position, 1.0f);
}

// 0x00486b80
void ToolTip::UnknownFunction486b80(const char* text, int* position, float time) {
    PCRenderTarget* target;
    PCTextureMap* texture;
    HDC dc;
    HGDIOBJ previous;
    SIZE size;
    int width;
    int height;
    int x;

    if (field_0x30) {
        field_0x30->Release();
        field_0x30 = 0;
    }
    if (text) {
        if (field_0x2c->field_0x3c && field_0x34 == -1) {
            field_0x34 = field_0x2c->field_0x3c->UnknownFunction4040f0(0);
            field_0x38 = 3;
        }
        if (!((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod17((void**)&dc)) {
            previous = SelectObject(dc, (HGDIOBJ)field_0x2c->field_0x1fc);
            GetTextExtentPoint32A(dc, text, strlen(text), &size);
            SelectObject(dc, previous);
            ((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod26(dc);
            target = (PCRenderTarget*)field_0x18;
            width = size.cx + 2;
            if (width >= target->field_0x0c - 2)
                width = target->field_0x0c - 2;
            height = size.cy + 2;
            field_0x4c.top = 0;
            field_0x4c.left = 0;
            field_0x4c.right = width;
            field_0x4c.bottom = height;
            field_0x3c.left = position[0];
            field_0x3c.top = position[1];
            x = field_0x3c.left - width / 2;
            x = x < 0 ? 0 : x;
            field_0x3c.left = __min(target->field_0x0c - width, x);
            field_0x3c.top = __min(target->field_0x10 - height, field_0x3c.top);
            field_0x3c.right = field_0x3c.left + width;
            field_0x3c.bottom = field_0x3c.top + height;
            texture = field_0x2c->UnknownFunction486740(width, height, 0x808080);
            field_0x30 = texture;
            if (texture && texture->field_0x70) {
                if (!texture->field_0x70->UnknownMethod17((void**)&dc)) {
                    previous = SelectObject(dc, (HGDIOBJ)field_0x2c->field_0x1fc);
                    SetTextColor(dc, 0xcccccc);
                    SetBkColor(dc, 0x404040);
                    DrawTextA(dc, text, strlen(text), &field_0x4c, 0x8025);
                    SelectObject(dc, previous);
                    texture->field_0x70->UnknownMethod26(dc);
                }
                field_0x64 = 1;
                field_0x60 = time;
                return;
            }
            field_0x64 = 0;
            field_0x60 = -1.0f;
            return;
        }
    }
    field_0x64 = 0;
    field_0x60 = -1.0f;
}

// 0x00487870
int GUIUser::UnknownFunction487870(UnknownGuiControl* control, UnknownGuiControl** previous) {
    int result = 1;

    if (control && control != field_0x1d8)
        result = control->UnknownVirtualSlot31();
    if (result) {
        if (field_0x1d8 && !field_0x1d8->field_0x25_bit3 && field_0x1d8 != control)
            field_0x1d8->UnknownVirtualSlot32(control);
        if (field_0x1d4 && !field_0x1d4->field_0x25_bit3 && field_0x1d4 != control)
            field_0x1d4->UnknownVirtualSlot33(control);
        if (previous)
            *previous = field_0x1d4;
        field_0x1d8 = control;
        field_0x1d4 = control;
        if (g_UnknownGlobal56e26c->field_0x0c->field_0x6c) {
            if (control->field_0x5c == 11) {
                UnknownFunction487990(!control->field_0x210 || strcmp(control->field_0x210, "0123456789") != 0);
            } else {
                UnknownFunction487990(0);
            }
        }
    }
    return result;
}

// 0x00488120
int GUIUser::UnknownFunction488120(GUIInputDevice* device) {
    GUIUser* owner = device ? device->field_0xc0 : 0;

    if (owner) {
        if (owner != this)
            return 0;
    } else {
        field_0x2c = device;
        if (device)
            device->field_0xc0 = this;
    }
    return 1;
}

