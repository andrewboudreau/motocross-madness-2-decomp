// Near-miss RaceStatus.cpp candidate, kept out of src/reconstructed until it
// matches. See docs/RACESTATUS.md.
//
// UnknownFunction4e5d00 (0x004e5d00, 519 bytes): builds a view's status
// list. The statements, calls and loops line up; only register allocation
// differs: retail keeps the list pointer in ebp and the index in ebx (also
// reusing it for the constant 1 in the calloc count and the race-mode test)
// and keeps `tail` in the view argument's stack slot, while VC6 here folds
// the first index to 1, keeps `tail` in ebx and reloads `list`. Every
// declaration order, `tail = list` for the first node and an unsigned index
// leave that allocation.

#include "../../src/reconstructed/RaceStatus.h"

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x004e5d00
int UnknownFunction4e5d00(UnknownEventRacerPart** list, UnknownKrustyBikeView* view) {
    UnknownEventRacerPart* status;
    UnknownEventRacerPart** tail;
    int index;
    int i;

    g_UnknownGlobal689c6c = 0;
    if (!list)
        return 0;
    index = 1;
    status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 214);
    if (!status)
        return 0;
    status->field_0x04 = view->field_0x38;
    status->field_0x34 = view->field_0x0c8;
    view->field_0x38->field_0x744 = status;
    status->field_0x00 = index;
    status->field_0x28 = view->field_0x38->field_0x00c;
    *list = status;
    tail = &status->field_0x50;
    if (g_UnknownGlobal56e26c->field_0x18 == 1 && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 4) {
        for (i = 0; i < g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24; i++) {
            if (!UnknownFunction4e5c70(*list, view->field_0x40[i])) {
                status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 235);
                if (!status)
                    return 0;
                index++;
                status->field_0x04 = view->field_0x40[i];
                status->field_0x34 = view->field_0x0c8;
                view->field_0x40[i]->field_0x744 = status;
                status->field_0x00 = index;
                status->field_0x28 = view->field_0x40[i]->field_0x00c;
                *tail = status;
                tail = &status->field_0x50;
            }
        }
    }
    if (view->field_0x3c) {
        for (i = 0; i < view->field_0x158; i++) {
            if (!UnknownFunction4e5c70(*list, view->field_0x3c[i]) && view->field_0x3c[i] != view->field_0x38) {
                status = (UnknownEventRacerPart*)DebugCalloc(1, sizeof(UnknownEventRacerPart), __FILE__, 257);
                if (!status)
                    return 0;
                index++;
                status->field_0x04 = view->field_0x3c[i];
                status->field_0x34 = view->field_0x0c8;
                view->field_0x3c[i]->field_0x744 = status;
                status->field_0x00 = index;
                status->field_0x28 = view->field_0x3c[i]->field_0x00c;
                *tail = status;
                tail = &status->field_0x50;
            }
        }
    }
    return 1;
}
