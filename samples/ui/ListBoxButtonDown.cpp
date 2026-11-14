// Complete nonexact list-box mouse selection; original TU ownership is unproven.
// Retail 0x00477e90 spans 342 bytes. See docs/LIST_BOX_BUTTON_DOWN.md.
// 320 of 342. Left: retail forms &rect last (`lea ecx` after both point
// loads); VC6 here forms it first, which moves the scratch registers of the
// pushes and of the two loads after the call.
// Ruled out (2026-11-13): `PtInRect(&rect, *point)` with no height local is
// retail's instruction sequence with ebx and ebp swapped (point in ebp, row
// offset in ebx: 155, 344 bytes); 0..7 dead initialised locals, 0..63
// prepended typedefs, `(POINT*)position` at every use, a POINT reference, the
// base call given `(int)point`, an inline by-value/pointer/reference PtInRect
// wrapper, a `const RECT*` local and an `inside` result local keep that swap.
// A `height` local (also as an embedded assignment) gives point ebx but reads
// x first (322/319). The copies in y, x order (319), `POINT hitPoint = {..}`,
// an (x, y) inline wrapper and the copies ahead of the bottom store (117 or
// less) are not better.
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/GUIManager.h"

int UIListBox::ButtonDown(int button, int position) {
    POINT* point = (POINT*)position;
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && point) {
        if (button == 0) {
            if (selectable) {
                int found = 0;
                RECT rect = *(RECT*)screenArea;
                for (int row = firstVisibleRow; row < firstVisibleRow + visibleRowCount; row++) {
                    // No height local: retail adds the top to the call's result in eax.
                    rect.bottom = rect.top + GetRowHeight(row);
                    // Retail reads y before x, including when point is invalid; VC6
                    // forwards these copies into the by-value pushes in that order.
                    POINT hitPoint;
                    hitPoint.x = point->x;
                    hitPoint.y = point->y;
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
        } else if (button == 1 && allowWScroll) {
            dragStartX = point->x;
            return UIControl::ButtonDown(button, position);
        }
    }
    return UIControl::ButtonDown(button, position);
}
