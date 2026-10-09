// Near misses for GUIManager.cpp (canonical file:
// src/reconstructed/GUIManager.cpp, included below for its types). Check:
// compile this file and compare each function with
// src/reconstructed/GUIManager.bindings.json.
//
// 0x004853b0 (SetUp) is exact in src/reconstructed/GUIManager.cpp with a `short`
//   font-size parameter (docs/VC6_FRAME_LAYOUT.md: dead argument slots).
// 0x00485a70 (345 bytes): same calls and argument order; retail keeps
//   `flags & 4` in edi and `wait` in ebp, VC6 here keeps `a`/`parent` in
//   ebp (a `flags & 4` local makes it worse).
// 0x00486170 (896 bytes): same logic; retail keeps three more stack locals
//   (frame 0xb0 against 0xa4) and allocates registers differently around
//   the row copy and the 8-bit dim loop.
// 0x00486b10 (106 bytes): retail stores the centred x before loading the
//   control's bottom and passes the text in eax; VC6 here hoists the load
//   (also with a POINT, a centre helper, a `left` local, a pointer to the
//   array or a ShowText call in each branch).
// 0x00486b80 (553 bytes, ratio 0.97): retail computes the width limit as
//   `sub eax, 2` (VC6 here: `add eax, -2`) and spills width/height into the
//   SIZE slots later. VC6 emits `sub` only for a named `limit` local (the
//   repeated `field_0x0c - 2` is CSE'd as `add -2`), but that form moves the
//   textArea stores (218/554); updating `size.cx`/`size.cy` in place gives
//   retail's tail (457/555) and loses the head.
// 0x00488120 (49 bytes): retail keeps the `device ? owner : 0` branch
//   (jmp + xor) that VC6 here threads into the owner test (also with an
//   inline helper, an if/else assignment, early-return layouts, `owner == 0`
//   first, a result variable, a switch and an assignment in the condition).

#include "../../src/reconstructed/GUIManager.cpp"

// 0x00485a70
UIDialog* GUIManager::ShowDialog(UIDialog* dialog, int a, int flags, int b,
                                                   UIDialog* parent, int c, int d, int wait) {
    CameraRect screen;
    CameraRect* rect;

    if (!(flags & 4) && !(flags & 2)) {
        if (!FindInputDialog() || !FindInputDialog()->field_0x148)
            ReleaseBackgroundGrab();
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
            GrabBackground(flags & 8);
    }
    if (wait)
        UnknownFunction4865e0(waitCursorImage, field_0x1f0 == 0);
    if (dialog->UnknownVirtualSlot27(field_0x18, (CameraRect*)b, a, flags, guiSoundGroup, guiTextures, dialogDirectory,
                                     parent, guiBackground, dialogFontName, dialogFontSize, this, users[0],
                                     (const char*)d)) {
        dialog->field_0x144 = flags;
        UnknownFunction485bd0(dialog, a, wait);
        return dialog;
    }
    if (wait)
        UnknownFunction4865e0(cursorImage, field_0x1f0 == 0);
    return 0;
}

// 0x00486170
PCTextureMap* GUIManager::CopyScreenToTexture(int dim, CameraRect* rect) {
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

    format = g_TrackGame->renderTarget->field_0x28;
    bytesPerPixel = UnknownFunction511970(format);
    if (guiBackground)
        guiBackground->UnknownFunction404c80();
    g_TrackGame->field_0x34->UnknownFunction468dd0("GUICursor");
    g_TrackGame->field_0x34->UnknownFunction468dd0("ToolTip");
    g_TrackGame->UnknownVirtualSlot8();
    g_TrackGame->UnknownVirtualSlot9();
    g_TrackGame->renderTarget->UnknownFunction4e8cc0();
    g_TrackGame->field_0x34->UnknownFunction468f10("GUICursor");
    g_TrackGame->field_0x34->UnknownFunction468f10("ToolTip");
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    display = g_TrackGame->display;
    if (display->renderSurface)
        surface = display->renderSurface;
    else
        surface = display->primarySurface;
    if (!surface->Lock(0, &desc, 0x811, 0)) {
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
        surface->Unlock(0);
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
        texture = new(__FILE__, 916) PCTextureMap(guiTextures, 1);
        texture->UnknownVirtualSlot4(bits, width, height, width, width, format, format, 0, 4, 0, 0, 0, 2, 1, 0, 0x80,
                                     0xff00ff);
        DebugFree(bits, __FILE__, 921);
    }
    return texture;
}

// 0x00486b10
void ToolTip::UnknownFunction486b10(UnknownGuiControl* control) {
    int position[2];
    GUIInputDevice* device = control->ownerDialog->guiUser->pointerDevice;

    if (device) {
        position[0] = device->pointerPosition.x;
        position[1] = device->pointerPosition.y + 16;
    } else {
        position[0] = control->area.left + (control->area.right - control->area.left) / 2;
        position[1] = control->area.bottom;
    }
    ShowText(control->toolTipText, position, 1.0f);
}

// 0x00486b80
void ToolTip::ShowText(const char* text, int* position, float time) {
    PCRenderTarget* target;
    PCTextureMap* texture;
    HDC dc;
    HGDIOBJ previous;
    SIZE size;
    int width;
    int height;
    int x;

    if (textTexture) {
        textTexture->Release();
        textTexture = 0;
    }
    if (text) {
        if (tipGui->guiBackground && backgroundRegion == -1) {
            backgroundRegion = tipGui->guiBackground->UnknownFunction4040f0(0);
            restoreFrames = 3;
        }
        if (!((PCRenderTarget*)field_0x18)->renderSurface->GetDC((void**)&dc)) {
            previous = SelectObject(dc, (HGDIOBJ)tipGui->toolTipFont);
            GetTextExtentPoint32A(dc, text, strlen(text), &size);
            SelectObject(dc, previous);
            ((PCRenderTarget*)field_0x18)->renderSurface->ReleaseDC(dc);
            target = (PCRenderTarget*)field_0x18;
            width = size.cx + 2;
            if (width >= target->field_0x0c - 2)
                width = target->field_0x0c - 2;
            height = size.cy + 2;
            textArea.top = 0;
            textArea.left = 0;
            textArea.right = width;
            textArea.bottom = height;
            screenArea.left = position[0];
            screenArea.top = position[1];
            x = screenArea.left - width / 2;
            x = x < 0 ? 0 : x;
            screenArea.left = __min(target->field_0x0c - width, x);
            screenArea.top = __min(target->field_0x10 - height, screenArea.top);
            screenArea.right = screenArea.left + width;
            screenArea.bottom = screenArea.top + height;
            texture = tipGui->CreateFilledTexture(width, height, 0x808080);
            textTexture = texture;
            if (texture && texture->systemSurface) {
                if (!texture->systemSurface->GetDC((void**)&dc)) {
                    previous = SelectObject(dc, (HGDIOBJ)tipGui->toolTipFont);
                    SetTextColor(dc, 0xcccccc);
                    SetBkColor(dc, 0x404040);
                    DrawTextA(dc, text, strlen(text), &textArea, 0x8025);
                    SelectObject(dc, previous);
                    texture->systemSurface->ReleaseDC(dc);
                }
                shown = 1;
                showDelay = time;
                return;
            }
            shown = 0;
            showDelay = -1.0f;
            return;
        }
    }
    shown = 0;
    showDelay = -1.0f;
}

// 0x00488120
int GUIUser::SetPointerDevice(GUIInputDevice* device) {
    GUIUser* owner = device ? device->ownerUser : 0;

    if (owner) {
        if (owner != this)
            return 0;
    } else {
        pointerDevice = device;
        if (device)
            device->ownerUser = this;
    }
    return 1;
}

