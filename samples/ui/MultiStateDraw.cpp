// Complete retail 0x00478570..0x0047880f (672 bytes).
// Readable nonexact candidate; original translation-unit ownership is unproven.
// See docs/MULTI_STATE_DRAW.md for the actual-helper validation boundary.
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/BackgroundImage.h"
#include "../../src/reconstructed/PCTextureMap.h"

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
