// UIEditBox primary slot 40, retail 0x004738a0..0x00473c6c.
// Reviewed candidate; original translation-unit ownership is not established.
// See docs/EDIT_BOX_DRAW.md for complete extent and finite behavior evidence.

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/GameUi.h"

#include "../../src/reconstructed/BackgroundImage.h"
#include "../../src/reconstructed/ContainerList.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/DialogEventKind.h"
#include "../../src/reconstructed/DialogProc.h"
#include "../../src/reconstructed/GUIManager.h"
#include "../../src/reconstructed/InputDevice.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/MatrixUtil.h"
#include "../../src/reconstructed/PCAudio.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Palette8.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/RenderTarget.h"
#include "../../src/reconstructed/RenderInterfaces.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Tgafile.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/UnknownResourceManager.h"


#include <imm.h>
#include "../../src/reconstructed/Display.h"

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
                // Retail stores to the member (`lea ebx, [esi + 0x214]`). The
                // member layout is preserved; this complete candidate remains
                // nonexact (975 source bytes versus 973 retail bytes).
                *(CameraRect*)caretRect = rect;
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
