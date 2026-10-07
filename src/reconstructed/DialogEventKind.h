#pragma once

// The kinds GameUi.cpp's senders store in UnknownDialogEvent+0x08 (DialogProc.h) (named after the
// sender; kinds 3, 9, 11, 12 and 14-17 are left numeric).
enum UnknownDialogEventKind {
    kDialogCommand = 1,          // a button or check box was clicked (+0x04 names it)
    kDialogListSelect = 2,       // UIListBox slot 66: a row was clicked
    kDialogCreate = 4,           // UIDialog slot 27, before the controls exist
    kDialogInit = 5,             // UIDialog slot 28: the controls exist
    kDialogClose = 6,            // UIDialog::Release
    kDialogTimer = 7,            // a UITimer without a control ran out (+0x00 is its id)
    kDialogEditDone = 10,        // UIEditBox: Enter or the focus left it
    kDialogListDoubleClick = 13, // UIListBox slot 66: the same row twice within the double-click time
    kDialogFrame = 18,           // UIDialog slot 10, once a frame when asked for
    kDialogEditChange = 19       // UIEditBox: a character was typed
};

// Kept out of DialogProc.h: declaring it there changes VC6's register
// choice in BikeRaceNearMisses' 0x00419970.
