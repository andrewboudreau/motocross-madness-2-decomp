#pragma once

#include "DlgProcs.h"

// DemoDlg's inline constructor (DlgProcs.h declares the class). BikeRace::Create
// (0x00417ed0) expands it after allocating 0x7f58 bytes (bikerace.cpp line
// 0x2fb): UIDialog::UIDialog(1, "credits.dtm") (0x00469db0), then the vtable
// 0x00550e18 (0x00419329). Kept apart from DlgProcs.h only because an inline
// body there changes the code of another file's function (see DlgProcs.h).
inline DemoDlg::DemoDlg() : UIDialog(1, "credits.dtm") {}
