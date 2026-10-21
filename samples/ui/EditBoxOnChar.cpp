// Complete nonexact edit-box character handler, retail 0x00474150/1044.
// Original translation-unit ownership is unproven. See docs/EDIT_BOX_ON_CHAR.md.
#include <string.h>
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/DialogEventKind.h"
#include "../../src/reconstructed/DialogProc.h"
#include "../../src/reconstructed/GUIManager.h"
#include "../../src/reconstructed/PCAudio.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/RenderInterfaces.h"

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
