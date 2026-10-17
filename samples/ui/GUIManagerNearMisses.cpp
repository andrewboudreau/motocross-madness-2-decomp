// Near misses for GUIManager.cpp (canonical file:
// src/reconstructed/GUIManager.cpp, included below for its types). Check:
// compile this file and compare each function with
// src/reconstructed/GUIManager.bindings.json.
//
// 0x004853b0 (SetUp) is exact in src/reconstructed/GUIManager.cpp with a `short`
//   font-size parameter (docs/VC6_FRAME_LAYOUT.md: dead argument slots).
// 0x00485a70 (345 bytes): same calls and argument order; retail keeps
//   `flags & 4` in edi and `wait` in ebp, VC6 here keeps `a`/`parent` in
//   ebp (a `flags & 4` local makes it worse: 21/347, the local lands in
//   ecx; also with the local assigned in the condition). Inverting the
//   first test (`(flags & 4) || (flags & 2)` with the grab code first)
//   compiles the same.
// 0x00486170 (896 bytes): same logic; retail keeps three more stack locals
//   (frame 0xb0 against 0xa4) and allocates registers differently around
//   the row copy and the 8-bit dim loop. Retail has two return tails
//   (`mov eax, edi` for the new texture, `mov eax, ebx` with ebx cleared
//   at the top for the Lock failure) and never uses that zero for the
//   null tests or `push 0`; VC6 here caches 0 in ebx for both. A
//   `return texture` in the success branch gives the two tails (304, 73
//   instruction diffs) but folds the failure return to `xor eax, eax`;
//   the texture's zero assigned at its declaration, after the format
//   read or just before the Lock test does not change that; an early
//   `return texture` or `return 0` on a Lock failure gives 294. The row
//   copy written without the row pointer (`memcpy(bits + y * width *
//   bytesPerPixel, ...)`) reproduces retail's copy loop instruction for
//   instruction (38 instruction diffs against 70) but scores 249/901;
//   the 8-bit dim loop as `for (x = line & 1; x < width; x += 2)
//   bits[line * width + x] = 0;` gives 68.
// 0x00486b10 (106 bytes, 89/106): one ternary per component (below; two
//   separate if/else statements compile the same) gives retail's block
//   order and `pop edi` placement (one if/else block: 86/106). Retail still
//   stores the centred x before loading the control's bottom into eax and
//   passes the text in eax and &position in edx; VC6 here hoists the bottom
//   load into edx beside the cdq and swaps the call registers. Tried: y
//   before x (81), `!device` ternaries/ifs, a POINT, a pointer to the array
//   or `*p++` stores, `&position[0]`, x/y/bottom/text locals, casts or
//   parentheses on the text, a centre helper, a ShowText call in each
//   branch, and the centre written as `width / 2 + left`, a `width` or
//   `left` local, or `left` then `+=`; parentheses around either whole
//   ternary, around its arms or around the centre leave it unchanged.
//   Swept (tools/sweep_variants.py): a `CameraRect& area` alias on every
//   subset of the four area reads x a text local x x/y order; best 89.
//   Also <= 89 (48 forms): a `GUIUser* user` local, x and y as if/else
//   assignments of locals (with a `half` local), a text local, a `tip = this`
//   alias and an `int* p = position` local.
// 0x00488120 (52 bytes, 46/52): retail keeps the `device ? owner : 0` branch
//   (jmp + xor) and tests the owner for zero before comparing it with
//   `this`. VC6 threads the xor arm into the next test whenever that test is
//   the zero test (inline helper, if/else assignment, early-return layouts,
//   `owner == 0` first, a result variable, a switch, an assignment in the
//   condition, an int-cast ternary, `owner && owner != this`, an `owned`
//   flag, nested if/else, goto, a `const` or `void*` owner, a `self` local,
//   `!device ? 0 : owner`, `this == owner`); comparing with `this` first
//   (below) keeps the branch, so only the order of the two tests differs.
//   Also threaded (15/52): an inline owner helper (early returns or a
//   ternary) with nested `if (owner) { if (owner == this) ... }`, `!= this`
//   or `return owner == this;`, a `(GUIUser*)0` or `device != 0` ternary,
//   a parenthesised ternary and an extra `hasOwner` local. Also threaded
//   (6-18/52): `owner = 0; if (device) owner = ...` with nested tests, a
//   `result` or `same` local, `return owner == this;` after a zero test, a
//   nested ternary and a `while (owner)` wrapper.

#include "../../src/reconstructed/GUIManager.cpp"

// 0x00485a70
UIDialog* GUIManager::ShowDialog(UIDialog* dialog, int a, int flags, int b,
                                                   UIDialog* parent, int c, int d, int wait) {
    CameraRect screen;
    CameraRect* rect;

    if (!(flags & 4) && !(flags & 2)) {
        if (!FindInputDialog() || !FindInputDialog()->isModal)
            ReleaseBackgroundGrab();
    } else {
        rect = 0;
        if (flags & 0x10) {
            screen.left = screen.top = 0;
            screen.right = ((RenderTarget*)renderContext)->targetWidth;
            screen.bottom = ((RenderTarget*)renderContext)->targetHeight;
            rect = &screen;
        }
        if (parent)
            parent->SetModalPause(1, flags & 8, rect);
        else if ((flags & 4) && (flags & 0x10))
            GrabBackground(flags & 8);
    }
    if (wait)
        RecreateCursors(waitCursorImage, paletteChanged == 0);
    if (dialog->UnknownVirtualSlot27(renderContext, (CameraRect*)b, a, flags, guiSoundGroup, guiTextures, dialogDirectory,
                                     parent, guiBackground, dialogFontName, dialogFontSize, this, users[0],
                                     (const char*)d)) {
        dialog->showFlags = flags;
        AttachDialog(dialog, a, wait);
        return dialog;
    }
    if (wait)
        RecreateCursors(cursorImage, paletteChanged == 0);
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

    format = g_TrackGame->renderTarget->surfacePixelFormat;
    bytesPerPixel = PixelFormatBytesPerPixel(format);
    if (guiBackground)
        guiBackground->ReleaseDc();
    g_TrackGame->childRoot->DisableByClassName("GUICursor");
    g_TrackGame->childRoot->DisableByClassName("ToolTip");
    g_TrackGame->RenderFrame();
    g_TrackGame->PresentFrame();
    g_TrackGame->renderTarget->AdvanceFrame();
    g_TrackGame->childRoot->EnableByClassName("GUICursor");
    g_TrackGame->childRoot->EnableByClassName("ToolTip");
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    display = g_TrackGame->display;
    if (display->renderSurface)
        surface = display->renderSurface;
    else
        surface = display->primarySurface;
    if (!surface->Lock(0, &desc, 0x811, 0)) {
        if (rect) {
            right = rect->right;
            left = rect->left;
            top = rect->top;
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
        texture->CreateSurface(bits, width, height, width, width, format, format, 0, 4, 0, 0, 0, 2, 1, 0, 0x80,
                               0xff00ff);
        DebugFree(bits, __FILE__, 921);
    }
    return texture;
}

// 0x00486b10
void ToolTip::ShowControlToolTip(UnknownGuiControl* control) {
    int position[2];
    GUIInputDevice* device = control->ownerDialog->guiUser->pointerDevice;

    position[0] = device ? device->pointerPosition.x
                         : control->area.left + (control->area.right - control->area.left) / 2;
    position[1] = device ? device->pointerPosition.y + 16 : control->area.bottom;
    ShowText(control->toolTipText, position, 1.0f);
}

// 0x00488120
int GUIUser::SetPointerDevice(GUIInputDevice* device) {
    GUIUser* owner = device ? device->ownerUser : 0;

    if (owner == this)
        return 1;
    if (owner)
        return 0;
    pointerDevice = device;
    if (device)
        device->ownerUser = this;
    return 1;
}
