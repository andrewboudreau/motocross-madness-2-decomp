// Complete nonexact list-box mouse selection; original TU ownership is unproven.
// Retail 0x00477e90 spans 342 bytes. See docs/LIST_BOX_BUTTON_DOWN.md.
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/GUIManager.h"

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
                    // Retail reads y before x, including when point is invalid.
                    POINT hitPoint;
                    hitPoint.y = ((volatile POINT*)point)->y;
                    hitPoint.x = ((volatile POINT*)point)->x;
                    if (PtInRect(&rect, hitPoint) && !(rowTable[row].flags & 1)) {
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
