// Near misses for gameui.cpp (canonical file: src/reconstructed/GameUi.cpp,
// included below for its types and matched functions). Check: compile this
// file and compare each function with GameUiNearMisses.bindings.json.
//
// 0x004705d0 UIControl slot 49 (91/101) and 0x004733a0 UIButton slot 49
//   (85/99): the value loaded for each call lands in another register
//   (retail: ecx/edx, VC6 here: edx/eax); an int SetFontColor, a local for the
//   state, `if (enabled)` and the inverted branch order give the same bytes;
//   so do an `enabled != enable` block, one colour local for both calls,
//   inline wrappers for both calls, inline accessors for every operand (32
//   combinations) and an int-returning slot 29/SetFontColor. An extra member
//   copy in the first branch moves the second branch's registers, so the
//   choice is function-wide. Swept with no gain (576 + 128 forms for
//   UIControl, 64+ for UIButton): early return or one block, inverted
//   branches, `!enable`, locals for the old state/savedState/currentState/
//   currentTextColor/textColor/stateImages[4] (also as `UIAnim*`), `!= 0`
//   image tests, parentheses on each operand and per-branch `enabled`
//   stores. Retail UIControl and UIButton pick opposite registers for the
//   same fragments, so the choice is an allocation tie-break. A
//   `UIControl* self = this;` copy used for any subset of the 13 member
//   accesses (120 of 8192 sampled with tools/sweep_variants.py) is folded
//   away: every form gives 91. Also 91/85: casts on the colours and on
//   `enabled` in the comparison (18 forms), template inline helpers taking
//   `T*` (10), 1-8 extern ints or 1-5 forward structs declared before the
//   functions (54), dead statements at the start of the function and of each
//   branch (125), function order, and in a standalone TU every int/unsigned/
//   long type for the states, `enabled` and the colours (char and short
//   `enabled` are worse).
// 0x00477e90 UIListBox slot 55 (322/342): an `int height` local for the
//   first GetRowHeight call gives retail's ebx (point) and ebp (row offset);
//   without it (155/344) the two are swapped. With the local, retail still
//   loads rect.top into ecx and the point's y before x and adds the height
//   into eax; VC6 here adds into the rect.top register and loads x first.
//   `height += rect.top`, `height + rect.top`, a `top` local, a POINT copy
//   before or after, the local at function scope or reused for the second
//   call, a nested flags test, `(POINT*)b` casts, an owner local, `point !=
//   0`, `found != 0`, guarded do-while rows and `rect.bottom = h; bottom +=
//   top` were tried (456 swept combinations, best 322). The form without the
//   local (`rect.bottom = rect.top + GetRowHeight(row);`, as in the exact
//   UIListBox slot 10 at 0x00476020) matches retail apart from the
//   ebx/ebp swap (ebp makes `[ebp]` take a disp8, so 344 bytes); no
//   declaration shift (k = 0..63) changes either form. An inline helper
//   `HitRow(rect, GetRowHeight(row), point)` storing `r.bottom = r.top + h`
//   gives retail's `add eax, <top>` (319/342, 11 instruction diffs); VC6
//   then still loads point->x before y.
// 0x0047b490 colour-key test (120/220): keeping the key pixel in the red
//   term's variable (`pixel |= green | blue`) keeps it in a register and
//   spills the row counter into the key's argument slot as retail does;
//   VC6 still ORs green|blue first where retail ORs red|green then blue
//   (no term order, parenthesisation or split changes the chain). The
//   strict 120 form tests `x < width` after the inner loop; the
//   structurally closest form (14 instruction diffs, 54 strict: the extent
//   shifts) keeps `found = 1; break;` in the inner loop and walks a row
//   pointer initialised in the for (`for (rows = .., row = bits; ..)`).
//   It still differs in the OR chain, in retail's edi save inside the row
//   loop (VC6 here copies the row pointer before the rows test, so edi is
//   saved in the prologue) and in the row step (retail `lea eax, [edx +
//   edx]; add edi, eax`, VC6 here `lea edi, [edi + edx*2]`; a char* step
//   and `&row[width]` give the same). Retail places `return 0` before
//   `return found` after Unlock; `if (!Unlock) return 0; return found;`
//   becomes neg/sbb/and, early returns for a failed Lock duplicate the
//   epilogue, and goto, result variable and while row loops do not give it.
// 0x0046ef00 UIDialog slot 10 (352/555): the timer's +0x0c is cleared before
//   +0x14 is loaded, and the joystick branches after the first sit after the
//   epilogue. `while (1)`/`for (;;)` device loops, `continue` per branch and
//   the joystick test as an inline helper (with and without returns) keep
//   them inline. So do `else continue` after the first branch's centred
//   test or its `if (next)` call, a `continue` after the centring store, a
//   `deviceKind != 2` continue and a `for` device loop (334); the x test
//   inverted (`x > -1` first) gives 347. Reading the target through
//   `timer->targetControl` twice moves the +0x0c store before the load as
//   retail does, but changes the call's registers (172); a control local
//   read before or after the store gives 352.
// 0x004734c0 UIButton slot 28 (256/328) and 0x00478e10 UIMultiState slot 28
//   (231/303; UIMultiState tests its own area in place, without UIButton's
//   RECT copy, which gives retail's 0x84 frame and head): retail keeps the 8-bit result in edi and the 16-bit one in
//   memory, tests Unlock's result and returns separately when Lock fails.
//   Ternary, separate-result and goto forms keep the result in memory; an
//   early return on Lock failure, an empty `if (Unlock)` and an inline
//   unlock-and-return helper compile the same, and an Unlock and return in
//   each format branch (cross-jumped) puts the result in ebp (76/323). A
//   `result = opaque` copy before Unlock (returned once or from both arms of
//   the Unlock test, or as `Unlock ? opaque : opaque`) is propagated away.
//   Also 256 or worse (20-form sweep): an explicit `else opaque = 0` in the
//   8-bit test (246), `opaque = byte != key`, `else if` and `format == 8 &&`
//   chains, crossed with a `result` copy, an ignored Unlock result and an
//   `Unlock(0) != 0` test. Setting `result = 1` and jumping to the unlock in
//   the 8-bit branch (`goto unlock`; the 16-bit branch keeps `opaque = 1`
//   and falls into `result = opaque;`) gives retail's prologue and
//   `mov edi, 1; jmp` (266/354, 332 bytes; 240/303 at 307 bytes for
//   0x00478e10), but VC6 still sets edi for the 16-bit hit instead of
//   writing the memory result, drops retail's dead `test eax, eax` after
//   Unlock and folds the Lock-failure return into `xor eax, eax`: retail
//   never treats `opaque` as a known constant. Not kept (a goto for a
//   partial gain).
// 0x004738a0 UIEditBox slot 40 (206/975): retail keeps the width and the
//   redraw count in memory.
// 0x00474150 UIEditBox slot 20 (1018/1044): Backspace reads the length and
//   the cursor into locals after the lead-byte test (retail loads both before
//   the branch and keeps them in ecx/eax across it). Retail gives the cursor
//   eax (loaded after `test eax, eax`), VC6 here edx, and the register
//   choice ripples into the Enter case (`eventCode` in eax). BOOL/char/
//   unsigned locals, function-scope locals, other declaration orders (worse)
//   and the locals before the call (worse) were tried. Also worse
//   (2026-10-11): only one of the two locals (1004 with `length`, 351 with
//   `cursor`), neither (339), the call as the if condition, and the two
//   decrements in the other order (1002). Also <= 1008 (about 60 forms): a
//   `char* text` local, `cursor + controlText`, a `count` local,
//   `&controlText[cursor]` with `length + 1 - cursor`, length-first stores,
//   an Enter-case reorder and all 6 declaration orders x 3 lead-test forms
//   x 3 decrement forms.
// 0x00478570 UIMultiState slot 40 (139/682): retail keeps &+0x1bc in ebx and
//   shares its spill slot with the DC.
// 0x0047a400 drop-down layout (702/962): the open area's bottom written from
//   the list (`top + (list.bottom - 2)`) gives retail's left, top, right
//   store order and the late scroll-bottom stores; with `top +
//   scroll.bottom` (any spelling, store order or read-back of area->top)
//   VC6 stores area->top last and adds into the scroll register (320).
//   Left: VC6 forms the bottom as `lea edx, [list.bottom + top - 2]` where
//   retail adds the scroll bottom's `sub eax, 2` value into top, and swaps
//   ebx/edi (zero and list top), which shifts the scratch registers of the
//   later sprintf/SetName blocks. The scroll.bottom adjustment's position,
//   the list rect's spellings, scroll declaration forms and a 0..47
//   typedef prefix leave 702; `+=` on the area members and the scroll
//   rect's sides set before its top/bottom were worse.
// 0x0046a920 UIDialog's resource parser (16 KB; 92% of instructions equal
//   with stack offsets and relocations ignored, 74% raw): the control flow,
//   calls and constants follow retail. Retail keeps the current control in
//   ebx across the section loop and the pass counter in memory; VC6 here
//   gives ebx to the pass counter (and to the state/item counters), so the
//   control is spilled. Retail's frame is 0x80 bytes larger: VC6 lays locals
//   out by uses per byte, ties permuted by a quicksort over the last-use
//   order (docs/VC6_FRAME_LAYOUT.md; `python3 tools/frame_layout.py OBJ
//   ReadDialogResource --va 0x46a920` maps the slots). The large arrays,
//   `parameters` and the most used key buffers have retail's offsets;
//   `toolTip` has four references here and two in retail, `defaultFontName`
//   two here and three in retail, the two-use key buffers leave the control
//   loop in another last-use order, three retail scalar slots are spill
//   homes the candidate keeps in registers, and retail has one 0x80-byte
//   local at frame offset 0x2174 (between `toolTip` and `fxSoundOut`) that no
//   instruction references and no source form reproduces (locals used only
//   in dead code get no slot). The jump table makes ckm report an unresolved
//   $L label; compare with sdiff.

#include "../../src/reconstructed/GameUi.cpp"

#include <imm.h>

#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Palette8.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x004705d0
void UIControl::SetEnabled(int enable) {
    if (enabled == enable)
        return;
    enabled = enable;
    if (enable) {
        SetState(savedState);
        SetFontColor(currentTextColor);
    } else {
        if (stateImages[4]) {
            savedState = currentState;
            SetState(4);
        }
        currentTextColor = textColor;
        SetFontColor(0x999999);
    }
}

// 0x004733a0
void UIButton::SetEnabled(int enable) {
    if (enabled == enable)
        return;
    enabled = enable;
    if (enable) {
        SetState(0);
        SetFontColor(currentTextColor);
    } else {
        if (stateImages[4]) {
            savedState = currentState;
            SetState(4);
        }
        currentTextColor = textColor;
        SetFontColor(0x999999);
    }
}

// 0x00477e90
int UIListBox::ButtonDown(int a, int b) {
    POINT* point = (POINT*)b;
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && point) {
        if (a == 0) {
            if (selectable) {
                int found = 0;
                RECT rect = *(RECT*)screenArea;
                for (int row = firstVisibleRow; row < firstVisibleRow + visibleRowCount; row++) {
                    int height = GetRowHeight(row);
                    rect.bottom = rect.top + height;
                    if (PtInRect(&rect, *point) && !(rowTable[row].flags & 1)) {
                        SelectRow(row);
                        ReportClick(&found);
                        break;
                    }
                    if (!found)
                        rect.top += GetRowHeight(row);
                }
                if (found)
                    return 1;
            }
        } else if (a == 1 && allowWScroll) {
            dragStartX = point->x;
            return UIControl::ButtonDown(a, b);
        }
    }
    return UIControl::ButtonDown(a, b);
}

// ---------------------------------------------------------------------------
// Drawing, input and loading near misses

// cdecl 0x0047b490: whether `texture` has a pixel of colour `key`.
int TextureHasColor(TextureMap* texture, int key) {
    int found = 0;
    unsigned short* bits = (unsigned short*)texture->LockSurface(0, 0, 0x11);
    if (bits) {
        int pixel;
        int green;
        if (texture->mapFormat == 0x235) {
            pixel = (key >> 8) & 0xf800;
            green = (key >> 5) & 0x7e0;
        } else if (texture->mapFormat == 0x22b) {
            pixel = (key >> 9) & 0x7c00;
            green = (key >> 6) & 0x3e0;
        } else {
            goto unlock;
        }
        pixel |= green | ((key >> 3) & 0x1f);
        if (pixel) {
            for (int rows = texture->textureHeight; rows > 0; rows--) {
                int x;
                for (x = 0; x < texture->textureWidth; x++) {
                    if (bits[x] == pixel)
                        break;
                }
                if (x < texture->textureWidth)
                    found = 1;
                bits += texture->textureWidth;
            }
        }
unlock:
        if (!texture->UnlockSurface(0))
            return 0;
        return found;
    }
    return 0;
}


// 0x0046ef00: runs the timers and the joystick focus moves.
int UIDialog::Tick(float frameTime) {
    if (lastFrameTime != 0.0f)
        lastFrameTime = frameTime;
    else
        lastFrameTime = 0.001f;
    if (sendFrameEvent) {
        UnknownDialogEvent event;
        event.gui = guiManager;
        event.handled = 0;
        event.kind = kDialogFrame;
        event.controlName = 0;
        event.dialog = this;
        event.control = 0;
        HandleEvent(&event);
        sendFrameEvent = 0;
    }
    if (!isPaused && !isClosing) {
        UITimer* timer;
        for (int i = 0; (timer = timerList.Get(i)) != 0; i++) {
            timer->elapsedTime += (int)(frameTime * 1000.0f);
            if ((unsigned int)timer->elapsedTime >= (unsigned int)timer->periodTime) {
                timer->elapsedTime = 0;
                UIControl* control;
                if ((control = timer->targetControl) != 0) {
                    control->HandleTimer((int)timer);
                } else {
                    UnknownDialogEvent event;
                    event.code = timer->timerId;
                    event.gui = guiManager;
                    event.handled = 0;
                    event.kind = kDialogTimer;
                    event.controlName = 0;
                    event.dialog = this;
                    event.control = 0;
                    HandleEvent(&event);
                    if (event.handled)
                        return 1;
                }
            }
        }
        GUIUser* user = guiUser;
        UIControl* focus = (UIControl*)user->hoverControl;
        user->SetCursorAnimation(focus && focus->mouseAnim ? (UnknownCursorAnimation*)focus->mouseAnim
                                                                 : cursorAnimation);
        GUIInputDevice* device;
        int j = 0;
        while ((device = guiUser->GetAcceptedDevice(j++)) != 0) {
            if (device->inputDevice->deviceKind == 2) {
                int x = device->pointerPosition.x;
                int y = device->pointerPosition.y;
                if (focus) {
                    if (x <= -1) {
                        if (joystickCentred) {
                            UIControl* next = focus->PreviousFocusableControl();
                            joystickCentred = 0;
                            if (next)
                                RequestHover(next, 0, 0);
                        }
                    } else if (x >= 1) {
                        if (joystickCentred) {
                            UIControl* next = focus->NextFocusableControl();
                            joystickCentred = 0;
                            if (next)
                                RequestHover(next, 0, 0);
                        }
                    } else if (y <= -1) {
                        if (joystickCentred)
                            joystickCentred = 0;
                    } else if (y >= 1) {
                        if (joystickCentred)
                            joystickCentred = 0;
                    } else {
                        joystickCentred = 1;
                    }
                }
            }
        }
        UpdatePointerHover();
        GameObject::Tick(lastFrameTime);
    }
    return 1;
}

// 0x004734c0: whether `point` is on an opaque pixel of the image.
int UIButton::ContainsPoint(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->surfacePixelFormat;
    RECT rect = *(RECT*)screenArea;
    if (PtInRect(&rect, point)) {
        point.x -= rect.left;
        point.y -= rect.top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)GetStateTexture(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / PixelFormatBytesPerPixel(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}

// 0x00478e10: the same test as UIButton slot 28 (0x004734c0), but on the
// control's own area (no RECT copy: retail passes &screenArea to PtInRect).
int UIMultiState::ContainsPoint(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->surfacePixelFormat;
    if (PtInRect((RECT*)screenArea, point)) {
        point.x -= ((RECT*)screenArea)->left;
        point.y -= ((RECT*)screenArea)->top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)GetStateTexture(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / PixelFormatBytesPerPixel(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}

// 0x004738a0: draws the image, the text scrolled to keep the caret in view,
// and the caret.
int UIEditBox::Draw() {
    int width = GetWidth();
    int frames = -1;
    int redraw = redrawFrames;
    if (!controlText)
        SetEditText("");
    if (drawnTexture) {
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->DrawImage((PCTextureMap*)drawnTexture, (CameraRect*)screenArea,
                                                           visibleArea,
                                                           drawnTexture->usesColorKey ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            frames = redrawFrames;
            redrawFrames = redraw;
        } else {
            ((RenderTarget*)renderContext)->Blit(screenArea, drawnTexture, visibleArea,
                                                            drawnTexture->usesColorKey ? 0x1008000 : 0x1000000);
            redrawFrames = redraw;
        }
    } else {
        redrawFrames = redraw;
    }
    void* dc;
    int more;
    CameraRect rect;
    do {
        if (textAlign & 1) {
            int offset = width - textWidth;
            textScrollX = offset > 0 ? 0 : offset;
        } else if (textAlign & 2) {
            if (GetWidth() > textWidth)
                textScrollX = width / 2 - textWidth / 2;
            else
                textScrollX = width - textWidth;
        } else if (textAlign & 4) {
            textScrollX = width - textWidth;
        } else {
            int offset = width - textWidth;
            textScrollX = offset > 0 ? 0 : offset;
        }
        int x = screenArea[0] + textScrollX;
        int y;
        if (textAlign & 8)
            y = screenArea[1];
        else if (textAlign & 0x10)
            y = (screenArea[3] - screenArea[1]) / 2 - fontHeight / 2 + screenArea[1];
        else if (textAlign & 0x20)
            y = screenArea[3] - fontHeight;
        else
            y = screenArea[1];
        rect.left = x + textLeftMargin;
        rect.top = y + textTopMargin;
        rect.right = screenArea[2] - textLeftMargin;
        rect.bottom = screenArea[3] - textTopMargin;
        if (ownerDialog->BeginControlDraw(&dc, (CameraRect*)screenArea, &more, this)) {
            if (controlTextLength)
                ownerDialog->DrawAlignedText(dc, &rect, controlText, controlTextLength, textColor, fontHeight, 0,
                                                  textDrop, dropColor, 9, 1, 0);
            if (!more && ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                int caret = rect.left + textWidth;
                int limit = ownerDialog->screenArea.left + screenArea[0] + width - 1;
                if (limit < caret)
                    caret = limit;
                rect.left = caret;
                rect.top++;
                rect.right = caret + 1;
                int height = fontHeight;
                if (!height)
                    height = ownerDialog->dialogFontHeight;
                rect.bottom = rect.top + height + 1;
                CameraRect* caretRect = (CameraRect*)caretRect;
                *caretRect = rect;
                if (backgroundBrush)
                    FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)backgroundBrush);
                else
                    FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            }
            ownerDialog->EndControlDraw(dc);
            if (g_TrackGame->display->freezeFrameIndex &&
                ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                if (ownerDialog && ownerDialog->dialogBackground)
                    ownerDialog->dialogBackground->ReleaseDc();
                HIMC context = ImmGetContext((HWND)g_TrackGame->windowHandle);
                if (ImmGetOpenStatus(context)) {
                    COMPOSITIONFORM composition;
                    composition.dwStyle = CFS_FORCE_POSITION;
                    composition.ptCurrentPos.x = ((CameraRect*)caretRect)->left;
                    composition.ptCurrentPos.y = ((CameraRect*)caretRect)->top;
                    ImmSetCompositionWindow(context, &composition);
                }
                if (context)
                    ImmReleaseContext((HWND)g_TrackGame->windowHandle, context);
            }
        }
    } while (more);
    if (frames >= 0)
        redrawFrames = frames;
    if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this)
        RequestRedraw();
    else
        needsRedraw = 1;
    return 1;
}

// 0x00474150: typed characters: Backspace, Enter (kind 10) and inserted
// characters, double-byte ones in two steps; then measures the text and
// sends kind 0x13.
int UIEditBox::OnChar(int value) {
    UnknownDialogEvent event;
    char c = (char)value;
    int result = 0;
    event.handled = 0;
    if (!UIControl::OnChar(value) &&
        ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
        if (clickSound) {
            clickSound->SetVolume(ownerGui->soundVolume, 0);
            clickSound->PlayWithOptions(1, 0, 0);
        }
        switch (c) {
        case 8:
            if (controlTextLength && textLength > 0) {
                char* previous = CharPrevA(controlText, controlText + textLength);
                if (previous) {
                    int lead = IsDBCSLeadByte(*previous);
                    int length = controlTextLength;
                    int cursor = textLength;
                    if (lead) {
                        memmove(previous, controlText + cursor, length - cursor + 1);
                        controlTextLength -= 2;
                        textLength -= 2;
                    } else {
                        memmove(previous, controlText + cursor, length - cursor + 1);
                        controlTextLength--;
                        textLength--;
                    }
                }
            }
            break;
        case 9:
        case 0x1b:
            break;
        case 0xd:
            event.kind = kDialogEditDone;
            event.code = eventCode;
            event.controlName = GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->HandleEvent(&event);
            if (event.handled)
                return 1;
            break;
        default: {
            int lead = !leadBytePending && IsDBCSLeadByte(c) ? 1 : 0;
            if (!leadBytePending && !lead) {
                c = FilterCharacter(c);
                if (!c)
                    break;
            }
            leadBytePending = 0;
            int room;
            if (lead) {
                leadBytePending = 1;
                room = controlTextLength < textCapacity - 2;
                dropTrailByte = !room;
            } else if (dropTrailByte) {
                dropTrailByte = 0;
                room = 0;
            } else {
                room = controlTextLength < textCapacity - 1;
            }
            if (room) {
                memmove(controlText + textLength + 1, controlText + textLength, controlTextLength - textLength + 1);
                controlText[textLength] = c;
                textLength++;
                controlTextLength++;
            } else if (fullSound) {
                fullSound->SetVolume(ownerGui->soundVolume, 0);
                fullSound->PlayWithOptions(1, 0, 0);
            }
            break;
        }
        }
        controlText[controlTextLength] = 0;
        void* dc;
        if (!((PCRenderTarget*)ownerDialog->UnknownInlineField18())->renderSurface->GetDC(&dc)) {
            HGDIOBJ font = SelectObject((HDC)dc, (HGDIOBJ)(fontHandle ? fontHandle : (int)ownerDialog->dialogFont));
            SIZE size;
            GetTextExtentPoint32A((HDC)dc, controlText, controlTextLength, &size);
            textExtent[1] = size.cx;
            GetTextExtentPoint32A((HDC)dc, controlText, textLength, &size);
            textWidth = size.cx;
            SelectObject((HDC)dc, font);
            ((PCRenderTarget*)ownerDialog->UnknownInlineField18())->renderSurface->ReleaseDC(dc);
        }
        RequestRedraw();
        redrawFrames++;
        event.kind = kDialogEditChange;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
        result = 1;
    }
    return result;
}

// 0x00478570: draws the state's image, then its text.
int UIMultiState::Draw() {
    int frames = -1;
    int redraw = redrawFrames;
    CameraRect rect;
    if (stateTable && drawnTexture) {
        TextureMap* texture = drawnTexture;
        rect = *(CameraRect*)screenArea;
        if (sourceBlit) {
            rect.right = (int)(texture->textureWidth * ownerDialog->scaleX) + rect.left;
            rect.bottom = (int)(texture->textureHeight * ownerDialog->scaleY) + rect.top;
        }
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->DrawImage((PCTextureMap*)texture, &rect, 0,
                                                           texture->usesColorKey ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            frames = redrawFrames;
        } else if (!((RenderTarget*)renderContext)->Blit(&rect, texture, 0,
                                                                     texture->usesColorKey ? 0x1008000 : 0x1000000)) {
            return 0;
        }
    }
    if ((stateTable && stateTable[selectedState].text && stateTable[selectedState].textLength) ||
        (sourceBlit && controlText && controlTextLength)) {
        void* dc;
        int more;
        redrawFrames = redraw;
        do {
            if (ownerDialog->BeginControlDraw(&dc, (CameraRect*)screenArea, &more, this)) {
                if (sourceBlit) {
                    rect = *(CameraRect*)screenArea;
                    rect.left += stateTable[selectedState].image->GetCurrentFrame()->frameWidth +
                                 textLeftMargin;
                    rect.right -= textLeftMargin;
                    rect.top += textTopMargin;
                    rect.bottom -= textTopMargin;
                    ownerDialog->DrawAlignedText(dc, &rect, controlText, controlTextLength, textColor, fontHeight,
                                                      (UnknownGameUiTextRun*)textLines, textDrop, dropColor,
                                                      0x11, 1, 0);
                } else {
                    ownerDialog->DrawControlText(dc, this, 1);
                }
                ownerDialog->EndControlDraw(dc);
            }
        } while (more);
        if (frames >= 0)
            redrawFrames = frames;
    }
    return 1;
}

// 0x0047a400: names the parts after the list ("<name>_BUT", "_LB", "_SB",
// "_LBK", "_SBK", "_TBK") and lays them out below and beside the text.
void UIDropDownList::LayoutParts() {
    int left = screenArea[0];
    CameraRect full;
    full.left = 0;
    full.top = 0;
    full.right = screenArea[2] - left;
    int top = screenArea[1];
    full.bottom = screenArea[3] - top;
    CameraRect button;
    UIAnim* image = buttonPart->stateImages[0];
    if (image) {
        button = full;
        button.left = full.right + 1;
        button.right = image->frameList[0]->frameWidth + button.left;
    } else {
        button.left = 0;
        button.top = 0;
        button.right = 0;
        button.bottom = 0;
    }
    CameraRect list = full;
    list.top = full.bottom + 1;
    list.bottom = rowHeight + list.top;
    CameraRect scroll = list;
    scroll.top += 2;
    scroll.bottom -= 2;
    scroll.left = button.left;
    scroll.right = button.right;
    CameraRect* area = (CameraRect*)openArea;
    char name[50];
    area->left = left;
    area->top = top;
    area->right = left + scroll.right;
    area->bottom = top + (list.bottom - 2);
    sprintf(name, "%s_BUT", GetName());
    buttonPart->SetName(name);
    *(CameraRect*)buttonPart->screenArea = button;
    *(CameraRect*)buttonPart->bounds = *(CameraRect*)buttonPart->screenArea;
    if (!button.left)
        buttonPart->Disable();
    sprintf(name, "%s_LB", GetName());
    listPart->SetName(name);
    *(CameraRect*)listPart->screenArea = list;
    *(CameraRect*)listPart->bounds = *(CameraRect*)listPart->screenArea;
    sprintf(name, "%s_SB", GetName());
    scrollBarPart->SetName(name);
    *(CameraRect*)scrollBarPart->screenArea = scroll;
    *(CameraRect*)scrollBarPart->bounds = *(CameraRect*)scrollBarPart->screenArea;
    sprintf(name, "%s_LBK", GetName());
    staticPart0->SetName(name);
    *(CameraRect*)staticPart0->screenArea = list;
    *(CameraRect*)staticPart0->bounds = *(CameraRect*)staticPart0->screenArea;
    sprintf(name, "%s_SBK", GetName());
    staticPart1->SetName(name);
    *(CameraRect*)staticPart1->screenArea = scroll;
    *(CameraRect*)staticPart1->bounds = *(CameraRect*)staticPart1->screenArea;
    sprintf(name, "%s_TBK", GetName());
    staticPart2->SetName(name);
    *(CameraRect*)staticPart2->screenArea = full;
    *(CameraRect*)staticPart2->bounds = *(CameraRect*)staticPart2->screenArea;
    listPart->textLeftMargin = textLeftMargin;
    listPart->textTopMargin = textTopMargin;
    static_cast<UIListBox*>(listPart)->SetRowTextColor(textColor, -1);
    if (textColor != 0xffffff)
        static_cast<UIListBox*>(listPart)->SetSelectColor(0xffffff);
    listPart->SetDropColor(dropColor);
}

// ---------------------------------------------------------------------------
// The dialog resource parser

// 0x0065b600: the last multi-state text the parser passed on.
char* g_UnknownGlobal65b600;

// 0x0046a920: reads the dialog resource at `offset` of `stream`. Its sections
// are listed between markers ("Set_Anim", "Set_Sound", "Set_Control") into
// the dialog's named entries (+0x9cc), then read in four passes: "Set_Info"
// (pass 0), the images (1), the sounds (2) and the controls with their
// "Set_Default" sections (3).
int UIDialog::ReadDialogResource(void* stream, int offset) {
    char backgroundImage[0x80];
    backgroundImage[0] = 0;
    int depth = g_TrackGame->renderTarget->surfacePixelFormat;
    memset(sectionTable, 0, sizeof(sectionTable));
    int setType = 1;
    UnknownParameterBlock parameters;
    parameters.Attach((UnknownParameterStream*)stream, offset, 1);
    char buffer[0x2800];
    parameters.GetSectionNames(buffer, sizeof(buffer));
    UnknownTokenizer* sections = new(__FILE__, 0x273) UnknownTokenizer(buffer);
    for (char* name = sections->NextToken("?"); name; name = sections->NextToken("?")) {
        if (!_strnicmp(name, "Set_Anim", strlen("Set_Anim"))) {
            setType = 1;
        } else if (!_strnicmp(name, "Set_Sound", strlen("Set_Sound"))) {
            setType = 2;
        } else if (!_strnicmp(name, "Set_Control", strlen("Set_Control"))) {
            setType = 4;
        } else {
            strcpy(sectionTable[sectionCount].sectionName, name);
            sectionTable[sectionCount].sectionObject = 0;
            if (!_strnicmp(name, "Set_Info", strlen("Set_Info")))
                sectionTable[sectionCount].field_0x38 = 0;
            else if (!_strnicmp(name, "Set_Default", strlen("Set_Default")))
                sectionTable[sectionCount].field_0x38 = 3;
            else
                sectionTable[sectionCount].field_0x38 = setType;
            sectionCount++;
        }
    }
    delete sections;

    // The "Set_Default" sections change these.
    int defaultMaxItems = 100;
    int defaultScale = 100;
    int defaultEnable = 1;
    int defaultShow = 1;
    int defaultSelectable = 1;
    int defaultTextColor = 0xffff;
    int defaultTextDrop = 1;
    int defaultColors[10];
    defaultColors[4] = 0xffff;
    int defaultSelectBoxColor = 0xff000000;
    int defaultItemBoxColor = 0xff000000;
    char defaultType[0x80];
    strcpy(defaultType, "");
    int defaultGroupId = 0;
    char defaultText[0x80];
    strcpy(defaultText, "");
    int defaultAttachId = 0;
    int defaultNumStates = 2;
    int defaultDefault = 0;
    int defaultAutoSort = 0;
    int defaultSelectColor = 0xff88ff;
    int defaultAllowWScroll = 0;
    int defaultTickNum = 0;
    int defaultFXDelay = 0;
    int defaultMaxLen = 1000;
    int defaultFontHeight = 0;
    int defaultDropColor = 0;
    int defaultKeyBind = 0;
    int defaultBold = 0;
    int defaultItalic = 0;
    int defaultSourceBlit = 0;
    int defaultPre3D = 0;
    int defaultPermanent = 0;
    int defaultShapeBounds = 0;
    int defaultListHeight = 20;
    int defaultRelAnchor = 0;
    int defaultMoveable = 0;
    defaultColors[0] = 0;
    defaultColors[1] = 0x8800;
    defaultColors[2] = 0x66ff;
    defaultColors[3] = 0xff6600;
    defaultColors[5] = 0xffffff;
    defaultColors[6] = 0xbbbbbb;
    defaultColors[7] = 0xbb55cc;
    defaultColors[8] = 0x880000;
    defaultColors[9] = 0x999999;
    char defaultFontName[0x80];
    strcpy(defaultFontName, "");
    char defaultMouseAnim[0x80];
    strcpy(defaultMouseAnim, "");
    char defaultSortLBName[0x80];
    strcpy(defaultSortLBName, "");
    char defaultAnchor[0x80];
    strcpy(defaultAnchor, "");
    char defaultTextAlign[0x80];
    strcpy(defaultTextAlign, "");
    char defaultTextAlignV[0x80];
    strcpy(defaultTextAlignV, "");
    char defaultFX[0x80];
    strcpy(defaultFX, "");
    char defaultFXSound[0x80];
    strcpy(defaultFXSound, "");
    char defaultFXAnimIn[0x80];
    strcpy(defaultFXAnimIn, "");
    char defaultFXAnimOut[0x80];
    strcpy(defaultFXAnimOut, "");
    char defaultFXSoundIn[0x80];
    strcpy(defaultFXSoundIn, "");
    char defaultFXSoundOut[0x80];
    strcpy(defaultFXSoundOut, "");
    char defaultAnimNorm[0x80];
    strcpy(defaultAnimNorm, "");
    char defaultAnimFocus[0x80];
    strcpy(defaultAnimFocus, "");
    char defaultAnimPush[0x80];
    strcpy(defaultAnimPush, "");
    char defaultAnimDisable[0x80];
    strcpy(defaultAnimDisable, "");
    char defaultAnimOff[0x80];
    strcpy(defaultAnimOff, "");
    char defaultAnimOn[0x80];
    strcpy(defaultAnimOn, "");
    char defaultSoundNorm[0x80];
    strcpy(defaultSoundNorm, "");
    char defaultSoundFocus[0x80];
    strcpy(defaultSoundFocus, "");
    char defaultSoundPush[0x80];
    strcpy(defaultSoundPush, "");
    char defaultSoundClick[0x80];
    strcpy(defaultSoundClick, "");
    char defaultSoundEnd[0x80];
    strcpy(defaultSoundEnd, "");
    char defaultAnimThumb[0x80];
    strcpy(defaultAnimThumb, "");
    char defaultAnimListBack[0x80];
    strcpy(defaultAnimListBack, "");
    char defaultAnimScrollBack[0x80];
    strcpy(defaultAnimScrollBack, "");
    char defaultAnimTextBack[0x80];
    strcpy(defaultAnimTextBack, "");
    char defaultStates[50][0x80];
    int n;
    for (n = 0; n < 50; n++)
        strcpy(defaultStates[n], "");
    char defaultStateTexts[50][0x80];
    for (n = 0; n < 50; n++)
        strcpy(defaultStateTexts[n], "");
    char defaultItems[50][0x80];
    for (n = 0; n < 50; n++)
        strcpy(defaultItems[n], "");

    // The current control's values (some keep the last control's).
    int value;
    int groupId;
    short item;
    CameraRect area;
    UIButton* button;
    UIMultiState* multiState;
    UIListBox* listBox;
    UIRadioButton* radioButton;
    UIStatic* staticControl;
    UIScrollBar* scrollBar;
    UIEditBox* editBox;
    UIStaticText* staticText;
    UIDropDownList* dropDownList;
    UIListBox* dropDownListBox;
    short state;
    int fontHeight;
    char type[0x80];
    UIControl* control;
    int isDefault;
    for (int pass = 0; pass < 4; pass++) {
        for (int i = 0; i < sectionCount; i++) {
            control = 0;
            isDefault = 0;
            parameters.SelectSection(sectionTable[i].sectionName);
            switch (sectionTable[i].field_0x38) {
            case 0:
                if (pass == 0) {
                    parameters.GetInt("ScreenWidth", g_TrackGame->renderTarget->targetWidth,
                                                     &screenWidth);
                    parameters.GetInt("ScreenHeight", g_TrackGame->renderTarget->targetHeight,
                                                     &screenHeight);
                    parameters.GetInt("NoScale", 0, &scaleToScreen);
                    scaleToScreen = !scaleToScreen;
                    parameters.GetInt("Popup", 1, &isPopup);
                    RenderTarget* target = (RenderTarget*)renderContext;
                    if (scaleToScreen) {
                        scaleX = (float)target->targetWidth / screenWidth;
                        scaleY = (float)target->targetHeight / screenHeight;
                    } else if (screenWidth * 2 > target->targetWidth ||
                               screenHeight * 2 > target->targetHeight) {
                        scaleX = scaleY = 1.0f;
                    }
                    if (isPopup) {
                        parameters.GetString("DlgAlignV", "MIDDLE", buffer, -1);
                        short align;
                        if (!_stricmp(buffer, "TOP"))
                            align = 8;
                        else
                            align = _stricmp(buffer, "BOTTOM") ? 0x10 : 0x20;
                        parameters.GetString("DlgAlignH", "CENTER", buffer, -1);
                        if (!_stricmp(buffer, "RIGHT"))
                            SetPopupAlignment(align | 4);
                        else if (!_stricmp(buffer, "LEFT"))
                            SetPopupAlignment(align | 1);
                        else
                            SetPopupAlignment(align | 2);
                    } else {
                        popupAlignment = 9;
                    }
                    for (int color = 0; color < 10; color++) {
                        sprintf(buffer, "%s%d", "TextColor", color);
                        parameters.GetInt(buffer, defaultColors[color], &value);
                        textColors[color] = value;
                    }
                    parameters.GetString("MouseCursorAnim", "", buffer, -1);
                    if (buffer[0])
                        cursorAnimation = (UnknownCursorAnimation*)FindSectionObject(buffer);
                    parameters.GetString("BackgroundFile", "", buffer, -1);
                    if (buffer[0] && dialogBackground) {
                        UnknownTgaFile* file = LoadFile(buffer, 0, (int)g_ResourceManager);
                        if (file) {
                            ownsBackground = 1;
                            if (parentDialog)
                                parentBackground = dialogBackground->field_0x2c;
                            dialogBackground->field_0x2c =
                                new(__FILE__, 0x367) PCTextureMap((TextureMapManager*)dialogTextures, 1);
                            dialogBackground->field_0x2c->CreateSurface(
                                file->bits, file->width, file->height, file->width, file->width, 0x22b, depth,
                                dialogPalette ? (UnknownTexturePalette*)((Palette8*)dialogPalette)->colorMapper : 0,
                                4, dialogPalette ? ((Palette8*)dialogPalette)->ddPalette : 0, 0, 0, 2, 1, 0,
                                0x80, 0xff00ff);
                            dialogBackground->MarkAllDirty();
                            FreeFile(file);
                        }
                    } else if (parentDialog) {
                        dialogBackground = parentDialog->dialogBackground;
                    }
                    char fontName[0x80];
                    parameters.GetString("FontName", "", fontName, -1);
                    float points;
                    parameters.GetFloat("PointSize", 0, &points);
                    int size = (int)points;
                    if (size < points)
                        size++;
                    parameters.GetFloat("FontHeight", 0, &points);
                    if (points > 0) {
                        size = (int)points;
                        if (size < points)
                            size++;
                    }
                    if (fontName[0] && size) {
                        if (dialogFont)
                            DeleteObject((HGDIOBJ)dialogFont);
                        dialogFontHeight = (int)(size * scaleY);
                        strcpy(dialogFontFace, fontName);
                        int bold;
                        int italic;
                        parameters.GetInt("Bold", 0, &bold);
                        parameters.GetInt("Italic", 0, &italic);
                        const char* face = guiManager ? guiManager->toolTipFontFace : "";
                        if (*face) {
                            strcpy(dialogFontFace, face);
                            bold = guiManager->toolTipBold;
                            italic = guiManager->toolTipItalic;
                        }
                        dialogFontHeight = (int)((guiManager ? *(float*)&guiManager->fontSizeOffset : 0.0f) +
                                                   dialogFontHeight);
                        dialogFont = CreateFontA(dialogFontHeight, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM, italic,
                                                         0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialogFontFace);
                        dialogFontItalic = italic;
                        dialogFontBold = bold;
                    }
                    if (g_TrackGame->renderTarget->surfacePixelFormat == 8) {
                        int load = 1;
                        if (guiManager) {
                            if (guiManager->UnknownFunction4864f0())
                                load = 0;
                            dialogPalette = guiManager->guiPalette;
                        }
                        parameters.GetString("PaletteFile", "", buffer, -1);
                        if (buffer[0] && load) {
                            UnknownResourceEntry* resource = g_ResourceManager->FindItem(buffer, 1);
                            if (resource) {
                                dialogPalette = new(__FILE__, 0x3b6) Palette8(resource->archive);
                                ownsPalette = 1;
                            }
                        }
                        if (guiManager && load)
                            guiManager->SetGuiPalette((Palette8*)dialogPalette);
                    }
                    parameters.GetString("BackgroundImage", "", backgroundImage, -1);
                    if (backgroundImage[0]) {
                        CameraRect frameArea;
                        frameArea.left = 0;
                        frameArea.top = 0;
                        frameArea.right = screenArea.right - screenArea.left;
                        frameArea.bottom = screenArea.bottom - screenArea.top;
                        UIStatic* frame = new(__FILE__, 0x3cd) UIStatic(0, &frameArea, this);
                        frame->SetName("FRMBIStatic");
                        frame->permanent = 1;
                        controlContainer->AppendChild(frame, -1);
                        SetupControl(frame, 0, 0);
                    }
                }
                break;
            case 1:
                if (pass == 1 && imageCount < 500) {
                    imageTable[imageCount] = new(__FILE__, 0x3df) UIAnim(dialogTextures, (void*)1);
                    int delay;
                    int loops;
                    parameters.GetInt("MSecDelay", 0, &delay);
                    parameters.GetInt("LoopCount", 0, &loops);
                    imageTable[imageCount]->SetFrameDelay(delay);
                    imageTable[imageCount]->SetFrameCount(loops);
                    parameters.GetString("Files1", "", buffer, -1);
                    UnknownTokenizer files(buffer);
                    for (char* file = files.NextToken(","); file; file = files.NextToken(",")) {
                        if (g_ResourceManager->FindItem(sectionTable[i].sectionName, 1))
                            imageTable[imageCount]->LoadFromModule(
                                sectionTable[i].sectionName, (int)g_ResourceManager, (int)soundGroup,
                                dialogPalette);
                        else if (g_ResourceManager->FindItem(file, 1))
                            imageTable[imageCount]->LoadFromModule(
                                file, (int)g_ResourceManager, (int)soundGroup, dialogPalette);
                        else
                            imageTable[imageCount]->LoadFile(file, dialogPalette);
                    }
                    sectionTable[i].sectionObject = (Sound*)imageTable[imageCount];
                    imageCount++;
                }
                break;
            case 2:
                if (pass == 2 && soundCount < 20) {
                    int isStatic;
                    int copies;
                    parameters.GetInt("Static", 1, &isStatic);
                    parameters.GetInt("Copies", 0, &copies);
                    parameters.GetString("File", "", buffer, -1);
                    int flags = isStatic ? 1 : 2;
                    if (g_ResourceManager->FindItem(sectionTable[i].sectionName, 1)) {
                        soundTable[soundCount] =
                            LoadEntrySound(soundGroup, sectionTable[i].sectionName, flags, 3, copies, -1);
                    } else if (g_ResourceManager->FindItem(buffer, 1)) {
                        soundTable[soundCount] =
                            LoadEntrySound(soundGroup, buffer, flags, 3, copies, -1);
                    } else {
                        soundTable[soundCount] = new(__FILE__, 0x41e) Sound(soundGroup, 1);
                        soundTable[soundCount]->Load(buffer, 0, flags, 3, copies, -1);
                    }
                    sectionTable[i].sectionObject = soundTable[soundCount];
                    soundCount++;
                }
                break;
            case 3:
                if (pass != 3)
                    break;
                isDefault = 1;
            case 4: {
                if (pass != 3)
                    break;
                area.top = 0;
                area.left = 0;
                area.bottom = 0;
                area.right = 0;
                if (!isDefault) {
                    if (parameters.GetString("Xy", "", buffer, -1)) {
                        sscanf(buffer, "%d%*[,.] %d", &area.left, &area.top);
                        area.right = area.left;
                        area.bottom = area.top;
                    }
                    if (parameters.GetString("Xywh", "", buffer, -1)) {
                        sscanf(buffer, "%d%*[,.] %d%*[,.] %d%*[,.] %d", &area.left, &area.top, &area.right,
                               &area.bottom);
                        area.right += area.left;
                        area.bottom += area.top;
                    }
                    if (parameters.GetString("Xyxy", "", buffer, -1))
                        sscanf(buffer, "%d%*[,.] %d%*[,.] %d%*[,.] %d", &area.left, &area.top, &area.right,
                               &area.bottom);
                }
                parameters.GetInt("GroupId", defaultGroupId, isDefault ? &defaultGroupId : &groupId);
                parameters.GetString("Type", defaultType, isDefault ? defaultType : type, -1);

                if (!_stricmp(type, "BUTTON") || !_stricmp(type, "SCROLLUP") || !_stricmp(type, "SCROLLDOWN") ||
                    isDefault) {
                    if (!isDefault) {
                        if (!_stricmp(type, "BUTTON")) {
                            control = new(__FILE__, 0x45b) UIButton(0, &area, this);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLDOWN")) {
                            control = new(__FILE__, 0x45c) UIScrollCtl(10, 0, &area, this);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLUP")) {
                            control = new(__FILE__, 0x45d) UIScrollCtl(9, 0, &area, this);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        }
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        button->SetNormalImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        button->SetFocusImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        button->SetPushImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimDisable", defaultAnimDisable,
                                                     isDefault ? defaultAnimDisable : buffer, -1);
                    if (!isDefault)
                        button->SetDisabledImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("SortLBName", defaultSortLBName,
                                                     isDefault ? defaultSortLBName : buffer, -1);
                    if (!isDefault)
                        button->SetSortList((UIListBox*)FindSectionObject(buffer));
                }

                if (!_stricmp(type, "MULTISTATE") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x474) UIMultiState(0, &area, this);
                        multiState = (UIMultiState*)AddControl(control, groupId, 0);
                    }
                    int states;
                    parameters.GetInt("NumStates", defaultNumStates,
                                                     isDefault ? &defaultNumStates : &states);
                    if (!isDefault)
                        static_cast<UIMultiState*>(multiState)->SetStateCount(states);
                    int selected;
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &selected);
                    if (!isDefault)
                        static_cast<UIMultiState*>(multiState)->SetCurrentState(selected);
                    state = 0;
                    char stateKey[0x80];
                    char textKey[0x80];
                    char stateBuffer[0x80];
                    char textBuffer[0x80];
                    sprintf(stateKey, "%s%d", "State", state);
                    sprintf(textKey, "%s%d", "StateText", state);
                    int gotState = parameters.GetString(stateKey, defaultStates[0],
                                                                    isDefault ? defaultStates[0] : stateBuffer, -1);
                    int gotText = parameters.GetString(textKey, defaultStateTexts[0],
                                                                   isDefault ? defaultStateTexts[0] : textBuffer, -1);
                    while (gotState | gotText) {
                        if (!isDefault) {
                            g_UnknownGlobal65b600 = textBuffer[0] ? textBuffer : 0;
                            multiState->SetStateEntry(state, (UIAnim*)FindSectionObject(stateBuffer),
                                                              g_UnknownGlobal65b600);
                        }
                        state++;
                        // Retail tests the list items' counter here.
                        if (isDefault && item >= 50)
                            break;
                        sprintf(stateKey, "%s%d", "State", state);
                        sprintf(textKey, "%s%d", "StateText", state);
                        gotState = parameters.GetString(stateKey, state < 50 ? defaultStates[state] : "",
                                                                    isDefault ? defaultStates[state] : stateBuffer, -1);
                        gotText = parameters.GetString(textKey, state < 50 ? defaultStateTexts[state] : "",
                                                                   isDefault ? defaultStateTexts[state] : textBuffer, -1);
                    }
                }

                if (!_stricmp(type, "LISTBOX") || isDefault) {
                    parameters.GetInt("MaxItems", 100, isDefault ? &defaultMaxItems : &value);
                    if (!isDefault) {
                        control = new(__FILE__, 0x4a3) UIListBox(0, value, &area, this);
                        listBox = (UIListBox*)AddControl(control, groupId, 0);
                    }
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SelectRow(value);
                    parameters.GetInt("AutoSort", defaultDefault, isDefault ? &defaultAutoSort : &value);
                    if (!isDefault)
                        listBox->SetAutoSort(value);
                    parameters.GetInt("SelectColor", defaultSelectColor,
                                                     isDefault ? &defaultSelectColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectColor(value);
                    parameters.GetInt("Selectable", defaultSelectable,
                                                     isDefault ? &defaultSelectable : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectable(value);
                    parameters.GetInt("AllowWScroll", defaultAllowWScroll,
                                                     isDefault ? &defaultAllowWScroll : &value);
                    if (!isDefault)
                        listBox->SetAllowWScroll(value);
                    parameters.GetInt("SelectBoxColor", defaultSelectBoxColor,
                                                     isDefault ? &defaultSelectBoxColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectBoxColor(value);
                    parameters.GetInt("ItemBoxColor", defaultItemBoxColor,
                                                     isDefault ? &defaultItemBoxColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetItemBoxColor(value, -1);
                    char itemKey[0x80];
                    item = 1;
                    sprintf(itemKey, "%s%d", "Item", 1);
                    if (parameters.GetString(itemKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault) {
                                if (buffer[0] == '@')
                                    static_cast<UIListBox*>(listBox)->AddImageRow((UIAnim*)FindSectionObject(buffer + 1), 0, 0);
                                else
                                    static_cast<UIListBox*>(listBox)->AddRow(buffer, 0, 0);
                            }
                            item++;
                            if (isDefault && item >= 50)
                                break;
                            sprintf(itemKey, "%s%d", "Item", item);
                        } while (parameters.GetString(itemKey, item < 50 ? defaultItems[item] : "",
                                                                  isDefault ? defaultItems[item] : buffer, -1));
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault) {
                        listBox->SetImage(0, FindSectionObject(buffer));
                        listBox->SetImage(1, FindSectionObject(buffer));
                        listBox->SetImage(2, FindSectionObject(buffer));
                        listBox->SetImage(4, FindSectionObject(buffer));
                    }
                }

                if (!_stricmp(type, "RADIOBUTTON") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4e3) UIRadioButton(0, &area, this);
                        radioButton = (UIRadioButton*)AddControl(control, groupId, 0);
                    }
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        static_cast<UIMultiState*>(radioButton)->SetCurrentState(value);
                    parameters.GetString("AnimOff", defaultAnimOff, isDefault ? defaultAnimOff : buffer, -1);
                    if (!isDefault)
                        radioButton->SetStateEntry(0, (UIAnim*)FindSectionObject(buffer), 0);
                    parameters.GetString("AnimOn", defaultAnimOn, isDefault ? defaultAnimOn : buffer, -1);
                    if (!isDefault)
                        radioButton->SetStateEntry(1, (UIAnim*)FindSectionObject(buffer), 0);
                }

                if (!_stricmp(type, "STATIC") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4f3) UIStatic(0, &area, this);
                        staticControl = (UIStatic*)AddControl(control, groupId, 0);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticControl->SetImage(0, FindSectionObject(buffer));
                }

                if (!_stricmp(type, "VSCROLLBAR") || !_stricmp(type, "HSCROLLBAR") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x500) UIScrollBar(7, 0, &area, this);
                        scrollBar = (UIScrollBar*)AddControl(control, groupId, 0);
                    }
                    if (!_stricmp(type, "HSCROLLBAR"))
                        control->controlType = 8;
                    int ticks;
                    int position;
                    int scale;
                    parameters.GetInt("TickNum", defaultTickNum, isDefault ? &defaultTickNum : &ticks);
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &position);
                    parameters.GetInt("Scale", defaultScale, isDefault ? &defaultScale : &scale);
                    if (!isDefault) {
                        static_cast<UIScrollBar*>(scrollBar)->SetSnapStepCount(ticks);
                        static_cast<UIScrollBar*>(scrollBar)->SetScrollRange(scale);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        scrollBar->trackImage = (UIAnim*)FindSectionObject(buffer);
                    parameters.GetString("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        scrollBar->SetImage(0, FindSectionObject(buffer));
                        scrollBar->SetImage(1, FindSectionObject(buffer));
                        scrollBar->SetImage(2, FindSectionObject(buffer));
                        scrollBar->SetImage(3, FindSectionObject(buffer));
                        if (control->bounds[2] == control->bounds[0]) {
                            UIAnim* image = control->stateImages[0];
                            control->bounds[2] = image->frameList[0]->frameWidth + control->bounds[0];
                            control->bounds[3] = control->bounds[1] + image->frameList[0]->frameHeight;
                        }
                        static_cast<UIScrollBar*>(scrollBar)->SetThumbPosition(position, ticks ? ticks
                                                                         : control->bounds[2] - control->bounds[0] + 1);
                        scrollBar->SetThumbSizeFromState(0);
                    }
                }

                if (!_stricmp(type, "EDITBOX") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x52c) UIEditBox(0, &area, this, 100, 0, 0);
                        editBox = (UIEditBox*)AddControl(control, groupId, 0);
                    }
                    parameters.GetInt("MaxLen", defaultMaxLen, isDefault ? &defaultMaxLen : &value);
                    if (!isDefault)
                        static_cast<UIEditBox*>(editBox)->SetCapacity(value);
                    parameters.GetString("SoundClick", defaultSoundClick,
                                                     isDefault ? defaultSoundClick : buffer, -1);
                    if (!isDefault)
                        editBox->SetClickSound((int)FindSectionObject(buffer));
                    parameters.GetString("SoundEnd", defaultSoundEnd, isDefault ? defaultSoundEnd : buffer, -1);
                    if (!isDefault)
                        editBox->SetFullSound((int)FindSectionObject(buffer));
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault) {
                        editBox->SetImage(0, FindSectionObject(buffer));
                        editBox->SetImage(1, FindSectionObject(buffer));
                        editBox->SetImage(2, FindSectionObject(buffer));
                        editBox->SetImage(4, FindSectionObject(buffer));
                    }
                }

                if (!_stricmp(type, "STATICTEXT") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x545) UIStaticText(0, &area, this, 0, 0xffff);
                        staticText = (UIStaticText*)AddControl(control, groupId, 0);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticText->SetImage(0, FindSectionObject(buffer));
                }

                if (!_strnicmp(type, "DROPDOWNLIST", 12) || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x54f) UIDropDownList(0, &area, this, 0, 0xffff);
                        dropDownList = (UIDropDownList*)AddControl(control, groupId, 0);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetNormalImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetFocusImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetPushImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        UIScrollBar* thumb = dropDownList->scrollBarPart;
                        thumb->SetImage(0, FindSectionObject(buffer));
                        thumb->SetImage(1, FindSectionObject(buffer));
                        thumb->SetImage(2, FindSectionObject(buffer));
                        thumb->SetImage(3, FindSectionObject(buffer));
                        thumb->SetThumbSizeFromState(0);
                    }
                    parameters.GetInt("ListHeight", defaultListHeight,
                                                     isDefault ? &defaultListHeight : &value);
                    if (!isDefault) {
                        dropDownList->LayoutForRowHeight(value);
                        dropDownListBox = dropDownList->listPart;
                    }
                    char listKey[0x80];
                    item = 1;
                    sprintf(listKey, "%s%d", "Item", 1);
                    if (parameters.GetString(listKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault)
                                static_cast<UIListBox*>(dropDownListBox)->AddRow(buffer, 0, 0);
                            item++;
                            if (isDefault && item >= 50)
                                break;
                            sprintf(listKey, "%s%d", "Item", item);
                        } while (parameters.GetString(listKey, item < 50 ? defaultItems[item] : "",
                                                                  isDefault ? defaultItems[item] : buffer, -1));
                    }
                    control->SetName(sectionTable[i].sectionName);
                    int leftMargin;
                    int topMargin;
                    int textColor;
                    int dropColor;
                    parameters.GetInt("TextLMargin", 0, &leftMargin);
                    parameters.GetInt("TextTMargin", 0, &topMargin);
                    parameters.GetInt("TextColor", defaultTextColor, &textColor);
                    parameters.GetInt("DropColor", defaultDropColor, &dropColor);
                    char soundNorm[0x80];
                    char soundFocus[0x80];
                    char soundPush[0x80];
                    char soundClick[0x80];
                    parameters.GetString("SoundNorm", defaultSoundNorm, soundNorm, -1);
                    parameters.GetString("SoundFocus", defaultSoundFocus, soundFocus, -1);
                    parameters.GetString("SoundPush", defaultSoundPush, soundPush, -1);
                    parameters.GetString("SoundClick", defaultSoundClick, soundClick, -1);
                    if (!isDefault) {
                        dropDownList->buttonPart->SetSound(0, FindSectionObject(soundNorm));
                        dropDownList->buttonPart->SetSound(1, FindSectionObject(soundFocus));
                        dropDownList->buttonPart->SetSound(2, FindSectionObject(soundPush));
                        dropDownList->buttonPart->SetSound(3, FindSectionObject(soundClick));
                        dropDownList->textLeftMargin = leftMargin;
                        dropDownList->textTopMargin = topMargin;
                        dropDownList->SetFontColor(textColor);
                        dropDownList->SetDropColor(dropColor);
                        dropDownList->LayoutParts();
                    }
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault) {
                        int row = value - 1 < 0 ? 0 : value - 1;
                        static_cast<UIListBox*>(dropDownListBox)->SelectRow(row);
                        dropDownList->SetText(static_cast<UIListBox*>(dropDownListBox)->GetRowText(row));
                    }
                    parameters.GetString("AnimTextBack", defaultAnimTextBack,
                                                     isDefault ? defaultAnimTextBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->SetCornerImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimListBack", defaultAnimListBack,
                                                     isDefault ? defaultAnimListBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->SetStaticImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimScrollBack", defaultAnimScrollBack,
                                                     isDefault ? defaultAnimScrollBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->SetButtonImage((UIAnim*)FindSectionObject(buffer));
                }

                // Every control's settings.
                char fontName[0x80];
                parameters.GetString("FontName", defaultFontName, fontName, -1);
                float height;
                parameters.GetFloat("FontHeight", (float)defaultFontHeight, &height);
                int size = (int)height;
                if (size < height)
                    size++;
                if (isDefault)
                    defaultFontHeight = size;
                else
                    fontHeight = size;
                char anchor[0x80];
                int relAnchor;
                int moveable;
                char textAlign[0x80];
                char textAlignV[0x80];
                int textLMargin;
                int textTMargin;
                int fxDelay;
                char fx[0x80];
                char fxSound[0x80];
                char fxAnimIn[0x80];
                char fxSoundIn[0x80];
                char fxAnimOut[0x80];
                char fxSoundOut[0x80];
                char mouseAnim[0x80];
                char soundNorm[0x80];
                char soundFocus[0x80];
                char soundPush[0x80];
                char soundClick[0x80];
                char text[0x80];
                int attachId;
                int textColor;
                int dropColor;
                int keyBind;
                int italic;
                int bold;
                int textDrop;
                int sourceBlit;
                int enable;
                int show;
                int pre3D;
                int permanent;
                int shapeBounds;
                char toolTip[0x80];
                parameters.GetString("Anchor", defaultAnchor, isDefault ? defaultAnchor : anchor, -1);
                parameters.GetInt("RelAnchor", defaultRelAnchor, isDefault ? &defaultRelAnchor : &relAnchor);
                parameters.GetInt("Moveable", defaultMoveable, isDefault ? &defaultMoveable : &moveable);
                parameters.GetString("TextAlign", defaultTextAlign, isDefault ? defaultTextAlign : textAlign, -1);
                parameters.GetString("TextAlignV", defaultTextAlignV, isDefault ? defaultTextAlignV : textAlignV,
                                                 -1);
                parameters.GetInt("TextLMargin", 0, &textLMargin);
                parameters.GetInt("TextTMargin", 0, &textTMargin);
                parameters.GetInt("FXDelay", defaultFXDelay, isDefault ? &defaultFXDelay : &fxDelay);
                parameters.GetString("FX", defaultFX, isDefault ? defaultFX : fx, -1);
                parameters.GetString("FXSound", defaultFXSound, isDefault ? defaultFXSound : fxSound, -1);
                parameters.GetString("FXAnimIn", defaultFXAnimIn, isDefault ? defaultFXAnimIn : fxAnimIn, -1);
                parameters.GetString("FXSoundIn", defaultFXSoundIn, isDefault ? defaultFXSoundIn : fxSoundIn, -1);
                parameters.GetString("FXAnimOut", defaultFXAnimOut, isDefault ? defaultFXAnimOut : fxAnimOut, -1);
                parameters.GetString("FXSoundOut", defaultFXSoundOut, isDefault ? defaultFXSoundOut : fxSoundOut,
                                                 -1);
                parameters.GetString("MouseAnim", defaultMouseAnim, isDefault ? defaultMouseAnim : mouseAnim, -1);
                parameters.GetString("SoundNorm", defaultSoundNorm, isDefault ? defaultSoundNorm : soundNorm, -1);
                parameters.GetString("SoundFocus", defaultSoundFocus, isDefault ? defaultSoundFocus : soundFocus,
                                                 -1);
                parameters.GetString("SoundPush", defaultSoundPush, isDefault ? defaultSoundPush : soundPush, -1);
                parameters.GetString("SoundClick", defaultSoundClick, isDefault ? defaultSoundClick : soundClick,
                                                 -1);
                parameters.GetString("Text", defaultText, isDefault ? defaultText : text, -1);
                parameters.GetInt("AttachId", defaultAttachId, isDefault ? &defaultAttachId : &attachId);
                parameters.GetInt("TextColor", defaultTextColor, isDefault ? &defaultTextColor : &textColor);
                parameters.GetInt("DropColor", defaultDropColor, isDefault ? &defaultDropColor : &dropColor);
                parameters.GetInt("KeyBind", defaultKeyBind, isDefault ? &defaultKeyBind : &keyBind);
                parameters.GetInt("Italic", defaultItalic, isDefault ? &defaultItalic : &italic);
                parameters.GetInt("Bold", defaultBold, isDefault ? &defaultBold : &bold);
                parameters.GetInt("TextDrop", defaultTextDrop, isDefault ? &defaultTextDrop : &textDrop);
                parameters.GetInt("SourceBlit", defaultSourceBlit, isDefault ? &defaultSourceBlit : &sourceBlit);
                parameters.GetInt("Enable", defaultEnable, isDefault ? &defaultEnable : &enable);
                parameters.GetInt("Show", defaultShow, isDefault ? &defaultShow : &show);
                parameters.GetInt("Pre3D", defaultPre3D, isDefault ? &defaultPre3D : &pre3D);
                parameters.GetInt("Permanent", defaultPermanent, isDefault ? &defaultPermanent : &permanent);
                parameters.GetInt("ShapeBounds", defaultShapeBounds,
                                                 isDefault ? &defaultShapeBounds : &shapeBounds);
                parameters.GetString("ToolTipText", "", toolTip, -1);
                if (isDefault)
                    break;

                control->SetAttachId(attachId);
                control->groupId = groupId;
                control->SetAnchor((UIControl*)FindSectionObject(anchor), relAnchor);
                control->SetName(sectionTable[i].sectionName);
                control->moveable = moveable;
                if (control->bounds[2] == control->bounds[0]) {
                    UIAnim* image;
                    if (!_stricmp(type, "MULTISTATE") || !_stricmp(type, "RADIOBUTTON"))
                        image = ((UIMultiState*)control)->stateTable[((UIMultiState*)control)->selectedState].image;
                    else
                        image = control->stateImages[0];
                    control->bounds[2] = image->frameList[0]->frameWidth + control->bounds[0];
                    control->bounds[3] = control->bounds[1] + image->frameList[0]->frameHeight;
                }
                if (fontName[0] || (fontHeight && guiManager && guiManager->dialogFontName)) {
                    control->ownsFont = 1;
                    if (control->fontHandle)
                        DeleteObject((HGDIOBJ)control->fontHandle);
                    control->fontHandle = 0;
                    if (fontHeight)
                        control->fontHeight = fontHeight;
                    if (bold > 500)
                        bold = 1;
                    else if (bold > 1)
                        bold = 0;
                    const char* face = guiManager ? guiManager->toolTipFontFace : "";
                    if (*face) {
                        strcpy(control->fontFace, face);
                        bold = guiManager->toolTipBold;
                        italic = guiManager->toolTipItalic;
                    }
                    control->fontHeight = (int)((guiManager ? *(float*)&guiManager->fontSizeOffset : 0.0f) +
                                                 control->fontHeight);
                    if (control->fontHeight != dialogFontHeight || bold != dialogFontBold ||
                        italic != dialogFontItalic) {
                        control->fontHeight = (int)(control->fontHeight * scaleY);
                        control->fontHandle = (int)CreateFontA(control->fontHeight, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM,
                                                                italic, 0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialogFontFace);
                        strcpy(control->fontFace, dialogFontFace);
                        control->bold = bold;
                        control->italic = italic;
                    }
                }
                if (sourceBlit)
                    control->sourceBlit = 1;
                control->SetShapeBounds(shapeBounds);
                if (guiManager->languageModule) {
                    int textId;
                    int toolTipId;
                    parameters.GetInt("LocalTextId", 0, &textId);
                    parameters.GetInt("LocalToolTipId", 0, &toolTipId);
                    if (textId)
                        control->SetTextFromResource(guiManager->languageModule, textId);
                    if (toolTipId) {
                        char tip[0x400];
                        if (LoadStringA((HINSTANCE)guiManager->languageModule, toolTipId, tip, sizeof(tip))) {
                            if (control->toolTipText)
                                DebugFree(control->toolTipText, __FILE__, 0x646);
                            control->toolTipText = (char*)DebugMalloc(strlen(tip) + 1, __FILE__, 0x647);
                            strcpy(control->toolTipText, tip);
                        } else {
                            sprintf(tip, "Resource string '%d' load fail\n", toolTipId);
                            if (control->toolTipText)
                                DebugFree(control->toolTipText, __FILE__, 0x64d);
                            control->toolTipText = 0;
                        }
                    }
                } else {
                    if (toolTip[0]) {
                        control->toolTipText = (char*)DebugMalloc(strlen(toolTip) + 1, __FILE__, 0x656);
                        strcpy(control->toolTipText, toolTip);
                    }
                    if (text[0])
                        control->SetText(text);
                }
                control->textLeftMargin = textLMargin;
                control->textTopMargin = textTMargin;
                control->SetTextDrop(textDrop != 0);
                control->SetFontColor(textColor);
                control->SetDropColor(dropColor);
                int align;
                if (!_stricmp(textAlignV, "TOP"))
                    align = 8;
                else
                    align = _stricmp(textAlignV, "BOTTOM") ? 0x10 : 0x20;
                if (!_stricmp(textAlign, "RIGHT")) {
                    align |= 4;
                } else if (!_stricmp(textAlign, "CENTER")) {
                    align |= 2;
                } else {
                    switch (control->controlType) {
                    case 1:
                    case 2:
                    case 4:
                    case 5:
                    case 9:
                    case 10:
                        align |= 2;
                        break;
                    default:
                        align |= 1;
                        break;
                    }
                }
                control->SetTextAlign(align);
                if (!_stricmp(fx, "GROW")) {
                    control->StartTransition(100, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "LSLIDE")) {
                    control->StartTransition(101, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "TSLIDE")) {
                    control->StartTransition(103, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "RSLIDE")) {
                    control->StartTransition(102, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "BSLIDE")) {
                    control->StartTransition(104, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "ANIM")) {
                    control->StartTransition(106, 0, 1, 0, -1);
                    control->fxAnimIn = (UIAnim*)FindSectionObject(fxAnimIn);
                    control->fxAnimOut = (UIAnim*)FindSectionObject(fxAnimOut);
                    control->fxSoundIn = FindSectionObject(fxSoundIn);
                    control->fxSoundOut = FindSectionObject(fxSoundOut);
                }
                control->SetSound(0, FindSectionObject(soundNorm));
                control->SetSound(1, FindSectionObject(soundFocus));
                control->SetSound(2, FindSectionObject(soundPush));
                control->SetSound(3, FindSectionObject(soundClick));
                control->SetEnabled(enable != 0);
                control->Show(show != 0, 1);
                control->mouseAnim = (int)FindSectionObject(mouseAnim);
                control->keyBind = keyBind;
                control->post3D = pre3D == 0;
                control->permanent = permanent;
                control->drawnTexture = control->GetStateTexture(-1);
                control->ScaleArea();
                sectionTable[i].sectionObject = (Sound*)control;
                break;
            }
            }
        }
    }

    if (backgroundImage[0]) {
        UIControl* frame = FindControl("FRMBIStatic", 5);
        UIAnim* image = (UIAnim*)FindSectionObject(backgroundImage);
        if (image) {
            int width = image->GetCurrentTexture()->textureWidth;
            int imageHeight = image->GetCurrentTexture()->textureHeight;
            frame->SetImage(0, image);
            frame->screenArea[2] = width;
            frame->screenArea[3] = imageHeight;
            *(CameraRect*)frame->bounds = *(CameraRect*)frame->screenArea;
            frame->drawnTexture = frame->GetStateTexture(0);
            screenArea.right = screenArea.left + width;
            screenArea.bottom = screenArea.top + imageHeight;
            frame->ScaleArea();
        }
    }
    if (isPopup && screenWidth && screenHeight) {
        int align = popupAlignment;
        RenderTarget* target = (RenderTarget*)renderContext;
        if (align & 1) {
            screenArea.right -= screenArea.left;
            screenArea.left = 0;
        } else if (align & 4) {
            int shift = target->targetWidth - screenArea.left * 2 - screenArea.right;
            screenArea.left += shift;
            screenArea.right += shift;
        } else {
            int shift = (target->targetWidth - screenArea.left - screenArea.right) / 2 -
                        screenArea.left;
            screenArea.left += shift;
            screenArea.right += shift;
        }
        if (align & 8) {
            screenArea.bottom -= screenArea.top;
            screenArea.top = 0;
        } else if (align & 0x20) {
            int shift = target->targetHeight - screenArea.top * 2 - screenArea.bottom;
            screenArea.top += shift;
            screenArea.bottom += shift;
        } else {
            int shift = (target->targetHeight - screenArea.top - screenArea.bottom) / 2 -
                        screenArea.top;
            screenArea.top += shift;
            screenArea.bottom += shift;
        }
    }
    return 1;
}
